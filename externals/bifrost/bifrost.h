#ifndef BIFROST_H
#define BIFROST_H

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>
#undef GLAD_GL_IMPLEMENTATION

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "stb/stb_image.h"

#include <cstdint>

namespace bifrost
{
    /*************
     * 
     *  BIFROST STRUCTS
     * 
     * */

    struct Framebuffer
    {
        unsigned int id;
        unsigned int texture_id;
        unsigned int width;
        unsigned int height;
    };

    struct Shader
    {
        unsigned int id;
    };

    struct Texture
    {
        unsigned int id;
        unsigned int width;
        unsigned int height;        
    };

    struct Camera2d
    {
        glm::mat4 projection;
    };

    /*************
     * 
     *  BIFROST CORE
     * 
     * */

    Framebuffer GenFramebuffer(unsigned int width, unsigned int height, unsigned int texture_filter = GL_NEAREST, unsigned int texture_wrap = GL_CLAMP_TO_EDGE, unsigned int internal_format = GL_RGB);
    Shader GenShader(const char* vert_shader_file, const char* frag_shader_file);
    Shader GenShaderFromSource(char* vert_shader_code, char* frag_shader_code);
    unsigned int GenVec4Vao(const float vertices[], unsigned int count);
    unsigned int GenVec2Vao(const float vertices[], unsigned int count);
    Texture LoadTexture(const char* filename);
    Camera2d GenOrthogonalCamera2d(glm::vec2 origin, glm::vec2 dimensions);

    /*************
     * 
     *  BIFROST DRAWING
     * 
     * */

    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, glm::vec3 color);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, glm::vec4 color);
    
    // Textured
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, bifrost::Texture texture);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, bifrost::Texture texture, glm::vec3 color);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, bifrost::Texture texture, glm::vec4 color);
    
    // Textured with UV
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, bifrost::Texture texture, glm::vec2 source_origin, glm::vec2 source_size);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, bifrost::Texture texture, glm::vec2 source_origin, glm::vec2 source_size, glm::vec3 color);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, bifrost::Texture texture, glm::vec2 source_origin, glm::vec2 source_size, glm::vec4 color);

    // Text
    void DrawDebugText(Camera2d camera, glm::vec2 origin, float height, const char* format, ...);
    void DrawDebugText(Camera2d camera, glm::vec2 origin, float height, glm::vec3 color, const char* format, ...);
    void DrawDebugText(Camera2d camera, glm::vec2 origin, float height, glm::vec4 color, const char* format, ...);

    static void InitializeDrawing();
    static void DrawDebugText_Internal(Camera2d camera, glm::vec2 origin, float height, glm::vec4 color, const char* format, va_list args);

    /*************
     * 
     *  BIFROST RANDOM
     * 
     * */

    void Seed(uint32_t seed);
    uint32_t Random();
    float RandomFloat();

    /*************
     * 
     *  BIFROST MATH
     * 
     * */

    template <typename T> T Lerp(T min, T max, float t);
    //float Unlerp(float min, float max, float v);
    //float Remap(float min_in, float max_in, float v, float min_out, float max_out);
    //float Clamp(float min, float max, float v);
}

#ifdef BIFROST_IMPLEMENTATION

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

static bool initialized = false;

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

static bifrost::Texture debug_font_texture;

static uint32_t seed = 0;

