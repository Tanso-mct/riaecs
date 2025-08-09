#pragma once

#ifdef MEMALLOCFIXEDBLOCK_EXPORTS
#define MEM_ALLOC_FIXED_BLOCK_API __declspec(dllexport)
#else
#define MEM_ALLOC_FIXED_BLOCK_API __declspec(dllimport)
#endif