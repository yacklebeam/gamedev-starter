#include "bifrost_dungeon.h"

namespace
{

#include "tilemap_png.h"

}

bifrost::Texture bifrost::GetDungeonTexture()
{
    return bifrost::LoadTexture(tilemap_png, static_cast<int>(tilemap_png_len));
}

