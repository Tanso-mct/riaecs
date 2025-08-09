#include "mem_alloc_fixed_block_test/pch.h"

#include "mem_alloc_fixed_block/mem_alloc_fixed_block.h"
#pragma comment(lib, "mem_alloc_fixed_block.lib")

TEST(FixedBlockAllocator, AllocateAndFree)
{
    struct Data
    {
        int value;

        Data() : value(0) {}
        ~Data() { value = 0; }
    };

    const size_t MAX_COUNT = 1000;
    const size_t BLOCK_SIZE = std::max(sizeof(Data), riaecs::MAX_FREE_BLOCK_SIZE);
    const size_t POOL_SIZE = MAX_COUNT * BLOCK_SIZE;

    // Get the factory for creating pools and allocators
    std::unique_ptr<riaecs::IPoolFactory> poolFactory 
    = std::make_unique<mem_alloc_fixed_block::FixedBlockPoolFactory>();

    std::unique_ptr<riaecs::IAllocatorFactory> allocatorFactory 
    = std::make_unique<mem_alloc_fixed_block::FixedBlockAllocatorFactory>();

    std::unique_ptr<riaecs::IPool> pool = poolFactory->Create(POOL_SIZE);
    std::unique_ptr<riaecs::IAllocator> allocator = allocatorFactory->Create(*pool, BLOCK_SIZE);

    Data *data1 = nullptr;
    const int DATA1_EXPECTED_VALUE = 7;
    {
        std::byte *mem = allocator->Malloc(BLOCK_SIZE, *pool);
        EXPECT_NE(mem, nullptr);
        data1 = reinterpret_cast<Data*>(mem);
        data1->value = DATA1_EXPECTED_VALUE;
    }

    Data *data2 = nullptr;
    const int DATA2_EXPECTED_VALUE = 10;
    {
        std::byte *mem = allocator->Malloc(BLOCK_SIZE, *pool);
        EXPECT_NE(mem, nullptr);
        data2 = reinterpret_cast<Data*>(mem);
        data2->value = DATA2_EXPECTED_VALUE;
    }

    // Verify data in allocated blocks
    EXPECT_EQ(data1->value, DATA1_EXPECTED_VALUE);
    EXPECT_EQ(data2->value, DATA2_EXPECTED_VALUE);

    data1->~Data(); // Call destructor explicitly
    allocator->Free(reinterpret_cast<std::byte*>(data1), *pool);
    data1 = nullptr; // Reset pointer to avoid dangling pointer

    data2->~Data(); // Call destructor explicitly
    allocator->Free(reinterpret_cast<std::byte*>(data2), *pool);
    data2 = nullptr; // Reset pointer to avoid dangling pointer

    // Try to allocate again after freeing
    Data *data3 = nullptr;
    const int DATA3_EXPECTED_VALUE = 100;
    {
        std::byte *mem = allocator->Malloc(BLOCK_SIZE, *pool);
        EXPECT_NE(mem, nullptr);
        data3 = reinterpret_cast<Data*>(mem);
        data3->value = DATA3_EXPECTED_VALUE;
    }

    // Verify data in newly allocated block
    EXPECT_EQ(data3->value, DATA3_EXPECTED_VALUE);
}