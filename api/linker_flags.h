#pragma once
#include "core.h"
#include <bitset>
#include <optional>
#include <string>
#include <vector>

namespace metabuild
{
    class linker_flags
    {
    public:
        enum pie_mode
        {
            PIE_DEFAULT = 0,
            PIE,
            NO_PIE,
            STATIC_PIE,
        };

        enum output_type
        {
            OUT_DEFAULT = 0,
            OUT_EXEC,
            OUT_DYN,
            OUT_PIE,
            OUT_REL,
            OUT_NOLTO_REL,
        };

    private:
        enum disable_core_flags_features
        {
            NOSTARTFILES = 0,
            NODEFAULTLIBS,
            NOLIBC,
            NOSTDLIB,
            NOSTDLIBCXX
        };

        std::bitset<5> disable_core_flags;
        std::optional<std::string> custom_entry;
        std::vector<std::string> linker_raw_flags;
        std::optional<std::string> linker_backend;
        std::vector<std::string> libs;

        output_type linker_out_type = OUT_DEFAULT;
        pie_mode mode = PIE_DEFAULT;
        bool pthreads = false;
        bool partial_link = false;
        bool all_dynamic = false;


        METABUILD_INLINE linker_flags& set_core_flags(disable_core_flags_features feat, bool disable)
        {
            disable_core_flags.set(feat, disable);
            return *this;
        }

        friend class gcc_like_linker;

    public:
        METABUILD_INLINE constexpr linker_flags& set_out_type(output_type s)
        {
            linker_out_type = s;
            return *this;
        }

        METABUILD_INLINE constexpr linker_flags& set_pie_mode(pie_mode m)
        {
            mode = m;
            return *this;
        }

        METABUILD_INLINE constexpr linker_flags& set_pthreads(bool enable = true)
        {
            pthreads = enable;
            return *this;
        }

        METABUILD_INLINE constexpr linker_flags& partial(bool enable = true)
        {
            partial_link = enable;
            return *this;
        }

        METABUILD_INLINE linker_flags& no_start_files(bool disable = true) { return set_core_flags(NOSTARTFILES, disable); }
        METABUILD_INLINE linker_flags& no_default_libraries(bool disable = true) { return set_core_flags(NODEFAULTLIBS, disable); }
        METABUILD_INLINE linker_flags& no_libc(bool disable = true) { return set_core_flags(NOLIBC, disable); }
        METABUILD_INLINE linker_flags& no_stdlib(bool disable = true) { return set_core_flags(NOSTDLIB, disable); }
        METABUILD_INLINE linker_flags& no_stdlibcxx(bool disable = true) { return set_core_flags(NOSTDLIBCXX, disable); }

        METABUILD_INLINE constexpr linker_flags& set_entry_point(const std::string& e)
        {
            custom_entry = e;
            return *this;
        }

        METABUILD_INLINE constexpr linker_flags& add_raw_flags(const std::string& e)
        {
            linker_raw_flags.push_back(e);
            return *this;
        }

        METABUILD_INLINE constexpr linker_flags& set_linker_backend(const std::string& s)
        {
            linker_backend = s;
            return *this;
        }

        METABUILD_INLINE constexpr linker_flags& add_library(const std::string& name)
        {
            libs.push_back(name);
            return *this;
        }

        METABUILD_INLINE constexpr linker_flags& libcxx() { return add_library("c++"); }

        METABUILD_INLINE constexpr linker_flags& libstdcxx() { return add_library("stdc++"); }

        METABUILD_INLINE constexpr linker_flags& dynamic(bool enable = true)
        {
            all_dynamic = enable;
            return *this;
        }
    };
} // namespace metabuild
