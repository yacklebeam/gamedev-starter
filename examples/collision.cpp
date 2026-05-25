#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <bifrost/bifrost.h>
#include <bifrost/bifrost_input.h>
#include <bifrost/bifrost_collision.h>

namespace
{
    bifrost::Camera2d camera{};

    void FramebufferSizeCallback(GLFWwindow*, int width, int height)
    {
        glViewport(0, 0, width, height);
        camera = bifrost::GenUICamera(width, height);
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

    GLFWwindow* window = glfwCreateWindow(800, 600, "collision example", NULL, NULL);
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

    bifrost::InputHandler input{};
    input.AddKeyBind(GLFW_KEY_W, "up");
    input.AddKeyBind(GLFW_KEY_S, "down");
    input.AddKeyBind(GLFW_KEY_A, "left");
    input.AddKeyBind(GLFW_KEY_D, "right");
    input.AddKeyBind(GLFW_KEY_ESCAPE, "quit");
    input.BindOnPressed("quit", [&window]() { glfwSetWindowShouldClose(window, GLFW_TRUE); });

    // Player: moves with WASD, pushed out of the wall on overlap
    glm::vec2 player_size = glm::vec2(40.0f);
    glm::vec2 player_pos  = camera.dimensions / 2.0f - glm::vec2(80.0f, 0.0f);
    auto player_hitbox    = bifrost::GenRectHitbox(player_size);

    // Static wall in the center of the screen
    glm::vec2 wall_size = glm::vec2(30.0f, 160.0f);
    glm::vec2 wall_pos  = camera.dimensions / 2.0f;
    auto wall_hitbox    = bifrost::GenRectHitbox(wall_size);

    const float speed = 220.0f;
    float last_time = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        float now = glfwGetTime();
        float dt  = now - last_time;
        last_time = now;

        input.PollEvents(window);

        glm::vec2 move = input.GetAxis("left", "right", "down", "up");
        player_pos += move * speed * dt;

        // Resolve overlap: push the player out by the penetration vector
        auto result = bifrost::GetCollision(player_hitbox, player_pos, 0.0f,
                                            wall_hitbox,   wall_pos,   0.0f);
        if (result.hit)
            player_pos += result.penetration;

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw wall
        bifrost::DrawRectangle(camera, wall_pos, wall_size, glm::vec4(0.5f, 0.5f, 0.55f, 1.0f));
        bifrost::DrawHitbox(camera, wall_hitbox, wall_pos, 0.0f, glm::vec3(0.4f, 0.4f, 0.5f));

        // Draw player, tinted red while colliding
        auto player_color = result.hit
            ? glm::vec4(1.0f, 0.3f, 0.3f, 1.0f)
            : glm::vec4(0.2f, 0.6f, 1.0f, 1.0f);
        bifrost::DrawRectangle(camera, player_pos, player_size, player_color);
        bifrost::DrawHitbox(camera, player_hitbox, player_pos, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));

        bifrost::DrawDebugText(camera, glm::vec2(10.0f, camera.dimensions.y - 32.0f), 24.0f,
                               "collision example -- WASD to move");
        bifrost::DrawDebugText(camera, glm::vec2(10.0f, 10.0f), 24.0f,
                               glm::vec3(0.8f, 0.8f, 0.8f),
                               result.hit ? "HIT  penetration: (%.1f, %.1f)" : "no collision",
                               result.penetration.x, result.penetration.y);

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
