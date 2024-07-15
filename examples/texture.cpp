#if _WIN32
#include <windows.h>
#endif

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define BIFROST_IMPLEMENTATION
#include "bifrost/bifrost.h"

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

    bifrost::Texture texture = bifrost::LoadTexture("sample-texture.png");
    bifrost::Texture font_texture = bifrost::LoadTexture("debug-font.png");

    bifrost::Camera2d camera = bifrost::GenOrthogonalCamera2d(glm::vec2(0.0f), glm::vec2(1600.0f, 900.0f));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_MULTISAMPLE);

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

        bifrost::DrawRectangle(camera, glm::vec2(100.0f), glm::vec2(200.0f), texture);
        
        // Include a color to tint the texture
        bifrost::DrawRectangle(camera, glm::vec2(300.0f), glm::vec2(200.0f), texture, glm::vec3(1.0f, 0.0f, 0.0f));

        // Color can include an alpha channel
        // Can also provide a source origin and source size (in pixels)
        bifrost::DrawRectangle(camera,
            glm::vec2(500.0f), glm::vec2(70.0f, 120.0f),
            font_texture,
            glm::vec2(7, 36), glm::vec2(7, 12),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        glfwSwapBuffers(window);

    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

