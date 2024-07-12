#ifndef BIFROST_FONTS_H
#define BIFROST_FONTS_H

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef BIFROST_IMPLEMENTATION
#define BIFROST_IMPLEMENTATION
#endif
#include "bifrost/bifrost.h"

#ifndef BIFROST_DRAWING_IMPLEMENTATION
#define BIFROST_DRAWING_IMPLEMENTATION
#endif
#include "bifrost/bifrost_drawing.h"

#include <stdarg.h>

namespace bifrost
{
    struct BitmapFont
    {
        Texture texture;
        unsigned int glyph_rows;
        unsigned int glyph_columns;
    };

    void InitializeFonts();
    void DrawDebugText(Camera2d camera, glm::vec2 origin, float height, const char* format, ...);
    void DrawDebugText(Camera2d camera, glm::vec2 origin, float height, glm::vec3 color, const char* format, ...);
    void DrawDebugText(Camera2d camera, glm::vec2 origin, float height, glm::vec4 color, const char* format, ...);

    static void DrawDebugText_Internal(Camera2d camera, glm::vec2 origin, float height, glm::vec4 color, const char* format, va_list args);

}

static bifrost::BitmapFont debug_font;

#ifdef BIFROST_FONTS_IMPLEMENTATION

namespace bifrost
{
    void InitializeFonts()
    {
        bifrost::InitializeDrawing();

        debug_font = {};
        debug_font.texture = LoadTexture("debug-font.png");
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
        char buffer[1000];
        vsprintf_s(buffer, 1000, format, args);

        int i = 0;
        while(buffer[i] != '\0' && i < 1000)
        {
            int index = int(buffer[i]) - 32;
            int x = index % 16;
            int y = index / 16;
            DrawRectangle(camera, origin, glm::vec2(height / 12.0f * 7.0f, height), debug_font.texture, glm::vec2(x * 7, y * 12), glm::vec2(7, 12), color);
            float char_width = 6.0f;
            if (buffer[i] == '.' || buffer[i] == '!' || buffer[i] == ',' || buffer[i] == '\'' || buffer[i] == 'i' || buffer[i] == 'l' || buffer[i] == 'j')
                char_width = 3.0f;
            else if (buffer[i] == '@')
                char_width = 7.0f;
            origin += glm::vec2(height / 12.0f * char_width, 0.0f);
            i++;
        }
    }
}

#undef BIFROST_FONTS_IMPLEMENTATION

#endif // BIFROST_FONTS_IMPLEMENTATION

#endif // BIFROST_FONTS_H