#include "compiler.h"
#include <linker.h>
#include "log.h"
#include <fmt/ranges.h>
namespace metabuild
{
    METABUILD_PUBLIC linker::linker(const std::string& vendor, const std::string& name, const std::string& version, const std::filesystem::path& exec)
        : vendor(vendor), name(name), version(version), exec(exec)
    {
    }

    METABUILD_PUBLIC tl::expected<void, std::string> linker::link(const std::string& out, const std::vector<std::filesystem::path>& p, const linker_flags& flags) const
    {
        std::filesystem::create_directory(binary_root() / "link");
        auto out_path = binary_root() / "link" / out;
        auto args = parse_flags(flags);

        args.push_back("-o");
        args.push_back(out_path);
        args.insert(args.begin(), p.begin(), p.end());

        std::string sout;
        std::string serr;
        debug(fmt::format("{} {}", cmd().path().string(), fmt::join(args, "\n")));

        auto result = cmd().invoke(args, sout, serr);

        if (result != 0)
            return tl::unexpected(serr);
        return tl::expected<void, std::string>();
    }

#define _PRED(out, pred, val)                                                                                                                        \
    if (pred)                                                                                                                                        \
    out.push_back(val)

    inline constexpr const char* OUT_TYPE_FLAGS[] = {
        nullptr, "-flinker-output=exec", "-flinker-output=dyn", "-flinker-output=pie", "-flinker-output=rel", "-flinker-output=nolto-rel"};

    inline constexpr const char* PIE_MODE_FLAGS[] = {nullptr, "-pie", "-no-pie", "-static-pie"};

    class gcc_like_linker : public linker
    {
    public:
        gcc_like_linker(const std::string& vendor, const std::string& name, const std::string& version, const std::filesystem::path& exec)
            : linker(vendor, name, version, exec){};

        virtual std::vector<std::string> parse_flags(const linker_flags& flags) const override
        {
            std::vector<std::string> out = {"-v"};

            std::string raw = "-Wl,";
            for (const auto& i : flags.linker_raw_flags)
                raw += i + ',';
            raw.pop_back();

            _PRED(out, flags.disable_core_flags.test(linker_flags::NOSTARTFILES), "-nostartfiles");
            _PRED(out, flags.disable_core_flags.test(linker_flags::NODEFAULTLIBS), "-nodefaultlibs");
            _PRED(out, flags.disable_core_flags.test(linker_flags::NOLIBC), "-nolibc");
            _PRED(out, flags.disable_core_flags.test(linker_flags::NOSTDLIB), "-nostdlib");
            _PRED(out, flags.disable_core_flags.test(linker_flags::NOSTDLIBCXX), "-nostdlib++");
            _PRED(out, flags.custom_entry, "--entry=" + flags.custom_entry.value());
            _PRED(out, !flags.linker_raw_flags.empty(), raw);
            _PRED(out, flags.linker_backend, "-fuse-ld=" + flags.linker_backend.value());


            _PRED(out, OUT_TYPE_FLAGS[flags.linker_out_type], OUT_TYPE_FLAGS[flags.linker_out_type]);
            _PRED(out, PIE_MODE_FLAGS[flags.mode], PIE_MODE_FLAGS[flags.mode]);

            _PRED(out, flags.pthreads, "-pthread");
            _PRED(out, flags.partial_link, "-r");
            _PRED(out, flags.all_dynamic, "-rdynamic");

            for (const auto& i : flags.libs)
                out.push_back("-l" + i);
            return out;
        }
    };

    METABUILD_PUBLIC const linker& system_linker()
    {
        static std::unique_ptr<linker> ld;
        if (!ld)
        {
            const auto& cc = system_compiler_c();
            ld.reset(new gcc_like_linker(cc.get_vendor(), cc.get_name(), cc.get_version(), cc.cmd().path()));
        }

        return *ld;
    }
} // namespace metabuild
