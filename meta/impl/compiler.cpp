#include "../utils/mmap.h"
#include "../utils/sha256.h"
#include "../utils/utils.h"
#include "command.h"
#include "core.h"
#include "expected.h"
#include "log.h"
#include <boost/algorithm/string/replace.hpp>
#include <compiler.h>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fmt/ranges.h>
#include <optional>
#include <stdexcept>
#include <string>

namespace metabuild
{
    METABUILD_PUBLIC compiler::compiler(const std::string& vendor, const std::string& name, const std::string& version,
                                        const std::filesystem::path& exec)
        : vendor(vendor), name(name), version(version), exec(exec)
    {
    }

    static std::string flatten_path(const std::filesystem::path& p)
    {
        std::string flattened_name = normalize_path(p).string();
        flattened_name.erase(0, 1);
        boost::replace_all(flattened_name, "/", "_");
        return flattened_name;
    }

    static tl::expected<std::filesystem::path, std::string> do_compile(const std::filesystem::path& in, const std::filesystem::path& out,
                                                                       program_arguments& args, const compiler& c)
    {
        args.push_back("-c");
        args.push_back("-o");
        args.push_back(out);
        args.push_back(in);

        std::string sout;
        std::string serr;

        debug(fmt::format("{} {}", c.cmd().path().string(), fmt::join(args, "\n")));

        auto result = c.cmd().invoke(args, sout, serr);

        if (result != 0)
            return tl::unexpected(serr);
        return out;
    }

    METABUILD_PUBLIC tl::expected<std::filesystem::path, std::string> compiler::compile(const std::filesystem::path& in, const compiler_flags& flags,
                                                                                        const std::filesystem::path& root) const
    {
        std::filesystem::create_directories(root);
        std::string out_fname = flatten_path(in) + ".o";
        auto out_path = root / out_fname;
        auto args = parse_flags(flags);
        return do_compile(in, out_path, args, *this);
    }

    METABUILD_PUBLIC tl::expected<std::pair<std::filesystem::path, bool>, std::string> compiler::lazy_compile(const std::filesystem::path& in,
                                                                                                              const compiler_flags& flags,
                                                                                                              const std::filesystem::path& root) const
    {
        std::filesystem::create_directories(root);

        auto args = parse_flags(flags);
        std::string id = get_id();

        // we use the file content, compiler id and flags in order to generate a hash that uniquely identifies a binary
        sha s;
        s.update(mmap_file(in).buffer());
        s.update(std::span<uint8_t>((uint8_t*)id.c_str(), id.size()));
        for (const auto& i : args)
            s.update(std::span<uint8_t>((uint8_t*)i.c_str(), i.size()));

        // path stuff
        std::string prefix = flatten_path(in);
        std::string out_fname = prefix + "_" + s.digest_str() + ".o";
        auto out_path = root / out_fname;

        // for caching
        if (std::filesystem::exists(out_path))
            return {tl::in_place, out_path, false};

        // remove old artifacts so that we don't bloat
        for (const auto& i : std::filesystem::directory_iterator(root))
        {
            if (i.path().filename().string().starts_with(prefix))
                std::filesystem::remove(i.path());
        }

        return do_compile(in, out_path, args, *this).map([](const auto& i) { return std::pair<std::filesystem::path, bool>{i, true}; });
    }

    inline static constexpr const char* STDLIB_FLAGS[] = {nullptr, "-stdlib=libc++", "-stdlib=libstdc++"};

    inline static constexpr const char* DEBUG_TYPE_FLAGS[] = {
        nullptr,
        "-ggdb",
        "-g",
        "-g0",
    };

    inline static constexpr const char* OPTIMIZATION_FLAGS[] = {
        nullptr, "-O0", "-O1", "-O2", "-O3", "-Os",
    };

    inline static constexpr const char* LANG_STD_FLAGS[] = {
        nullptr, "-std=c++03", "-std=c++11", "-std=c++14", "-std=c++17", "-std=c++20", "-std=c89", "-std=c90", "-std=c99", "-std=c11",
    };

    inline static constexpr std::pair<const char*, const char*> BAD_FLAGS_EXACT[] = {
        {"-o", "cannot specify output directly in flags"},
        {"-c", "cannot specify output type in flags"},
        {"-s", "cannot specify output type in flags"},
        {"-e", "cannot specify output type in flags"},
    };

    inline static constexpr std::pair<const char*, const char*> WARN_FLAGS_PREFIX[] = {
        {"-std", "do not directly specify language/library standard via raw flags"},
        {"-o", "do not directly specify optimization level via raw flags"},
        {"-i", "do not directly specify include directory via raw flags"},
    };

