#pragma once

#include <glad/gl.h>

namespace bifrost
{
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

    Framebuffer GenFramebuffer(unsigned int width, unsigned int height, unsigned int texture_filter = GL_NEAREST, unsigned int texture_wrap = GL_CLAMP_TO_EDGE, unsigned int internal_format = GL_RGB);
    Shader GenShader(const char* vert_shader_file, const char* frag_shader_file);
    unsigned int GenVec4Vao(const float vertices[], unsigned int count);
    unsigned int GenVec2Vao(const float vertices[], unsigned int count);
}

#ifdef BIFROST_IMPLEMENTATION

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

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

    unsigned int GenVec4Vao(const float vertices[], unsigned int count)
    {
        unsigned int vao;
        unsigned int vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * count, vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);

        glBindVertexArray(0);

        return vao;
    }

    unsigned int GenVec2Vao(const float vertices[], unsigned int count)
    {
        unsigned int vao;
        unsigned int vbo;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * count, vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);

        glBindVertexArray(0);

        return vao;
    }
}
#endif