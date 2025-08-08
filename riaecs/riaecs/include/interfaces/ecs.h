#pragma once

#include "riaecs/include/types/id.h"
#include "riaecs/include/interfaces/registry.h"
#include "riaecs/include/interfaces/factory.h"

#include <memory>
#include <vector>

namespace riaecs
{
    using Entity = ID;
    using ComponentFactory = IFactory<void*, void*>;
    using ComponentFactoryRegistry = IRegistry<ComponentFactory>;

    class IECSWorld
    {
    public:
        virtual ~IECSWorld() = default;

        virtual std::unique_ptr<Entity> CreateEntity() = 0;
        virtual void DestroyEntity(const Entity &entity) = 0;

        virtual void AddComponent(const Entity &entity, size_t componentID) = 0;
        virtual void RemoveComponent(const Entity &entity, size_t componentID) = 0;
        virtual bool HasComponent(const Entity &entity, size_t componentID) const = 0;
        virtual void *GetComponent(const Entity &entity, size_t componentID) = 0;

        virtual std::vector<Entity> View(size_t componentID) const = 0;
    };

} // namespace riaecs