#pragma once

#include "riaecs/include/types/id.h"
#include "riaecs/include/interfaces/registry.h"
#include "riaecs/include/interfaces/factory.h"
#include "riaecs/include/interfaces/memory.h"

#include <memory>
#include <unordered_set>

namespace riaecs
{
    using Entity = ID;
    using ComponentFactory = IFactory<void*, void*>;
    using ComponentFactoryRegistry = IRegistry<ComponentFactory>;

    using IPoolFactory = IFactory<IPool>;
    using IAllocatorFactory = IFactory<IAllocator>;

    class IECSWorld
    {
    public:
        virtual ~IECSWorld() = default;

        virtual void SetComponentFactoryRegistry(std::unique_ptr<ComponentFactoryRegistry> registry) = 0;
        virtual void SetPoolFactory(std::unique_ptr<IPoolFactory> poolFactory) = 0;
        virtual void SetAllocatorFactory(std::unique_ptr<IAllocatorFactory> allocatorFactory) = 0;
        virtual bool IsReady() const = 0;

        virtual Entity CreateEntity() = 0;
        virtual void DestroyEntity(const Entity &entity) = 0;

        virtual void AddComponent(const Entity &entity, size_t componentID) = 0;
        virtual void RemoveComponent(const Entity &entity, size_t componentID) = 0;
        virtual bool HasComponent(const Entity &entity, size_t componentID) const = 0;
        virtual void *GetComponent(const Entity &entity, size_t componentID) = 0;

        virtual std::unordered_set<Entity> View(size_t componentID) const = 0;
    };

} // namespace riaecs