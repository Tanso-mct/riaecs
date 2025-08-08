#include "riaecs_unit_test/pch.h"

#include "riaecs/include/ecs.h"
#pragma comment(lib, "riaecs.lib")

TEST(ECS, World)
{
    riaecs::ECSWorld ecsWorld;

    // Check if the ECSWorld is not ready initially
    EXPECT_THROW(ecsWorld.IsReady(), std::runtime_error);
}