#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <bifrost/bifrost.h>
#include <bifrost/bifrost_input.h>
#include <bifrost/bifrost_dungeon.h>
#include <bifrost/bifrost_collision.h>

#include <stdio.h>

#include "level.h"

namespace
{
    void GlfwErrorCallback(int error, const char* description);
    void GlfwFramebufferSizeCallback(GLFWwindow* window, int width, int height);
}

#if _WIN32
int main();
int WinMain()
{
    return main();
}
#endif

int main()
{
    glfwSetErrorCallback(GlfwErrorCallback);

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(800, 600, "yakl - gamedev-starter", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, GlfwFramebufferSizeCallback);
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);

    glEnable(GL_MULTISAMPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    Level level("level.csv");
    if (!level.loaded)
        fprintf(stderr, "Warning: failed to load level.csv\n");

    float level_w = level.width  * 32.0f;
    float level_h = level.height * 32.0f;
    float offset_x = (800.0f - level_w) * 0.5f;
    float offset_y = (600.0f - level_h) * 0.5f;
    bifrost::Camera2d camera = bifrost::GenOrthogonalCamera2d(
        {-offset_x, -offset_y}, {800.0f - offset_x, 600.0f - offset_y});

    auto dungeon_texture = bifrost::GetDungeonTexture();

    // Player tile: col 4, row 7 from top (5th from left, 4th from bottom in 11-row sheet)
    // gl_row = (SHEET_ROWS - 1) - 7 = 3
    const glm::vec2 player_tile_src  = {4.0f * 17.0f, 3.0f * 17.0f};
    const glm::vec2 player_tile_size = {16.0f, 16.0f};
    const float     player_speed     = 160.0f;

    // Start at tile (col=2, row=2) — 3rd from left, 3rd from top
    const glm::vec2 spawn_pos = {
        2.0f * 32.0f + 16.0f,
        (level.height - 1 - 2) * 32.0f + 16.0f
    };
    glm::vec2 player_pos = spawn_pos;
    auto player_hitbox = bifrost::GenRectHitbox({0.0f, -12.0f}, {32.0f, 8.0f});

    bifrost::InputHandler input{};
    input.AddKeyBind(GLFW_KEY_ESCAPE, "quit");
    input.AddKeyBind(GLFW_KEY_W, "up");
    input.AddKeyBind(GLFW_KEY_S, "down");
    input.AddKeyBind(GLFW_KEY_A, "left");
    input.AddKeyBind(GLFW_KEY_D, "right");
    input.AddKeyBind(GLFW_KEY_R, "reset");
    input.BindOnPressed("quit",  [&window]() { glfwSetWindowShouldClose(window, GLFW_TRUE); });
    input.BindOnPressed("reset", [&player_pos, &spawn_pos]() { player_pos = spawn_pos; });

    double prev_time = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        double now = glfwGetTime();
        float dt = (float)(now - prev_time);
        prev_time = now;

        input.PollEvents(window);

        glm::vec2 dir = input.GetAxis("left", "right", "down", "up");
        player_pos += dir * player_speed * dt;

        for (const auto& hb : level.hitboxes)
        {
            bifrost::Hitbox at_origin = {{0.0f, 0.0f}, hb.offsets};
            auto result = bifrost::GetCollision(player_hitbox, player_pos, 0.0f, at_origin, hb.origin, 0.0f);
            if (result.hit)
                player_pos += result.penetration;
        }

        glClearColor(0.18f, 0.18f, 0.18f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (level.loaded)
            level.Draw(camera);

        bifrost::DrawRectangle(camera, player_pos, {32.0f, 32.0f},
            dungeon_texture, player_tile_src, player_tile_size);

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

namespace
{
void GlfwFramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void GlfwErrorCallback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}
}
