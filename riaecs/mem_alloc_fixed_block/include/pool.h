#pragma once
#include "mem_alloc_fixed_block/include/dll_config.h"

#include "riaecs/riaecs.h"

namespace mem_alloc_fixed_block
{
    class MEM_ALLOC_FIXED_BLOCK_API FixedBlockPool : public riaecs::IPool
    {
    private:
        const size_t SIZE;
        std::unique_ptr<std::byte[]> pool_;

    public:
        FixedBlockPool(const size_t size);
        ~FixedBlockPool() override = default;

        FixedBlockPool(const FixedBlockPool&) = delete;
        FixedBlockPool& operator=(const FixedBlockPool&) = delete;

        /***************************************************************************************************************
         * IPool Implementation
        /**************************************************************************************************************/

        std::byte *GetPool() override { return pool_.get(); }
        size_t GetSize() const override { return SIZE; }
    };

    class MEM_ALLOC_FIXED_BLOCK_API FixedBlockPoolFactory : public riaecs::IPoolFactory
    {
    public:
        FixedBlockPoolFactory() = default;
        ~FixedBlockPoolFactory() override = default;

        /***************************************************************************************************************
         * IPoolFactory Implementation
        /**************************************************************************************************************/

        std::unique_ptr<riaecs::IPool> Create(size_t size) const override;
        void Destroy(std::unique_ptr<riaecs::IPool> product) const override;

        size_t GetProductSize() const override { return sizeof(FixedBlockPool); }
    };

} // namespace mem_alloc_fixed_block