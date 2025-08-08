#pragma once

#include <utility>

namespace riaecs
{
    struct PairEqual 
    {
        template <typename T1, typename T2>
        bool operator()(const std::pair<T1, T2>& lhs, const std::pair<T1, T2>& rhs) const 
        {
            return lhs == rhs;
        }
    };

} // namespace riaecs 