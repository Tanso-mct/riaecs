#pragma once

#include "riaecs/include/interfaces/container.h"
#include "riaecs/include/utilities.h"
#include "riaecs/include/id.h"

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
            {
                NotifyError
                (
                    {"Container already created with count: ", std::to_string(objects_.size())},
                    __FILE__, __LINE__, __FUNCTION__
                );
            }

            objects_.resize(count);
            generations_.resize(count, CONTAINER_DEFAULT_GENERATION);
        }

        std::unique_ptr<T> Release(const IID &id) override
        {
            std::unique_lock<std::shared_mutex> lock(mutex_);

            if (id.Get() >= objects_.size())
            {
                NotifyError
                (
                    {"ID out of range: ", std::to_string(id.Get())},
                    __FILE__, __LINE__, __FUNCTION__
                );
            }

            if (!objects_[id.Get()])
            {
                NotifyError
                (
                    {"No object found for ID: ", std::to_string(id.Get())},
                    __FILE__, __LINE__, __FUNCTION__
                );
            }

            if (id.GetGeneration() != generations_[id.Get()])
            {
                NotifyError
                (
                    {"ID generation mismatch for ID: ", std::to_string(id.Get())},
                    __FILE__, __LINE__, __FUNCTION__
                );
            }

            // Update the generation for the ID
            generations_[id.Get()]++;

            return std::move(objects_[id.Get()]);
        }

        std::unique_ptr<IID> Add(std::unique_ptr<T> object) override
        {
            std::unique_lock<std::shared_mutex> lock(mutex_);

            size_t id = objects_.size();

            objects_.emplace_back(std::move(object));
            generations_.emplace_back(CONTAINER_DEFAULT_GENERATION);

            return std::make_unique<ID>(id, generations_[id]);
        }

        std::unique_ptr<T> Erase(const IID &id) override
        {
            std::unique_lock<std::shared_mutex> lock(mutex_);

            if (id.Get() >= objects_.size())
            {
                NotifyError
                (
                    {"ID out of range: ", std::to_string(id.Get())},
                    __FILE__, __LINE__, __FUNCTION__
                );
            }

            if (objects_[id.Get()] == nullptr)
            {
                NotifyError
                (
                    {"No object found for ID: ", std::to_string(id.Get())},
                    __FILE__, __LINE__, __FUNCTION__
                );
            }

            if (id.GetGeneration() != generations_[id.Get()])
            {
                NotifyError
                (
                    {"ID generation mismatch for ID: ", std::to_string(id.Get())},
                    __FILE__, __LINE__, __FUNCTION__
                );
            }

            // Move the will be erased object to return it
            std::unique_ptr<T> object = std::move(objects_[id.Get()]);

            // Reset the object
            objects_[id.Get()] = nullptr;

            // Update the generation for the ID
            generations_[id.Get()]++;

            return object;
        }

        ReadOnlyObject<T> Get(const IID &id) const override
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);

            if (id.Get() >= objects_.size())
            {
                NotifyError
                (
                    {"ID out of range: ", std::to_string(id.Get())},
                    __FILE__, __LINE__, __FUNCTION__
                );
            }

            if (objects_[id.Get()] == nullptr)
            {
                NotifyError
                (
                    {"No object found for ID: ", std::to_string(id.Get())},
                    __FILE__, __LINE__, __FUNCTION__
                );
            }

            if (id.GetGeneration() != generations_[id.Get()])
            {
                NotifyError
                (
                    {"ID generation mismatch for ID: ", std::to_string(id.Get())},
                    __FILE__, __LINE__, __FUNCTION__
                );
            }

            return ReadOnlyObject<T>(std::move(lock), *objects_[id.Get()]);
        }

        void Set(const IID &id, std::unique_ptr<T> object) override
        {
            std::unique_lock<std::shared_mutex> lock(mutex_);

            if (id.Get() >= objects_.size())
            {
                NotifyError
                (
                    {"ID out of range: ", std::to_string(id.Get())},
                    __FILE__, __LINE__, __FUNCTION__
                );
            }

            if (id.GetGeneration() != generations_[id.Get()])
            {
                NotifyError
                (
                    {"ID generation mismatch for ID: ", std::to_string(id.Get())},
                    __FILE__, __LINE__, __FUNCTION__
                );
            }

            objects_[id.Get()] = std::move(object);
        }

        size_t GetGeneration(size_t index) const override
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);

            if (index >= generations_.size())
            {
                NotifyError
                (
                    {"Index out of range: ", std::to_string(index)},
                    __FILE__, __LINE__, __FUNCTION__
                );
            }

            return generations_[index];
        }

        bool Contains(const IID &id) const override
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);

            if (id.Get() >= objects_.size())
            {
                NotifyError
                (
                    {"ID out of range: ", std::to_string(id.Get())},
                    __FILE__, __LINE__, __FUNCTION__
                );
            }

            return objects_[id.Get()] != nullptr && id.GetGeneration() == generations_[id.Get()];
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