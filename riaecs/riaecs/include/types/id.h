#pragma once

#include <utility>

namespace riaecs
{
    constexpr size_t ID_DEFAULT_GENERATION = 0;

    class ID
    {
    private:
        size_t index_;
        size_t generation_;

    public:
        ID(size_t index, size_t generation) : index_(index), generation_(generation) {}
        ID() = delete;
        ~ID() = default;

        size_t GetIndex() const { return index_; }
        size_t GetGeneration() const { return generation_; }

        bool operator==(const ID &other) const
        {
            return index_ == other.index_ && GetGeneration() == other.GetGeneration();
        }
    };

} // namespace riaecs

namespace std 
{
    template <>
    struct hash<riaecs::ID> 
    {
        std::size_t operator()(const riaecs::ID &id) const noexcept 
        {
            return std::hash<size_t>()(id.GetIndex()) ^ (std::hash<size_t>()(id.GetGeneration()) << 1);
        }
    };
}