#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <bifrost/bifrost.h>
#include <bifrost/bifrost_input.h>

namespace
{
    bifrost::Camera2d camera{};

    void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
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

    GLFWwindow* window = glfwCreateWindow(800, 600, "input example", NULL, NULL);
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

    const float speed = 200.0f;
    glm::vec2 pos = camera.dimensions / 2.0f;

    float last_time = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        float now = glfwGetTime();
        float dt = now - last_time;
        last_time = now;

        input.PollEvents(window);

        glm::vec2 move = input.GetAxis("left", "right", "down", "up");
        pos += move * speed * dt;

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        bifrost::DrawRectangle(camera, pos, glm::vec2(60.0f), glm::vec4(0.2f, 0.6f, 1.0f, 1.0f));

        bifrost::DrawDebugText(camera, glm::vec2(10.0f, camera.dimensions.y - 32.0f), 24.0f, "input example -- WASD to move");
        bifrost::DrawDebugText(camera, glm::vec2(10.0f, 10.0f), 24.0f, glm::vec3(0.8f, 0.8f, 0.8f), "pos: (%.0f, %.0f)", pos.x, pos.y);

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
