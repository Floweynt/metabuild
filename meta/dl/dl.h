#include <dlfcn.h>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace dl
{
    struct address_symbol
    {
        const char* file_name;
        void* file_base;
        const char* symbol_name;
        void* symbol_addr;
    };

    class dynamic_library
    {
        void* handle;

    public:
        using open_mode = int;

        inline static constexpr open_mode LAZY = RTLD_LAZY;
        inline static constexpr open_mode EAGER = RTLD_NOW;

        inline static constexpr open_mode GLOBAL = RTLD_GLOBAL;
        inline static constexpr open_mode LOCAL = RTLD_LOCAL;

        inline static constexpr open_mode NODELETE = RTLD_NODELETE;
        inline static constexpr open_mode NOLOAD = RTLD_NOLOAD;
        inline static constexpr open_mode DEEPBIND = RTLD_DEEPBIND;

        inline dynamic_library(const std::string& path, open_mode mode = LAZY) { open(path.c_str(), mode); }
        inline dynamic_library(const std::filesystem::path& path, open_mode mode = LAZY) { open(path.c_str(), mode); }
        inline dynamic_library(const std::string_view& path, open_mode mode = LAZY) { open(path.data(), mode); }
        inline dynamic_library(const char* path, open_mode mode = LAZY) { open(path, mode); }

        constexpr dynamic_library() = default;
        constexpr dynamic_library(const dynamic_library&) = delete;
        constexpr dynamic_library(dynamic_library&&) = default;

        inline bool open(const std::filesystem::path& path, open_mode mode = LAZY) { return open(path.c_str(), mode); }
        inline bool open(const std::string& path, open_mode mode = LAZY) { return open(path.c_str(), mode); }
        inline bool open(const std::string_view& path, open_mode mode = LAZY) { return open(path.data(), mode); }
        inline bool open(const char* path, open_mode mode = LAZY) { return handle = dlopen(path, mode); }

        inline ~dynamic_library() { close(); }

        constexpr operator bool() const { return handle; }
        constexpr bool is_open() const { return handle; }

        void close();
        address_symbol search_symbol(std::uintptr_t ptr) const;

        template <typename T = void*>
            requires(std::is_pointer_v<T>)
        T symbol(const char* sym) const
        {
            if (!handle)
                return nullptr;
            return (T)dlsym(handle, sym);
        }

        template <typename T = void*>
            requires(std::is_pointer_v<T>)
        T symbol(const std::string& sym) const
        {
            return symbol<T>(sym.c_str());
        }

        template <typename T = void*>
            requires(std::is_pointer_v<T>)
        T symbol(const std::string_view& sym) const
        {
            return symbol<T>(sym.data());
        }

        template <typename T>
        address_symbol dladdr(T* ptr) const
        {
            return search_symbol((std::uintptr_t)ptr);
        }

        inline const char* error() { return dlerror(); }
    };
} // namespace dl
