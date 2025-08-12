#include "riaecs/src/pch.h"
#include "riaecs/include/global_registry.h"

RIAECS_API std::unique_ptr<riaecs::IComponentFactoryRegistry> riaecs::gComponentFactoryRegistry 
= std::make_unique<riaecs::ComponentFactoryRegistry>();

RIAECS_API std::unique_ptr<riaecs::IComponentMaxCountRegistry> riaecs::gComponentMaxCountRegistry 
= std::make_unique<riaecs::ComponentMaxCountRegistry>();

RIAECS_API std::unique_ptr<riaecs::ISystemFactoryRegistry> riaecs::gSystemFactoryRegistry 
= std::make_unique<riaecs::SystemFactoryRegistry>();

RIAECS_API std::unique_ptr<riaecs::IAssetFactoryRegistry> riaecs::gAssetFactoryRegistry 
= std::make_unique<riaecs::AssetFactoryRegistry>();

RIAECS_API std::unique_ptr<riaecs::IAssetSourceRegistry> riaecs::gAssetSourceRegistry 
= std::make_unique<riaecs::AssetSourceRegistry>();

RIAECS_API std::unique_ptr<riaecs::IFileLoaderRegistry> riaecs::gFileLoaderRegistry 
= std::make_unique<riaecs::FileLoaderRegistry>();