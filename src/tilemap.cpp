#include "tilemap.h"

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

Tilemap::Tilemap(const char* filename, unsigned int tile_count_x, unsigned int tile_count_y, const unsigned int map[], unsigned int map_width, unsigned int map_height)
{
    map_width_ = map_width;
    map_height_ = map_height;
    tile_count_x_ = tile_count_x;
    tile_count_y_ = tile_count_y;

    int texture_width, texture_height, texture_channel_count;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filename, &texture_width, &texture_height, &texture_channel_count, 0); 
    glGenTextures(1, &texture_id_);
    glBindTexture(GL_TEXTURE_2D, texture_id_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_width, texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    shader_ = bifrost::GenShader("tiles.vert", "tiles.frag");

    uv_size_ = glm::vec2(1.0f / tile_count_x, 1.0f / tile_count_y);

    const float vertices[] = {
        // positions, uvs
        -0.5f,  0.5f, 0.0f,       uv_size_.y,
        -0.5f, -0.5f, 0.0f,       0.0f,
         0.5f, -0.5f, uv_size_.x, 0.0f,
         0.5f,  0.5f, uv_size_.x, uv_size_.y,
    };

    const unsigned int indices[] = {
        0, 1, 2,   // first triangle
        2, 3, 0    // second triangle
    };

    offsets_ = new glm::vec2[map_width_ * map_height_];
    models_ = new glm::mat4[map_width_ * map_height_];
    for (unsigned int i = 0; i < map_width_ * map_height_; ++i)
    {
        unsigned int tile = map[i];

        models_[i] = glm::translate(glm::mat4(1.0f), glm::vec3((float)(i % map_width_) + 0.5f, (float)(map_height - (i / map_width_)) - 0.5f, 0.0f));
        offsets_[i] = glm::vec2((float)(tile % tile_count_x), (float)(tile / tile_count_x));
    }

    unsigned int vbo;
    unsigned int ebo;
    unsigned int vbo_models;

    glGenVertexArrays(1, &vao_);
    
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenBuffers(1, &vbo_offsets_);
    glGenBuffers(1, &vbo_models);

    glBindVertexArray(vao_);

    // bind buffer data for mesh/quad
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);

        glEnableVertexAttribArray(1); 
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));
    }

        // bind buffer data for uv offsets
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_offsets_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * map_width_ * map_height_, offsets_, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(2); 
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);
        glVertexAttribDivisor(2, 1);
    }

    // bind buffer data for model matrices
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo_models);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * map_width_ * map_height_, models_, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(3); 
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
        glEnableVertexAttribArray(4); 
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(1 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(5); 
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(6); 
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));
    
        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);
    }

    glBindVertexArray(0);
}

Tilemap::~Tilemap()
{
    glDeleteProgram(shader_.id);
}

void Tilemap::SetTileId(unsigned int index, unsigned int tile_id)
{
    offsets_[index] = glm::vec2((float)(tile_id % tile_count_x_), (float)(tile_id / tile_count_x_));

    glBindBuffer(GL_ARRAY_BUFFER, vbo_offsets_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * map_width_ * map_height_, offsets_, GL_DYNAMIC_DRAW);
}

void Tilemap::Render(glm::mat4 view_proj)
{
    glDisable(GL_MULTISAMPLE);  
    glBindTexture(GL_TEXTURE_2D, texture_id_);
    glBindVertexArray(vao_);

    glUseProgram(shader_.id);
    glUniformMatrix4fv(glGetUniformLocation(shader_.id, "vp"), 1, GL_FALSE, glm::value_ptr(view_proj));
    glUniform4fv(glGetUniformLocation(shader_.id, "tile_uv_size"), 1, glm::value_ptr(uv_size_));
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, map_width_ * map_height_);
    
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_MULTISAMPLE);
}