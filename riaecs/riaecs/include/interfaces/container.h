#pragma once

#include "riaecs/include/types/id.h"
#include "riaecs/include/types/object.h"

#include <memory>

namespace riaecs
{
    constexpr size_t CONTAINER_DEFAULT_GENERATION = 0;

    template <typename T>
    class IContainer
    {
    public:
        virtual ~IContainer() = default;

        virtual void Create(size_t count) = 0;
        virtual std::unique_ptr<T> Release(const ID &id) = 0;

        virtual ID Add(std::unique_ptr<T> object) = 0;
        virtual std::unique_ptr<T> Erase(const ID &id) = 0;

        virtual ReadOnlyObject<T> Get(const ID &id) const = 0;
        virtual void Set(const ID &id, std::unique_ptr<T> object) = 0;

        virtual size_t GetGeneration(size_t index) const = 0;
        virtual bool Contains(const ID &id) const = 0;
        virtual size_t GetCount() const = 0;
        virtual void Clear() = 0;
    };

} // namespace riaecs