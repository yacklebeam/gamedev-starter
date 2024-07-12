#ifndef BIFROST_DRAWING_H
#define BIFROST_DRAWING_H

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef BIFROST_IMPLEMENTATION
#define BIFROST_IMPLEMENTATION
#endif
#include "bifrost/bifrost.h"

static glm::mat4 screen_space_projection;
static const float quad_vertices[] = {
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,

    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
};
static unsigned int quad_vao;
static unsigned int uv_quad_vao;
static unsigned int uv_quad_vbo;
static bifrost::Shader basic_shader;
static bifrost::Shader texture_shader;
static bifrost::Shader uv_texture_shader;

static char* basic_vs =
R"(
#version 450 core
layout (location = 0) in vec2 position;
out vec2 texture_coords;
uniform mat4 mvp;
void main()
{
    gl_Position = mvp * vec4(position, 0.0, 1.0);
    texture_coords = position;
}
)";

static char* uv_vs =
R"(
#version 450 core
layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;
out vec2 texture_coords;
uniform mat4 mvp;
void main()
{
    gl_Position = mvp * vec4(position, 0.0, 1.0);
    texture_coords = uv;
}
)";

static char* basic_fs =
R"(
#version 450 core
uniform vec4 color;
out vec4 fragment_color;
void main()
{
    fragment_color = color;
}
)";

static char* textured_fs =
R"(
#version 450 core
in vec2 texture_coords;
uniform vec4 color;
uniform sampler2D tex;
out vec4 fragment_color;
void main()
{
    fragment_color = texture(tex, texture_coords) * vec4(color);
}
)";

namespace bifrost
{
    void InitializeDrawing();
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, glm::vec3 color);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, glm::vec4 color);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, bifrost::Texture texture);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, bifrost::Texture texture, glm::vec3 color);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, bifrost::Texture texture, glm::vec4 color);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, bifrost::Texture texture, glm::vec2 source_origin, glm::vec2 source_size);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, bifrost::Texture texture, glm::vec2 source_origin, glm::vec2 source_size, glm::vec3 color);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, bifrost::Texture texture, glm::vec2 source_origin, glm::vec2 source_size, glm::vec4 color);
}

#ifdef BIFROST_DRAWING_IMPLEMENTATION

namespace bifrost
{
    void InitializeDrawing()
    {
        quad_vao = bifrost::GenVec2Vao(quad_vertices, 6);

        uv_quad_vao = bifrost::GenVec2Vao(quad_vertices, 6);
        glBindVertexArray(uv_quad_vao);
        glGenBuffers(1, &uv_quad_vbo);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uv_quad_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 0, nullptr, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);
        glBindVertexArray(0);

        basic_shader = bifrost::GenShaderFromSource(basic_vs, basic_fs);
        texture_shader = bifrost::GenShaderFromSource(basic_vs, textured_fs);
        uv_texture_shader = bifrost::GenShaderFromSource(uv_vs, textured_fs);
    }

    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, glm::vec3 color)
    {
        DrawRectangle(camera, origin, size, glm::vec4(color, 1.0f));
    }

    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, glm::vec4 color)
    {
        auto model = glm::translate(glm::mat4(1.0f), glm::vec3(origin.x, origin.y, 0.0f));
        model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));

        glBindVertexArray(quad_vao);
        
        glUseProgram(basic_shader.id);
        glUniformMatrix4fv(glGetUniformLocation(basic_shader.id, "mvp"), 1, GL_FALSE, glm::value_ptr(camera.projection * model));
        glUniform4fv(glGetUniformLocation(basic_shader.id, "color"), 1, glm::value_ptr(color));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glBindVertexArray(0);
    }

    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, bifrost::Texture texture)
    {
        DrawRectangle(camera, origin, size, texture, glm::vec4(1.0f));
    }

    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, bifrost::Texture texture, glm::vec3 color)
    {
        DrawRectangle(camera, origin, size, texture, glm::vec4(color, 1.0f));
    }

    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, bifrost::Texture texture, glm::vec4 color)
    {
        auto model = glm::translate(glm::mat4(1.0f), glm::vec3(origin.x, origin.y, 0.0f));
        model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));

        glDisable(GL_DEPTH_TEST);
        glBindVertexArray(quad_vao);
        glBindTexture(GL_TEXTURE_2D, texture.id);
        
        glUseProgram(texture_shader.id);
        glUniformMatrix4fv(glGetUniformLocation(texture_shader.id, "mvp"), 1, GL_FALSE, glm::value_ptr(camera.projection * model));
        glUniform4fv(glGetUniformLocation(texture_shader.id, "color"), 1, glm::value_ptr(color));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glBindTexture(GL_TEXTURE_2D, 0);        
        glBindVertexArray(0);
    }

    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, bifrost::Texture texture, glm::vec2 source_origin, glm::vec2 source_size)
    {
        DrawRectangle(camera, origin, size, texture, source_origin, source_size, glm::vec4(1.0f));
    }

    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, bifrost::Texture texture, glm::vec2 source_origin, glm::vec2 source_size, glm::vec3 color)
    {
        DrawRectangle(camera, origin, size, texture, source_origin, source_size, glm::vec4(color, 1.0f));
    }

    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, bifrost::Texture texture, glm::vec2 source_origin, glm::vec2 source_size, glm::vec4 color)
    {
        glm::vec2 uv_start = glm::vec2(source_origin.x / (float)texture.width, source_origin.y / (float)texture.height);
        glm::vec2 uv_end = uv_start + glm::vec2(source_size.x / (float)texture.width, source_size.y / (float)texture.height);
        // calculate UVs
        float uvs[] = {
            uv_start.x, uv_end.y,
            uv_start.x, uv_start.y,
            uv_end.x,   uv_start.y,

            uv_end.x,   uv_start.y,
            uv_end.x,   uv_end.y,
            uv_start.x, uv_end.y,
        };
        glBindBuffer(GL_ARRAY_BUFFER, uv_quad_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, uvs, GL_DYNAMIC_DRAW);

        auto model = glm::translate(glm::mat4(1.0f), glm::vec3(origin.x, origin.y, 0.0f));
        model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));

        glDisable(GL_DEPTH_TEST);
        glBindVertexArray(uv_quad_vao);
        glBindTexture(GL_TEXTURE_2D, texture.id);
        
        glUseProgram(uv_texture_shader.id);
        glUniformMatrix4fv(glGetUniformLocation(uv_texture_shader.id, "mvp"), 1, GL_FALSE, glm::value_ptr(camera.projection * model));
        glUniform4fv(glGetUniformLocation(uv_texture_shader.id, "color"), 1, glm::value_ptr(color));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        glBindTexture(GL_TEXTURE_2D, 0);        
        glBindVertexArray(0);
    }

}

#undef BIFROST_DRAWING_IMPLEMENTATION

#endif //BIFROST_DRAWING_IMPLEMENTATION

#endif //BIFROST_DRAWING_H