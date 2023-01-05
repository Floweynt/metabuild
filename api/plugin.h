// this file represents the API that the client should expose, notably the entry point
#pragma once
#include "core.h"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#define METABUILD_ENTRY extern "C"

int prog_main(int argc, char** argv);

namespace metabuild METABUILD_PUBLIC
{
    using build_handler = std::function<int(const std::vector<std::string>& args)>;
    class build_registration
    {
        std::unordered_map<std::string, build_handler> handlers;
        friend int ::prog_main(int argc, char** argv);

        METABUILD_INLINE build_registration() {}

    public:
        METABUILD_INLINE void add(const std::string& name, const build_handler& h) { handlers[name] = h; }
    };
} // namespace metabuild

METABUILD_ENTRY void metabuild_register(metabuild::build_registration& reg);
