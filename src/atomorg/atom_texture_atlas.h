#include <stdio.h>
#include <string.h>

#include "atomorg.h"
#include "atom_math.h"

struct sub_texture
{
    char Name[50];
    v4 SubTextureRectangle;
};

struct texture_atlas
{
    sub_texture* Textures;
    u32 SubTextureCount;
    r32 TextureWidth;
    r32 TextureHeight;
};

s32
GetLine(char** Line, char** Stream)
{
    char* String = *Stream;
    char* Start = String;

    while(*String && *String != '\n')
    {
        String++;
    }

    if(*String == '\0')
        return 0;

    *String = '\0';

    *Line = Start;
    *Stream = String + 1;

    return 1;
}

u32
StrLength(char* s)
{
    u32 Length = 0;

    while(*s)
    {
        Length++;
        s++;
    }

    return Length;
}

void
StrCpy(char* d, const char* s, u32 Len)
{
    Assert(d != NULL);
    Assert(s != NULL);

    for(u32 i = 0; i < Len; ++i)
    {
        d[i] = s[i];
    }

    d[Len] = '\0';
}

char*
LSkip(char* s)
{
    while(*s && isspace((u32)*s))
    {
        s++;
    }

    return s;
}

char*
RearStrip(char* s)
{
    char* end = s + StrLength(s);
    while(end > s && isspace((u8)(*--end)))
        *end = '\0';

    return s;
}

char*
FindChar(char* s, char c)
{
    char* f = s;

    while(*f && *f != c)
    {
        f++;
    }

    return f;
}

u32
StrCmp(char* a, char* b)
{
    s32 LengthA = StrLength(a);
    s32 LengthB = StrLength(b);

    if(LengthA == LengthB)
    {
        for(s32 i = 0; i < LengthA; ++i)
        {
            if(a[i] == b[i])
                continue;

            return 0;
        }
        return 1;
    }

    return 0;
}

b32
IsDigit(char c)
{
    static char Table[10] = {'0', '1', '2', '3', '4', '5', '6', '7', '8' ,'9'};

    for(s32 i = 0; i < 10; ++i)
    {
        if(Table[i] == c)
            return 1;
    }

    return 0;
}

void
ParseNumber(char* n, u32* Number)
{
    char NumArray[20];
    s32 i;

    n = RearStrip(LSkip(n));

    for(i = 0; IsDigit(*n); ++i)
    {
        NumArray[i] = *n;
        n++;
    }

    NumArray[i] = '\0';

    *Number = atoi(NumArray);
}

#define MAX_SUBTEXTURE_COUNT 256

u32
LoadTextureAtlas(texture_atlas* OutAtlas, char* File, r32 TextureWidth, r32 TextureHeight)
{
    file_params Params = GetFileParams(File);
    char* Buffer = (char*)malloc(sizeof(char) * Params.Size + 1);
    if(LoadFile(File, Buffer) == -1) return 0;

    char* Temp = Buffer;

    char* Line;
    char* Start;

    texture_atlas Atlas = {};

    MEMORY_LEAK
    Atlas.Textures = (sub_texture*)malloc(MAX_SUBTEXTURE_COUNT * sizeof(sub_texture));

    while(GetLine(&Line, &Temp))
    {
        Start = LSkip(Line);
        printf("Start: %s\n", Start);
        if(*Start == '<')
        {
            Start++;
            char* Temp2 = FindChar(Start, ' ');
            *Temp2 = '\0';

            if(StrCmp(Start, "SubTexture"))
            {
                char* AttributeList = FindChar(LSkip(++Temp2), '>');
                *AttributeList = '\0';
                *(AttributeList - 1) = '\0';

                char* Attribute;
                char* Front;
                char* End;

                do
                {
                   Front = Temp2;
                   Attribute = FindChar(RearStrip(LSkip(Temp2)), ' ');

                   if(*Attribute != '\0')
                   {
                       *Attribute = '\0';
                       Attribute++;
                   }

                   End = FindChar(RearStrip(LSkip(Temp2)), '=');
                   *End = '\0';

                   End++;

                   if(StrCmp(Front, "name"))
                   {
                       char* Name = Atlas.Textures[Atlas.SubTextureCount].Name;
                       char* Local = ++End;

                       char* Delim = FindChar(Local, '.');

                       *Delim = '\0';

                       StrCpy(Name, Local, (StrLength(End) > 50) ? 50 : StrLength(Local));
                   }
                   else if(StrCmp(Front, "x"))
                   {
                       u32 Number;
                       ParseNumber(End + 1, &Number);
                       Atlas.Textures[Atlas.SubTextureCount].SubTextureRectangle.x = Number;
                   }
                   else if(StrCmp(Front, "y"))
                   {
                       u32 Number;
                       ParseNumber(End + 1, &Number);
                       Atlas.Textures[Atlas.SubTextureCount].SubTextureRectangle.y = Number;
                   }
                   else if(StrCmp(Front, "width"))
                   {
                       u32 Number;
                       ParseNumber(End + 1, &Number);
                       Atlas.Textures[Atlas.SubTextureCount].SubTextureRectangle.z = Number;
                   }
                   else if(StrCmp(Front, "height"))
                   {
                       u32 Number;
                       ParseNumber(End + 1, &Number);
                       Atlas.Textures[Atlas.SubTextureCount].SubTextureRectangle.w = Number;
                   }

                   Temp2 = Attribute;

                }while(*Attribute);
                Atlas.SubTextureCount++;
            }
        }
        else
            ATOM_ENSURE(false, "Unexpected identifier: %c\n", *Start);
    }

    free(Buffer);

    OutAtlas->SubTextureCount = Atlas.SubTextureCount;
    OutAtlas->Textures = Atlas.Textures;
    OutAtlas->TextureWidth = TextureWidth;
    OutAtlas->TextureHeight = TextureHeight;

    return 1;
}

u32
GetSubTextureRectangle(texture_atlas* Atlas, char* Name, v4* Rect)
{
    Assert(Atlas != NULL);
    Assert(Rect != NULL);
    Assert(Name != NULL);

    for(u32 i = 0; i < Atlas->SubTextureCount; ++i)
    {
        if(StrCmp(Atlas->Textures[i].Name, Name))
        {
            *Rect = Atlas->Textures[i].SubTextureRectangle;
            return 1;
        }
    }

    return 0;
}

