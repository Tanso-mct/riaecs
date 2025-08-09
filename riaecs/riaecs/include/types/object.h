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
        ReadOnlyObject(std::shared_lock<std::shared_mutex> &&lock, const T &ref) : 
            lock_(std::move(lock)), ref_(ref) 
        {
        }

        const T& operator()() 
        {
            return ref_;
        }

        std::shared_lock<std::shared_mutex> &&TakeLock() 
        {
            return lock_;
        }
    };

    template <typename T>
    class ReadOnlyObject<T*>
    {
    private:
        std::shared_lock<std::shared_mutex> lock_;
        T* ptr_;

    public:
        ReadOnlyObject(std::shared_lock<std::shared_mutex> &&lock, T* ptr) : 
            lock_(std::move(lock)), ptr_(ptr) 
        {
        }

        T* operator()() 
        {
            return ptr_;
        }

        std::shared_lock<std::shared_mutex> &&TakeLock() 
        {
            return std::move(lock_);
        }
    };

} // namespace riaecs