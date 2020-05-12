#pragma once

#include <stdint.h>

typedef int8_t s8;
typedef uint8_t u8;

typedef u8 ubyte;
typedef u8 b32;

typedef int32_t s32;
typedef int64_t s64;

typedef uint32_t u32;
typedef uint64_t u64;

typedef float r32;
typedef double r64;

#define ATOM_DEBUG

#define ATOM_ENSURE(x, msg, ...) \
    {                            \
        if(!(x)) { fprintf(stderr, msg, __VA_ARGS__); *(u8*)0 = 1;}  \
    }

#if defined(ATOM_DEBUG)
#define Assert(x) ATOM_ENSURE(x, "Assertion failed: %s\n In File: %s at line: %i\n", #x, __FILE__, __LINE__);
#else
#define Assert(x)
#endif

// Used in placing using malloc and os memory
#define MEMORY_LEAK

struct file_params
{
    u32 Size;
    b32 Exists;
};

extern file_params
GetFileParams(char* Path);

extern s32
LoadFile(char* Path, char* Buffer);

extern s32
LoadFileRaw(char* Path, void* Buffer);
