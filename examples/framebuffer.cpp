#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <bifrost/bifrost.h>

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

    GLFWwindow* window = glfwCreateWindow(800, 600, "framebuffer example", NULL, NULL);
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

    auto framebuffer = bifrost::GenFramebuffer(screen_size.x, screen_size.y);

    auto clear_color = glm::vec4(0.1f, 0.1f, 0.15f, 1.0f);

    while (!glfwWindowShouldClose(window))
    {
        screen_size = camera.dimensions;
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        glm::vec2 center = camera.dimensions * 0.5f;

        {
            BIFROST_ACTIVATE_FRAMEBUFFER(framebuffer);

            glEnable(GL_DEPTH_TEST);
            glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            bifrost::DrawRectangle(camera, center, glm::vec2(150.0f, 150.0f), 15.0f, glm::vec3(1.0f, 1.0f, 1.0f));
        }

        auto scene = framebuffer.texture;

        // Draw the captured texture to screen with colour tints.
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        auto half = glm::vec2(screen_size) * 0.5f;
        bifrost::DrawRectangle(camera, center + glm::vec2(-center.x * 0.5f,  center.y * 0.5f), half, scene, glm::vec3(1.0f, 0.0f, 0.0f));
        bifrost::DrawRectangle(camera, center + glm::vec2( center.x * 0.5f,  center.y * 0.5f), half, scene, glm::vec3(0.0f, 1.0f, 0.0f));
        bifrost::DrawRectangle(camera, center + glm::vec2(-center.x * 0.5f, -center.y * 0.5f), half, scene, glm::vec3(0.0f, 0.0f, 1.0f));
        bifrost::DrawRectangle(camera, center + glm::vec2( center.x * 0.5f, -center.y * 0.5f), half, scene);

        bifrost::DrawDebugText(camera, glm::vec2(10.0f, camera.dimensions.y - 32.0f), 24.0f, "framebuffer example");
        bifrost::DrawDebugText(camera, glm::vec2(10.0f, 10.0f), 24.0f, glm::vec3(0.8f, 0.8f, 0.8f), "press ESC to quit");

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
