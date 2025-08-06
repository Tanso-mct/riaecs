#pragma once

#ifdef RIAECS_EXPORTS
#define RIAECS_API __declspec(dllexport)
#else
#define RIAECS_API __declspec(dllimport)
#endif