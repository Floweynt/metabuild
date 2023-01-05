#include "../state.h"
#include <core.h>

namespace metabuild
{
    METABUILD_PUBLIC std::filesystem::path binary_root() { return state_data::get_instance().binary_dir; }
    METABUILD_PUBLIC std::filesystem::path source_root() { return state_data::get_instance().sources_dir; }
} // namespace metabuild
