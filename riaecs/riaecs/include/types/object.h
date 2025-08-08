#pragma once

#include <shared_mutex>

namespace riaecs
{
    template<typename T>
    class ReadOnlyObject 
    {
    private:
        std::shared_lock<std::shared_mutex> lock_;
        const T& ref_;

    public:
        ReadOnlyObject(std::shared_lock<std::shared_mutex>&& lock, const T& ref) : 
            lock_(std::move(lock)), ref_(ref) 
        {
        }

        const T& operator()() 
        {
            return ref_;
        }
    };

} // namespace riaecs