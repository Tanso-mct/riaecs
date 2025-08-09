#pragma once
#include "mem_alloc_fixed_block/include/dll_config.h"

#include "riaecs/riaecs.h"

namespace mem_alloc_fixed_block
{
    class MEM_ALLOC_FIXED_BLOCK_API FixedBlockAllocator : public riaecs::IAllocator
    {
    private:
        struct FreeBlock
        {
            FreeBlock *next;
        };
    
        FreeBlock *freeList_ = nullptr;
        const size_t BLOCK_SIZE_;

    public:
        FixedBlockAllocator(riaecs::IPool &pool, size_t blockSize);
        ~FixedBlockAllocator() override;

        FixedBlockAllocator(const FixedBlockAllocator&) = delete;
        FixedBlockAllocator& operator=(const FixedBlockAllocator&) = delete;

        /***************************************************************************************************************
         * IAllocator Implementation
        /**************************************************************************************************************/

        std::byte *Malloc(size_t size, riaecs::IPool &pool) override;
        void Free(std::byte *ptr, riaecs::IPool &pool) override;
    };

    class MEM_ALLOC_FIXED_BLOCK_API FixedBlockAllocatorFactory : public riaecs::IAllocatorFactory
    {
    public:
        FixedBlockAllocatorFactory() = default;
        ~FixedBlockAllocatorFactory() override = default;

        /***************************************************************************************************************
         * IAllocatorFactory Implementation
        /**************************************************************************************************************/

        std::unique_ptr<riaecs::IAllocator> Create(riaecs::IPool &pool, size_t blockSize) const override;
        size_t GetProductSize() const override { return sizeof(FixedBlockAllocator); }
    };

} // namespace mem_alloc_fixed_block