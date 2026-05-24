#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <bifrost/bifrost.h>
#include <bifrost/bifrost_input.h>
#include <bifrost/bifrost_dungeon.h>

// The dungeon texture is a 12x11 grid of 16x16 tiles with 1px gaps (17px stride).
static constexpr int   TILE_COLS   = 12;
static constexpr int   TILE_ROWS   = 11;
static constexpr float TILE_SRC    = 16.0f;
static constexpr float TILE_STRIDE = 17.0f;
static constexpr float TILE_DRAW   = 48.0f;  // on-screen size per tile in the sheet view

namespace
{
    bifrost::Camera2d camera{};

    void FramebufferSizeCallback(GLFWwindow*, int width, int height)
    {
        glViewport(0, 0, width, height);
        camera = bifrost::GenUICamera(width, height);
    }

    glm::vec2 TileUV(int col, int row)
    {
        return glm::vec2(col * TILE_STRIDE, row * TILE_STRIDE);
    }
}

int main()
{
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(900, 600, "dungeon example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    auto screen_size = bifrost::GetScreenSize(*window);
    glViewport(0, 0, screen_size.x, screen_size.y);
    camera = bifrost::GenUICamera(screen_size.x, screen_size.y);

    auto dungeon = bifrost::GetDungeonTexture();

    bifrost::InputHandler input{};
    input.AddKeyBind(GLFW_KEY_RIGHT, "right");
    input.AddKeyBind(GLFW_KEY_LEFT,  "left");
    input.AddKeyBind(GLFW_KEY_UP,    "up");
    input.AddKeyBind(GLFW_KEY_DOWN,  "down");
    input.AddKeyBind(GLFW_KEY_ESCAPE, "quit");
    input.BindOnPressed("quit", [&window]() { glfwSetWindowShouldClose(window, GLFW_TRUE); });

    int sel_col = 0;
    int sel_row = 0;
    input.BindOnPressed("right", [&sel_col]() { sel_col = (sel_col + 1) % TILE_COLS; });
    input.BindOnPressed("left",  [&sel_col]() { sel_col = (sel_col - 1 + TILE_COLS) % TILE_COLS; });
    input.BindOnPressed("down",    [&sel_row]() { sel_row = (sel_row - 1 + TILE_ROWS) % TILE_ROWS; });
    input.BindOnPressed("up",  [&sel_row]() { sel_row = (sel_row + 1) % TILE_ROWS; });

    while (!glfwWindowShouldClose(window))
    {
        input.PollEvents(window);

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // --- Sheet view (bottom-left) ---
        // Draw the full atlas, tile by tile, so each tile is visible at TILE_DRAW size.
        const float sheet_x = 16.0f;
        const float sheet_y = 16.0f;
        const float half    = TILE_DRAW / 2.0f;

        for (int row = 0; row < TILE_ROWS; row++)
        {
            for (int col = 0; col < TILE_COLS; col++)
            {
                glm::vec2 pos = glm::vec2(sheet_x + col * TILE_DRAW + half,
                                          sheet_y + row * TILE_DRAW + half);
                bifrost::DrawRectangle(camera, pos, glm::vec2(TILE_DRAW), dungeon,
                                       TileUV(col, row), glm::vec2(TILE_SRC));
            }
        }

        // Highlight the selected tile in the sheet
        glm::vec2 sel_pos = glm::vec2(sheet_x + sel_col * TILE_DRAW + half, sheet_y + sel_row * TILE_DRAW + half);
        float h = half;
        bifrost::DrawLine(camera, sel_pos + glm::vec2(-h, -h), sel_pos + glm::vec2( h, -h), 2.0f, glm::vec3(1.0f, 1.0f, 0.0f));
        bifrost::DrawLine(camera, sel_pos + glm::vec2( h, -h), sel_pos + glm::vec2( h,  h), 2.0f, glm::vec3(1.0f, 1.0f, 0.0f));
        bifrost::DrawLine(camera, sel_pos + glm::vec2( h,  h), sel_pos + glm::vec2(-h,  h), 2.0f, glm::vec3(1.0f, 1.0f, 0.0f));
        bifrost::DrawLine(camera, sel_pos + glm::vec2(-h,  h), sel_pos + glm::vec2(-h, -h), 2.0f, glm::vec3(1.0f, 1.0f, 0.0f));

        // --- Preview (right side) ---
        // Draw the selected tile large so you can see the detail.
        const float preview_size = 192.0f;
        glm::vec2 preview_pos = glm::vec2(
            sheet_x + TILE_COLS * TILE_DRAW + 32.0f + preview_size / 2.0f,
            sheet_y + preview_size / 2.0f
        );
        bifrost::DrawRectangle(camera, preview_pos, glm::vec2(preview_size), dungeon, TileUV(sel_col, sel_row), glm::vec2(TILE_SRC));

        // --- Labels ---
        float text_y = sheet_y + TILE_ROWS * TILE_DRAW + 12.0f;
        bifrost::DrawDebugText(camera, glm::vec2(sheet_x, text_y), 12.0f, "arrow keys to select tile   col:%d row:%d", sel_col, sel_row);
        bifrost::DrawDebugText(camera, glm::vec2(sheet_x, text_y + 24.0f), 12.0f, glm::vec3(0.6f, 0.6f, 0.6f), "uv origin: (%.0f, %.0f)  size: %.0f", TileUV(sel_col, sel_row).x, TileUV(sel_col, sel_row).y, TILE_SRC);

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
