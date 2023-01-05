#include <filesystem>
#include <string>
#include <vector>

std::vector<std::string> get_path();

inline std::filesystem::path normalize_path(const std::filesystem::path& p) { return std::filesystem::absolute(p).lexically_normal(); }

namespace std
{
    template <typename T>
    struct hash<vector<T>>
    {
        size_t operator()(const vector<T>& v) const
        {
            size_t seed = v.size();
            for (const T& i : v)
            {
                seed ^= std::hash<T>{}(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };
} // namespace std

template <typename T>
auto hash_for(const T& args)
{
    return std::hash<T>{}(args);
}

constexpr void hash_combine(std::size_t& seed) {}

template <typename T, typename... Rest>
constexpr void hash_combine(std::size_t& seed, const T& v, Rest... rest)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    hash_combine(seed, rest...);
}
