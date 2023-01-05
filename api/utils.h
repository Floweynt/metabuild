#pragma once
#include "core.h"
#include <functional>
#include <variant>

namespace metabuild
{
    [[noreturn]] METABUILD_INLINE void unreachable()
    {
#if defined(__GNUC__)
        __builtin_unreachable();
#else
        static_assert(false, "not implemented");
#endif
    }

    template <typename T>
    class lazy
    {
        std::variant<std::function<T()>, T> producer;

    public:
        template <typename Fn>
        METABUILD_INLINE constexpr lazy(Fn&& args) : producer(std::function<T()>(std::forward<Fn>(args)))
        {
        }
        METABUILD_INLINE constexpr lazy(T&& args) : producer(std::forward<T>(args)) {}

        METABUILD_INLINE constexpr const T& get_value()
        {
            if (producer.index() == 0)
                producer = std::get<0>(producer)();
            return std::get<1>(producer);
        }
    };
} // namespace metabuild
