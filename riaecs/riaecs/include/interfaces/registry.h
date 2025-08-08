#pragma once

#include "riaecs/include/types/object.h"

#include <memory>

namespace riaecs
{
    template <typename T>
    class IRegistry
    {
    public:
        virtual ~IRegistry() = default;

        virtual size_t Add(std::unique_ptr<T> entry) = 0;
        virtual ReadOnlyObject<T> Get(size_t id) const = 0;

        virtual size_t GetCount() const = 0;
    };

} // namespace riaecs