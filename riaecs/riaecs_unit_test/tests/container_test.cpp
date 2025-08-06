#include "riaecs_unit_test/pch.h"

#include "riaecs/include/container.h"
#pragma comment(lib, "riaecs.lib");

TEST(Container, Create)
{
    riaecs::Container<int> container;
    container.Create(10);
    EXPECT_EQ(container.GetCount(), 10);
}