namespace bifrost
{
    Framebuffer GenFramebuffer(unsigned int width, unsigned int height, unsigned int texture_filter, unsigned int texture_wrap, unsigned int internal_format)
    {
        Framebuffer buffer = {};

        buffer.width = width;
        buffer.height = height;

        glGenFramebuffers(1, &buffer.id);
        glBindFramebuffer(GL_FRAMEBUFFER, buffer.id);

        glGenTextures(1, &buffer.texture_id);
        glBindTexture(GL_TEXTURE_2D, buffer.texture_id);

        glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture_wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture_wrap);
        glBindTexture(GL_TEXTURE_2D, 0);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer.texture_id, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return buffer;
    }

    Shader GenShader(const char* vertex_shader, const char* fragment_shader)
    {
        Shader shader = {};

        unsigned int vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
        unsigned int geometry_shader_id = glCreateShader(GL_GEOMETRY_SHADER);
        unsigned int fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

        shader.id = glCreateProgram();

        if (strlen(vertex_shader))
        {
            std::string shader_code;
            std::ifstream shader_file;

            shader_file.open(vertex_shader);
            std::stringstream shader_stream;
            shader_stream << shader_file.rdbuf();
            shader_code = shader_stream.str();

            const char* code = shader_code.c_str();

            glShaderSource(vertex_shader_id, 1, &code, NULL);
            glCompileShader(vertex_shader_id);
            glAttachShader(shader.id, vertex_shader_id);
        }

        if (strlen(fragment_shader))
        {
            std::string shader_code;
            std::ifstream shader_file;

            shader_file.open(fragment_shader);
            std::stringstream shader_stream;
            shader_stream << shader_file.rdbuf();
            shader_code = shader_stream.str();

            const char* code = shader_code.c_str();

            glShaderSource(fragment_shader_id, 1, &code, NULL);
            glCompileShader(fragment_shader_id);
            glAttachShader(shader.id, fragment_shader_id);
        }

        glLinkProgram(shader.id);

        glDeleteShader(vertex_shader_id);
        glDeleteShader(geometry_shader_id);
        glDeleteShader(fragment_shader_id);

        return shader;
    }

    Shader GenShaderFromSource(char* vertex_shader, char* fragment_shader)
    {
        Shader shader = {};

        unsigned int vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
        unsigned int geometry_shader_id = glCreateShader(GL_GEOMETRY_SHADER);
        unsigned int fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

        shader.id = glCreateProgram();

        if (strlen(vertex_shader))
        {
            glShaderSource(vertex_shader_id, 1, &vertex_shader, NULL);
            glCompileShader(vertex_shader_id);
            glAttachShader(shader.id, vertex_shader_id);
        }

        if (strlen(fragment_shader))
        {
            glShaderSource(fragment_shader_id, 1, &fragment_shader, NULL);
            glCompileShader(fragment_shader_id);
            glAttachShader(shader.id, fragment_shader_id);
        }

        glLinkProgram(shader.id);

        glDeleteShader(vertex_shader_id);
        glDeleteShader(geometry_shader_id);
        glDeleteShader(fragment_shader_id);

        return shader;
    }

    unsigned int GenVec4Vao(const float vertices[], unsigned int vertex_count)
    {
        unsigned int vao;
        unsigned int vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertex_count * 4, vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);

        glBindVertexArray(0);

        return vao;
    }

    unsigned int GenVec2Vao(const float vertices[], unsigned int vertex_count)
    {
        unsigned int vao;
        unsigned int vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertex_count * 2, vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);

        glBindVertexArray(0);

        return vao;
    }

    Texture LoadTexture(const char* filename)
    {
        Texture texture = {};

        int texture_width, texture_height, texture_channel_count;
        stbi_set_flip_vertically_on_load(true);
        unsigned char *data = stbi_load(filename, &texture_width, &texture_height, &texture_channel_count, 0); 
        glGenTextures(1, &texture.id);
        glBindTexture(GL_TEXTURE_2D, texture.id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_width, texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);

        texture.width = texture_width;
        texture.height = texture_height;

        return texture;
    }

    Camera2d GenOrthogonalCamera2d(glm::vec2 origin, glm::vec2 dimensions)
    {
        Camera2d camera = {};

        camera.projection = glm::ortho(origin.x, dimensions.x, origin.y, dimensions.y, -1.0f, 1.0f);

        return camera;
    }

    static void InitializeDrawing()
    {
        if (initialized)
            return;

        initialized = true;

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

        debug_font_texture = LoadTexture("debug-font.png");
    }

    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, glm::vec3 color)
    {
        DrawRectangle(camera, origin, size, glm::vec4(color, 1.0f));
    }

    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, glm::vec4 color)
    {
        InitializeDrawing();
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
        InitializeDrawing();
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
        InitializeDrawing();
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

    void DrawDebugText(Camera2d camera, glm::vec2 origin, float height, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        DrawDebugText_Internal(camera, origin, height, glm::vec4(1.0f), format, args);
        va_end(args);
    }

    void DrawDebugText(Camera2d camera, glm::vec2 origin, float height, glm::vec3 color, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        DrawDebugText_Internal(camera, origin, height, glm::vec4(color, 1.0f), format, args);
        va_end(args);
    }

    void DrawDebugText(Camera2d camera, glm::vec2 origin, float height, glm::vec4 color, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        DrawDebugText_Internal(camera, origin, height, color, format, args);
        va_end(args);
    }

    static void DrawDebugText_Internal(Camera2d camera, glm::vec2 origin, float height, glm::vec4 color, const char* format, va_list args)
    {
        InitializeDrawing();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        char buffer[1000];
        vsprintf_s(buffer, 1000, format, args);

        int i = 0;
        while(buffer[i] != '\0' && i < 1000)
        {
            int index = int(buffer[i]) - 32;
            int x = index % 16;
            int y = index / 16;
            DrawRectangle(camera, origin, glm::vec2(height / 12.0f * 7.0f, height), debug_font_texture, glm::vec2(x * 7, y * 12), glm::vec2(7, 12), color);
            float char_width = 6.0f;
            if (buffer[i] == '.' || buffer[i] == '!' || buffer[i] == ',' || buffer[i] == '\'' || buffer[i] == 'l')
                char_width = 3.0f;
            else if (buffer[i] == '@')
                char_width = 7.0f;
            else if (buffer[i] == 'i' || buffer[i] == 'j')
                char_width = 2.0f;
            origin += glm::vec2(height / 12.0f * char_width, 0.0f);
            i++;
        }
    }

    void Seed(uint32_t s)
    {
        if (s == 0)
            s = 0x45;
        seed = s;
    }

    uint32_t Random()
    {
        seed = (uint32_t)(((uint64_t)seed * 48271) % 0x7fffffff);
        return seed;
    }

    float RandomFloat()
    {
        uint32_t r = Random();
        return (float)r / (float)0x7fffffff;
    }
    
}

#undef BIFROST_IMPLEMENTATION

#endif // BIFROST_IMPLEMENTATION

#endif // BIFROST_H