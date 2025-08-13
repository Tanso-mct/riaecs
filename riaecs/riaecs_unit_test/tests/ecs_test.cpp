#include "riaecs_unit_test/pch.h"

#include "riaecs/include/ecs.h"
#include "riaecs/include/global_registry.h"
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
    riaecs::ComponentRegistrar<TestAComponent, 10> TestAComponentID;

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
    riaecs::ComponentRegistrar<TestBComponent, 10> TestBComponentID;

} // namespace

TEST(ECS, World)
{
    std::unique_ptr<riaecs::IECSWorld> ecsWorld = std::make_unique<riaecs::ECSWorld>();
    ecsWorld->SetComponentFactoryRegistry(std::move(riaecs::gComponentFactoryRegistry));
    ecsWorld->SetComponentMaxCountRegistry(std::move(riaecs::gComponentMaxCountRegistry));
    ecsWorld->SetPoolFactory(std::make_unique<mem_alloc_fixed_block::FixedBlockPoolFactory>());
    ecsWorld->SetAllocatorFactory(std::make_unique<mem_alloc_fixed_block::FixedBlockAllocatorFactory>());
    EXPECT_TRUE(ecsWorld->IsReady());

    ecsWorld->CreateWorld();

    // Create entity 1
    riaecs::Entity entity1 = ecsWorld->CreateEntity();
    EXPECT_EQ(entity1.GetIndex(), 0);

    // Add component to entity 1
    ecsWorld->AddComponent(entity1, TestAComponentID());

    {
        riaecs::Entity invalidGenerationEntity(0, 10);
        EXPECT_THROW(ecsWorld->AddComponent(invalidGenerationEntity, TestAComponentID()), std::runtime_error);

        riaecs::Entity invalidIndexEntity(10, 0);
        EXPECT_THROW(ecsWorld->AddComponent(invalidIndexEntity, TestAComponentID()), std::runtime_error);
    }

    // Get component
    {
        riaecs::ReadOnlyObject<TestAComponent*> test 
        = riaecs::GetComponent<TestAComponent>(*ecsWorld, entity1, TestAComponentID());

        EXPECT_NE(test(), nullptr);
        EXPECT_EQ(test()->value, INITIAL_A_VALUE);

        for (riaecs::Entity entity : ecsWorld->View(TestAComponentID())())
        {
            riaecs::ReadOnlyObject<TestAComponent*> testFromView 
            = riaecs::GetComponent<TestAComponent>(*ecsWorld, entity, TestAComponentID());

            EXPECT_NE(testFromView(), nullptr);
            EXPECT_EQ(testFromView()->value, INITIAL_A_VALUE);
        }
    }

    // Create entity 2
    riaecs::Entity entity2 = ecsWorld->CreateEntity();
    EXPECT_EQ(entity2.GetIndex(), 1);

    // Add component to entity 2
    ecsWorld->AddComponent(entity2, TestAComponentID());
    ecsWorld->AddComponent(entity2, TestBComponentID());

    // Should throw because entity already has this component
    EXPECT_THROW(ecsWorld->AddComponent(entity2, TestAComponentID()), std::runtime_error);

    // Get component
    {
        riaecs::ReadOnlyObject<TestBComponent*> test 
        = riaecs::GetComponent<TestBComponent>(*ecsWorld, entity2, TestBComponentID());

        EXPECT_NE(test(), nullptr);
        EXPECT_EQ(test()->intValue, INITIAL_B_INT_VALUE);
        EXPECT_EQ(test()->floatValue, INITIAL_B_FLOAT_VALUE);
        EXPECT_EQ(test()->doubleValue, INITIAL_B_DOUBLE_VALUE);
        EXPECT_EQ(test()->stringValue, INITIAL_B_STRING_VALUE);

        for (riaecs::Entity entity : ecsWorld->View(TestBComponentID())())
        {
            riaecs::ReadOnlyObject<TestBComponent*> testFromView 
            = riaecs::GetComponent<TestBComponent>(*ecsWorld, entity, TestBComponentID());

            EXPECT_NE(testFromView(), nullptr);
            EXPECT_EQ(testFromView()->intValue, INITIAL_B_INT_VALUE);
            EXPECT_EQ(testFromView()->floatValue, INITIAL_B_FLOAT_VALUE);
            EXPECT_EQ(testFromView()->doubleValue, INITIAL_B_DOUBLE_VALUE);
            EXPECT_EQ(testFromView()->stringValue, INITIAL_B_STRING_VALUE);
        }
    }

    size_t componentCount = 0;
    for (riaecs::Entity entity : ecsWorld->View(TestAComponentID())())
    {
        riaecs::ReadOnlyObject<TestAComponent*> testFromView 
        = riaecs::GetComponent<TestAComponent>(*ecsWorld, entity, TestAComponentID());

        EXPECT_NE(testFromView(), nullptr);
        EXPECT_EQ(testFromView()->value, INITIAL_A_VALUE);

        componentCount++;
    }
    EXPECT_EQ(componentCount, 2); // Both entities should have TestAComponent

    // Destroy entity 1
    ecsWorld->DestroyEntity(entity1);

    // Re create entity
    size_t registerIndex = riaecs::ECSWorld::CreateRegisterIndex();
    {
        riaecs::Entity entity3 = ecsWorld->CreateEntity();
        EXPECT_EQ(entity3.GetIndex(), 0);

        // Register entity
        ecsWorld->RegisterEntity(registerIndex, entity3);
    }

    // Check registered entity
    {
        riaecs::Entity registeredEntity = ecsWorld->GetRegisteredEntity(registerIndex);
        EXPECT_EQ(registeredEntity.GetIndex(), entity1.GetIndex()); // Should be the same index as entity1
        EXPECT_EQ(registeredEntity.GetGeneration(), entity1.GetGeneration() + 1); // Should be incremented after re-creation
    }

    ecsWorld->DestroyWorld();
}