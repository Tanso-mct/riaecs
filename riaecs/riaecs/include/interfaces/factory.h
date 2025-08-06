#pragma once

namespace riaecs
{
    template <typename PRODUCT, typename... ARGS>
    class IFactory
    {
    public:
        virtual ~IFactory() = default;
        virtual PRODUCT Create(ARGS...) = 0;a
    };

} // namespace riaecs