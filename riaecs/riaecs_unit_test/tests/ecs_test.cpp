#include "riaecs_unit_test/pch.h"

#include "riaecs/include/ecs.h"
#pragma comment(lib, "riaecs.lib")

#include "mem_alloc_fixed_block/mem_alloc_fixed_block.h"
#pragma comment(lib, "mem_alloc_fixed_block.lib")

TEST(ECS, World)
{
    const int INITIAL_VALUE = 7;

    class TestComponent
    {
    public:
        int value;

        TestComponent() : value(INITIAL_VALUE) 
        {
            std::cout << "TestComponent created with value: " << value << std::endl;
        }

        ~TestComponent() 
        { 
            std::cout << "TestComponent destroyed with value: " << value << std::endl;
            value = 0; 
        }
    };

    class TestComponentFactory : public riaecs::IComponentFactory
    {
    public:
        std::byte* Create(std::byte* data) const override
        {
            if (data == nullptr)
                return nullptr;

            TestComponent* component = new(data) TestComponent();
            return reinterpret_cast<std::byte*>(component);
        }

        void Destroy(std::byte* data) const override
        {
            if (data == nullptr)
                return;

            TestComponent* component = reinterpret_cast<TestComponent*>(data);
            component->~TestComponent();
        }

        size_t GetProductSize() const override { return sizeof(TestComponent); }
    };

    std::unique_ptr<riaecs::IECSWorld> ecsWorld = std::make_unique<riaecs::ECSWorld>();

    size_t componentID = 0;
    {
        std::unique_ptr<riaecs::IComponentFactoryRegistry> componentFactoryRegistry 
        = std::make_unique<riaecs::ComponentFactoryRegistry>();

        componentID = componentFactoryRegistry->Add(std::make_unique<TestComponentFactory>());

        ecsWorld->SetComponentFactoryRegistry(std::move(componentFactoryRegistry));
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
    ecsWorld->AddComponent(entity1, componentID);

    // Get component
    {
        riaecs::ReadOnlyObject<TestComponent*> test 
        = riaecs::GetComponent<TestComponent>(*ecsWorld, entity1, componentID);

        EXPECT_NE(test(), nullptr);
        EXPECT_EQ(test()->value, INITIAL_VALUE);

        for (riaecs::Entity entity : ecsWorld->View(componentID)())
        {
            riaecs::ReadOnlyObject<TestComponent*> testFromView 
            = riaecs::GetComponent<TestComponent>(*ecsWorld, entity, componentID);

            EXPECT_NE(testFromView(), nullptr);
            EXPECT_EQ(testFromView()->value, INITIAL_VALUE);
        }
    }

    // Create entity 2
    riaecs::Entity entity2 = ecsWorld->CreateEntity();
    EXPECT_EQ(entity2.GetIndex(), 1);

    // Destroy entity 1
    ecsWorld->DestroyEntity(entity1);

    // Re create entity
    riaecs::Entity entity3 = ecsWorld->CreateEntity();
    EXPECT_EQ(entity3.GetIndex(), 0);

    ecsWorld->DestroyWorld();
}