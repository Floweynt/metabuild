#pragma once
#include "core.h"
#include "utils.h"
#include <filesystem>
#include <future>
#include <span>
#include <string>
#include <unordered_map>

namespace metabuild METABUILD_PUBLIC
{
    using program_result = int;
     using program_arguments = std::vector<std::string>;
     using program_environment = std::unordered_map<std::string, std::string>;

     class command
    {
        const std::filesystem::path name;

    public:
        METABUILD_INLINE command(const std::filesystem::path& name) : name(name) {}

        METABUILD_INLINE constexpr const auto& path() const { return name; }

        // simple invoke commands
        [[nodiscard]] METABUILD_INLINE program_result invoke(const program_arguments& args) const
        {
            return invoke_async(args).get_value();
        }

        [[nodiscard]] METABUILD_INLINE lazy<program_result> invoke_async(const program_arguments& args) const
        {
            return invoke_async(args, {});
        };
        [[nodiscard]] METABUILD_INLINE program_result invoke(const program_arguments& args, const program_environment& env) const
        {
            return invoke_async(args).get_value();
        }
        [[nodiscard]] METABUILD_PUBLIC lazy<program_result> invoke_async(const program_arguments& args, const program_environment& env) const;

        // invoke with pipe out
        [[nodiscard]] METABUILD_INLINE program_result invoke(const program_arguments& args, std::string& out) const
        {
            std::string discard;
            return invoke(args, {}, out, discard);
        }

        [[nodiscard]] METABUILD_INLINE program_result invoke(const program_arguments& args, const program_environment& env,
                                                                            std::string& out) const
        {
            std::string discard;
            return invoke(args, env, out, discard);
        }

        [[nodiscard]] METABUILD_INLINE program_result invoke(const program_arguments& args, std::string& out, std::string& err) const
        {
            return invoke(args, {}, out, err);
        }

        [[nodiscard]] METABUILD_PUBLIC program_result invoke(const program_arguments& args, const program_environment& env, std::string& out,
                                                          std::string& err) const;
    };
} // namespace metabuild
