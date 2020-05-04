#include <SDL2/SDL.h>
#include <GLES3/gl3.h>
#include <x86intrin.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

struct M3
{
    union {
        struct
        {
            r32 a00, a01, a02;
            r32 a10, a11, a12;
            r32 a20, a21, a22;
        };
        r32 Data[9];
    };

    M3(r32 r) {
        a00 = r;
        a11 = r;
        a22 = r;

        a01 = 0;
        a02 = 0;

        a10 = 0;
        a12 = 0;

        a20 = 0;
        a21 = 0;
    }

    r32 operator[](s32 i) {
        return Data[i];
    }
};

M3 operator*(M3& f, M3& s)
{
    M3 Final(0);

    Final.a00 = f.a00 * s.a00 + f.a01 * s.a10 + f.a02 * s.a20;
    Final.a01 = f.a00 * s.a01 + f.a01 * s.a11 + f.a02 * s.a21;
    Final.a02 = f.a00 * s.a02 + f.a01 * s.a12 + f.a02 * s.a22;

    Final.a10 = f.a10 * s.a00 + f.a11 * s.a10 + f.a12 * s.a20;
    Final.a11 = f.a10 * s.a01 + f.a11 * s.a11 + f.a12 * s.a21;
    Final.a12 = f.a10 * s.a02 + f.a11 * s.a12 + f.a12 * s.a22;

    Final.a20 = f.a20 * s.a00 + f.a21 * s.a10 + f.a22 * s.a20;
    Final.a21 = f.a20 * s.a01 + f.a21 * s.a11 + f.a22 * s.a21;
    Final.a22 = f.a20 * s.a02 + f.a21 * s.a12 + f.a22 * s.a22;

    return Final;
}

struct V2
{
    union
    {
        struct
        {
            r32 x, y;
        };
        r32 Data[2];
    };

    V2(r32 x, r32 y) {
        this->x = x;
        this->y = y;
    }
};

M3 m3Translate(M3& m, V2 v)
{
    M3 Translation(1.0f);

    Translation.a20 = v.x;
    Translation.a21 = v.y;

    return Translation * m;
}

M3 m3Scale(M3& m, V2 v)
{
    M3 Scale(1.0f);

    Scale.a00 = v.x;
    Scale.a11 = v.y;

    return Scale * m;
}

// TODO(rajat): May want to load GL functions ourselves

