#include "riaecs_unit_test/pch.h"

#include "riaecs/include/ecs.h"
#pragma comment(lib, "riaecs.lib")

#include "mem_alloc_fixed_block/mem_alloc_fixed_block.h"
#pragma comment(lib, "mem_alloc_fixed_block.lib")

namespace
{
    constexpr int INITIAL_A_VALUE = 7;

    constexpr int INITIAL_B_INT_VALUE = 8;
    constexpr float INITIAL_B_FLOAT_VALUE = 3.14f;
    constexpr double INITIAL_B_DOUBLE_VALUE = 2.718;
    constexpr const char* INITIAL_B_STRING_VALUE = "default";

} // namespace

TEST(ECS, World)
{
    class TestAComponent
    {
    public:
        int value;

        TestAComponent() : value(INITIAL_A_VALUE) 
        {
            std::cout << "TestAComponent created with value: " << value << std::endl;
        }

        ~TestAComponent() 
        { 
            std::cout << "TestAComponent destroyed with value: " << value << std::endl;
            value = 0; 
        }
    };

    class TestAComponentFactory : public riaecs::IComponentFactory
    {
    public:
        std::byte* Create(std::byte* data) const override
        {
            if (data == nullptr)
                return nullptr;

            TestAComponent* component = new(data) TestAComponent();
            return reinterpret_cast<std::byte*>(component);
        }

        void Destroy(std::byte* data) const override
        {
            if (data == nullptr)
                return;

            TestAComponent* component = reinterpret_cast<TestAComponent*>(data);
            component->~TestAComponent();
        }

        size_t GetProductSize() const override { return sizeof(TestAComponent); }
    };

    class TestBComponent
    {
    public:
        int intValue;
        float floatValue;
        double doubleValue;
        std::string stringValue;

        TestBComponent() : 
        intValue(INITIAL_B_INT_VALUE), 
        floatValue(INITIAL_B_FLOAT_VALUE), 
        doubleValue(INITIAL_B_DOUBLE_VALUE), 
        stringValue(INITIAL_B_STRING_VALUE) 
        {
            std::cout << "TestBComponent created with values: " 
            << intValue << ", " << floatValue << ", " << doubleValue << ", " << stringValue << std::endl;
        }

        ~TestBComponent() 
        { 
            std::cout << "TestBComponent destroyed with values: " 
            << intValue << ", " << floatValue << ", " << doubleValue << ", " << stringValue << std::endl;
            intValue = 0; 
            floatValue = 0.0f; 
            doubleValue = 0.0; 
            stringValue.clear(); 
        }
    };

    class TestBComponentFactory : public riaecs::IComponentFactory
    {
    public:
        std::byte* Create(std::byte* data) const override
        {
            if (data == nullptr)
                return nullptr;

            TestBComponent* component = new(data) TestBComponent();
            return reinterpret_cast<std::byte*>(component);
        }

        void Destroy(std::byte* data) const override
        {
            if (data == nullptr)
                return;

            TestBComponent* component = reinterpret_cast<TestBComponent*>(data);
            component->~TestBComponent();
        }

        size_t GetProductSize() const override { return sizeof(TestBComponent); }
    };

    std::unique_ptr<riaecs::IECSWorld> ecsWorld = std::make_unique<riaecs::ECSWorld>();

    size_t testAComponentID;
    size_t testBComponentID;
    {
        std::unique_ptr<riaecs::IComponentFactoryRegistry> componentFactoryRegistry 
        = std::make_unique<riaecs::ComponentFactoryRegistry>();

        testAComponentID = componentFactoryRegistry->Add(std::make_unique<TestAComponentFactory>());
        testBComponentID = componentFactoryRegistry->Add(std::make_unique<TestBComponentFactory>());

        ecsWorld->SetComponentFactoryRegistry(std::move(componentFactoryRegistry));
    }

    {
        std::unique_ptr<riaecs::IComponentMaxCountRegistry> componentMaxCountRegistry 
        = std::make_unique<riaecs::ComponentMaxCountRegistry>();

        componentMaxCountRegistry->Add(std::make_unique<size_t>(10)); // Max count for TestAComponent
        componentMaxCountRegistry->Add(std::make_unique<size_t>(10)); // Max count for TestBComponent

        ecsWorld->SetComponentMaxCountRegistry(std::move(componentMaxCountRegistry));
    }

    {
        std::unique_ptr<riaecs::IPoolFactory> poolFactory 
        = std::make_unique<mem_alloc_fixed_block::FixedBlockPoolFactory>();

        ecsWorld->SetPoolFactory(std::move(poolFactory));
    }

    {
        std::unique_ptr<riaecs::IPoolFactory> poolFactory 
        = std::make_unique<mem_alloc_fixed_block::FixedBlockPoolFactory>();

        ecsWorld->SetPoolFactory(std::move(poolFactory));
    }

    {
        std::unique_ptr<riaecs::IAllocatorFactory> allocatorFactory 
        = std::make_unique<mem_alloc_fixed_block::FixedBlockAllocatorFactory>();

        ecsWorld->SetAllocatorFactory(std::move(allocatorFactory));
    }

    EXPECT_TRUE(ecsWorld->IsReady());

    ecsWorld->CreateWorld();

    // Create entity 1
    riaecs::Entity entity1 = ecsWorld->CreateEntity();
    EXPECT_EQ(entity1.GetIndex(), 0);

    // Add component to entity 1
    ecsWorld->AddComponent(entity1, testAComponentID);

    {
        riaecs::Entity invalidGenerationEntity(0, 10);
        EXPECT_THROW(ecsWorld->AddComponent(invalidGenerationEntity, testAComponentID), std::runtime_error);

        riaecs::Entity invalidIndexEntity(10, 0);
        EXPECT_THROW(ecsWorld->AddComponent(invalidIndexEntity, testAComponentID), std::runtime_error);
    }

    // Get component
    {
        riaecs::ReadOnlyObject<TestAComponent*> test 
        = riaecs::GetComponent<TestAComponent>(*ecsWorld, entity1, testAComponentID);

        EXPECT_NE(test(), nullptr);
        EXPECT_EQ(test()->value, INITIAL_A_VALUE);

        for (riaecs::Entity entity : ecsWorld->View(testAComponentID)())
        {
            riaecs::ReadOnlyObject<TestAComponent*> testFromView 
            = riaecs::GetComponent<TestAComponent>(*ecsWorld, entity, testAComponentID);

            EXPECT_NE(testFromView(), nullptr);
            EXPECT_EQ(testFromView()->value, INITIAL_A_VALUE);
        }
    }

    // Create entity 2
    riaecs::Entity entity2 = ecsWorld->CreateEntity();
    EXPECT_EQ(entity2.GetIndex(), 1);

    // Add component to entity 2
    ecsWorld->AddComponent(entity2, testAComponentID);
    ecsWorld->AddComponent(entity2, testBComponentID);

    // Should throw because entity already has this component
    EXPECT_THROW(ecsWorld->AddComponent(entity2, testAComponentID), std::runtime_error);

    // Get component
    {
        riaecs::ReadOnlyObject<TestBComponent*> test 
        = riaecs::GetComponent<TestBComponent>(*ecsWorld, entity2, testBComponentID);

        EXPECT_NE(test(), nullptr);
        EXPECT_EQ(test()->intValue, INITIAL_B_INT_VALUE);
        EXPECT_EQ(test()->floatValue, INITIAL_B_FLOAT_VALUE);
        EXPECT_EQ(test()->doubleValue, INITIAL_B_DOUBLE_VALUE);
        EXPECT_EQ(test()->stringValue, INITIAL_B_STRING_VALUE);

        for (riaecs::Entity entity : ecsWorld->View(testBComponentID)())
        {
            riaecs::ReadOnlyObject<TestBComponent*> testFromView 
            = riaecs::GetComponent<TestBComponent>(*ecsWorld, entity, testBComponentID);

            EXPECT_NE(testFromView(), nullptr);
            EXPECT_EQ(testFromView()->intValue, INITIAL_B_INT_VALUE);
            EXPECT_EQ(testFromView()->floatValue, INITIAL_B_FLOAT_VALUE);
            EXPECT_EQ(testFromView()->doubleValue, INITIAL_B_DOUBLE_VALUE);
            EXPECT_EQ(testFromView()->stringValue, INITIAL_B_STRING_VALUE);
        }
    }

    size_t componentCount = 0;
    for (riaecs::Entity entity : ecsWorld->View(testAComponentID)())
    {
        riaecs::ReadOnlyObject<TestAComponent*> testFromView 
        = riaecs::GetComponent<TestAComponent>(*ecsWorld, entity, testAComponentID);

        EXPECT_NE(testFromView(), nullptr);
        EXPECT_EQ(testFromView()->value, INITIAL_A_VALUE);

        componentCount++;
    }
    EXPECT_EQ(componentCount, 2); // Both entities should have TestAComponent

    // Destroy entity 1
    ecsWorld->DestroyEntity(entity1);

    // Re create entity
    riaecs::Entity entity3 = ecsWorld->CreateEntity();
    EXPECT_EQ(entity3.GetIndex(), 0);

    ecsWorld->DestroyWorld();
}