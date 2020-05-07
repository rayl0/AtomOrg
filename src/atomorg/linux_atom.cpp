#include "atomorg.h"

#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

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

        Assert(FileHandle != -1);

        struct stat FileState;
        stat(Path, &FileState);

        if(FileState.st_size == 0)
            return -1;

        read(FileHandle, Buffer, FileState.st_size);

        return 0;
    }

    return -1;
}

s32
LoadFile(char* Path, char* Buffer)
{
    s32 err = LoadFileRaw(Path, Buffer);

    struct stat FileState;
    stat(Path, &FileState);

    if(!err)
        Buffer[FileState.st_size] = '\0';

    return err;
}
