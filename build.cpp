#include "api/build_config.h"
#include "api/executable.h"
#include "api/log.h"
#include "api/plugin.h"
#include <algorithm>
#include <filesystem>
#include <mutex>

using namespace metabuild;

auto do_build()
{
    std::vector<std::filesystem::path> p;
    auto e = executable("metabuild", metabuild::compiler_flags::C11, metabuild::compiler_flags::CXX20,
                        {.cxx = compiler_flags().add_flags("-DFMT_HEADER_ONLY").add_flags("-fvisibility=hidden").add_flags("-D_IS_IMPL_SIDE"),
                         .ld = linker_flags().dynamic()})
                 .library("dl")
                 .library("m")
                 .library("fmt")
                 .include_dir(source_root() / "api")
                 .include_dir(source_root() / "meta/argparse/include")
                 .set_build_type(metabuild::RELEASE_DEBUGINFO)
                 .parallelize()
                 .libstdcxx();

    for (const auto& i : std::filesystem::recursive_directory_iterator(source_root()))
    {
        if (i.path().parent_path().filename() == "test" || i.path().parent_path().filename() == "samples")
            continue;
        if (i.path().extension() != ".cpp")
            continue;
        e.add_src(i.path());
    }

    return e.build();
}

auto do_build_debug()
{
    std::vector<std::filesystem::path> p;
    auto e = executable("metabuild", metabuild::compiler_flags::C11, metabuild::compiler_flags::CXX20,
                        {.cxx = compiler_flags().add_flags("-DFMT_HEADER_ONLY").add_flags("-fvisibility=hidden").add_flags("-D_IS_IMPL_SIDE"),
                         .ld = linker_flags().dynamic()})
                 .library("dl")
                 .library("m")
                 .library("fmt")
                 .include_dir(source_root() / "api")
                 .include_dir(source_root() / "meta/argparse/include")
                 .set_build_type(metabuild::DEBUG)
                 .parallelize()
                 .libstdcxx();

    for (const auto& i : std::filesystem::recursive_directory_iterator(source_root()))
    {
        if (i.path().parent_path().filename() == "test" || i.path().parent_path().filename() == "samples")
            continue;
        if (i.path().extension() != ".cpp")
            continue;
        e.add_src(i.path());
    }

    return e.build();
}

#include <fmt/core.h>

METABUILD_ENTRY void metabuild_register(build_registration& reg)
{
    reg.add("build", [](auto fn) -> int {
        auto o = do_build();
        info("rebuilding");
        if (o.invoke({"-V", "build-norebuild"}))
            fatal("failed to rebuild, panic");
        return 0;
    });

    reg.add("debug", [](auto fn) -> int {
        auto o = do_build_debug();
        return 0;
    });

    reg.add("build-norebuild", [](auto fn) -> int {
        do_build();
        return 0;
    });

    reg.add("install", [](auto fn) -> int {
        auto bin_path = do_build();
        // we need to copy api/* to /usr/local/bin/metabuild/api/
        // TODO: dynamically query this path
        auto install_api_path = "/usr/local/include/metabuild/api/";
        auto install_bin_path = "/usr/local/bin/";
        std::filesystem::create_directories(install_bin_path);
        std::filesystem::create_directories(install_api_path);

        command("/bin/sh").invoke({"-c", fmt::format("install {} {}", bin_path.path().string(), install_bin_path)});
        for(auto file : std::filesystem::directory_iterator(source_root()/"api"))
        {
            command("/bin/sh").invoke({"-c", fmt::format("install {} {}", file.path().string(), install_api_path)});
        }
        return 0;
    });
}
