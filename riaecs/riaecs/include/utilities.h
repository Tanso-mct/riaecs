#pragma once
#include "riaecs/include/dll_config.h"

#include <string>
#include <initializer_list>

namespace riaecs
{
    RIAECS_API std::string CreateMessage(const std::initializer_list<std::string> &lines);
    RIAECS_API std::string CreateMessage
    (
        const std::initializer_list<std::string> &lines, 
        const std::string &file, int line, const std::string &function
    );

} // namespace riaecs