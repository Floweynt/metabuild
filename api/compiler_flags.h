#pragma once
#include "core.h"
#include <filesystem>
#include <vector>

namespace metabuild METABUILD_PUBLIC
{
    class compiler_flags
    {
    public:
        enum optimization
        {
            OPTIMIZE_DEFAULT = 0,
            OPTIMIZE_OFF,
            OPTIMIZE_1,
            OPTIMIZE_2,
            OPTIMIZE_3,
            OPTIMIZE_SIZE,
        };

        enum standard
        {
            STD_DEFAULT = 0,
            CXX03,
            CXX11,
            CXX14,
            CXX17,
            CXX20,
            C89,
            C90,
            C99,
            C11,
        };

        enum standard_library
        {
            STDLIB_DEFAULT = 0,
            LIBCXX,
            LIBSTDCXX,
        };

        enum debug_type
        {
            DEBUG_DEFAULT = 0,
            DEBUG_GDB,
            DEBUG_ENABLED,
            DEBUG_DISABLED,
        };

    private:
        optimization opt = OPTIMIZE_DEFAULT;
        standard std = STD_DEFAULT;
        standard_library stdlib = STDLIB_DEFAULT;
        debug_type debug = DEBUG_DEFAULT;

        std::vector<std::filesystem::path> include_dirs;
        std::vector<std::string> additional_flags;

        friend class clang_compiler;
        friend class gcc_compiler;
    public:
        METABUILD_INLINE constexpr compiler_flags() = default;

        METABUILD_INLINE constexpr compiler_flags& set_standard(standard s)
        {
            std = s;
            return *this;
        }

        METABUILD_INLINE constexpr compiler_flags& set_optimization(optimization o)
        {
            opt = o;
            return *this;
        }

        METABUILD_INLINE constexpr compiler_flags& set_stdlib(standard_library s)
        {
            stdlib = s;
            return *this;
        }

        METABUILD_INLINE constexpr compiler_flags& set_debug(debug_type d)
        {
            debug = d;
            return *this;
        }

        METABUILD_INLINE constexpr compiler_flags& add_include_dirs(const std::filesystem::path& path)
        {
            include_dirs.push_back(path);
            return *this;
        }

        METABUILD_INLINE constexpr compiler_flags& add_flags(const std::string& flag)
        {
            additional_flags.push_back(flag);
            return *this;
        }
    };
} // namespace metabuild
