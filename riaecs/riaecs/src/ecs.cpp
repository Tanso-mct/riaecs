#include "riaecs/src/pch.h"
#include "riaecs/include/ecs.h"

#include "riaecs/include/utilities.h"
#include "riaecs/include/global_registry.h"

size_t riaecs::ECSWorld::nextRegisterIndex_ = 0;

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

void riaecs::ECSWorld::SetComponentMaxCountRegistry(std::unique_ptr<IComponentMaxCountRegistry> registry)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    componentMaxCountRegistry_ = std::move(registry);
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

    if (!componentMaxCountRegistry_)
    {
        riaecs::NotifyError({"ComponentMaxCountRegistry is not set"}, RIAECS_LOG_LOC);
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
        riaecs::ReadOnlyObject<size_t> maxCount = componentMaxCountRegistry_->Get(i);

        size_t blockSize = std::max(factory().GetProductSize(), riaecs::MAX_FREE_BLOCK_SIZE);
        componentPools_[i] = poolFactory_->Create(blockSize * maxCount());
        componentAllocators_[i] = allocatorFactory_->Create(*componentPools_[i], blockSize);
    }
}

void riaecs::ECSWorld::DestroyWorld()
{
    if (!IsReady())
        riaecs::NotifyError({"ECSWorld is not ready"}, RIAECS_LOG_LOC);

    // Destroy all entities
    for (size_t index = 0; index < entityExistFlags_.size(); ++index)
        if (entityExistFlags_[index])
            DestroyEntity(entities_[index]);

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

        entityExistFlags_[entity.GetIndex()] = true;
        entities_[entity.GetIndex()] = Entity(entity.GetIndex(), entity.GetGeneration() + 1);

        return entities_[entity.GetIndex()];
    }
    else
    {
        size_t index = entityExistFlags_.size();
        entityExistFlags_.push_back(true);
        entities_.push_back(Entity(index, riaecs::ID_DEFAULT_GENERATION));

        return entities_.back();
    }
}

void riaecs::ECSWorld::DestroyEntity(const Entity &entity)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

    if (!isReady_)
        riaecs::NotifyError({"ECSWorld is not ready"}, RIAECS_LOG_LOC);

    if (entity.GetIndex() >= entityExistFlags_.size())
        riaecs::NotifyError({"Entity index out of range"}, RIAECS_LOG_LOC);

    if (entities_[entity.GetIndex()].GetGeneration() != entity.GetGeneration())
        riaecs::NotifyError({"Entity generation mismatch"}, RIAECS_LOG_LOC);

    if (!entityExistFlags_[entity.GetIndex()])
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
    entityExistFlags_[entity.GetIndex()] = false;
}

size_t riaecs::ECSWorld::CreateRegisterIndex()
{
    return nextRegisterIndex_++;
}

void riaecs::ECSWorld::RegisterEntity(size_t index, const Entity &entity)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

    if (!isReady_)
        riaecs::NotifyError({"ECSWorld is not ready"}, RIAECS_LOG_LOC);

    if (registeredEntities_.find(index) != registeredEntities_.end())
        riaecs::NotifyError({"Entity already registered at index: " + std::to_string(index)}, RIAECS_LOG_LOC);

    registeredEntities_[index] = entity;
}

riaecs::Entity riaecs::ECSWorld::GetRegisteredEntity(size_t index) const
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    if (!isReady_)
        riaecs::NotifyError({"ECSWorld is not ready"}, RIAECS_LOG_LOC);

    auto it = registeredEntities_.find(index);
    if (it == registeredEntities_.end())
        riaecs::NotifyError({"No entity registered at index: " + std::to_string(index)}, RIAECS_LOG_LOC);

    return it->second;
}

