#pragma once

#include "riaecs/include/types/id.h"
#include "riaecs/include/types/object.h"
#include "riaecs/include/interfaces/registry.h"
#include "riaecs/include/interfaces/factory.h"
#include "riaecs/include/interfaces/memory.h"
#include "riaecs/include/interfaces/asset.h"

#include <memory>
#include <unordered_set>

namespace riaecs
{
    using Entity = ID;

    using IComponentFactory = IFactory<std::byte*, std::byte*>;
    using IComponentFactoryRegistry = IRegistry<IComponentFactory>;

    using IComponentMaxCountRegistry = IRegistry<size_t>;

    using IPoolFactory = IFactory<std::unique_ptr<IPool>, size_t>;
    using IAllocatorFactory = IFactory<std::unique_ptr<IAllocator>, IPool&, size_t>;

    class IECSWorld
    {
    public:
        virtual ~IECSWorld() = default;

        virtual void SetComponentFactoryRegistry(std::unique_ptr<IComponentFactoryRegistry> registry) = 0;
        virtual void SetComponentMaxCountRegistry(std::unique_ptr<IComponentMaxCountRegistry> registry) = 0;
        virtual void SetPoolFactory(std::unique_ptr<IPoolFactory> poolFactory) = 0;
        virtual void SetAllocatorFactory(std::unique_ptr<IAllocatorFactory> allocatorFactory) = 0;
        virtual bool IsReady() const = 0;

        virtual void CreateWorld() = 0;
        virtual void DestroyWorld() = 0;

        virtual Entity CreateEntity() = 0;
        virtual void DestroyEntity(const Entity &entity) = 0;

        virtual void RegisterEntity(size_t index, const Entity &entity) = 0;
        virtual Entity GetRegisteredEntity(size_t index) const = 0;

        virtual void AddComponent(const Entity &entity, size_t componentID) = 0;
        virtual void RemoveComponent(const Entity &entity, size_t componentID) = 0;
        virtual bool HasComponent(const Entity &entity, size_t componentID) const = 0;
        virtual ReadOnlyObject<std::byte*> GetComponent(const Entity &entity, size_t componentID) = 0;

        virtual ReadOnlyObject<std::unordered_set<Entity>> View(size_t componentID) const = 0;
    };

    template <typename T>
    ReadOnlyObject<T*> GetComponent(IECSWorld &world, const Entity &entity, size_t componentID)
    {
        ReadOnlyObject<std::byte*> componentData = world.GetComponent(entity, componentID);
        if (componentData() == nullptr)
            return ReadOnlyObject<T*>(std::move(componentData.TakeLock()), nullptr);

        T* data = reinterpret_cast<T*>(componentData());
        return ReadOnlyObject<T*>(std::move(componentData.TakeLock()), data);
    }

    class ISystemLoopCommandQueue;

    class ISystem
    {
    public:
        virtual ~ISystem() = default;

        // If returns true, the system loop will continue to run
        // If returns false, the system loop will stop
        virtual bool Update
        (
            IECSWorld &world, IAssetContainer &assetCont, 
            ISystemLoopCommandQueue &systemLoopCmdQueue
        ) = 0;
    };
    using ISystemFactory = IFactory<std::unique_ptr<ISystem>>;
    using ISystemFactoryRegistry = IRegistry<ISystemFactory>;

    class ISystemList
    {
    public:
        virtual ~ISystemList() = default;

        virtual void Add(size_t systemID) = 0;
        virtual ISystem &Get(size_t index) = 0;
        virtual size_t GetCount() const = 0;
        virtual void Clear() = 0;
    };
    using ISystemListFactory = IFactory<std::unique_ptr<ISystemList>>;

    class ISystemLoopCommand
    {
    public:
        virtual ~ISystemLoopCommand() = default;
        virtual void Execute(ISystemList &systemList, IECSWorld &world, IAssetContainer &assetCont) const = 0;
    };

    class ISystemLoopCommandQueue
    {
    public:
        virtual ~ISystemLoopCommandQueue() = default;

        virtual void Enqueue(std::unique_ptr<ISystemLoopCommand> cmd) = 0;
        virtual std::unique_ptr<ISystemLoopCommand> Dequeue() = 0;
        virtual bool IsEmpty() const = 0;
    };
    using ISystemLoopCommandQueueFactory = IFactory<std::unique_ptr<ISystemLoopCommandQueue>>;

    class ISystemLoop
    {
    public:
        virtual ~ISystemLoop() = default;

        virtual void SetSystemListFactory(std::unique_ptr<ISystemListFactory> factory) = 0;
        virtual void SetSystemLoopCommandQueueFactory(std::unique_ptr<ISystemLoopCommandQueueFactory> factory) = 0;
        virtual bool IsReady() const = 0;

        virtual void Initialize() = 0;
        virtual void Run(IECSWorld &world, IAssetContainer &assetCont) = 0;
    };

} // namespace riaecs