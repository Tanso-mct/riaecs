#pragma once
#include "riaecs/include/dll_config.h"

#include "riaecs/include/interfaces/asset.h"
#include "riaecs/include/container.h"
#include "riaecs/include/registry.h"

namespace riaecs
{
    using AssetFactoryRegistry = Registry<IAssetFactory>;
    using AssetSourceRegistry = Registry<AssetSource>;

    using AssetContainer = Container<IAsset>;

} // namespace riaecs