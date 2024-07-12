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

    static char* basic_vs =
    R"(
        #version 450 core
        layout (location = 0) in vec2 position;
        out vec2 uv;
        uniform mat4 mvp;
        void main()
        {
            gl_Position = mvp * vec4(position, 0.0, 1.0);
            uv = position;
        }
    )";

    static char* basic_fs =
    R"(
        #version 450 core
        uniform vec4 color;
        out vec4 fragment_color;
        void main()
        {
            fragment_color = color;
        }
    )";

    const float vertices[] = {
        -0.5f,  0.5f,
        -0.5f, -0.5f,
         0.5f, -0.5f,

         0.5f, -0.5f,
         0.5f,  0.5f,
        -0.5f,  0.5f,
    };

    // Helper function to generate a VAO and bind 
    unsigned int vao = bifrost::GenVec2Vao(vertices, 6);
    bifrost::Shader shader = bifrost::GenShaderFromSource(basic_vs, basic_fs);

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

        glBindVertexArray(vao);

        glUseProgram(shader.id);
        glUniformMatrix4fv(glGetUniformLocation(shader.id, "mvp"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
        glUniform4fv(glGetUniformLocation(shader.id, "color"), 1, glm::value_ptr(glm::vec4(1.0f)));
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);

    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

