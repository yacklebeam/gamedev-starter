#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <bifrost/bifrost.h>

#if _WIN32
int main();
int WinMain()
{
    return main();
}
#endif

int main()
{
    bifrost::EnableDefaultRendererOptions();
    auto window = bifrost::Initialize(800, 600, "yakl - gamedev-starter");
    if (!window) return -1;

    auto screen_size = bifrost::GetScreenSize(window);
    glViewport(0, 0, screen_size.x, screen_size.y);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    auto camera = bifrost::GenUICamera(screen_size.x, screen_size.y);

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Draw game
    	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        bifrost::DrawDebugText(camera, camera.dimensions * 0.5f, 24.0f, glm::vec3(1.0f), "HELLO!");

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}