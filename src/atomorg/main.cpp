#include <SDL2/SDL.h>
#include <GLES3/gl3.h>
#include <x86intrin.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "atomorg.h"
#include "atom_math.h"

u32
CreateGLBuffer(u32 Type, u32 Size, void* Data, u32 Usage)
{
    u32 NewBuffer;

    glGenBuffers(1, &NewBuffer);
    glBindBuffer(Type, NewBuffer);
    glBufferData(Type, Size, Data, Usage);

    return NewBuffer;
}

u32
CreateShader(char* File)
{
    s32 ShaderFileHandle = open(File, O_RDONLY);

    Assert(ShaderFileHandle != -1);

    struct stat ShaderFileStat;
    stat(File, &ShaderFileStat);
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

    glDeleteShader(VS);
    glDeleteShader(FS);

    free(Buffer);
    close(ShaderFileHandle);

    return Program;
}

struct texture
{
    u32 Id;
    s32 w, h;
    s32 n;
};

texture
CreateTexture(char* File)
{
    texture Texture;
    glGenTextures(1, &Texture.Id);
    glBindTexture(GL_TEXTURE_2D, Texture.Id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    s32 w, h, n;

    ubyte* PixelData = stbi_load(File, &w, &h, &n, 0);

    Texture.w = w;
    Texture.h = h;
    Texture.n = n;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, PixelData);

    return Texture;
}

struct render_ctx
{
    u32 VertexBuffer;
    u32 VertexArray;
    u32 IndexBuffer;
}static ctx;

void
InitRenderCtx()
{
    glGenVertexArrays(1, &ctx.VertexArray);
    glBindVertexArray(ctx.VertexArray);

    r32 VertexData[16] = {0, 0, 0, 1, 1, 1, 1, 0,
                           0, 0, 0, 1, 1, 1, 1, 0};

    ctx.VertexBuffer = CreateGLBuffer(GL_ARRAY_BUFFER, sizeof(r32) * 16, NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(r32) * 16, VertexData);

    u32 IndexData[8] = {0, 1, 2, 2, 3, 0};

    ctx.IndexBuffer = CreateGLBuffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(IndexData), IndexData, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(r32) * 8));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

struct ui_render_ctx
{
    u32 Shader;
}static uictx;

void
UIInitRenderCtx()
{
    uictx.Shader = CreateShader("./shaders/UIQuad.shader");
}

void
UISetRenderParams(c3 Color, r32 Alpha, u32 Texture,
                  u32 UseTexture, v4 DestRect, v4 *ScissorRect)
{
    glUseProgram(uictx.Shader);

    glUniform3f(glGetUniformLocation(uictx.Shader, "color"), Color.r, Color.g, Color.b);
    glUniform1f(glGetUniformLocation(uictx.Shader, "alpha"), Alpha);
    glUniform1f(glGetUniformLocation(uictx.Shader, "use_texture"), UseTexture);

    m3 Model = M3(1.0f);
    Model = m3Translate(Model, DestRect.min);
    Model = m3Scale(Model, DestRect.max);

    glUniformMatrix3fv(glGetUniformLocation(uictx.Shader, "model"), 1, GL_FALSE, Model.Data);

    if(UseTexture > 0.5f)
        glBindTexture(GL_TEXTURE_2D, Texture);

    if(ScissorRect != NULL)
    {
        glEnable(GL_SCISSOR_TEST);
        glScissor(ScissorRect->x, 576 - (ScissorRect->y + ScissorRect->w), ScissorRect->z, ScissorRect->w);
    }
}

void
UISetRenderParams(c3 Color, r32 Alpha, u32 Texture,
                  u32 UseTexture, v4 DestRect)
{
    UISetRenderParams(Color, Alpha, Texture, UseTexture, DestRect, NULL);
}

void
RenderQuad()
{
    glBindVertexArray(ctx.VertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, ctx.VertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx.IndexBuffer);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
}

void
RenderUIQuad(c3 Color, r32 Alpha, u32 Texture, u32 UseTexture, v4 DestRect, v4 *ScissorRect)
{
    UISetRenderParams(Color, Alpha, Texture, UseTexture, DestRect, ScissorRect);
    RenderQuad();
    if(ScissorRect)
        glDisable(GL_SCISSOR_TEST);
}

