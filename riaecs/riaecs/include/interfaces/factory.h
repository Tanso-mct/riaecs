#pragma once

namespace riaecs
{
    template <typename PRODUCT, typename... ARGS>
    class IFactory
    {
    public:
        virtual ~IFactory() = default;
        virtual PRODUCT Create(ARGS...) const = 0;
        virtual size_t GetProductSize() const = 0;
    };

} // namespace riaecs