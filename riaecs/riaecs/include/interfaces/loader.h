#pragma once

namespace riaecs
{
    template <typename DATA, typename... ARGS>
    class ILoader
    {
    public:
        virtual ~ILoader() = default;
        virtual DATA Load(ARGS...) = 0;
    };

} // namespace riaecs