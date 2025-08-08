#pragma once

#include "riaecs/include/interfaces/registry.h"
#include "riaecs/include/utilities.h"

#include <unordered_map>
#include <shared_mutex>

namespace riaecs
{
    template <typename T>
    class Registry : public IRegistry<T>
    {
    private:
        std::vector<std::unique_ptr<T>> factories_;
        mutable std::shared_mutex mutex_;

    public:
        Registry() = default;
        virtual ~Registry() = default;

        Registry(const Registry&) = delete;
        Registry& operator=(const Registry&) = delete;

        size_t Add(std::unique_ptr<T> entry) override
        {
            std::unique_lock<std::shared_mutex> lock(mutex_);

            if (!entry)
                NotifyError({"Entry cannot be null"}, __FILE__, __LINE__, __FUNCTION__);

            size_t id = factories_.size();
            factories_.emplace_back(std::move(entry));

            return id;
        }

        ReadOnlyObject<T> Get(size_t id) const override
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);

            if (id >= factories_.size())
                NotifyError({"ID out of range: ", std::to_string(id)}, __FILE__, __LINE__, __FUNCTION__);

            if (!factories_[id])
                NotifyError({"No entry found for ID: ", std::to_string(id)}, __FILE__, __LINE__, __FUNCTION__);

            return ReadOnlyObject<T>(std::move(lock), *factories_[id]);
        }

        virtual size_t GetCount() const override
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            return factories_.size();
        }
    };

} // namespace riaecs