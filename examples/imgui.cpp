#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include <bifrost/bifrost.h>

namespace
{
    bifrost::Camera2d camera{};

    void FramebufferSizeCallback(GLFWwindow*, int width, int height)
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

    GLFWwindow* window = glfwCreateWindow(800, 600, "imgui example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    auto screen_size = bifrost::GetScreenSize(*window);
    glViewport(0, 0, screen_size.x, screen_size.y);
    camera = bifrost::GenUICamera(screen_size.x, screen_size.y);

    // Options controlled by the panel
    glm::vec4 bg_color    = glm::vec4(0.1f, 0.1f, 0.15f, 1.0f);
    glm::vec4 square_color = glm::vec4(0.2f, 0.6f, 1.0f, 1.0f);
    float square_size = 100.0f;
    bool show_label   = true;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glClearColor(bg_color.r, bg_color.g, bg_color.b, bg_color.a);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw square
        glm::vec2 center = camera.dimensions / 2.0f;
        bifrost::DrawRectangle(camera, center, glm::vec2(square_size), square_color);

        if (show_label)
            bifrost::DrawDebugText(camera, glm::vec2(10.0f, camera.dimensions.y - 32.0f), 24.0f, "imgui example");

        // Options panel
        ImGui::Begin("Options");
        ImGui::ColorEdit3("Background", &bg_color.r);
        ImGui::ColorEdit3("Square", &square_color.r);
        ImGui::SliderFloat("Size", &square_size, 10.0f, 300.0f);
        ImGui::Checkbox("Show label", &show_label);
        ImGui::Separator();
        ImGui::Text("OpenGL: %s", (char*)glGetString(GL_VERSION));
        ImGui::Text("Viewport: %.0fx%.0f", camera.dimensions.x, camera.dimensions.y);
        if (ImGui::Button("Reset"))
        {
            bg_color     = glm::vec4(0.1f, 0.1f, 0.15f, 1.0f);
            square_color = glm::vec4(0.2f, 0.6f, 1.0f, 1.0f);
            square_size  = 100.0f;
            show_label   = true;
        }
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
