#include "api/build_config.h"
#include "api/executable.h"
#include "api/log.h"
#include "api/plugin.h"
#include <algorithm>
#include <filesystem>
#include <mutex>

using namespace metabuild;

static auto do_build()
{
    std::vector<std::filesystem::path> p;
    auto e = executable("metabuild", metabuild::compiler_flags::C11, metabuild::compiler_flags::CXX20,
                        {.cxx = compiler_flags().add_flags("-DFMT_HEADER_ONLY").add_flags("-fvisibility=hidden").add_flags("-D_IS_IMPL_SIDE"),
                         .ld = linker_flags().dynamic()})
                 .library("dl")
                 .library("m")
                 .include_dir(source_root() / "api")
                 .include_dir(source_root() / "meta/argparse/include")
                 .set_build_type(metabuild::RELEASE)
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

METABUILD_ENTRY void metabuild_register(build_registration& reg)
{
    reg.add("build", [](auto fn) -> int {
        auto o = do_build();
        info("rebuilding");
        if (o.invoke({"-V", "build-norebuild"}))
            fatal("failed to rebuild, panic");
        return 0;
    });

    reg.add("build-norebuild", [](auto fn) -> int {
        do_build();
        return 0;
    });
}
