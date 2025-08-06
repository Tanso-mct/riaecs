#include "riaecs/src/pch.h"
#include "riaecs/include/utilities.h"

#include "riaecs/include/log.h"

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

RIAECS_API void riaecs::NotifyError
(
    const std::initializer_list<std::string> &lines, 
    const std::string &file, int line, const std::string &function)
{
    // Create the error message
    std::string log = riaecs::CreateMessage(lines, __FILE__, __LINE__, __FUNCTION__);

    // Output the error message to console and error window
    riaecs::Log::OutToConsole(log, riaecs::CONSOLE_TEXT_COLOR_ERROR);
    riaecs::Log::OutToErrorWindow(log, "RIAECS");

    // Throw a runtime error with the log message
    throw std::runtime_error(log);
}