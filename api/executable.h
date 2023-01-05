#pragma once
#include "build_config.h"
#include "command.h"
#include "compiler_flags.h"
#include "core.h"
#include "linker_flags.h"
#include "utils.h"
#include <filesystem>
namespace metabuild METABUILD_PUBLIC
{
    namespace _detail 
    {
        struct opt_flags_package
        {
            compiler_flags c{};
            compiler_flags cxx{};
            linker_flags ld{};
        };

    } // namespace _detail

    class executable
    {
        compiler_flags cc_flags;
        compiler_flags cxx_flags;
        linker_flags ld_flags;
        std::vector<std::filesystem::path> c_src;
        std::vector<std::filesystem::path> cxx_src;
        std::string name;

        int use_threads;

    public:
        METABUILD_INLINE executable(const std::string& name, compiler_flags::standard c_std, compiler_flags::standard cxx_standard,
                          const _detail::opt_flags_package& f = {})
            : name(name), use_threads(-1)
        {
            set_build_type(get_build_config().default_build_type);
            cc_flags = f.c;
            cxx_flags = f.cxx;
            ld_flags = f.ld;
            cc_flags.set_standard(c_std);
            cxx_flags.set_standard(cxx_standard);
        }

        METABUILD_INLINE constexpr executable& library(const std::string& lib)
        {
            ld_flags.add_library(lib);
            return *this;
        }

        METABUILD_INLINE executable& include_dir(const std::string& dir)
        {
            cc_flags.add_include_dirs(dir);
            cxx_flags.add_include_dirs(dir);
            return *this;
        }

        METABUILD_PUBLIC executable& add_src(const std::filesystem::path& path);

        METABUILD_INLINE constexpr executable& set_build_type(build_type bt)
        {
            switch (bt)
            {
            case DEBUG:
                cc_flags.set_debug(compiler_flags::DEBUG_GDB);
                cc_flags.set_optimization(compiler_flags::OPTIMIZE_OFF);
                cxx_flags.set_debug(compiler_flags::DEBUG_GDB);
                cxx_flags.set_optimization(compiler_flags::OPTIMIZE_OFF);
                return *this;
            case RELEASE:
                cc_flags.set_debug(compiler_flags::DEBUG_DISABLED);
                cc_flags.set_optimization(compiler_flags::OPTIMIZE_3);
                cxx_flags.set_debug(compiler_flags::DEBUG_DISABLED);
                cxx_flags.set_optimization(compiler_flags::OPTIMIZE_3);
                return *this;
            case RELEASE_DEBUGINFO:
                cc_flags.set_debug(compiler_flags::DEBUG_ENABLED);
                cc_flags.set_optimization(compiler_flags::OPTIMIZE_3);
                cxx_flags.set_debug(compiler_flags::DEBUG_ENABLED);
                cxx_flags.set_optimization(compiler_flags::OPTIMIZE_3);
                return *this;
            }

            unreachable();
        };

        METABUILD_INLINE constexpr executable& parallelize(int threads = 0)
        {
            use_threads = threads;
            return *this;
        }

        METABUILD_INLINE constexpr executable& libstdcxx()
        {
            ld_flags.libstdcxx();
            return *this;
        }

        METABUILD_INLINE constexpr executable& libcxx()
        {
            ld_flags.libcxx();
            return *this;
        }

        METABUILD_PUBLIC command build(bool quiet = false) const;
    };
} // namespace metabuild
