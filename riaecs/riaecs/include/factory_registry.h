#pragma once

#include "riaecs/include/interfaces/registry.h"
#include "riaecs/include/utilities.h"

#include <unordered_map>
#include <shared_mutex>

namespace riaecs
{
    template <typename FACTORY>
    class FactoryRegistry : public IFactoryRegistry<FACTORY>
    {
    private:
        std::vector<std::unique_ptr<FACTORY>> factories_;
        mutable std::shared_mutex mutex_;

    public:
        FactoryRegistry() = default;
        virtual ~FactoryRegistry() = default;

        FactoryRegistry(const FactoryRegistry&) = delete;
        FactoryRegistry& operator=(const FactoryRegistry&) = delete;

        size_t Add(std::unique_ptr<FACTORY> factory) override
        {
            std::unique_lock<std::shared_mutex> lock(mutex_);

            if (!factory)
                NotifyError({"Factory cannot be null"}, __FILE__, __LINE__, __FUNCTION__);

            size_t id = factories_.size();
            factories_.emplace_back(std::move(factory));

            return id;
        }

        ReadOnlyObject<FACTORY> Get(size_t id) override
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);

            if (id >= factories_.size())
                NotifyError({"Factory ID out of range: ", std::to_string(id)}, __FILE__, __LINE__, __FUNCTION__);

            if (!factories_[id])
                NotifyError({"No factory found for ID: ", std::to_string(id)}, __FILE__, __LINE__, __FUNCTION__);

            return ReadOnlyObject<FACTORY>(std::move(lock), *factories_[id]);
        }

        virtual size_t GetCount() const override
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            return factories_.size();
        }
    };

} // namespace riaecs