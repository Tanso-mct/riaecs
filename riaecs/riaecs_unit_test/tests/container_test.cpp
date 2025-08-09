#include "riaecs_unit_test/pch.h"

#include "riaecs/include/container.h"
#pragma comment(lib, "riaecs.lib");

TEST(Container, Use)
{
    class TestObject
    {
    public:
        int value;
        TestObject(int v) : value(v) {}
    };

    riaecs::Container<TestObject> container;

    // Create the container with an initial size
    const size_t INITIAL_COUNT = 5;
    container.Create(INITIAL_COUNT);
    EXPECT_EQ(container.GetCount(), INITIAL_COUNT);

    // Set and get objects in the container
    for (int i = 0; i < INITIAL_COUNT; ++i)
    {
        riaecs::ID id = riaecs::ID(i, riaecs::ID_DEFAULT_GENERATION);
        std::unique_ptr<TestObject> object = std::make_unique<TestObject>(i);
        container.Set(id, std::move(object));

        EXPECT_EQ(container.Contains(id), true);
        EXPECT_EQ(container.Get(id)().value, i);
    }

    // Test releasing an object
    {
        const size_t TARGET_INDEX = 2;
        riaecs::ID targetID = riaecs::ID(TARGET_INDEX, riaecs::ID_DEFAULT_GENERATION);

        std::unique_ptr<TestObject> releasedObject = container.Release(targetID);
        EXPECT_EQ(releasedObject->value, TARGET_INDEX);

        // Re set the object at the released index
        riaecs::ID targetNewID = riaecs::ID(TARGET_INDEX, container.GetGeneration(TARGET_INDEX));

        const size_t NEW_OBJECT_VALUE = 10;
        std::unique_ptr<TestObject> newObject = std::make_unique<TestObject>(NEW_OBJECT_VALUE);
        container.Set(targetNewID, std::move(newObject));
        EXPECT_EQ(container.Contains(targetNewID), true);
        EXPECT_EQ(container.Get(targetNewID)().value, NEW_OBJECT_VALUE);
    }

    // Add a new object
    {
        const size_t NEW_OBJECT_VALUE = 100;
        std::unique_ptr<TestObject> newObject = std::make_unique<TestObject>(NEW_OBJECT_VALUE);
        riaecs::ID newID = container.Add(std::move(newObject));

        EXPECT_EQ(container.Contains(newID), true);
        EXPECT_EQ(container.Get(newID)().value, 100);
    }

    // Erase an object
    const size_t ERASE_INDEX = 4;
    {
        riaecs::ID eraseID 
        = riaecs::ID(ERASE_INDEX, riaecs::ID_DEFAULT_GENERATION);

        std::unique_ptr<TestObject> erasedObject = container.Erase(eraseID);
        EXPECT_EQ(container.Contains(eraseID), false);
    }

    // Add a new object after erasing
    {
        const size_t NEW_OBJECT_VALUE = 200;
        std::unique_ptr<TestObject> newObject = std::make_unique<TestObject>(NEW_OBJECT_VALUE);
        riaecs::ID newID = container.Add(std::move(newObject));
        EXPECT_EQ(newID.GetIndex(), ERASE_INDEX);

        EXPECT_EQ(container.Contains(newID), true);
        EXPECT_EQ(container.Get(newID)().value, NEW_OBJECT_VALUE);
    }
}