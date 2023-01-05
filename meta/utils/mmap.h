#pragma once
#include <cstdint>
#include <filesystem>
#include <span>

class mmap_file
{
    void* data;
    size_t len;

public:
    inline mmap_file(const std::filesystem::path& path) { open(path); }
    inline ~mmap_file() { close(); }

    void open(const std::filesystem::path& path);
    void close();

    constexpr bool is_open() const { return data == nullptr; }
    constexpr operator bool() const { return data == nullptr; }

    constexpr std::span<uint8_t> buffer() const { return std::span((uint8_t*)data, len); }
};

