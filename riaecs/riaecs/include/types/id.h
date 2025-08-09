#pragma once

#include <utility>

namespace riaecs
{
    constexpr size_t ID_DEFAULT_GENERATION = 0;

    class ID
    {
    public:
        ID(size_t index, size_t generation) : index_(index), generation_(generation) {}
        ID() = delete;
        ~ID() = default;

        const size_t index_;
        const size_t generation_;

        bool operator==(const ID &other) const
        {
            return index_ == other.index_ && generation_ == other.generation_;
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
            return std::hash<size_t>()(id.index_) ^ (std::hash<size_t>()(id.generation_) << 1);
        }
    };
}