#pragma once
#include "riaecs/include/dll_config.h"

#include "riaecs/include/interfaces/ecs.h"
#include "riaecs/include/interfaces/factory.h"
#include "riaecs/include/types/stl_hash.h"
#include "riaecs/include/types/stl_euqal.h"

#include "riaecs/include/registry.h"

#include <unordered_map>
#include <shared_mutex>
#include <queue>

namespace riaecs
{
    using ComponentFactoryRegistry = Registry<IComponentFactory>;
    using ComponentMaxCountRegistry = Registry<size_t>;

    class RIAECS_API ECSWorld : public IECSWorld
    {
    private:
        mutable std::shared_mutex mutex_;

        std::unique_ptr<IComponentFactoryRegistry> componentFactoryRegistry_ = nullptr;
        std::unique_ptr<IComponentMaxCountRegistry> componentMaxCountRegistry_ = nullptr;
        std::unique_ptr<IPoolFactory> poolFactory_ = nullptr;
        std::unique_ptr<IAllocatorFactory> allocatorFactory_ = nullptr;
        mutable bool isReady_ = false;

        std::vector<bool> entityExistFlags_;
        std::vector<Entity> entities_;
        std::vector<Entity> freeEntities_;

        std::vector<std::unique_ptr<IPool>> componentPools_;
        std::vector<std::unique_ptr<IAllocator>> componentAllocators_;

        std::unordered_map<Entity, std::unordered_set<size_t>> entityToComponents_;
        std::unordered_map<size_t, std::unordered_set<Entity>> componentToEntities_;
        std::unordered_map<std::pair<Entity, size_t>, std::byte*, PairHash, PairEqual> entityComponentToData_;

        std::unordered_set<Entity> emptyEntities_;

    public:
        ECSWorld() = default;
        virtual ~ECSWorld() = default;

        void SetComponentFactoryRegistry(std::unique_ptr<IComponentFactoryRegistry> registry) override;
        void SetPoolFactory(std::unique_ptr<IPoolFactory> poolFactory) override;
        void SetAllocatorFactory(std::unique_ptr<IAllocatorFactory> allocatorFactory) override;
        void SetComponentMaxCountRegistry(std::unique_ptr<IComponentMaxCountRegistry> registry) override;
        bool IsReady() const override;

        void CreateWorld() override;
        void DestroyWorld() override;

        Entity CreateEntity() override;
        void DestroyEntity(const Entity &entity) override;

        void AddComponent(const Entity &entity, size_t componentID) override;
        void RemoveComponent(const Entity &entity, size_t componentID) override;
        bool HasComponent(const Entity &entity, size_t componentID) const override;
        ReadOnlyObject<std::byte*> GetComponent(const Entity &entity, size_t componentID) override;

        ReadOnlyObject<std::unordered_set<Entity>> View(size_t componentID) const override;
    };

    template <typename T>
    class ComponentFactory : public IComponentFactory
    {
    public:
        std::byte *Create(std::byte *data) const override
        {
            if (data == nullptr)
                return nullptr;

            T* component = new(data) T();
            return reinterpret_cast<std::byte*>(component);
        }

        void Destroy(std::byte *data) const override
        {
            if (data == nullptr)
                return;

            T* component = reinterpret_cast<T*>(data);
            component->~T();
        }

        size_t GetProductSize() const override
        {
            return sizeof(T);
        }
    };

    template <typename T>
    class SystemFactory : public ISystemFactory
    {
    public:
        SystemFactory() = default;
        ~SystemFactory() override = default;

        /***************************************************************************************************************
         * IFactory Implementation
        /**************************************************************************************************************/

        std::unique_ptr<ISystem> Create() const override
        {
            return std::make_unique<T>();
        }

        void Destroy(std::unique_ptr<ISystem> product) const override
        {
            product.reset();
        }

        size_t GetProductSize() const override
        {
            return sizeof(T);
        }
    };

    using SystemFactoryRegistry = Registry<ISystemFactory>;

    class RIAECS_API SystemList : public ISystemList
    {
    private:
        std::vector<std::unique_ptr<ISystem>> systems_;
        mutable std::shared_mutex mutex_;

    public:
        SystemList() = default;
        virtual ~SystemList() override = default;

        SystemList(const SystemList&) = delete;
        SystemList& operator=(const SystemList&) = delete;

        /***************************************************************************************************************
         * ISystemList Implementation
        /**************************************************************************************************************/

        void Add(size_t systemID) override;
        ISystem &Get(size_t index) override;
        size_t GetCount() const override;
        void Clear() override;
    };

    class RIAECS_API SystemListFactory : public ISystemListFactory
    {
    public:
        SystemListFactory() = default;
        ~SystemListFactory() override = default;

        /***************************************************************************************************************
         * IFactory Implementation
        /**************************************************************************************************************/

        std::unique_ptr<ISystemList> Create() const override;
        void Destroy(std::unique_ptr<ISystemList> product) const override;
        size_t GetProductSize() const override;
    };

    class RIAECS_API SystemLoopCommandQueue : public ISystemLoopCommandQueue
    {
    private:
        std::queue<std::unique_ptr<ISystemLoopCommand>> commandQueue_;
        mutable std::shared_mutex mutex_;

    public:
        SystemLoopCommandQueue() = default;
        virtual ~SystemLoopCommandQueue() override = default;

        /***************************************************************************************************************
         * ISystemLoopCommandQueue Implementation
        /**************************************************************************************************************/

        void Enqueue(std::unique_ptr<ISystemLoopCommand> cmd) override;
        std::unique_ptr<ISystemLoopCommand> Dequeue() override;
        bool IsEmpty() const override;
    };

    class RIAECS_API SystemLoopCommandQueueFactory : public ISystemLoopCommandQueueFactory
    {
    public:
        SystemLoopCommandQueueFactory() = default;
        ~SystemLoopCommandQueueFactory() override = default;

        /***************************************************************************************************************
         * IFactory Implementation
        /**************************************************************************************************************/

        std::unique_ptr<ISystemLoopCommandQueue> Create() const override;
        void Destroy(std::unique_ptr<ISystemLoopCommandQueue> product) const override;
        size_t GetProductSize() const override;
    };

    class RIAECS_API SystemLoop : public ISystemLoop
    {
    private:
        mutable std::shared_mutex mutex_;

        std::unique_ptr<ISystemListFactory> listFactory_;
        std::unique_ptr<ISystemLoopCommandQueueFactory> loopCommandQueueFactory_;
        mutable bool isReady_ = false;

        std::unique_ptr<ISystemList> systemList_;
        std::unique_ptr<ISystemLoopCommandQueue> commandQueue_;

    public:
        SystemLoop() = default;
        virtual ~SystemLoop() override = default;

        /***************************************************************************************************************
         * ISystemLoop Implementation
        /**************************************************************************************************************/

        void SetSystemListFactory(std::unique_ptr<ISystemListFactory> factory) override;
        void SetSystemLoopCommandQueueFactory(std::unique_ptr<ISystemLoopCommandQueueFactory> factory) override;
        bool IsReady() const override;

        void Initialize() override;
        void Run(IECSWorld &world, IAssetContainer &assetCont) override;
    };

} // namespace riaecs