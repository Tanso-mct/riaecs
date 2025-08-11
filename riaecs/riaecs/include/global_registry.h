#pragma once
#include "riaecs/include/dll_config.h"

#include "riaecs/include/registry.h"
#include "riaecs/include/ecs.h"
#include "riaecs/include/asset.h"
#include "riaecs/include/file.h"

#include <memory>

namespace riaecs
{
    extern RIAECS_API std::unique_ptr<IComponentFactoryRegistry> gComponentFactoryRegistry;
    extern RIAECS_API std::unique_ptr<IComponentMaxCountRegistry> gComponentMaxCountRegistry;
    template <typename COMPONENT, size_t MAX_COUNT> 
    class ComponentRegistrar
    {
    private:
        size_t componentID_;

    public:
        ComponentRegistrar()
        {
            componentID_ = gComponentFactoryRegistry->Add(std::make_unique<ComponentFactory<COMPONENT>>());
            gComponentMaxCountRegistry->Add(std::make_unique<size_t>(MAX_COUNT));
        }

        size_t operator()() const
        {
            return componentID_;
        }
    };

    extern RIAECS_API std::unique_ptr<IAssetFactoryRegistry> gAssetFactoryRegistry;
    template <typename ASSET_FACTORY>
    class AssetFactoryRegistrar
    {
    private:
        size_t assetFactoryID_;

    public:
        AssetFactoryRegistrar()
        {
            assetFactoryID_ = gAssetFactoryRegistry->Add(std::make_unique<ASSET_FACTORY>());
        }

        size_t operator()() const
        {
            return assetFactoryID_;
        }
    };

    extern RIAECS_API std::unique_ptr<IAssetSourceRegistry> gAssetSourceRegistry;
    class AssetSourceRegistrar
    {
    private:
        size_t assetSourceID_;

    public:
        AssetSourceRegistrar(std::string path, size_t loaderID, size_t factoryID)
        {
            assetSourceID_ 
            = gAssetSourceRegistry->Add(std::make_unique<AssetSource>(std::move(path), loaderID, factoryID));
        }

        size_t operator()() const
        {
            return assetSourceID_;
        }
    };

    extern RIAECS_API std::unique_ptr<IFileLoaderRegistry> gFileLoaderRegistry;
    template <typename FILE_LOADER>
    class FileLoaderRegistrar
    {
    private:
        size_t fileLoaderID_;

    public:
        FileLoaderRegistrar()
        {
            fileLoaderID_ = gFileLoaderRegistry->Add(std::make_unique<FILE_LOADER>());
        }

        size_t operator()() const
        {
            return fileLoaderID_;
        }
    };

} // namespace riaecs