int main(int argc, char** argv)
{
    SDL_Init(SDL_INIT_EVERYTHING);

    u64 PerfCountFrequency = SDL_GetPerformanceFrequency();

    SDL_Window* Window = SDL_CreateWindow("AtomOrg", SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          1024, 576, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    SDL_GLContext Context = SDL_GL_CreateContext(Window);
    SDL_GL_MakeCurrent(Window, Context);

    bool IsRunning = true;

    fprintf(stderr, "%s\n", glGetString(GL_VERSION));

    u32 VertexArray;
    glGenVertexArrays(1, &VertexArray);
    glBindVertexArray(VertexArray);

    r32 VertexData[8] = {0, 0, 0, 1, 1, 1, 1, 0};
    u32 VertexBuffer;

    glGenBuffers(1, &VertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(r32) * 16, NULL, GL_DYNAMIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(r32) * 8, VertexData);

    u32 IndexData[6] = {0, 1, 2, 2, 3, 0};
    u32 IndexBuffer;

    glGenBuffers(1, &IndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(IndexData), IndexData, GL_STATIC_DRAW);

    // NOTE(rajat): Different platforms may have different glsl versions.
    s32 ShaderFileHandle = open("shaders/SimpleQuad.shader", O_RDONLY);

    struct stat ShaderFileStat;
    stat("shaders/SimpleQuad.shader", &ShaderFileStat);
    char* Buffer = (char*)malloc(sizeof(char) * ShaderFileStat.st_size + 1);
    Buffer[ShaderFileStat.st_size] = '\0';

    read(ShaderFileHandle, Buffer, ShaderFileStat.st_size);

    char* VertexShader = Buffer;
    char* End = Buffer;

    while(*End != '!')
    {
        ++End;
    }

    *End = '\0';

    char* FragmentShader = End + 3;

    u32 VS = glCreateShader(GL_VERTEX_SHADER);
    u32 FS = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(VS, 1, &VertexShader, NULL);
    glShaderSource(FS, 1, &FragmentShader, NULL);

    glCompileShader(VS);
    glCompileShader(FS);

    u32 Program = glCreateProgram();
    glAttachShader(Program, VS);
    glAttachShader(Program, FS);
    glLinkProgram(Program);
    glValidateProgram(Program);

    glUseProgram(Program);

    free(Buffer);
    close(ShaderFileHandle);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(r32) * 8));

    r32 Top = 0, Bottom = 576, Left = 0, Right = 1024;

    M3 Proj(1.0f);

    Proj.a00 = 2 / (Right - Left);
    Proj.a20 = -(Right + Left) / (Right - Left);

    Proj.a11 = 2 / (Bottom - Top);
    Proj.a21 = -(Bottom + Top) / (Bottom - Top);

    Proj.a22 = 1;

    for(s32 i = 0; i < 3; ++i)
    {
        for(s32 j = 0; j < 3; ++j)
        {
            fprintf(stderr, "%f ", Proj.Data[i * 3 + j]);
        }
        fprintf(stderr, "\n");
    }

    M3 Model(1.0f);

    Model = m3Translate(Model, V2(100, 0));
    Model = m3Scale(Model, V2(100, 100));

    u32 MatrixLoc = glGetUniformLocation(Program, "proj");
    glUniformMatrix3fv(MatrixLoc, 1, GL_FALSE, Proj.Data);

    u32 ModelLoc = glGetUniformLocation(Program, "model");
    glUniformMatrix3fv(ModelLoc, 1, GL_FALSE, Model.Data);

    u32 PlayerSheet;
    glGenTextures(1, &PlayerSheet);
    glBindTexture(GL_TEXTURE_2D, PlayerSheet);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    s32 w, h, n;

    stbi_set_flip_vertically_on_load(true);
    ubyte* PixelData = stbi_load("./assets/Player.png", &w, &h, &n, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, PixelData);

    printf("{w: %i, h: %i}\n", w, h);

    r32 TexCoords[16][8];

    for(s32 i = 0; i < 4; ++i)
    {
        for(s32 j = 0; j < 4; ++j)
        {
            r32 Texcoord[8] = { (j * 64.0f) / w, (i * 64.0f) / h, (j * 64.0f) / w, (i * 64.0f + 64.0f) / h,
                               (j * 64.0f + 64.0f) / w, (i * 64.0f + 64.0f) / h, (j * 64.0f + 64.0f) / w,
                               (i * 64.0f) / h};

            for(s32 k = 0; k < 8; ++k)
            {
                TexCoords[i * 4 + j][k] = Texcoord[k];
            }
        }
    }

    glBufferSubData(GL_ARRAY_BUFFER, sizeof(r32) * 8, sizeof(r32) * 8, TexCoords[0]);

    SDL_Event e;

    u64 LastCounter = SDL_GetPerformanceCounter();
    u64 LastCycleCount = _rdtsc();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    r32 x = 0;
    r32 y = 0;

    u32 Dir = 0;
    u32 LastDir = 0;

    b32 PlayerStop = 0;

    while(IsRunning)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.2, 0.2, 0.2, 1);

        if(PlayerStop)
        {
            if(LastDir > 0 && LastDir < 4)
                Dir = 0;
            if(LastDir > 3 && LastDir < 8)
                Dir = 4;
            if(LastDir > 7 && LastDir < 12)
                Dir = 8;
            if(LastDir > 11 && LastDir < 16)
                Dir = 12;
        }

        while(SDL_PollEvent(&e))
        {
            switch(e.type)
            {
            case SDL_QUIT:
                IsRunning = false;
                break;
            case SDL_KEYDOWN: {
                PlayerStop = false;
                if(e.key.keysym.sym == SDLK_UP)
                {
                    if(LastDir < 4)
                    {
                        y += 16;
                    }

                    if(LastDir >= 3)
                        Dir = 0;
                    else
                        Dir++;

                    LastDir = Dir;
                }
                else if(e.key.keysym.sym == SDLK_DOWN)
                {
                    if(LastDir >= 12 && LastDir < 16)
                    {
                        y -= 16;
                    }

                    if(LastDir >= 15 || LastDir < 12)
                        Dir = 12;
                    else
                        Dir++;

                    LastDir = Dir;
                }
                else if(e.key.keysym.sym == SDLK_LEFT)
                {
                    if(LastDir >= 8 && LastDir < 11)
                    {
                        x -= 16;
                    }

                    if(LastDir >= 11 || LastDir < 8)
                        Dir = 8;
                    else
                        Dir++;

                    LastDir = Dir;

                }
                else if(e.key.keysym.sym == SDLK_RIGHT)
                {
                    if(LastDir >= 4 && LastDir < 8)
                    {
                        x += 16;
                    }

                    if(LastDir >= 7 || LastDir < 4)
                        Dir = 4;
                    else
                        Dir++;

                    LastDir = Dir;
                }
            }   break;
            case SDL_KEYUP: {
                PlayerStop = true;
            }   break;
            }
        }

        glBufferSubData(GL_ARRAY_BUFFER, sizeof(r32) * 8, sizeof(r32) * 8, TexCoords[Dir]);

        glBindVertexArray(VertexArray);
        glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
        glUseProgram(Program);

        M3 Model(1.0f);

        Model = m3Translate(Model, V2(x, y));
        Model = m3Scale(Model, V2(64, 64));

        glUniformMatrix3fv(ModelLoc, 1, GL_FALSE, Model.Data);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
        glBindTexture(GL_TEXTURE_2D, PlayerSheet);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
        SDL_GL_SwapWindow(Window);

        u64 EndCounter = SDL_GetPerformanceCounter();
        u64 EndCycleCount = _rdtsc();
        u64 CounterElapsed = EndCounter - LastCounter;
        u64 CyclesElapsed = EndCycleCount - LastCycleCount;

        r64 MSPerFrame = (((1000.0f * (r64)CounterElapsed) / (r64)PerfCountFrequency));
        r64 FPS = (r64)PerfCountFrequency / (r64)CounterElapsed;
        r64 MCPF = ((r64)CyclesElapsed / (1000.0f * 1000.0f));

        printf("%.02fms/f, %.02f/s, %.02fmc/f\n", MSPerFrame, FPS, MCPF);

        LastCounter = EndCounter;
        LastCycleCount = EndCycleCount;
    }

    SDL_GL_DeleteContext(Context);
    SDL_DestroyWindow(Window);
}
