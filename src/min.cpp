#include <glad/gl.h>
#include <GLFW/glfw3.h>

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
        // Draw game
    	glClearColor(0.45f, 0.55f, 0.6f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        bifrost::DrawDebugText(camera, glm::vec2{10.0f, camera.dimensions.y - 12.0f}, 12.0f, vec3(1.0f), "ABCDEFGHIJKLMNOPQRSTUVWXYZ\nabcdefghijklmnopqrstuvwxyz\n1234567890-=!#%^*()_+[]{};':,.<>/?\\|~");

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}