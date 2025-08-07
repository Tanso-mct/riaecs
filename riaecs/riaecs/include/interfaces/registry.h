#pragma once

#include "riaecs/include/interfaces/thread.h"

#include <memory>

namespace riaecs
{
    template <typename FACTORY>
    class IFactoryRegistry
    {
    public:
        virtual ~IFactoryRegistry() = default;

        virtual size_t Add(std::unique_ptr<FACTORY> factory) = 0;
        virtual ReadOnlyObject<FACTORY> Get(size_t id) = 0;

        virtual size_t GetCount() const = 0;
    };

} // namespace riaecs