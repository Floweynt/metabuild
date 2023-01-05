#include "mmap.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <system_error>
#include <unistd.h>

void mmap_file::open(const std::filesystem::path& path)
{
    int fd = ::open(path.c_str(), O_RDONLY);
    if (fd < 0)
        throw std::system_error(errno, std::system_category());
    struct stat s;
    if (fstat(fd, &s) < 0)
        throw std::system_error(errno, std::system_category());

    len = s.st_size;
    if (!(data = mmap(nullptr, len, PROT_READ, MAP_PRIVATE, fd, 0)))
        throw std::system_error(errno, std::system_category());
}

void mmap_file::close()
{
    if (is_open())
        munmap(data, len);
}

