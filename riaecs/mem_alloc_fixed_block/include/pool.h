#pragma once
#include "mem_alloc_fixed_block/include/dll_config.h"

#include "riaecs/riaecs.h"

namespace mem_alloc_fixed_block
{
    template <size_t SIZE>
    class FixedBlockPool : public riaecs::IPool
    {
    private:
        std::byte pool[SIZE];

    public:
        FixedBlockPool() = default;
        ~FixedBlockPool() override = default;

        FixedBlockPool(const FixedBlockPool&) = delete;
        FixedBlockPool& operator=(const FixedBlockPool&) = delete;

        /***************************************************************************************************************
         * IPool Implementation
        /**************************************************************************************************************/

        std::byte *GetPool() override { return pool; }
        size_t GetSize() const override { return SIZE; }
    };

} // namespace mem_alloc_fixed_block