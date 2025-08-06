#pragma once
#include "riaecs/include/dll_config.h"

#include <vector>
#include <string>
#include <string_view>

namespace riaecs
{
    constexpr const char* CONSOLE_TEXT_COLOR_DEFAULT = "\033[0m";
    constexpr const char* CONSOLE_TEXT_COLOR_WARNING = "\033[33m";
    constexpr const char* CONSOLE_TEXT_COLOR_ERROR = "\033[31m";

    class RIAECS_API Log
    {
    private:
        static std::vector<std::string> logs;
        
    public:
        Log() = default;
        virtual ~Log() = default;

        static void OutToConsole(std::string_view msg, const char *color);
        static void OutToWindow(std::string_view msg, std::string_view title);
        static void OutToErrorWindow(std::string_view msg, std::string_view title);
    };

} // namespace riaecs