#include <SDL2/SDL.h>
#include <GLES3/gl3.h>
#include <x86intrin.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <stdint.h>

typedef int32_t s32;
typedef int64_t s64;

typedef uint32_t u32;
typedef uint64_t u64;

typedef float r32;
typedef double r64;

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

    r32 VertexData[8] = {0.5f, 0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f};
    u32 VertexBuffer;

    glGenBuffers(1, &VertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData), VertexData, GL_STATIC_DRAW);

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
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    SDL_Event e;

    u64 LastCounter = SDL_GetPerformanceCounter();
    u64 LastCycleCount = _rdtsc();

    while(IsRunning)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0, 1, 1, 1);

        while(SDL_PollEvent(&e))
        {
            switch(e.type)
            {
            case SDL_QUIT:
                IsRunning = false;
            }
        }

        glBindVertexArray(VertexArray);
        glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
        glUseProgram(Program);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
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
