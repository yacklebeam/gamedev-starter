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

    GLFWwindow* window = glfwCreateWindow(800, 600, "bifrost Example", NULL, NULL);
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
            uv = position + vec2(0.5);
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

    static char* tex_fs =
    R"(
        #version 450 core
        in vec2 uv;
        uniform sampler2D tex;
        out vec4 fragment_color;
        void main()
        {
            fragment_color = texture(tex, uv);
        }
    )";

    static char* red_fs =
    R"(
        #version 450 core
        in vec2 uv;
        uniform sampler2D tex;
        out vec4 fragment_color;
        void main()
        {
            vec4 t = texture(tex, uv);
            fragment_color = vec4(1.0, 0.0, 0.0, 1.0) * t.w;
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
    bifrost::Shader tex_shader = bifrost::GenShaderFromSource(basic_vs, tex_fs);
    bifrost::Shader red_shader = bifrost::GenShaderFromSource(basic_vs, red_fs);

    bifrost::Camera2d camera = bifrost::GenOrthogonalCamera2d(glm::vec2(0.0f), glm::vec2(800.0f, 600.0f));

    bifrost::Texture texture = bifrost::LoadTexture("hero.png");

    auto model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(400.0f, 220.0f, 0.0f));
    model = glm::scale(model, glm::vec3(100.0f, 100.0f, 0.0f));

    auto model2 = glm::mat4(1.0f);
    model2 = glm::translate(model2, glm::vec3(400.0f, 100.0f, 0.0f));
    model2 = glm::scale(model2, glm::vec3(800.0f, 200.0f, 0.0f));

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
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        bifrost::DrawDebugText(camera, glm::vec2(40.0f, 500.0f), 36.0f, glm::vec3(0.0f), "Stencil Test -- Red is 'behind' the wall");

        glBindVertexArray(vao);

        // draw "player"
        glUseProgram(tex_shader.id);
        glBindTexture(GL_TEXTURE_2D, texture.id);
        glUniformMatrix4fv(glGetUniformLocation(tex_shader.id, "mvp"), 1, GL_FALSE, glm::value_ptr(camera.projection * model));
        glUniform4fv(glGetUniformLocation(tex_shader.id, "color"), 1, glm::value_ptr(glm::vec4(1.0f)));
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // draw wall
        glEnable(GL_STENCIL_TEST);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glStencilMask(0xFF);
        glUseProgram(shader.id);
        glUniformMatrix4fv(glGetUniformLocation(shader.id, "mvp"), 1, GL_FALSE, glm::value_ptr(camera.projection * model2));
        glUniform4fv(glGetUniformLocation(shader.id, "color"), 1, glm::value_ptr(glm::vec4(glm::vec3(0.0f), 1.0f)));
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // draw "player" again
        glStencilFunc(GL_EQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glBindTexture(GL_TEXTURE_2D, texture.id);
        glUseProgram(red_shader.id);
        glUniformMatrix4fv(glGetUniformLocation(red_shader.id, "mvp"), 1, GL_FALSE, glm::value_ptr(camera.projection * model));
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisable(GL_STENCIL_TEST);

        glfwSwapBuffers(window);

    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

