#include "mem_alloc_fixed_block/src/pch.h"
#include "mem_alloc_fixed_block/include/pool.h"

#include "mem_alloc_fixed_block/include/allocator.h"

mem_alloc_fixed_block::FixedBlockPool::FixedBlockPool(const size_t size)
: SIZE(size)
{
    pool_ = std::make_unique<std::byte[]>(SIZE);
}

std::unique_ptr<riaecs::IPool> mem_alloc_fixed_block::FixedBlockPoolFactory::Create(size_t size) const
{
    return std::make_unique<mem_alloc_fixed_block::FixedBlockPool>(size);
}