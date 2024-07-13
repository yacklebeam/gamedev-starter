#if _WIN32
#include <windows.h>
#endif

#include <cstdint>

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

    bifrost::Camera2d camera = bifrost::GenOrthogonalCamera2d(glm::vec2(0.0f), glm::vec2(1600.0f, 900.0f));
    
    bifrost::Seed((uint32_t)glfwGetTime());

    glm::vec2 random_position = glm::vec2(750.0f, 400.0f); // start in center?

    /********************************
     * 
     *  MAIN LOOP
     * 
     * */
    int last_state = -1;
    int score = 0;
    int clicks = 0;
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
        if (state == GLFW_PRESS && last_state == GLFW_RELEASE)
        {
            double mouse_x, mouse_y;
            glfwGetCursorPos(window, &mouse_x, &mouse_y);
            mouse_y = 900.0 - mouse_y; // GLFW has 0 at top, OpenGL has 0 at bottom

            if ((float)mouse_x >= random_position.x && (float)mouse_x <= random_position.x + 100 && (float)mouse_y >= random_position.y && (float)mouse_y <= random_position.y + 100)
            {
                random_position = glm::vec2(bifrost::RandomFloat() * 1500.0f, bifrost::RandomFloat() * 800.0f);
                score++;
            }
            clicks++;
        }

        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);

        bifrost::DrawRectangle(camera, random_position, glm::vec2(100.0f), glm::vec3(0.0f));

        bifrost::DrawDebugText(camera, glm::vec2(10.0f, 10.0f), 36.0f, glm::vec3(0.0f), "Clicks: %d", clicks);
        bifrost::DrawDebugText(camera, glm::vec2(10.0f, 48.0f), 36.0f, glm::vec3(0.0f), "Score: %d", score);
        bifrost::DrawDebugText(camera, glm::vec2(10.0f, 86.0f), 36.0f, glm::vec3(0.0f), "Time: %.2fs", glfwGetTime());

        glfwSwapBuffers(window);
        last_state = state;
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