void riaecs::ECSWorld::AddComponent(const Entity &entity, size_t componentID)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

    if (!isReady_)
        riaecs::NotifyError({"ECSWorld is not ready"}, RIAECS_LOG_LOC);

    if (entity.GetIndex() >= entityExistFlags_.size())
        riaecs::NotifyError({"Entity index out of range"}, RIAECS_LOG_LOC);

    if (!entityExistFlags_[entity.GetIndex()])
        riaecs::NotifyError({"Entity does not exist"}, RIAECS_LOG_LOC);

    if (entities_[entity.GetIndex()].GetGeneration() != entity.GetGeneration())
        riaecs::NotifyError({"Entity generation mismatch"}, RIAECS_LOG_LOC);

    if (componentID >= componentPools_.size())
        riaecs::NotifyError({"Component ID out of range"}, RIAECS_LOG_LOC);

    if (componentPools_[componentID] == nullptr || componentAllocators_[componentID] == nullptr)
        riaecs::NotifyError({"Component pool or allocator not initialized for component ID"}, RIAECS_LOG_LOC);

    if (entityToComponents_[entity].find(componentID) != entityToComponents_[entity].end())
        riaecs::NotifyError({"Entity already has this component"}, RIAECS_LOG_LOC);

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

void riaecs::ECSWorld::RemoveComponent(const Entity &entity, size_t componentID)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

    if (!isReady_)
        riaecs::NotifyError({"ECSWorld is not ready"}, RIAECS_LOG_LOC);

    if (entity.GetIndex() >= entityExistFlags_.size())
        riaecs::NotifyError({"Entity index out of range"}, RIAECS_LOG_LOC);

    if (!entityExistFlags_[entity.GetIndex()])
        riaecs::NotifyError({"Entity does not exist"}, RIAECS_LOG_LOC);

    if (entities_[entity.GetIndex()].GetGeneration() != entity.GetGeneration())
        riaecs::NotifyError({"Entity generation mismatch"}, RIAECS_LOG_LOC);

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

bool riaecs::ECSWorld::HasComponent(const Entity &entity, size_t componentID) const
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    if (!isReady_)
        riaecs::NotifyError({"ECSWorld is not ready"}, RIAECS_LOG_LOC);

    if (entity.GetIndex() >= entityExistFlags_.size())
        riaecs::NotifyError({"Entity index out of range"}, RIAECS_LOG_LOC);

    if (!entityExistFlags_[entity.GetIndex()])
        riaecs::NotifyError({"Entity does not exist"}, RIAECS_LOG_LOC);

    if (entities_[entity.GetIndex()].GetGeneration() != entity.GetGeneration())
        riaecs::NotifyError({"Entity generation mismatch"}, RIAECS_LOG_LOC);

    if (componentID >= componentPools_.size())
        riaecs::NotifyError({"Component ID out of range"}, RIAECS_LOG_LOC);

    auto it = entityToComponents_.find(entity);
    return it != entityToComponents_.end() && it->second.find(componentID) != it->second.end();
}

riaecs::ReadOnlyObject<std::byte*> riaecs::ECSWorld::GetComponent(const Entity &entity, size_t componentID)
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    if (!isReady_)
        riaecs::NotifyError({"ECSWorld is not ready"}, RIAECS_LOG_LOC);

    if (entity.GetIndex() >= entityExistFlags_.size())
        riaecs::NotifyError({"Entity index out of range"}, RIAECS_LOG_LOC);

    if (!entityExistFlags_[entity.GetIndex()])
        riaecs::NotifyError({"Entity does not exist"}, RIAECS_LOG_LOC);

    if (entities_[entity.GetIndex()].GetGeneration() != entity.GetGeneration())
        riaecs::NotifyError({"Entity generation mismatch"}, RIAECS_LOG_LOC);

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

void riaecs::SystemList::Add(size_t systemID)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    riaecs::ReadOnlyObject<riaecs::ISystemFactory> factory = riaecs::gSystemFactoryRegistry->Get(systemID);
    std::unique_ptr<riaecs::ISystem> system = factory().Create();
    systems_.emplace_back(std::move(system));
}

riaecs::ISystem &riaecs::SystemList::Get(size_t index)
{
    std::shared_lock<std::shared_mutex> lock(mutex_);

    if (index >= systems_.size())
        riaecs::NotifyError({"System index out of range"}, RIAECS_LOG_LOC);

    if (!systems_[index])
        riaecs::NotifyError({"No system found at index: " + std::to_string(index)}, RIAECS_LOG_LOC);

    return *systems_[index];
}

size_t riaecs::SystemList::GetCount() const
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return systems_.size();
}

void riaecs::SystemList::Clear()
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    systems_.clear();
}

std::unique_ptr<riaecs::ISystemList> riaecs::SystemListFactory::Create() const
{
    return std::make_unique<riaecs::SystemList>();
}

