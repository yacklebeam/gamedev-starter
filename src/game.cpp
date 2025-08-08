#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#define BIFROST_IMPLEMENTATION
#include "bifrost/bifrost.h"

#include "game.h"

/***********************************
 *
 *  LOCAL GAME CODE VARIABLES
 *
 ***********************************/
namespace
{
    auto clear_color = glm::vec4{0.45f, 0.55f, 0.60f, 1.00f};
    bifrost::Camera2d ui_camera{};
}

/***********************************
 *
 *  void Init(Context& context)
 *      - Called once, at the start of the game after window has been initialized
 *      - Use this to initialize any variables, load assets, etc
 *
 ***********************************/
void Init(const Context& context)
{

    auto screen_size = bifrost::GetScreenSize(*context.window);
    ui_camera = bifrost::GenUICamera(glm::vec2{screen_size});
}

/***********************************
 *
 *  void Update(Context& context)
 *      - Called every update frame
 *      - context.frame_time will contain the delta time for the frame
 *      - context.time will contain the total elapsed time since the start of the program
 *
 ***********************************/
void Update(const Context& context) { float dt = (float)(context.frame_time); }

/***********************************
 *
 *  void Render(Context& context)
 *      - Called once every render frame
 *      - context.window will contain a pointer to the current GLFWwindow
 *
 ***********************************/
void Render(const Context& context) {
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    ImGui::Begin("INFO");
    ImGui::ColorEdit3("Background", &clear_color.x);
    ImGui::Text("OpenGL Version: %s", (char*)glGetString(GL_VERSION));
    auto screen_size = bifrost::GetScreenSize(*context.window);
    ImGui::Text("Resolution: %dx%d", screen_size.x, screen_size.y);
    ImGui::Text("Time: %.3fs", context.time);
    ImGui::Text("FPS: %.3fs", 1.0 / context.frame_time);
    ImGui::End();

    bifrost::DrawDebugText(ui_camera, glm::vec2{10.0f}, 24.0f, glm::vec3{1.0f}, "%.1fs", context.time);
}

/***********************************
 *
 *  void Cleanup(Context *context)
 *      - Called once, at the end of game before the window is destroyed
 *      - Use this to destroy any OpenGL resources, etc
 *
 ***********************************/
void Cleanup(const Context& context) {}
