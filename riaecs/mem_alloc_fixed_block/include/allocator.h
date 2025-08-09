#pragma once
#include "mem_alloc_fixed_block/include/dll_config.h"

#include "riaecs/riaecs.h"

namespace mem_alloc_fixed_block
{
    struct FixedFreeBlock
    {
        FixedFreeBlock *next;
    };

    class MEM_ALLOC_FIXED_BLOCK_API FixedBlockAllocator : public riaecs::IAllocator
    {
    private:
        FixedFreeBlock *freeList_ = nullptr;
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

    template <typename T>
    void FreeObject(T *&ptr, riaecs::IAllocator &allocator, riaecs::IPool &pool)
    {
        if (ptr != nullptr)
        {
            ptr->~T(); // Call destructor explicitly
            allocator.Free(reinterpret_cast<std::byte*>(ptr), pool);
            ptr = nullptr; // Reset pointer to avoid dangling pointer
        }
    }

    size_t MEM_ALLOC_FIXED_BLOCK_API EnsureMinBlockSize(size_t size);

} // namespace mem_alloc_fixed_block