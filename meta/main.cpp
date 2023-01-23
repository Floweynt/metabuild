#include "dl/dl.h"
#include "state.h"
#include "utils/mmap.h"
#include "utils/utils.h"
#include <argparse/argparse.hpp>
#include <compiler.h>
#include <core.h>
#include <dlfcn.h>
#include <exception>
#include <filesystem>
#include <fmt/core.h>
#include <iostream>
#include <log.h>
#include <plugin.h>
#include <vector>

using namespace metabuild;
namespace fs = std::filesystem;

#define VERSION "1.0.0"

[[nodiscard]] static dl::dynamic_library open_build()
{
    auto in = source_root() / "build.cpp";
    auto out = binary_root() / "metabuild";
    if (!fs::exists(in))
        fatal("no buildscript");

    if (!fs::exists(out) || fs::last_write_time(in) > fs::last_write_time(out))
    {
        auto compile_status = system_compiler_cpp().cmd().invoke({"-shared", "-DFMT_HEADER_ONLY", "-O3", "-o", out, "-fPIC", "-std=c++20", in});
        if (compile_status)
            fatal("unable to compile buildscript");
    }
    return dl::dynamic_library(out, dl::dynamic_library::EAGER | dl::dynamic_library::GLOBAL);
}

int prog_main(int argc, char** argv)
{
    argparse::ArgumentParser program(argv[0], VERSION);

    program.add_argument("-V", "--verbose")
        .action([&](const auto&) { state_data::get_instance().verbosity++; })
        .append()
        .help("increase output verbosity")
        .default_value(false)
        .implicit_value(true);

    program.add_argument("--list-buildtypes").help("lists available build types").default_value(false).implicit_value(true);
    program.add_argument("--sources").help("specifies the sources directory").default_value(std::string(".")).required();
    program.add_argument("--out").help("specifies the binary/output directory").default_value(std::string(".build/")).required();
    program.add_argument("build-type").help("sets the type of build");
    program.add_argument("buildscript-args").help("the arguments to pass to buildscript itself").append().nargs(argparse::nargs_pattern::any);

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err)
    {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    if (program["--help"] == true)
    {
        std::cout << program.help().str();
        std::exit(0);
    }

    if (program["--version"] == true)
    {
        std::cout << "metabuild v" VERSION;
        std::exit(0);
    }

    state_data::get_instance().sources_dir = normalize_path(program.get<std::string>("--sources"));
    state_data::get_instance().binary_dir = normalize_path(state_data::get_instance().sources_dir / program.get<std::string>("--out"));

    info("CC is: " + system_compiler_c().get_id());
    info("CXX is: " + system_compiler_cpp().get_id());

    fs::create_directories(binary_root());

    auto dl = open_build();
    if (!dl)
    {
        info(dlerror());
        fatal("failed to load metabuild script");
    }

    build_registration reg;
    auto entry = dl.symbol<void (*)(build_registration&)>("metabuild_register");

    if (!entry)
        fatal("the metabuild script should provide a function named 'metabuild_register', marked with the macro 'METABUILD_API'");

    entry(reg);

    std::string out;
    for (const auto& i : reg.handlers)
        out += i.first + " ";

    info(fmt::format("available build modes: {}", out));

    if (program["--list-buildtypes"] == true)
        std::exit(0);

    std::string build_type = program.get<std::string>("build-type");
    std::vector<std::string> build_args = program.get<std::vector<std::string>>("buildscript-args");

    if (!reg.handlers.contains(build_type))
        fatal(fmt::format("invalid build type: {}", build_type));

    try
    {
        reg.handlers[build_type](build_args);
    }
    catch (std::exception& e)
    {
        fatal(e.what());
    }
    return 0;
}

int main(int argc, char** argv)
{
    try
    {
        return prog_main(argc, argv);
    }
    catch (std::exception& e)
    {
        fatal(std::string("unexpected error occurred: \n") + e.what());
    }
}