void riaecs::SystemListFactory::Destroy(std::unique_ptr<ISystemList> product) const
{
    product.reset();
}

size_t riaecs::SystemListFactory::GetProductSize() const
{
    return sizeof(SystemList);
}

void riaecs::SystemLoopCommandQueue::Enqueue(std::unique_ptr<ISystemLoopCommand> cmd)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    commandQueue_.emplace(std::move(cmd));
}

std::unique_ptr<riaecs::ISystemLoopCommand> riaecs::SystemLoopCommandQueue::Dequeue()
{
    std::unique_lock<std::shared_mutex> lock(mutex_);

    if (commandQueue_.empty())
        return nullptr;

    std::unique_ptr<ISystemLoopCommand> cmd = std::move(commandQueue_.front());
    commandQueue_.pop();
    return cmd;
}

bool riaecs::SystemLoopCommandQueue::IsEmpty() const
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return commandQueue_.empty();
}

std::unique_ptr<riaecs::ISystemLoopCommandQueue> riaecs::SystemLoopCommandQueueFactory::Create() const
{
    return std::make_unique<SystemLoopCommandQueue>();
}

void riaecs::SystemLoopCommandQueueFactory::Destroy(std::unique_ptr<ISystemLoopCommandQueue> product) const
{
    product.reset();
}

size_t riaecs::SystemLoopCommandQueueFactory::GetProductSize() const
{
    return sizeof(SystemLoopCommandQueue);
}

void riaecs::SystemLoop::SetSystemListFactory(std::unique_ptr<ISystemListFactory> factory)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    listFactory_ = std::move(factory);
}

void riaecs::SystemLoop::SetSystemLoopCommandQueueFactory(std::unique_ptr<ISystemLoopCommandQueueFactory> factory)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    loopCommandQueueFactory_ = std::move(factory);
}

bool riaecs::SystemLoop::IsReady() const
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    if (!listFactory_)
    {
        riaecs::NotifyError({"SystemListFactory is not set"}, RIAECS_LOG_LOC);
        isReady_ = false;
        return isReady_;
    }

    if (!loopCommandQueueFactory_)
    {
        riaecs::NotifyError({"SystemLoopCommandQueueFactory is not set"}, RIAECS_LOG_LOC);
        isReady_ = false;
        return isReady_;
    }

    isReady_ = true;
    return isReady_;
}

void riaecs::SystemLoop::Initialize()
{
    if (!IsReady())
        riaecs::NotifyError({"SystemLoop is not ready"}, RIAECS_LOG_LOC);

    std::unique_lock<std::shared_mutex> lock(mutex_);

    systemList_ = listFactory_->Create();
    commandQueue_ = loopCommandQueueFactory_->Create();

    if (!systemList_)
        riaecs::NotifyError({"Failed to create System List"}, RIAECS_LOG_LOC);

    if (!commandQueue_)
        riaecs::NotifyError({"Failed to create System Loop Command Queue"}, RIAECS_LOG_LOC);
}

void riaecs::SystemLoop::Run(IECSWorld &world, IAssetContainer &assetCont)
{
    if (!isReady_)
        riaecs::NotifyError({"SystemLoop is not ready"}, RIAECS_LOG_LOC);

    std::unique_lock<std::shared_mutex> lock(mutex_);

    while (true)
    {
        // Process commands in the command queue
        while (!commandQueue_->IsEmpty())
        {
            std::unique_ptr<ISystemLoopCommand> cmd = commandQueue_->Dequeue();
            if (cmd)
                cmd->Execute(*systemList_, world, assetCont);
            else
                riaecs::NotifyError({"Invalid command in System Loop Command Queue"}, RIAECS_LOG_LOC);
        }

        if (systemList_->GetCount() == 0)
            break; // Exit the loop if no systems are available

        // Update systems
        bool continueLoop = true;
        for (size_t i = 0; i < systemList_->GetCount(); ++i)
        {
            ISystem &system = systemList_->Get(i);
            continueLoop = system.Update(world, assetCont, *commandQueue_);
            if (!continueLoop)
                break; // Stop the system update if any system returns false
        }

        if (!continueLoop)
            break; // Stop the system loop if any system returns false
    }
}