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
    glm::vec4 clear_color = glm::vec4(0.45f, 0.55f, 0.60f, 1.00f);
}

/***********************************
 *
 *  void Init(Context *context)
 *      - Called once, at the start of the game after window has been
 *initialized
 *      - Use this to initialize any variables, load assets, etc
 *
 ***********************************/
void Init(Context *context) {}

/***********************************
 *
 *  void Update(Context *context)
 *      - Called every update frame
 *      - context->frame_time will contain the delta time for the frame
 *
 ***********************************/
void Update(Context *context) { float dt = (float)(context->frame_time); }

/***********************************
 *
 *  void Render(Context *context)
 *      - Called once every render frame
 *      - context->window will contain a pointer to the current GLFWwindow
 *
 ***********************************/
void Render(Context *context) {
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui::Begin("INFO");
    ImGui::ColorEdit3("Background", (float *)&clear_color);
    ImGui::Text("OpenGL Version: %s", (char *)glGetString(GL_VERSION));
    int width, height;
    glfwGetWindowSize(context->window, &width, &height);
    ImGui::Text("Resolution: %dx%d", width, height);
    ImGui::Text("Time: %.3fs", context->time);
    ImGui::Text("FPS: %.3fs", 1.0 / context->frame_time);
    ImGui::End();
}

/***********************************
 *
 *  void Cleanup(Context *context)
 *      - Called once, at the end of game before the window is destroyed
 *      - Use this to destroy any OpenGL resources, etc
 *
 ***********************************/
void Cleanup(Context *context) {}
