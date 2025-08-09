#include "riaecs/src/pch.h"
#include "riaecs/include/ecs.h"

#include "riaecs/include/utilities.h"

void riaecs::ECSWorld::SetComponentFactoryRegistry(std::unique_ptr<IComponentFactoryRegistry> registry)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    componentFactoryRegistry_ = std::move(registry);
}

void riaecs::ECSWorld::SetPoolFactory(std::unique_ptr<IPoolFactory> poolFactory)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    poolFactory_ = std::move(poolFactory);
}

void riaecs::ECSWorld::SetAllocatorFactory(std::unique_ptr<IAllocatorFactory> allocatorFactory)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    allocatorFactory_ = std::move(allocatorFactory);
}

bool riaecs::ECSWorld::IsReady() const
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    if (!componentFactoryRegistry_)
    {
        riaecs::NotifyError({"ComponentFactoryRegistry is not set"}, RIAECS_LOG_LOC);
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

    std::unique_lock<std::shared_mutex> lock(mutex_);

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

    // Destroy all entities
    for (Entity entity = 0; entity < entityExistFlags_.size(); ++entity)
        DestroyEntity(entity);

    std::unique_lock<std::shared_mutex> lock(mutex_);

    // Clear all component data
    entityComponentToData_.clear();
    entityToComponents_.clear();
    componentToEntities_.clear();

    // Clear pools and allocators
    componentPools_.clear();
    componentAllocators_.clear();

    // Reset entity management
    entityExistFlags_.clear();
    freeEntities_.clear();

    // Reset ready state
    isReady_ = false;
}

riaecs::Entity riaecs::ECSWorld::CreateEntity()
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

    if (!isReady_)
        riaecs::NotifyError({"ECSWorld is not ready"}, RIAECS_LOG_LOC);

    if (!freeEntities_.empty())
    {
        Entity entity = freeEntities_.back();
        freeEntities_.pop_back();

        entityExistFlags_[entity] = true;
        return entity;
    }
    else
    {
        Entity entity = entityExistFlags_.size();
        entityExistFlags_.push_back(true);

        return entity;
    }
}

void riaecs::ECSWorld::DestroyEntity(Entity entity)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

    if (!isReady_)
        riaecs::NotifyError({"ECSWorld is not ready"}, RIAECS_LOG_LOC);

    if (entity >= entityExistFlags_.size())
        riaecs::NotifyError({"Entity index out of range"}, RIAECS_LOG_LOC);

    if (!entityExistFlags_[entity])
        return; // Already destroyed this entity
    
    // Remove all components associated with the entity
    auto it = entityToComponents_.find(entity);
    if (it != entityToComponents_.end())
    {
        for (size_t componentID : it->second)
        {
            // Remove the component from the entity
            componentToEntities_[componentID].erase(entity);

            // Get the component factory for the component ID
            riaecs::ReadOnlyObject<riaecs::IComponentFactory> factory = componentFactoryRegistry_->Get(componentID);

            // Free the component data which was allocated for this entity
            std::byte *componentData = entityComponentToData_[{entity, componentID}];
            factory().Destroy(componentData);
            componentAllocators_[componentID]->Free(componentData, *componentPools_[componentID]);
            entityComponentToData_.erase({entity, componentID});
        }

        // Remove the entity from the entityToComponents map
        entityToComponents_.erase(it);
    }

    // Store the entity in freeEntities for reuse
    freeEntities_.push_back(entity);

    // Update the entityExistFlags to mark it as not existing
    entityExistFlags_[entity] = false;
}

void riaecs::ECSWorld::AddComponent(Entity entity, size_t componentID)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

    if (!isReady_)
        riaecs::NotifyError({"ECSWorld is not ready"}, RIAECS_LOG_LOC);

    if (entity >= entityExistFlags_.size())
        riaecs::NotifyError({"Entity index out of range"}, RIAECS_LOG_LOC);

    if (!entityExistFlags_[entity])
        riaecs::NotifyError({"Entity does not exist"}, RIAECS_LOG_LOC);

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
    componentPtr = factory().Create(componentPtr);

    // Store to the maps
    entityToComponents_[entity].insert(componentID);
    componentToEntities_[componentID].insert(entity);
    entityComponentToData_[{entity, componentID}] = componentPtr;
}

