#pragma once

#include <shared_mutex>
#include <vector>

namespace riaecs
{
    template<typename T>
    class ReadOnlyObject 
    {
    private:
        std::shared_lock<std::shared_mutex> mainLock_;
        std::vector<std::unique_lock<std::shared_mutex>> subUniqueLocks_;
        std::vector<std::shared_lock<std::shared_mutex>> subSharedLocks_;

        const T& ref_;

    public:
        ReadOnlyObject(std::shared_lock<std::shared_mutex> mainlock, const T &ref) : 
            mainLock_(std::move(mainlock)), ref_(ref) 
        {
        }

        const T& operator()() 
        {
            return ref_;
        }

        std::shared_lock<std::shared_mutex> TakeLock() 
        {
            return std::move(mainLock_);
        }

        void AddSubLock(std::unique_lock<std::shared_mutex> subLock) 
        {
            subUniqueLocks_.emplace_back(std::move(subLock));
        }

        void AddSubLock(std::shared_lock<std::shared_mutex> subLock) 
        {
            subSharedLocks_.emplace_back(std::move(subLock));
        }
    };

    template <typename T>
    class ReadOnlyObject<T*>
    {
    private:
        std::shared_lock<std::shared_mutex> mainLock_;
        std::vector<std::unique_lock<std::shared_mutex>> subUniqueLocks_;
        std::vector<std::shared_lock<std::shared_mutex>> subSharedLocks_;

        T* ptr_;

    public:
        ReadOnlyObject(std::shared_lock<std::shared_mutex> mainLock, T* ptr) : 
            mainLock_(std::move(mainLock)), ptr_(ptr) 
        {
        }

        T* operator()() 
        {
            return ptr_;
        }

        std::shared_lock<std::shared_mutex> TakeLock() 
        {
            return std::move(mainLock_);
        }

        void AddSubLock(std::unique_lock<std::shared_mutex> subLock) 
        {
            subUniqueLocks_.emplace_back(std::move(subLock));
        }

        void AddSubLock(std::shared_lock<std::shared_mutex> subLock) 
        {
            subSharedLocks_.emplace_back(std::move(subLock));
        }
    };

} // namespace riaecs