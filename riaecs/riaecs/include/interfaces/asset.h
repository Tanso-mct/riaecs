#pragma once

#include "riaecs/include/interfaces/registry.h"
#include "riaecs/include/interfaces/factory.h"
#include "riaecs/include/interfaces/file.h"
#include "riaecs/include/interfaces/container.h"

#include <string>
#include <string_view>

namespace riaecs
{
    class IAsset
    {
    public:
        virtual ~IAsset() = default;
    };

    class IAssetStagingArea
    {
    public:
        virtual ~IAssetStagingArea() = default;
    };

    class IAssetFactory
    {
    public:
        virtual ~IAssetFactory() = default;
        
        virtual std::unique_ptr<IAssetStagingArea> Prepare() const = 0;
        virtual std::unique_ptr<IAsset> Create(const IFileData &fileData, IAssetStagingArea &stagingArea) const = 0;
        virtual void Commit(IAssetStagingArea &stagingArea) const = 0;
    };

    class AssetSource
    {
    private:
        std::string filePath;
        size_t fileLoaderID;
        size_t assetFactoryID;

    public:
        AssetSource(std::string path, size_t loaderID, size_t factoryID)
            : filePath(path), fileLoaderID(loaderID), assetFactoryID(factoryID) {}

        std::string_view GetFilePath() const { return filePath; }
        size_t GetFileLoaderID() const { return fileLoaderID; }
        size_t GetAssetFactoryID() const { return assetFactoryID; }
    };

    using IAssetFactoryRegistry = IRegistry<IAssetFactory>;
    using IAssetSourceRegistry = IRegistry<AssetSource>;

    using IAssetContainer = IContainer<IAsset>;

} // namespace riaecs