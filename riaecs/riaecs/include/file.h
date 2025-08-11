#pragma once
#include "riaecs/include/dll_config.h"

#include "riaecs/include/interfaces/file.h"
#include "riaecs/include/registry.h"

namespace riaecs
{
    using FileLoaderRegistry = Registry<IFileLoader>;

} // namespace riaecs