#pragma once

#include <memory>

namespace riaecs
{
    template <typename FACTORY>
    class IFactoryRegistry
    {
    public:
        virtual ~IFactoryRegistry() = default;

        virtual void Add(size_t id, std::unique_ptr<FACTORY> factory) = 0;
        virtual const FACTORY &GetFactory(size_t id) = 0;

        virtual size_t GetCount() const = 0;
    };

} // namespace riaecs