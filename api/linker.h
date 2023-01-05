#pragma once

#include "command.h"
#include "core.h"
#include "expected.h"
#include "linker_flags.h"
#include <filesystem>
#include <string>
#include <vector>
#include <concepts>

namespace metabuild
{
    class linker
    {
        const std::string vendor;
        const std::string name;
        const std::string version;
        const std::filesystem::path exec;

    protected:
        linker(const std::string& vendor, const std::string& name, const std::string& version, const std::filesystem::path& exec);
        virtual std::vector<std::string> parse_flags(const linker_flags& flags) const = 0;

    public:
        METABUILD_INLINE constexpr const auto& get_vendor() const { return vendor; }
        METABUILD_INLINE constexpr const auto& get_name() const { return name; }
        METABUILD_INLINE constexpr const auto& get_version() const { return version; }
        METABUILD_INLINE constexpr auto get_id() const { return vendor + "-" + name + "-" + version; }

        METABUILD_INLINE  command cmd() const { return command(exec); }

        template <std::convertible_to<std::filesystem::path>... Args>
        METABUILD_INLINE tl::expected<void, std::string> link(const std::string& out, Args&&... in, const linker_flags& flags) const
        {
            return link(out, {((std::filesystem::path)std::forward<Args>(in))...}, flags);
        }

        METABUILD_PUBLIC tl::expected<void, std::string> link(const std::string& out, const std::vector<std::filesystem::path>& p,
                                         const linker_flags& flags) const;

        METABUILD_PUBLIC virtual ~linker() = default;
    };

    METABUILD_PUBLIC const linker& system_linker();
} // namespace metabuild
