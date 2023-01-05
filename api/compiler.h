#pragma once
#include "command.h"
#include "compiler_flags.h"
#include "core.h"
#include "expected.h"
#include <filesystem>
#include <span>
#include <string>
#include <unordered_set>

namespace metabuild METABUILD_PUBLIC
{
    class compiler
    {
        const std::string vendor;
        const std::string name;
        const std::string version;
        const std::filesystem::path exec;

    protected:
        compiler(const std::string& vendor, const std::string& name, const std::string& version, const std::filesystem::path& exec);
        virtual std::vector<std::string> parse_flags(const compiler_flags& flags) const = 0;

    public:
        METABUILD_INLINE constexpr const auto& get_vendor() const { return vendor; }
        METABUILD_INLINE constexpr const auto& get_name() const { return name; }
        METABUILD_INLINE constexpr const auto& get_version() const { return version; }
        METABUILD_INLINE constexpr auto get_id() const { return vendor + "-" + name + "-" + version; }
        METABUILD_INLINE command cmd() const { return command(exec); }

        using compile_result = tl::expected<std::filesystem::path, std::string>;
        using lazy_compile_result = tl::expected<std::pair<std::filesystem::path, bool>, std::string>;

        METABUILD_PUBLIC compile_result compile(const std::filesystem::path& in, const compiler_flags& flags,
                                                const std::filesystem::path& root = binary_root() / "compile") const;
        METABUILD_PUBLIC lazy_compile_result lazy_compile(const std::filesystem::path& in, const compiler_flags& flags,
                                                          const std::filesystem::path& root = binary_root() / "lazy_compile") const;
        METABUILD_PUBLIC virtual ~compiler() = default;
    };

    METABUILD_PUBLIC const compiler& gcc_c();
    METABUILD_PUBLIC const compiler& gcc_cpp();
    METABUILD_PUBLIC const compiler& clang_c();
    METABUILD_PUBLIC const compiler& clang_cpp();
    METABUILD_PUBLIC const compiler& system_compiler_c();
    METABUILD_PUBLIC const compiler& system_compiler_cpp();

    METABUILD_INLINE const compiler& self_compiler()
    {
#if defined(__GNUC__) && !defined(__clang__)
        return gcc_cpp();
#elif defined(__clang__)
        return clang_cpp();
#endif
    }
} // namespace metabuild METABUILD_PUBLIC
