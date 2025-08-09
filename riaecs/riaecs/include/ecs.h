#pragma once
#include "riaecs/include/dll_config.h"

#include "riaecs/include/interfaces/ecs.h"
#include "riaecs/include/types/stl_hash.h"
#include "riaecs/include/types/stl_euqal.h"

#include "riaecs/include/registry.h"

#include <unordered_map>
#include <shared_mutex>

namespace riaecs
{
    using ComponentFactoryRegistry = Registry<IComponentFactory>;

    class RIAECS_API ECSWorld : public IECSWorld
    {
    private:
        mutable std::shared_mutex mutex_;

        std::unique_ptr<IComponentFactoryRegistry> componentFactoryRegistry_ = nullptr;
        std::unique_ptr<IPoolFactory> poolFactory_ = nullptr;
        std::unique_ptr<IAllocatorFactory> allocatorFactory_ = nullptr;
        mutable bool isReady_ = false;

        std::vector<bool> entityExistFlags_;
        std::vector<size_t> freeEntities_;

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
        bool IsReady() const override;

        void CreateWorld() override;
        void DestroyWorld() override;

        Entity CreateEntity() override;
        void DestroyEntity(Entity entity) override;

        void AddComponent(Entity entity, size_t componentID) override;
        void RemoveComponent(Entity entity, size_t componentID) override;
        bool HasComponent(Entity entity, size_t componentID) const override;
        ReadOnlyObject<std::byte*> GetComponent(Entity entity, size_t componentID) override;

        ReadOnlyObject<std::unordered_set<Entity>> View(size_t componentID) const override;
    };

} // namespace riaecs