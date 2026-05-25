#include "level.h"
#include <bifrost/bifrost_dungeon.h>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <string>

static constexpr int LAYER_COUNT   = 3;
static constexpr int TILE_SRC_SIZE = 16;
static constexpr int TILE_STRIDE   = 17;
static constexpr int TILE_DISPLAY  = 32;
static constexpr int SHEET_COLS    = 12;
static constexpr int SHEET_ROWS    = 11;

static const char* LAYER_SECTION[LAYER_COUNT] = { "ground", "walls", "objects" };

static glm::vec2 TileSourceOrigin(int tile)
{
    int col    = tile % SHEET_COLS;
    int row    = tile / SHEET_COLS;
    int gl_row = (SHEET_ROWS - 1) - row;
    return { (float)col * TILE_STRIDE, (float)gl_row * TILE_STRIDE };
}

Level::Level(const char* path)
{
    texture = bifrost::GetDungeonTexture();

    std::ifstream f(path);
    if (!f) return;

    std::string line;
    if (!std::getline(f, line)) return;
    if (line.empty() || line[0] != '[') return;

    bool dims_set = false;
    do
    {
        if (line == "[hitboxes]")
        {
            std::string hb_line;
            while (std::getline(f, hb_line) && !hb_line.empty())
            {
                float x = 0, y = 0, w = 0, h = 0;
                if (sscanf(hb_line.c_str(), "%f,%f,%f,%f", &x, &y, &w, &h) == 4 && w > 0 && h > 0)
                {
                    float px = (x + w * 0.5f) * TILE_DISPLAY;
                    float py = (height - y - h * 0.5f) * TILE_DISPLAY;
                    hitboxes.push_back(bifrost::GenRectHitbox({px, py}, {w * TILE_DISPLAY, h * TILE_DISPLAY}));
                }
            }
            break;
        }

        int layer_idx = -1;
        for (int l = 0; l < LAYER_COUNT; ++l)
        {
            if (line == std::string("[") + LAYER_SECTION[l] + "]")
            {
                layer_idx = l;
                break;
            }
        }
        if (layer_idx < 0) return;

        std::string dim_line;
        if (!std::getline(f, dim_line)) break;
        int w = 0, h = 0;
        if (sscanf(dim_line.c_str(), "%d,%d", &w, &h) != 2 || w <= 0 || h <= 0) return;

        if (!dims_set)
        {
            width  = w;
            height = h;
            for (int l = 0; l < LAYER_COUNT; ++l)
                tiles[l].assign(w * h, {-1, 0});
            dims_set = true;
        }

        for (int row = 0; row < h; ++row)
        {
            std::string row_line;
            if (!std::getline(f, row_line)) break;
            std::istringstream ss(row_line);
            std::string tok;
            for (int col = 0; col < w && std::getline(ss, tok, ','); ++col)
            {
                TileCell c = {-1, 0};
                size_t colon = tok.find(':');
                if (colon != std::string::npos)
                {
                    c.tile     = std::stoi(tok.substr(0, colon));
                    c.rotation = std::stoi(tok.substr(colon + 1));
                }
                else
                {
                    c.tile = std::stoi(tok);
                }
                tiles[layer_idx][row * w + col] = c;
            }
        }
    }
    while (std::getline(f, line) && !line.empty());

    loaded = dims_set;
}

void Level::Draw(bifrost::Camera2d camera) const
{
    for (int l = 0; l < LAYER_COUNT; ++l)
    {
        for (int row = 0; row < height; ++row)
        {
            for (int col = 0; col < width; ++col)
            {
                TileCell cell = tiles[l][row * width + col];
                if (cell.tile < 0) continue;
                float cx = col * TILE_DISPLAY + TILE_DISPLAY * 0.5f;
                float cy = (height - 1 - row) * TILE_DISPLAY + TILE_DISPLAY * 0.5f;
                bifrost::DrawRectangle(camera, {cx, cy}, {(float)TILE_DISPLAY, (float)TILE_DISPLAY},
                    cell.rotation * 90.0f,
                    texture, TileSourceOrigin(cell.tile), glm::vec2(TILE_SRC_SIZE));
            }
        }
    }
}
