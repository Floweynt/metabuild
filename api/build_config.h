#pragma once
#include "core.h"

namespace metabuild METABUILD_PUBLIC
{
    enum build_type
    {
        DEBUG,
        RELEASE,
        RELEASE_DEBUGINFO
    };

    struct build_config
    {
        const build_type default_build_type;
    };

    METABUILD_PUBLIC const build_config& get_build_config();
}; // namespace std