void riaecs::ECSWorld::RemoveComponent(Entity entity, size_t componentID)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

    if (!isReady_)
        riaecs::NotifyError({"ECSWorld is not ready"}, RIAECS_LOG_LOC);

    if (entity >= entityExistFlags_.size())
        riaecs::NotifyError({"Entity index out of range"}, RIAECS_LOG_LOC);

    if (!entityExistFlags_[entity])
        riaecs::NotifyError({"Entity does not exist"}, RIAECS_LOG_LOC);

    if (componentID >= componentPools_.size())
        riaecs::NotifyError({"Component ID out of range"}, RIAECS_LOG_LOC);

    // Check if the entity has the component
    auto it = entityToComponents_.find(entity);
    if (it != entityToComponents_.end() && it->second.find(componentID) != it->second.end())
    {
        // Remove the component from the entity
        it->second.erase(componentID);
        componentToEntities_[componentID].erase(entity);

        // Get the component factory for the component ID
        riaecs::ReadOnlyObject<riaecs::IComponentFactory> factory = componentFactoryRegistry_->Get(componentID);

        // Free the component data which was allocated for this entity
        std::byte *componentData = entityComponentToData_[{entity, componentID}];
        factory().Destroy(componentData);
        componentAllocators_[componentID]->Free(componentData, *componentPools_[componentID]);
        entityComponentToData_.erase({entity, componentID});
    }
}

bool riaecs::ECSWorld::HasComponent(Entity entity, size_t componentID) const
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    if (!isReady_)
        riaecs::NotifyError({"ECSWorld is not ready"}, RIAECS_LOG_LOC);

    if (entity >= entityExistFlags_.size())
        riaecs::NotifyError({"Entity index out of range"}, RIAECS_LOG_LOC);

    if (!entityExistFlags_[entity])
        riaecs::NotifyError({"Entity does not exist"}, RIAECS_LOG_LOC);

    if (componentID >= componentPools_.size())
        riaecs::NotifyError({"Component ID out of range"}, RIAECS_LOG_LOC);

    auto it = entityToComponents_.find(entity);
    return it != entityToComponents_.end() && it->second.find(componentID) != it->second.end();
}

riaecs::ReadOnlyObject<std::byte*> riaecs::ECSWorld::GetComponent(Entity entity, size_t componentID)
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    if (!isReady_)
        riaecs::NotifyError({"ECSWorld is not ready"}, RIAECS_LOG_LOC);

    if (entity >= entityExistFlags_.size())
        riaecs::NotifyError({"Entity index out of range"}, RIAECS_LOG_LOC);

    if (!entityExistFlags_[entity])
        riaecs::NotifyError({"Entity does not exist"}, RIAECS_LOG_LOC);

    if (componentID >= componentPools_.size())
        riaecs::NotifyError({"Component ID out of range"}, RIAECS_LOG_LOC);

    auto it = entityComponentToData_.find({entity, componentID});
    if (it != entityComponentToData_.end())
        return riaecs::ReadOnlyObject<std::byte*>(std::move(lock), it->second);
    
    return riaecs::ReadOnlyObject<std::byte*>(std::move(lock), nullptr);
}

riaecs::ReadOnlyObject<std::unordered_set<riaecs::Entity>> riaecs::ECSWorld::View(size_t componentID) const
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    if (!isReady_)
        riaecs::NotifyError({"ECSWorld is not ready"}, RIAECS_LOG_LOC);

    if (componentID >= componentPools_.size())
        riaecs::NotifyError({"Component ID out of range"}, RIAECS_LOG_LOC);

    auto it = componentToEntities_.find(componentID);
    if (it != componentToEntities_.end())
        return riaecs::ReadOnlyObject<std::unordered_set<riaecs::Entity>>(std::move(lock), it->second);

    return riaecs::ReadOnlyObject<std::unordered_set<riaecs::Entity>>(std::move(lock), emptyEntities_);
}