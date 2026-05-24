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

    GLFWwindow* window = glfwCreateWindow(800, 600, "basic example", NULL, NULL);
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

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw a square in the center of the screen
        glm::vec2 center = camera.dimensions / 2.0f;
        bifrost::DrawRectangle(camera, center, glm::vec2(150.0f, 150.0f), glm::vec4(0.2f, 0.6f, 1.0f, 1.0f));

        // Draw some debug text
        bifrost::DrawDebugText(camera, glm::vec2(10.0f, camera.dimensions.y - 32.0f), 24.0f, "basic bifrost example");
        bifrost::DrawDebugText(camera, glm::vec2(10.0f, 10.0f), 24.0f, glm::vec3(0.8f, 0.8f, 0.8f), "press ESC to quit");

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
