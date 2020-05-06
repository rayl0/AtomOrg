#include "atomorg.h"

static platform_api PlatformAPI;

GAME_INIT(GameInit)
{
    PlatformAPI.GetFileParams = API->GetFileParams;
    PlatformAPI.LoadFile = API->LoadFile;
    PlatformAPI.LoadFileRaw = API->LoadFileRaw;
}

GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
}
