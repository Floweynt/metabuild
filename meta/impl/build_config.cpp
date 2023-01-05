#include "core.h"
#include <build_config.h>

namespace metabuild
{
    METABUILD_PUBLIC const build_config& get_build_config()
    {
        static build_config c{
            build_type::DEBUG
        };
        return c;
    }
}
