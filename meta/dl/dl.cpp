#include "dl.h"

namespace dl
{
    void dynamic_library::close()
    {
        if (handle)
            dlclose(handle);
        handle = nullptr;
    }

    address_symbol dynamic_library::search_symbol(std::uintptr_t ptr) const
    {
        Dl_info info;
        if (::dladdr((void*)ptr, &info) == 0)
            throw std::runtime_error("dladdr fail");
        if (info.dli_saddr == nullptr)
            throw std::runtime_error("symbol not found");

        return {info.dli_fname, info.dli_fbase, info.dli_sname, info.dli_saddr};
    }
} // namespace dl
