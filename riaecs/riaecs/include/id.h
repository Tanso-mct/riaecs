#pragma once

#include "riaecs/include/interfaces/id.h"

namespace riaecs
{
    class ID : public IID
    {
    private:
        size_t id_;
        size_t generation_;

    public:
        ID(size_t id, size_t generation)
            : id_(id), generation_(generation) 
        {
        }

        /***************************************************************************************************************
         * IID Implementation
        /**************************************************************************************************************/

        size_t Get() const override { return id_; }
        size_t GetGeneration() const override { return generation_; }
    };

} // namespace riaecs