#include "riaecs_unit_test/pch.h"

#include "riaecs/include/asset.h"
#include "riaecs/include/global_registry.h"
#pragma comment(lib, "riaecs.lib")

namespace
{
    class TestFileData : public riaecs::IFileData
    {
    public:
        TestFileData() 
        {
            std::cout << "TestFileData created" << std::endl;
        }

        ~TestFileData() override
        { 
            std::cout << "TestFileData destroyed" << std::endl; 
        }
    };

    class TestFileLoader : public riaecs::IFileLoader
    {
    public:
        TestFileLoader() 
        {
            std::cout << "TestFileLoader created" << std::endl;
        }

        ~TestFileLoader() override
        { 
            std::cout << "TestFileLoader destroyed" << std::endl; 
        }

        std::unique_ptr<riaecs::IFileData> Load(std::string_view filePath) const override
        {
            std::cout << "Loading file data from: " << filePath << std::endl;
            return std::make_unique<TestFileData>();
        }
    };
    riaecs::FileLoaderRegistrar<TestFileLoader> TestFileLoaderID;

    constexpr int INITIAL_ASSET_VALUE = 10;
    class TestAsset : public riaecs::IAsset
    {
    public:
        TestAsset() 
        {
            std::cout << "TestAsset created" << std::endl;
        }

        ~TestAsset() override
        { 
            std::cout << "TestAsset destroyed" << std::endl; 
        }

        int value = INITIAL_ASSET_VALUE;
    };

    class TestAssetStagingArea : public riaecs::IAssetStagingArea
    {
    public:
        TestAssetStagingArea() 
        {
            std::cout << "TestAssetStagingArea created" << std::endl;
        }

        ~TestAssetStagingArea() override
        { 
            std::cout << "TestAssetStagingArea destroyed" << std::endl; 
        }
    };

    class TestAssetFactory : public riaecs::IAssetFactory
    {
    public:
        TestAssetFactory() 
        {
            std::cout << "TestAssetFactory created" << std::endl;
        }

        ~TestAssetFactory() override
        { 
            std::cout << "TestAssetFactory destroyed" << std::endl; 
        }

        std::unique_ptr<riaecs::IAssetStagingArea> Prepare() const override
        {
            std::cout << "Preparing asset staging area" << std::endl;
            return std::make_unique<TestAssetStagingArea>();
        }

        std::unique_ptr<riaecs::IAsset> Create
        (
            const riaecs::IFileData &fileData, riaecs::IAssetStagingArea &stagingArea
        ) const override
        {
            std::cout << "Creating asset from file data" << std::endl;
            return std::make_unique<TestAsset>();
        }

        void Commit(riaecs::IAssetStagingArea &stagingArea) const override
        {
            std::cout << "Committing asset staging area" << std::endl;
        }
    };
    riaecs::AssetFactoryRegistrar<TestAssetFactory> TestAssetFactoryID;

    riaecs::AssetSourceRegistrar TestAssetSourceRegistrar
    (
        "test_asset_path", 
        TestFileLoaderID(), 
        TestAssetFactoryID()
    );

} // namespace

TEST(Asset, Create)
{
    riaecs::ReadOnlyObject<riaecs::AssetSource> assetSource 
    = riaecs::gAssetSourceRegistry->Get(TestAssetSourceRegistrar());

    riaecs::ReadOnlyObject<riaecs::IFileLoader> fileLoader 
    = riaecs::gFileLoaderRegistry->Get(assetSource().GetFileLoaderID());

    riaecs::ReadOnlyObject<riaecs::IAssetFactory> assetFactory 
    = riaecs::gAssetFactoryRegistry->Get(assetSource().GetAssetFactoryID());

    // Load file data using the file loader
    std::unique_ptr<riaecs::IFileData> fileData = fileLoader().Load(assetSource().GetFilePath());

    // Prepare the asset staging area using the asset factory
    std::unique_ptr<riaecs::IAssetStagingArea> stagingArea = assetFactory().Prepare();

    // Create the asset using the file data
    std::unique_ptr<riaecs::IAsset> asset = assetFactory().Create(*fileData, *stagingArea);

    // Commit the asset staging area
    assetFactory().Commit(*stagingArea);

    // Release the file data and staging area after asset creation
    fileData.reset();
    stagingArea.reset();

    // Check if the asset was created successfully
    ASSERT_NE(asset, nullptr);

    // Verify the asset properties
    TestAsset &testAsset = dynamic_cast<TestAsset&>(*asset);
    ASSERT_EQ(testAsset.value, INITIAL_ASSET_VALUE);

    // Release the asset
    asset.reset();
}