    inline static constexpr std::pair<const char*, const char*> WARN_FLAGS_EXACT[] = {
        {"-ggdb", "do not directly specify debugging level via raw flags"},
        {"-g", "do not directly specify debugging level via raw flags"},
    };

#define PUSH_LOOKUP_FLAGS(out, lut, val)                                                                                                             \
    if ((lut)[val])                                                                                                                                  \
        out.push_back((lut)[val]);
    class clang_compiler final : public compiler
    {
    public:
        clang_compiler(const std::string& vendor, const std::string& name, const std::string& version, const std::filesystem::path& exec)
            : compiler(vendor, name, version, exec){};

        virtual std::vector<std::string> parse_flags(const compiler_flags& flags) const override
        {
            std::vector<std::string> out;

            PUSH_LOOKUP_FLAGS(out, OPTIMIZATION_FLAGS, flags.opt);
            PUSH_LOOKUP_FLAGS(out, LANG_STD_FLAGS, flags.std);
            PUSH_LOOKUP_FLAGS(out, STDLIB_FLAGS, flags.stdlib);
            PUSH_LOOKUP_FLAGS(out, DEBUG_TYPE_FLAGS, flags.debug);

            for (const auto& i : flags.include_dirs)
                out.push_back("-I" + normalize_path(i).string());

            for (const auto& i : flags.additional_flags)
            {
                for (const auto& j : BAD_FLAGS_EXACT)
                    if (i == j.first)
                        throw metabuild_error(error_code::BAD_COMPILE_FLAG, j.second);
                for (const auto& j : WARN_FLAGS_EXACT)
                    if (i == j.first)
                        warn(j.second);
                for (const auto& j : WARN_FLAGS_PREFIX)
                    if (i.starts_with(j.first))
                        warn(j.second);
            }

            out.insert(out.end(), flags.additional_flags.begin(), flags.additional_flags.end());
            return out;
        }
    };

    class gcc_compiler final : public compiler
    {
    public:
        gcc_compiler(const std::string& vendor, const std::string& name, const std::string& version, const std::filesystem::path& exec)
            : compiler(vendor, name, version, exec){};

        virtual std::vector<std::string> parse_flags(const compiler_flags& flags) const override
        {
            std::vector<std::string> out;

            PUSH_LOOKUP_FLAGS(out, OPTIMIZATION_FLAGS, flags.opt);
            PUSH_LOOKUP_FLAGS(out, LANG_STD_FLAGS, flags.std);
            PUSH_LOOKUP_FLAGS(out, STDLIB_FLAGS, flags.stdlib);
            PUSH_LOOKUP_FLAGS(out, DEBUG_TYPE_FLAGS, flags.debug);

            for (const auto& i : flags.include_dirs)
                out.push_back("-I" + normalize_path(i).string());

            for (const auto& i : flags.additional_flags)
            {
                for (const auto& j : BAD_FLAGS_EXACT)
                    if (i == j.first)
                        throw metabuild_error(error_code::BAD_COMPILE_FLAG, j.second);
                for (const auto& j : WARN_FLAGS_EXACT)
                    if (i == j.first)
                        warn(j.second);
                for (const auto& j : WARN_FLAGS_PREFIX)
                    if (i.starts_with(j.first))
                        warn(j.second);
            }

            out.insert(out.end(), flags.additional_flags.begin(), flags.additional_flags.end());
            return out;
        }
    };

    template <typename c, typename t>
    static compiler* make_compiler_util(const std::string& vendor, const std::string& compiler, t get_version)
    {
        auto path = get_path();
        auto cc = std::find_if(path.begin(), path.end(),
                               [&compiler](const auto& p) { return std::filesystem::exists(std::filesystem::path(p) / compiler); });
        if (cc == path.end())
            throw metabuild_error(error_code::COMPILER_NOT_FOUND, "unable to find " + compiler + ", is this compiler installed on your $path?");
        std::string version_out;
        (void)command(std::filesystem::path(*cc) / vendor).invoke({"--version"}, version_out);
        return new c(vendor, compiler, get_version(version_out), *cc);
    }

    METABUILD_PUBLIC const compiler& gcc_c()
    {
        static std::unique_ptr<compiler> comp;
        if (!comp)
        {
            comp.reset(make_compiler_util<gcc_compiler>("gnu", "gcc", [](const std::string& str) {
                std::istringstream iss(str);
                std::string ver;
                iss >> ver >> ver >> ver;
                return ver;
            }));
        }
        return *comp;
    }

