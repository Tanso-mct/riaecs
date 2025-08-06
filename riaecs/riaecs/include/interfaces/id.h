#pragma once

namespace riaecs
{
    class IID
    {
    public:
        virtual ~IID() = default;

        virtual size_t Get() const = 0;
        virtual size_t GetGeneration() const = 0;
    };

} // namespace riaecs