void
RenderUIQuad(c3 Color, r32 Alpha, u32 Texture, u32 UseTexture, v4 DestRect)
{
    RenderUIQuad(Color, Alpha, Texture, UseTexture, DestRect, NULL);
}

void
RenderUIQuad(c3 Color, r32 Alpha, v4 DestRect)
{
    RenderUIQuad(Color, Alpha, 0, 0, DestRect);
}

void
RenderUIQuad(u32 Texture, r32 Alpha, v4 DestRect)
{
    RenderUIQuad(C3(0u, 0u, 0u), Alpha, Texture, 1, DestRect);
}

void
RenderUIQuad(u32 Texture, v4 DestRect, v4 ScissorRect)
{
    RenderUIQuad(C3(0u, 0u, 0u), 1.0f, Texture, 1.0f, DestRect, &ScissorRect);
}

file_params
GetFileParams(char* Path)
{
    file_params Params;

    struct stat FileState;
    stat(Path, &FileState);

    Params.Size = FileState.st_size;
    Params.Exists = (Params.Size == 0) ? 0 : 1;

    return Params;
}

s32
LoadFileRaw(char* Path, void* Buffer)
{
    if(Buffer)
    {
        s32 FileHandle;
        FileHandle = open(Path, O_RDONLY);

        file_params Params = GetFileParams(Path);

        if(Params.Size == 0)
            return -1;

        read(FileHandle, Buffer, Params.Size);

        return 0;
    }

    return -1;
}

