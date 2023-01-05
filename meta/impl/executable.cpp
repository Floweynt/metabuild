#include "compiler.h"
#include "core.h"
#include "linker.h"
#include "log.h"
#include "../utils/thread_pool.h"
#include <executable.h>
#include <filesystem>
#include <string>
#include <vector>

namespace metabuild
{
    METABUILD_PUBLIC executable& executable::add_src(const std::filesystem::path& path)
    {
        if (path.extension() == ".c")
            c_src.push_back(path);
        else if (path.extension() == ".cpp")
            cxx_src.push_back(path);
        else
            throw metabuild_error(error_code::UNKNOWN_SRC_TYPE, "bad source: " + path.string());
        return *this;
    }

    METABUILD_PUBLIC command executable::build(bool quiet) const
    {
        std::vector<std::filesystem::path> p;
        bool relink = false;
        if (use_threads == -1)
        {
            for (const auto& i : c_src)
            {
                if (!quiet)
                    info("compiling " + i.string());
                auto compile_out = system_compiler_c().lazy_compile(i, cc_flags, binary_root() / "executable" / name / "obj");
                if (!compile_out)
                    fatal("compile error: \n" + compile_out.error());
                p.push_back(compile_out.value().first);
                relink |= compile_out.value().second;
            }
            for (const auto& i : cxx_src)
            {
                if (!quiet)
                    info("compiling " + i.string());
                auto compile_out = system_compiler_cpp().lazy_compile(i, cxx_flags, binary_root() / "executable" / name / "obj");
                if (!compile_out)
                    fatal("compile error: \n" + compile_out.error());
                p.push_back(compile_out.value().first);
                relink |= compile_out.value().second;
            }
        }
        else
        {
            std::mutex mtx;
            BS::thread_pool tp(use_threads);
            if (!quiet)
                info("compiling with " + std::to_string(tp.get_thread_count()) + " threads");

            for (const auto& i : c_src)
            {
                (void)tp.submit([i, this, quiet, &p, &mtx, &relink]() {
                    if (!quiet)
                        info("compiling " + i.string());
                    auto compile_out = system_compiler_c().lazy_compile(i, cc_flags, binary_root() / "executable" / name / "obj");
                    if (!compile_out)
                        fatal("compile error: \n" + compile_out.error());
                    std::lock_guard g(mtx);
                    p.push_back(compile_out.value().first);
                    relink |= compile_out.value().second;
                });
            }

            for (const auto& i : cxx_src)
            {
                (void)tp.submit([i, this, quiet, &mtx, &p, &relink]() {
                    if (!quiet)
                        info("compiling " + i.string());
                    auto compile_out = system_compiler_cpp().lazy_compile(i, cxx_flags, binary_root() / "executable" / name / "obj");
                    if (!compile_out)
                        fatal("compile error: \n" + compile_out.error());
                    std::lock_guard g(mtx);
                    p.push_back(compile_out.value().first);
                    relink |= compile_out.value().second;
                });
            }

            tp.wait_for_tasks();
        }

        if (relink)
        {
            if (!quiet)
                info("linking executable");
            auto link_result = system_linker().link(name, p, ld_flags);
            if (!link_result)
                fatal("linker error: \n" + link_result.error());
        }
        else
        {
            if (!quiet)
                info("linking (skipped)");
        }
        return command(binary_root() / "link" / name);
    }
} // namespace metabuild
