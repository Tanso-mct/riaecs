#pragma once

#include "riaecs/include/interfaces/ecs.h"

namespace riaecs
{
    class ISystem
    {
    public:
        virtual ~ISystem() = default;
        virtual void Update(IECSWorld &world) = 0;
    };

} // namespace riaecs