s32
LoadFile(char* Path, char* Buffer)
{
    s32 err = LoadFileRaw(Path, Buffer);

    file_params Params = GetFileParams(Path);

    if(!err)
        Buffer[Params.Size + 1] = '\0';

    return err;
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

    InitRenderCtx();

    u32 Program = CreateShader("./shaders/SimpleQuad.shader");

    // NOTE(rajat): Different platforms may have different glsl versions.
    m3 Proj = m3Ortho(0, 1024, 576, 0);

    m3 Model = M3(1.0f);

    Model = m3Translate(Model, V2(100, 0));
    Model = m3Scale(Model, V2(100, 100));

    u32 MatrixLoc = glGetUniformLocation(Program, "proj");
    glUniformMatrix3fv(MatrixLoc, 1, GL_FALSE, Proj.Data);

    u32 ModelLoc = glGetUniformLocation(Program, "model");
    glUniformMatrix3fv(ModelLoc, 1, GL_FALSE, Model.Data);

    texture PlayerSheet = CreateTexture("./assets/Player.png");
    texture TileSheet = CreateTexture("./assets/Tiles.png");

    r32 w = PlayerSheet.w;
    r32 h = PlayerSheet.h;

#if 1
    UIInitRenderCtx();
    glUniformMatrix3fv(glGetUniformLocation(uictx.Shader, "proj"), 1, GL_FALSE, Proj.Data);
#endif

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


    SDL_Event e;

    u64 LastCounter = SDL_GetPerformanceCounter();
    u64 LastCycleCount = _rdtsc();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    game_state GameState = {};
    game_input GameInput = {};

    platform_api PlatformAPI = {};

    PlatformAPI.GetFileParams = GetFileParams;
    PlatformAPI.LoadFile = LoadFile;
    PlatformAPI.LoadFileRaw = LoadFileRaw;

    r32 x = 0;
    r32 y = 0;

    u32 Dir = 0;
    u32 LastDir = 0;

    b32 PlayerStop = 0;

    s32 MouseX = 0;
    s32 MouseY = 0;

    b32 Drag = false;

    v4 HitBox[16];

    for(s32 j = 0; j < 4; ++j)
    {
        for(s32 i = 0; i < 4; ++i)
        {
            HitBox[j * 4 + i] = V4(i * 64.0f, j * 64.0f, i * 64.0f + 64.0f, j * 64.0f + 64.0f);
        }
    }

    v4 QuadList[50];
    u32 TexCoordIndexList[50];

    while(IsRunning)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.7, 0.5, 0.7, 1);

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

        while(SDL_PollEvent(&e) != 0)
        {
            b32 Hold = GameInput.Pointer.Hold;
            v2 at = GameInput.Pointer.at;

            GameInput = {};
            GameInput.Pointer.Hold = Hold;
            GameInput.Pointer.at = at;

            switch(e.type)
            {
            case SDL_QUIT:
                IsRunning = false;
                break;
            case SDL_KEYDOWN: {
                PlayerStop = false;
                if(e.key.keysym.sym == SDLK_DOWN)
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
                else if(e.key.keysym.sym == SDLK_UP)
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
                else if(e.key.keysym.sym == SDLK_RIGHT)
                {
                    if(LastDir >= 8 && LastDir < 11)
                    {
                        x += 16;
                    }

                    if(LastDir >= 11 || LastDir < 8)
                        Dir = 8;
                    else
                        Dir++;

                    LastDir = Dir;

                }
                else if(e.key.keysym.sym == SDLK_LEFT)
                {
                    if(LastDir >= 4 && LastDir < 8)
                    {
                        x -= 16;
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
            case SDL_MOUSEMOTION: {
                MouseX = e.motion.x;
                MouseY = e.motion.y;

                GameInput.Pointer.x = e.motion.x;
                GameInput.Pointer.y = e.motion.y;
            }   break;
            case SDL_MOUSEBUTTONUP: {
                GameInput.Pointer.Hold = false;
                GameInput.Pointer.Hit = true;
            }   break;
            case SDL_MOUSEBUTTONDOWN: {
                GameInput.Pointer.Hold = true;
            }   break;
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, ctx.VertexBuffer);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(r32) * 8, sizeof(r32) * 8, TexCoords[Dir]);

        glUseProgram(Program);

        m3 Model = M3(1.0f);

        Model = m3Translate(Model, V2(x - 64 / 2, y - 64 / 2));
        Model = m3Scale(Model, V2(64, 64));

        glUniformMatrix3fv(ModelLoc, 1, GL_FALSE, Model.Data);
        glBindTexture(GL_TEXTURE_2D, PlayerSheet.Id);

        RenderQuad();

        r32 ATexCoords[8] = {0, 0, 0, 1, 1, 1, 1, 0};
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(r32) * 8, sizeof(r32) * 8, ATexCoords);

        glUseProgram(uictx.Shader);

        u32 MatrixLoc = glGetUniformLocation(uictx.Shader, "proj");
        glUniformMatrix3fv(MatrixLoc, 1, GL_FALSE, Proj.Data);

        RenderUIQuad(C3(0.2f, 0.2f, 0.2f), 1.0f, V4(100, 100, 256, 256));

        // TODO(rajat): Send the whole texture data to the draw calls to support clipping.
        RenderUIQuad(TileSheet.Id, V4(100, 100, TileSheet.w, TileSheet.h), V4(100, 100, 256, 256));

        static r32 Selected = -1;

        for(s32 i = 0; i < 16; ++i)
        {
            if((100 + HitBox[i].min.x < MouseX && (100 + HitBox[i].max.x) > MouseX) &&
               (100 + HitBox[i].min.y < MouseY && (100 + HitBox[i].max.y) > MouseY))
            {
                Selected = i;
            }
        }

        if(Drag)
        {
            glBindBuffer(GL_ARRAY_BUFFER, ctx.VertexBuffer);
            glBufferSubData(GL_ARRAY_BUFFER, sizeof(r32) * 8, sizeof(r32) * 8, TexCoords[(s32)Selected]);

            RenderUIQuad(PlayerSheet.Id, Selected, V4(MouseX, MouseY, HitBox[0].max.x, HitBox[0].max.y));
        }

        SDL_GL_SwapWindow(Window);

        u64 EndCounter = SDL_GetPerformanceCounter();
        u64 EndCycleCount = _rdtsc();
        u64 CounterElapsed = EndCounter - LastCounter;
        u64 CyclesElapsed = EndCycleCount - LastCycleCount;

        r64 MSPerFrame = (((1000.0f * (r64)CounterElapsed) / (r64)PerfCountFrequency));

#if 0
        r64 FPS = (r64)PerfCountFrequency / (r64)CounterElapsed;
        r64 MCPF = ((r64)CyclesElapsed / (1000.0f * 1000.0f));

        printf("%.02fms/f, %.02f/s, %.02fmc/f\n", MSPerFrame, FPS, MCPF);
#endif

        LastCounter = EndCounter;
        LastCycleCount = EndCycleCount;
    }

    SDL_GL_DeleteContext(Context);
    SDL_DestroyWindow(Window);
}
