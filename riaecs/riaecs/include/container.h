#pragma once

#include "riaecs/include/interfaces/container.h"
#include "riaecs/include/types/id.h"
#include "riaecs/include/utilities.h"

#include <stdexcept>
#include <shared_mutex>

namespace riaecs
{
    template <typename T>
    class Container : public IContainer<T>
    {
    private:
        std::vector<std::unique_ptr<T>> objects_;
        std::vector<size_t> generations_;
        std::vector<size_t> freeIndices_;

        mutable std::shared_mutex mutex_;

    public:
        Container() = default;
        virtual ~Container() = default;

        Container(const Container&) = delete;
        Container& operator=(const Container&) = delete;

        /***************************************************************************************************************
         * IContainer Implementation
        /**************************************************************************************************************/

        void Create(size_t count) override
        {
            std::unique_lock<std::shared_mutex> lock(mutex_);

            if (objects_.size() != 0)
                NotifyError({"Container already created with count: " + std::to_string(objects_.size())}, RIAECS_LOG_LOC);

            objects_.resize(count);
            generations_.resize(count, CONTAINER_DEFAULT_GENERATION);
        }

        std::unique_ptr<T> Release(const ID &id) override
        {
            std::unique_lock<std::shared_mutex> lock(mutex_);

            if (id.index_ >= objects_.size())
                NotifyError({"ID out of range: " + std::to_string(id.index_)}, RIAECS_LOG_LOC);

            if (!objects_[id.index_])
                NotifyError({"No object found for ID: " + std::to_string(id.index_)}, RIAECS_LOG_LOC);

            if (id.generation_ != generations_[id.index_])
                NotifyError({"ID generation mismatch for ID: " + std::to_string(id.index_)}, RIAECS_LOG_LOC);

            // Update the generation for the ID
            generations_[id.index_]++;

            return std::move(objects_[id.index_]);
        }

        ID Add(std::unique_ptr<T> object) override
        {
            std::unique_lock<std::shared_mutex> lock(mutex_);

            if (freeIndices_.empty())
            {
                size_t id = objects_.size();

                objects_.emplace_back(std::move(object));
                generations_.emplace_back(CONTAINER_DEFAULT_GENERATION);

                return ID(id, generations_[id]);
            }
            else
            {
                size_t id = freeIndices_.back();
                freeIndices_.pop_back();

                objects_[id] = std::move(object);
                generations_[id] = CONTAINER_DEFAULT_GENERATION;

                return ID(id, generations_[id]);
            }

        }

        std::unique_ptr<T> Erase(const ID &id) override
        {
            std::unique_lock<std::shared_mutex> lock(mutex_);

            if (id.index_ >= objects_.size())
                NotifyError({"ID out of range: " + std::to_string(id.index_)}, RIAECS_LOG_LOC);

            if (objects_[id.index_] == nullptr)
                NotifyError({"No object found for ID: " + std::to_string(id.index_)}, RIAECS_LOG_LOC);

            if (id.generation_ != generations_[id.index_])
                NotifyError({"ID generation mismatch for ID: " + std::to_string(id.index_)}, RIAECS_LOG_LOC);

            // Move the will be erased object to return it
            std::unique_ptr<T> object = std::move(objects_[id.index_]);

            // Reset the object
            objects_[id.index_] = nullptr;

            // Update the generation for the ID
            generations_[id.index_]++;

            // Store the index in freeIndices_ for potential reuse
            freeIndices_.push_back(id.index_);

            return object;
        }

        ReadOnlyObject<T> Get(const ID &id) const override
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);

            if (id.index_ >= objects_.size())
                NotifyError({"ID out of range: " + std::to_string(id.index_)}, RIAECS_LOG_LOC);

            if (objects_[id.index_] == nullptr)
                NotifyError({"No object found for ID: " + std::to_string(id.index_)}, RIAECS_LOG_LOC);

            if (id.generation_ != generations_[id.index_])
                NotifyError({"ID generation mismatch for ID: " + std::to_string(id.index_)}, RIAECS_LOG_LOC);

            return ReadOnlyObject<T>(std::move(lock), *objects_[id.index_]);
        }

        void Set(const ID &id, std::unique_ptr<T> object) override
        {
            std::unique_lock<std::shared_mutex> lock(mutex_);

            if (id.index_ >= objects_.size())
                NotifyError({"ID out of range: " + std::to_string(id.index_)}, RIAECS_LOG_LOC);

            if (id.generation_ != generations_[id.index_])
                NotifyError({"ID generation mismatch for ID: " + std::to_string(id.index_)}, RIAECS_LOG_LOC);

            objects_[id.index_] = std::move(object);
        }

        size_t GetGeneration(size_t index) const override
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);

            if (index >= generations_.size())
                NotifyError({"Index out of range: " + std::to_string(index)}, RIAECS_LOG_LOC);

            return generations_[index];
        }

        bool Contains(const ID &id) const override
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);

            if (id.index_ >= objects_.size())
                NotifyError({"ID out of range: " + std::to_string(id.index_)}, RIAECS_LOG_LOC);

            return objects_[id.index_] != nullptr && id.generation_ == generations_[id.index_];
        }

        size_t GetCount() const override
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            return objects_.size();
        }

        void Clear() override
        {
            std::unique_lock<std::shared_mutex> lock(mutex_);
            objects_.clear();
            generations_.clear();
        }

    };

} // namespace riaecs