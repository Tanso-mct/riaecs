#include "riaecs_unit_test/pch.h"

#include "riaecs/include/factory_registry.h"
#include "riaecs/include/interfaces/factory.h"
#pragma comment(lib, "riaecs.lib")

constexpr int TEST_FACTORY_A = 0;
constexpr int TEST_FACTORY_B = 1;

TEST(FactoryRegistry, Use)
{
    using ITestFactory = riaecs::IFactory<int>;

    class TestAFactory : public ITestFactory
    {
    public:
        int Create() const override { return TEST_FACTORY_A; }
    };

    class TestBFactory : public ITestFactory
    {
    public:
        int Create() const override { return TEST_FACTORY_B; }
    };

    riaecs::FactoryRegistry<ITestFactory> registry;

    // Add a factory to the registry
    size_t idA = 0;
    {
        std::unique_ptr<ITestFactory> factoryA = std::make_unique<TestAFactory>();
        idA = registry.Add(std::move(factoryA));
        EXPECT_EQ(idA, 0);
        EXPECT_EQ(registry.GetCount(), 1);
    }

    size_t idB = 0;
    {
        std::unique_ptr<ITestFactory> factoryB = std::make_unique<TestBFactory>();
        idB = registry.Add(std::move(factoryB));
        EXPECT_EQ(idB, 1);
        EXPECT_EQ(registry.GetCount(), 2);
    }

    // Get factories from the registry
    {
        riaecs::ReadOnlyObject<ITestFactory> factoryA = registry.Get(idA);
        EXPECT_EQ(factoryA().Create(), TEST_FACTORY_A);

        riaecs::ReadOnlyObject<ITestFactory> factoryB = registry.Get(idB);
        EXPECT_EQ(factoryB().Create(), TEST_FACTORY_B);
    }
}