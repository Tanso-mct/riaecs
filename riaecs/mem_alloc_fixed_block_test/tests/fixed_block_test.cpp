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
    const size_t BLOCK_SIZE = mem_alloc_fixed_block::EnsureMinBlockSize(sizeof(Data));
    const size_t POOL_SIZE = MAX_COUNT * sizeof(Data);

    mem_alloc_fixed_block::FixedBlockPool<POOL_SIZE> pool;
    mem_alloc_fixed_block::FixedBlockAllocator allocator(pool, BLOCK_SIZE);

    Data *data1 = nullptr;
    const int DATA1_EXPECTED_VALUE = 7;
    {
        std::byte *mem = allocator.Malloc(BLOCK_SIZE, pool);
        EXPECT_NE(mem, nullptr);
        data1 = reinterpret_cast<Data*>(mem);
        data1->value = DATA1_EXPECTED_VALUE;
    }

    Data *data2 = nullptr;
    const int DATA2_EXPECTED_VALUE = 10;
    {
        std::byte *mem = allocator.Malloc(BLOCK_SIZE, pool);
        EXPECT_NE(mem, nullptr);
        data2 = reinterpret_cast<Data*>(mem);
        data2->value = DATA2_EXPECTED_VALUE;
    }

    // Verify data in allocated blocks
    EXPECT_EQ(data1->value, DATA1_EXPECTED_VALUE);
    EXPECT_EQ(data2->value, DATA2_EXPECTED_VALUE);

    mem_alloc_fixed_block::FreeObject(data1, allocator, pool);
    EXPECT_EQ(data1, nullptr); // Ensure data1 is reset to nullptr after freeing

    mem_alloc_fixed_block::FreeObject(data2, allocator, pool);
    EXPECT_EQ(data2, nullptr); // Ensure data2 is reset to nullptr after freeing

    // Try to allocate again after freeing
    Data *data3 = nullptr;
    const int DATA3_EXPECTED_VALUE = 100;
    {
        std::byte *mem = allocator.Malloc(BLOCK_SIZE, pool);
        EXPECT_NE(mem, nullptr);
        data3 = reinterpret_cast<Data*>(mem);
        data3->value = DATA3_EXPECTED_VALUE;
    }

    // Verify data in newly allocated block
    EXPECT_EQ(data3->value, DATA3_EXPECTED_VALUE);
}