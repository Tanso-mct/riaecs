#pragma once
#include "riaecs/include/dll_config.h"

#include "riaecs/include/interfaces/ecs.h"
#include "riaecs/include/types/stl_hash.h"
#include "riaecs/include/types/stl_euqal.h"

#include <unordered_map>

namespace riaecs
{
    class RIAECS_API ECSWorld : public IECSWorld
    {
    private:
        std::unique_ptr<IComponentFactoryRegistry> componentFactoryRegistry_ = nullptr;
        std::unique_ptr<IComponentDestroyerRegistry> componentDestroyerRegistry_ = nullptr;
        std::unique_ptr<IPoolFactory> poolFactory_ = nullptr;
        std::unique_ptr<IAllocatorFactory> allocatorFactory_ = nullptr;
        mutable bool isReady_ = false;

        size_t nextEntityIndex_ = 0;
        std::vector<ID> freeEntityIDs_;

        std::vector<std::unique_ptr<IPool>> componentPools_;
        std::vector<std::unique_ptr<IAllocator>> componentAllocators_;

        std::unordered_map<Entity, std::unordered_set<size_t>> entityToComponents_;
        std::unordered_map<size_t, std::unordered_set<Entity>> componentToEntities_;
        std::unordered_map<std::pair<Entity, size_t>, std::byte*, PairHash, PairEqual> entityComponentToData_;

    public:
        ECSWorld() = default;
        virtual ~ECSWorld() = default;

        void SetComponentFactoryRegistry(std::unique_ptr<IComponentFactoryRegistry> registry) override;
        void SetComponentDestroyerRegistry(std::unique_ptr<IComponentDestroyerRegistry> registry) override;
        void SetPoolFactory(std::unique_ptr<IPoolFactory> poolFactory) override;
        void SetAllocatorFactory(std::unique_ptr<IAllocatorFactory> allocatorFactory) override;
        bool IsReady() const override;

        void CreateWorld() override;
        void DestroyWorld() override;

        Entity CreateEntity() override;
        void DestroyEntity(const Entity &entity) override;

        void AddComponent(const Entity &entity, size_t componentID) override;
        void RemoveComponent(const Entity &entity, size_t componentID) override;
        bool HasComponent(const Entity &entity, size_t componentID) const override;
        void *GetComponent(const Entity &entity, size_t componentID) override;

        std::unordered_set<Entity> View(size_t componentID) const override;
    };

} // namespace riaecs