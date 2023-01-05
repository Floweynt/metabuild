#pragma once

#include <filesystem>
#include <stdexcept>

#ifdef _IS_IMPL_SIDE
#define METABUILD_PUBLIC [[gnu::visibility("default")]]
#define METABUILD_INLINE inline
#else
#define METABUILD_PUBLIC
#define METABUILD_INLINE inline
#endif

namespace metabuild
{
    enum class error_code
    {
        COMPILER_NOT_FOUND,
        UNKNOWN_COMPILER,
        BAD_COMPILE_FLAG,
        UNKNOWN_SRC_TYPE
    };

    class METABUILD_PUBLIC metabuild_error : public std::runtime_error  
    {
        error_code c;

    public:
        METABUILD_INLINE metabuild_error(error_code code, const std::string& str) : runtime_error(str), c(code) {}
        METABUILD_INLINE constexpr auto code() { return c; }
    };

    METABUILD_PUBLIC std::filesystem::path binary_root();
    METABUILD_PUBLIC std::filesystem::path source_root();
} // namespace metabuild
