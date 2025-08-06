#include "riaecs_unit_test/pch.h"

#include "riaecs/include/log.h"
#pragma comment(lib, "riaecs.lib");

TEST(Log, OutToConsole)
{
    riaecs::Log::OutToConsole("Test message\n", riaecs::CONSOLE_TEXT_COLOR_DEFAULT);
    riaecs::Log::OutToConsole("Test warning message\n", riaecs::CONSOLE_TEXT_COLOR_WARNING);
    riaecs::Log::OutToConsole("Test error message\n", riaecs::CONSOLE_TEXT_COLOR_ERROR);
}

TEST(Log, OutToWindow)
{
    riaecs::Log::OutToWindow("Test message in window", "Test Window Title");
}

TEST(Log, OutToErrorWindow)
{
    riaecs::Log::OutToErrorWindow("Test error message in window", "Test Error Window Title");
}