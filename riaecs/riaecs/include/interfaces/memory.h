#pragma once

namespace riaecs
{
    class IPool
    {
    public:
        virtual ~IPool() = default;

        virtual std::byte *GetPool() = 0;
        virtual size_t GetSize() const = 0;
    };

    class IAllocator
    {
    public:
        virtual ~IAllocator() = default;

        virtual std::byte *Malloc(size_t size, IPool &pool) = 0;
        virtual void Free(std::byte *ptr, IPool &pool) = 0;
    };

} // namespace riaecs