    METABUILD_PUBLIC const compiler& gcc_cpp()
    {
        static std::unique_ptr<compiler> comp;
        if (!comp)
        {
            comp.reset(make_compiler_util<gcc_compiler>("gnu", "g++", [](const std::string& str) {
                std::istringstream iss(str);
                std::string ver;
                iss >> ver >> ver >> ver;
                return ver;
            }));
        }
        return *comp;
    }

    METABUILD_PUBLIC const compiler& clang_c()
    {
        static std::unique_ptr<compiler> comp;
        if (!comp)
        {
            comp.reset(make_compiler_util<clang_compiler>("llvm", "clang", [](const std::string& str) {
                std::istringstream iss(str);
                std::string ver;
                iss >> ver >> ver >> ver;
                return ver;
            }));
        }
        return *comp;
    }

    METABUILD_PUBLIC const compiler& clang_cpp()
    {
        static std::unique_ptr<compiler> comp;
        if (!comp)
        {
            comp.reset(make_compiler_util<clang_compiler>("llvm", "clang++", [](const std::string& str) {
                std::istringstream iss(str);
                std::string ver;
                iss >> ver >> ver >> ver;
                return ver;
            }));
        }
        return *comp;
    }

    static std::optional<std::filesystem::path> get_env_as_path(const char* path)
    {
        auto env = getenv(path);
        if (!env)
            return std::nullopt;
        if (!std::filesystem::exists(env))
            return std::nullopt;
        return env;
    }

    METABUILD_PUBLIC const compiler& system_compiler_c()
    {
        static std::unique_ptr<compiler> comp;
        if (!comp)
        {
            auto path = get_path();

            auto env_cc = get_env_as_path("CC");
            auto sys_cc =
                std::find_if(path.begin(), path.end(), [](const auto& p) { return std::filesystem::exists(std::filesystem::path(p) / "cc"); });

            std::filesystem::path cc;

            if (env_cc)
                cc = env_cc.value();
            else if (sys_cc != path.end())
                cc = std::filesystem::path(*sys_cc) / "cc";
            else
                throw metabuild_error(error_code::COMPILER_NOT_FOUND, "unable to find the system c compiler");

            std::string version_out;

            (void)command(cc).invoke({"--version"}, version_out);

            std::string vendor;
            std::string compiler;
            std::string version;

            // okay, we should check if this is gcc
            if (version_out.find("(GCC)") != std::string::npos)
            {
                std::istringstream iss(version_out);
                vendor = "gnu";
                compiler = "gcc";
                iss >> version >> version >> version;

                comp.reset(new gcc_compiler(vendor, compiler, version, cc));
            }
            else if (version_out.find("clang") != std::string::npos)
            {
                std::istringstream iss(version_out);
                vendor = "llvm";
                compiler = "clang";
                iss >> version >> version >> version;

                comp.reset(new clang_compiler(vendor, compiler, version, cc));
            }
            else
                throw metabuild_error(error_code::UNKNOWN_COMPILER, "unknown compiler type");
        }

        return *comp;
    }

    METABUILD_PUBLIC const compiler& system_compiler_cpp()
    {
        static std::unique_ptr<compiler> comp;
        if (!comp)
        {
            auto path = get_path();

            auto env_cc = get_env_as_path("CXX");
            auto sys_cc =
                std::find_if(path.begin(), path.end(), [](const auto& p) { return std::filesystem::exists(std::filesystem::path(p) / "c++"); });

            std::filesystem::path cc;

            if (env_cc)
                cc = env_cc.value();
            else if (sys_cc != path.end())
                cc = std::filesystem::path(*sys_cc) / "c++";
            else
                throw metabuild_error(error_code::COMPILER_NOT_FOUND, "unable to find the system c++ compiler");

            std::string version_out;
            (void)command(cc).invoke({"--version"}, version_out);
            std::string vendor;
            std::string compiler;
            std::string version;

            // okay, we should check if this is gcc
            if (version_out.find("(GCC)") != std::string::npos)
            {
                std::istringstream iss(version_out);
                vendor = "gnu";
                compiler = "g++";
                iss >> version >> version >> version;

                comp.reset(new gcc_compiler(vendor, compiler, version, cc));
            }
            else if (version_out.find("clang") != std::string::npos)
            {
                std::istringstream iss(version_out);
                vendor = "llvm";
                compiler = "clang++";
                iss >> version >> version >> version;

                comp.reset(new clang_compiler(vendor, compiler, version, cc));
            }
            else
                throw metabuild_error(error_code::UNKNOWN_COMPILER, "unknown compiler type");
        }

        return *comp;
    }
} // namespace metabuild
