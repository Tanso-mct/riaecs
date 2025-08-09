#include "mem_alloc_fixed_block/src/pch.h"
#include "mem_alloc_fixed_block/include/allocator.h"

#pragma comment(lib, "riaecs.lib")

mem_alloc_fixed_block::FixedBlockAllocator::FixedBlockAllocator(riaecs::IPool &pool, size_t blockSize)
: BLOCK_SIZE_(blockSize)
{
    if (BLOCK_SIZE_ == 0)
        riaecs::NotifyError({"Block size must be greater than zero"}, RIAECS_LOG_LOC);

    std::byte *poolStart = pool.GetPool();
    size_t poolSize = pool.GetSize();
    size_t blockCount = poolSize / BLOCK_SIZE_;

    if (blockCount == 0)
        riaecs::NotifyError({"Pool size is too small for the given block size"}, RIAECS_LOG_LOC);

    // Initialize the free list
    for (size_t i = 0; i < blockCount; ++i)
    {
        FixedFreeBlock *block = reinterpret_cast<FixedFreeBlock*>(poolStart + i * BLOCK_SIZE_);
        block->next = freeList_;
        freeList_ = block;
    }
}

mem_alloc_fixed_block::FixedBlockAllocator::~FixedBlockAllocator()
{
    freeList_ = nullptr;
}

std::byte *mem_alloc_fixed_block::FixedBlockAllocator::Malloc(size_t size, riaecs::IPool &pool)
{
    if (size > BLOCK_SIZE_)
        riaecs::NotifyError({"Requested size exceeds block size"}, RIAECS_LOG_LOC);

    if (freeList_ == nullptr)
        riaecs::NotifyError({"No free blocks available"}, RIAECS_LOG_LOC);

    // Allocate a block from the free list
    FixedFreeBlock *block = freeList_;
    freeList_ = block->next;

    return reinterpret_cast<std::byte*>(block);
}

void mem_alloc_fixed_block::FixedBlockAllocator::Free(std::byte *ptr, riaecs::IPool &pool)
{
    if (ptr == nullptr)
        return; // Nothing to free

    // Cast the pointer back to FixedFreeBlock
    FixedFreeBlock *block = reinterpret_cast<FixedFreeBlock*>(ptr);

    // Add the block back to the free list
    block->next = freeList_;
    freeList_ = block;

    // Reset the ptr to nullptr to avoid dangling pointers
    ptr = nullptr;
}

size_t MEM_ALLOC_FIXED_BLOCK_API mem_alloc_fixed_block::EnsureMinBlockSize(size_t size)
{
    if (size == 0)
        riaecs::NotifyError({"Block size must be greater than zero"}, RIAECS_LOG_LOC);

    // Ensure the block size is a multiple of the alignment requirement
    return std::max(size, sizeof(FixedFreeBlock));
}