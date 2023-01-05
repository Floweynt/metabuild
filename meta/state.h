#pragma once

#include "singleton.h"
#include <filesystem>
namespace metabuild
{
    struct state_data : singleton<state_data>
    {
        std::filesystem::path sources_dir;
        std::filesystem::path binary_dir;
        int verbosity;
    };
} // namespace metabuild
