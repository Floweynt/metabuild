#pragma once
#include "core.h"
#include <string>

namespace metabuild
{
    METABUILD_PUBLIC void debug(const std::string& str);
    METABUILD_PUBLIC void verbose(const std::string& str);
    METABUILD_PUBLIC void info(const std::string& str);
    METABUILD_PUBLIC void warn(const std::string& str);
    METABUILD_PUBLIC void error(const std::string& str);
    METABUILD_PUBLIC void fatal(const std::string& str);
} // namespace metabuild
