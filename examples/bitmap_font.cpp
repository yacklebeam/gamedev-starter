#if _WIN32
#include <windows.h>
#endif

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define BIFROST_IMPLEMENTATION
#include "bifrost/bifrost.h"

#define BIFROST_FONTS_IMPLEMENTATION
#include "bifrost/bifrost_fonts.h"

#if _WIN32
int main(int argc, char* argv[]);
int WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PSTR CmdLine, INT CmdShow)
{
    return main(__argc, __argv);
}
#endif

int main(int argc, char* argv[])
{
    /********************************
     *
     *  CREATE WINDOW
     * 
     * */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(1600, 900, "bifrost Example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);

    /********************************
     * 
     *  Setup OpenGL objects
     * 
     * */

    bifrost::InitializeFonts();

    bifrost::Camera2d camera = bifrost::GenOrthogonalCamera2d(glm::vec2(0.0f), glm::vec2(1600.0f, 900.0f));

    glDisable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /********************************
     * 
     *  MAIN LOOP
     * 
     * */
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        bifrost::DrawDebugText(camera, glm::vec2(10.0f, 100.0f), 24.0f, glm::vec3(0.0f), "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        bifrost::DrawDebugText(camera, glm::vec2(10.0f), 24.0f, "abcdefghijklmnopqrstuvwxyz");
        bifrost::DrawDebugText(camera, glm::vec2(10.0f, 200.0f), 24.0f, "{|} [!] <?> /\\ '\" :; -+=_,.() 1234567890#@%%^* bob@email.com");

        glfwSwapBuffers(window);

    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

