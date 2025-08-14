#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cstdint>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

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
        glm::vec2 dimensions;
    };

    /*************
     * 
     *  BIFROST CORE
     * 
     * */

    Framebuffer GenFramebuffer(unsigned int width, unsigned int height, unsigned int texture_filter = GL_NEAREST, unsigned int texture_wrap = GL_CLAMP_TO_EDGE, unsigned int internal_format = GL_RGB);
    Shader GenShader(const char* vert_shader_file, const char* frag_shader_file);
    Shader GenShaderFromSource(const char* vert_shader_code, const char* frag_shader_code);
    Shader GenShader(const char* vert_shader_file, const char* geom_shader_file, const char* frag_shader_file);
    Shader GenShaderFromSource(const char* vert_shader_code, const char* geom_shader_code, const char* frag_shader_code);
    unsigned int GenVec4Vao(const float vertices[], const unsigned int count);
    unsigned int GenVec2Vao(const float vertices[], const unsigned int count);
    Texture LoadTexture(const char* filename);
    Texture LoadTexture(const unsigned char* data, const int texture_width, const int texture_height);
    Camera2d GenOrthogonalCamera2d(const glm::vec2 origin, const glm::vec2 dimensions);
    Camera2d GenUICamera(const int width, const int height);
    glm::ivec2 GetScreenSize(GLFWwindow& window);

    /*************
     * 
     *  BIFROST DRAWING
     * 
     * */

    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, glm::vec3 color);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, glm::vec4 color);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, float angle, glm::vec3 color);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, float angle, glm::vec4 color);
    
    // Textured
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, bifrost::Texture texture);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, bifrost::Texture texture, glm::vec3 color);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, bifrost::Texture texture, glm::vec4 color);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, float angle, bifrost::Texture texture);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, float angle, bifrost::Texture texture, glm::vec3 color);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, float angle, bifrost::Texture texture, glm::vec4 color);
    
    // Textured with UV
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, bifrost::Texture texture, glm::vec2 source_origin, glm::vec2 source_size);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, bifrost::Texture texture, glm::vec2 source_origin, glm::vec2 source_size, glm::vec3 color);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, bifrost::Texture texture, glm::vec2 source_origin, glm::vec2 source_size, glm::vec4 color);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, float angle, bifrost::Texture texture, glm::vec2 source_origin, glm::vec2 source_size);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, float angle, bifrost::Texture texture, glm::vec2 source_origin, glm::vec2 source_size, glm::vec3 color);
    void DrawRectangle(bifrost::Camera2d camera, glm::vec2 origin, glm::vec2 size, float angle, bifrost::Texture texture, glm::vec2 source_origin, glm::vec2 source_size, glm::vec4 color);

    // Text
    glm::vec2 DrawDebugText(Camera2d camera, glm::vec2 origin, float height, const char* format, ...);
    glm::vec2 DrawDebugText(Camera2d camera, glm::vec2 origin, float height, glm::vec3 color, const char* format, ...);
    glm::vec2 DrawDebugText(Camera2d camera, glm::vec2 origin, float height, glm::vec4 color, const char* format, ...);
    void EnableTextWrap(float width);
    void DisableTextWrap();

    // Lines
    void DrawLine(bifrost::Camera2d camera, glm::vec2 begin, glm::vec2 end, float width, glm::vec3 color);
    void DrawLine(bifrost::Camera2d camera, glm::vec2 begin, glm::vec2 end, float width, glm::vec4 color);

    static void InitializeDrawing();
    static glm::vec2 DrawDebugText_Internal(Camera2d camera, glm::vec2 origin, float height, glm::vec4 color, const char* format, va_list args);

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
    template <typename T> float Unlerp(T min, T max, T v);
    template <typename T> T Remap(T min_in, T max_in, T v, T min_out, T max_out);
    template <typename T> T Clamp(T min, T max, T v);
}