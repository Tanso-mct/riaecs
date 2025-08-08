#pragma once

namespace riaecs
{
    class ID
    {
    public:
        ID(size_t index, size_t generation) : index_(index), generation_(generation) {}
        ID() = delete;
        ~ID() = default;

        const size_t index_;
        const size_t generation_;
    };

} // namespace riaecs