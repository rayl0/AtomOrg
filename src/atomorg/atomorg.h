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

#if defined(ATOM_DEBUG)
#define Assert(x) { if(!(x)) { fprintf(stderr, "Assertion failed: %s\n In File: %s at line: %i\n", #x, __FILE__, __LINE__); *(u8*)0 = 1; } }
#else
#define Assert(x)
#endif

#ifdef __cpluscplus
extern "C" {
#endif

#include "atom_math.h"

typedef struct game_button
{
    b32 EndedDown;
}game_button;

typedef struct game_input
{
    struct
    {
        union
        {
            struct
            {
                v2 at;
            };
            s32 x, y;
        };
        b32 Hit;
        b32 Hold;
    }Pointer;

    union
    {
        struct
        {
            game_button MoveUp;
            game_button MoveDown;
            game_button MoveLeft;
            game_button MoveRight;

            game_button A;
            game_button B;
            game_button X;
            game_button Y;

            game_button Select;
            game_button Start;
        };
    };
}game_input;

typedef struct game_state
{
}game_state;

typedef struct file_params
{
    b32 Exists;
    u32 Size;
}file_params;

typedef struct platform_api
{
    file_params (*GetFileParams)(char* Path);
    s32 (*LoadFile)(char* Path, char* Buffer);
    s32 (*LoadFileRaw)(char* Path, void* Buffer);
}platform_api;

#define GAME_INIT(x) void x(game_state* State, platform_api* API);
#define GAME_UPDATE_AND_RENDER(x) void x(game_state* State, game_input*Input);

#ifdef __cpluscplus
}
#endif
