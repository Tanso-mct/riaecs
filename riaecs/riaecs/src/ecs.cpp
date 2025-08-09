#include "riaecs/src/pch.h"
#include "riaecs/include/ecs.h"

#include "riaecs/include/utilities.h"

void riaecs::ECSWorld::SetComponentFactoryRegistry(std::unique_ptr<IComponentFactoryRegistry> registry)
{
    componentFactoryRegistry_ = std::move(registry);
}

void riaecs::ECSWorld::SetComponentDestroyerRegistry(std::unique_ptr<IComponentDestroyerRegistry> registry)
{
    componentDestroyerRegistry_ = std::move(registry);
}

void riaecs::ECSWorld::SetPoolFactory(std::unique_ptr<IPoolFactory> poolFactory)
{
    poolFactory_ = std::move(poolFactory);
}

void riaecs::ECSWorld::SetAllocatorFactory(std::unique_ptr<IAllocatorFactory> allocatorFactory)
{
    allocatorFactory_ = std::move(allocatorFactory);
}

bool riaecs::ECSWorld::IsReady() const
{
    if (!componentFactoryRegistry_)
    {
        riaecs::NotifyError({"ComponentFactoryRegistry is not set"}, RIAECS_LOG_LOC);
        isReady_ = false;
        return isReady_;
    }

    if (!componentDestroyerRegistry_)
    {
        riaecs::NotifyError({"ComponentDestroyerRegistry is not set"}, RIAECS_LOG_LOC);
        isReady_ = false;
        return isReady_;
    }

    if (!poolFactory_)
    {
        riaecs::NotifyError({"PoolFactory is not set"}, RIAECS_LOG_LOC);
        isReady_ = false;
        return isReady_;
    }

    if (!allocatorFactory_)
    {
        riaecs::NotifyError({"AllocatorFactory is not set"}, RIAECS_LOG_LOC);
        isReady_ = false;
        return isReady_;
    }

    isReady_ = true;
    return isReady_;
}

void riaecs::ECSWorld::CreateWorld()
{
    if (!IsReady())
        riaecs::NotifyError({"ECSWorld is not ready"}, RIAECS_LOG_LOC);

    // Initialize component pools and allocators
    size_t componentCount = componentFactoryRegistry_->GetCount();
    componentPools_.resize(componentCount);
    componentAllocators_.resize(componentCount);

    for (size_t i = 0; i < componentCount; ++i)
    {
        riaecs::ReadOnlyObject<IComponentFactory> factory = componentFactoryRegistry_->Get(i);

        size_t blockSize = std::max(factory().GetProductSize(), riaecs::MAX_FREE_BLOCK_SIZE);
        componentPools_[i] = poolFactory_->Create(blockSize);
        componentAllocators_[i] = allocatorFactory_->Create(*componentPools_[i], blockSize);
    }
}

void riaecs::ECSWorld::DestroyWorld()
{
    if (!IsReady())
        riaecs::NotifyError({"ECSWorld is not ready"}, RIAECS_LOG_LOC);

    // Clear all component data
    entityComponentToData_.clear();
    entityToComponents_.clear();
    componentToEntities_.clear();

    // Clear pools and allocators
    componentPools_.clear();
    componentAllocators_.clear();

    // Reset entity management
    nextEntityIndex_ = 0;
    freeEntityIDs_.clear();

    // Reset ready state
    isReady_ = false;
}

riaecs::Entity riaecs::ECSWorld::CreateEntity()
{
    if (!isReady_)
        riaecs::NotifyError({"ECSWorld is not ready"}, RIAECS_LOG_LOC);

    if (!freeEntityIDs_.empty())
    {
        Entity entity = freeEntityIDs_.back();
        freeEntityIDs_.pop_back();
        
        return Entity(entity.index_, entity.generation_ + 1);
    }
    else
    {
        Entity entity(nextEntityIndex_, riaecs::ID_DEFAULT_GENERATION);
        nextEntityIndex_++;

        return entity;
    }
}

void riaecs::ECSWorld::DestroyEntity(const Entity &entity)
{
    if (!isReady_)
        riaecs::NotifyError({"ECSWorld is not ready"}, RIAECS_LOG_LOC);

    if (entity.index_ >= nextEntityIndex_)
        riaecs::NotifyError({"Entity index out of range"}, RIAECS_LOG_LOC);

    freeEntityIDs_.push_back(entity);
    
    // Remove all components associated with the entity
    auto it = entityToComponents_.find(entity);
    if (it != entityToComponents_.end())
    {
        for (size_t componentID : it->second)
        {
            // Remove the component from the entity
            componentToEntities_[componentID].erase(entity);

            // Get the component destroyer for the component ID
            riaecs::ReadOnlyObject<IComponentDestroyer> destroyer = componentDestroyerRegistry_->Get(componentID);

            // Free the component data which was allocated for this entity
            std::byte *componentData = entityComponentToData_[{entity, componentID}];
            destroyer().Destroy(componentData);
            componentAllocators_[componentID]->Free(componentData, *componentPools_[componentID]);
            entityComponentToData_.erase({entity, componentID});
        }

        // Remove the entity from the entityToComponents map
        entityToComponents_.erase(it);
    }
}

