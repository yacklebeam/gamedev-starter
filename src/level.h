#pragma once

#include <bifrost/bifrost.h>
#include <bifrost/bifrost_collision.h>
#include <vector>

struct Level
{
    bool loaded = false;
    int width = 0;
    int height = 0;
    std::vector<bifrost::Hitbox> hitboxes;

    Level(const char* path);
    void Draw(bifrost::Camera2d camera) const;

private:
    struct TileCell { int tile; int rotation; };

    std::vector<TileCell> tiles[3];
    bifrost::Texture texture{};
};
