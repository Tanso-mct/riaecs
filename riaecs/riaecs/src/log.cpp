#include "riaecs/src/pch.h"
#include "riaecs/include/log.h"

void riaecs::Log::OutToConsole(std::string_view msg, const char *color)
{
#ifdef _DEBUG

    // Log to Console
    std::cout << color;
    std::cout << msg;
    std::cout << riaecs::CONSOLE_TEXT_COLOR_DEFAULT;

    // Log to Visual Studio Debug Console
    OutputDebugStringA(msg.data());

#endif
}

void riaecs::Log::OutToWindow(std::string_view msg, std::string_view title)
{
#ifdef _DEBUG

    // Log to a window
    MessageBoxA(nullptr, msg.data(), title.data(), MB_OK | MB_ICONINFORMATION);

#endif
}

void riaecs::Log::OutToErrorWindow(std::string_view msg, std::string_view title)
{
#ifdef _DEBUG

    // Log to an error window
    MessageBoxA(nullptr, msg.data(), title.data(), MB_OK | MB_ICONERROR | MB_TASKMODAL | MB_SETFOREGROUND | MB_TOPMOST);

#endif
}