void riaecs::ECSWorld::AddComponent(const Entity &entity, size_t componentID)
{
    if (!isReady_)
        riaecs::NotifyError({"ECSWorld is not ready"}, RIAECS_LOG_LOC);

    if (entity.index_ >= nextEntityIndex_)
        riaecs::NotifyError({"Entity index out of range"}, RIAECS_LOG_LOC);

    if (componentID >= componentPools_.size())
        riaecs::NotifyError({"Component ID out of range"}, RIAECS_LOG_LOC);

    if (componentPools_[componentID] == nullptr || componentAllocators_[componentID] == nullptr)
        riaecs::NotifyError({"Component pool or allocator not initialized for component ID"}, RIAECS_LOG_LOC);

    // Get the component factory for the component ID
    riaecs::ReadOnlyObject<riaecs::IComponentFactory> factory = componentFactoryRegistry_->Get(componentID);

    // Allocate memory for the component using the allocator
    size_t blockSize = std::max(factory().GetProductSize(), riaecs::MAX_FREE_BLOCK_SIZE);
    std::byte *componentPtr = componentAllocators_[componentID]->Malloc(blockSize, *componentPools_[componentID]);

    if (!componentPtr)
        riaecs::NotifyError({"Failed to allocate memory for component"}, RIAECS_LOG_LOC);

    // Initialize the component using the factory
    factory().Create(componentPtr);

    // Store to the maps
    entityToComponents_[entity].insert(componentID);
    componentToEntities_[componentID].insert(entity);
    entityComponentToData_[{entity, componentID}] = componentPtr;
}

void riaecs::ECSWorld::RemoveComponent(const Entity &entity, size_t componentID)
{
    if (!isReady_)
        riaecs::NotifyError({"ECSWorld is not ready"}, RIAECS_LOG_LOC);

    if (entity.index_ >= nextEntityIndex_)
        riaecs::NotifyError({"Entity index out of range"}, RIAECS_LOG_LOC);

    if (componentID >= componentPools_.size())
        riaecs::NotifyError({"Component ID out of range"}, RIAECS_LOG_LOC);

    // Check if the entity has the component
    auto it = entityToComponents_.find(entity);
    if (it != entityToComponents_.end() && it->second.find(componentID) != it->second.end())
    {
        // Remove the component from the entity
        it->second.erase(componentID);
        componentToEntities_[componentID].erase(entity);

        // Get the component destroyer for the component ID
        riaecs::ReadOnlyObject<IComponentDestroyer> destroyer = componentDestroyerRegistry_->Get(componentID);

        // Free the component data which was allocated for this entity
        std::byte *componentData = entityComponentToData_[{entity, componentID}];
        destroyer().Destroy(componentData);
        componentAllocators_[componentID]->Free(componentData, *componentPools_[componentID]);
        entityComponentToData_.erase({entity, componentID});
    }
}

bool riaecs::ECSWorld::HasComponent(const Entity &entity, size_t componentID) const
{
    if (!isReady_)
        riaecs::NotifyError({"ECSWorld is not ready"}, RIAECS_LOG_LOC);

    if (entity.index_ >= nextEntityIndex_)
        riaecs::NotifyError({"Entity index out of range"}, RIAECS_LOG_LOC);

    if (componentID >= componentPools_.size())
        riaecs::NotifyError({"Component ID out of range"}, RIAECS_LOG_LOC);

    auto it = entityToComponents_.find(entity);
    return it != entityToComponents_.end() && it->second.find(componentID) != it->second.end();
}

void *riaecs::ECSWorld::GetComponent(const Entity &entity, size_t componentID)
{
    if (!isReady_)
        riaecs::NotifyError({"ECSWorld is not ready"}, RIAECS_LOG_LOC);

    if (entity.index_ >= nextEntityIndex_)
        riaecs::NotifyError({"Entity index out of range"}, RIAECS_LOG_LOC);

    if (componentID >= componentPools_.size())
        riaecs::NotifyError({"Component ID out of range"}, RIAECS_LOG_LOC);

    auto it = entityComponentToData_.find({entity, componentID});
    if (it != entityComponentToData_.end())
        return it->second;
    
    return nullptr; // Component not found
}

std::unordered_set<riaecs::Entity> riaecs::ECSWorld::View(size_t componentID) const
{
    if (!isReady_)
        riaecs::NotifyError({"ECSWorld is not ready"}, RIAECS_LOG_LOC);

    if (componentID >= componentPools_.size())
        riaecs::NotifyError({"Component ID out of range"}, RIAECS_LOG_LOC);

    auto it = componentToEntities_.find(componentID);
    if (it != componentToEntities_.end())
        return it->second;

    return {}; // No entities with this component
}