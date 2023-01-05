#include "../api/log.h"
#include "../state.h"
#include <iostream>
#include <mutex>

namespace metabuild
{
    static std::mutex mtx;
    METABUILD_PUBLIC void debug(const std::string& str)
    {
        std::lock_guard g(mtx);
        if (state_data::get_instance().verbosity >= 2)
            std::cout << "[D]: " << str << '\n';
    }

    METABUILD_PUBLIC void verbose(const std::string& str)
    {
        std::lock_guard g(mtx);
        if (state_data::get_instance().verbosity >= 1)
            std::cout << "[V]: " << str << '\n';
    }

    METABUILD_PUBLIC void info(const std::string& str)
    {
        std::lock_guard g(mtx);
        std::cout << "[INFO]: " << str << '\n';
    }

    METABUILD_PUBLIC void warn(const std::string& str)
    {
        std::lock_guard g(mtx);
        std::cout << "[\x1b[33mWARN\x1b[0m]: " << str << '\n';
    }

    METABUILD_PUBLIC void error(const std::string& str)
    {
        std::lock_guard g(mtx);
        std::cout << "[\x1b[31mERROR\x1b[0m]: " << str << '\n';
    }

    METABUILD_PUBLIC void fatal(const std::string& str)
    {
        std::lock_guard g(mtx);
        std::cout << "[\x1b[31mFATAL\x1b[0m]: " << str << '\n';
        exit(-1);
    }
} // namespace metabuild
