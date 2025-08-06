#include "riaecs/src/pch.h"
#include "riaecs/include/utilities.h"

RIAECS_API std::string riaecs::CreateMessage(const std::initializer_list<std::string> &lines)
{
    std::string message;
    for (const auto &line : lines)
        message += line + "\n";

    return message;
}

RIAECS_API std::string riaecs::CreateMessage
(
    const std::initializer_list<std::string> &lines, 
    const std::string &file, int line, const std::string &function
){
    std::string message = CreateMessage(lines);
    message += file + " : " + std::to_string(line) + " : " + function + "\n";

    return message;
}