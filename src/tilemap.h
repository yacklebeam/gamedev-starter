#pragma once

#include "bifrost.h"

#include <glm/glm.hpp>

class Tilemap
{
public:
    Tilemap(const char* filename, unsigned int tile_count_x, unsigned int tile_count_y, const unsigned int map[], unsigned int map_width, unsigned int map_height);
    ~Tilemap();

    void Render(glm::mat4 view_proj);
    void SetTileId(unsigned int index, unsigned int tile_id);
private:
    unsigned int texture_id_;
    unsigned int vao_;
    unsigned int vbo_offsets_;
    glm::vec2 uv_size_;
    bifrost::Shader shader_;

    unsigned int map_width_;
    unsigned int map_height_;
    unsigned int tile_count_x_;
    unsigned int tile_count_y_;


    glm::mat4 *models_;
    glm::vec2 *offsets_;
};