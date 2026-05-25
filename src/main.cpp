#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include <bifrost/bifrost.h>
#include <bifrost/bifrost_input.h>
#include <bifrost/bifrost_dungeon.h>
#include <bifrost/bifrost_collision.h>

#include <miniaudio/miniaudio.h>

#include <stdio.h>
#include <vector>
#include <format>

namespace
{
    void GlfwErrorCallback(int error, const char* description);
    void GlfwFramebufferSizeCallback(GLFWwindow* window, int width, int height);

    bifrost::Camera2d ui_camera{};
}

#if _WIN32
int main();
int WinMain()
{
    return main();
}
#endif

int main()
{
    glfwSetErrorCallback(GlfwErrorCallback);

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(800, 600, "yakl - gamedev-starter", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, GlfwFramebufferSizeCallback);
    glfwMakeContextCurrent(window);
    gladLoadGL(glfwGetProcAddress);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");
    
    glEnable(GL_MULTISAMPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);


    ma_result result;
    ma_engine engine;

    result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        return result;
    }

    ma_sound sound;
    result = ma_sound_init_from_file(&engine, "sample-9s.wav", MA_SOUND_FLAG_DECODE, NULL, NULL, &sound);
    if (result != MA_SUCCESS) {
        return result;
    }
    //ma_sound_set_pitch(&sound, 1.0f);
    //ma_sound_start(&sound);

    auto clear_color = glm::vec4{0.45f, 0.55f, 0.60f, 1.00f};
    auto font_color = glm::vec4{1.0f};
    int font_size = 48;
    auto screen_size = bifrost::GetScreenSize(*window);
    ui_camera = bifrost::GenUICamera(screen_size.x, screen_size.y);
    auto dungeon_texture = bifrost::GetDungeonTexture();

    bifrost::InputHandler input{};
    bifrost::InputHandler meta_input{};

    bool show_info_panel = false;
    bool dragging = false;
    int x = 0;
    int y = 0;

    input.AddKeyBind(GLFW_KEY_RIGHT, "right");
    input.AddKeyBind(GLFW_KEY_LEFT, "left");
    input.AddKeyBind(GLFW_KEY_UP, "up");
    input.AddKeyBind(GLFW_KEY_DOWN, "down");
    input.BindOnPressed("left", [&x]() { x--; if (x < 0) x += 12; });
    input.BindOnPressed("right", [&x]() { x++; x %= 12; });
    input.BindOnPressed("down", [&y]() { y--; if (y < 0) y += 11; });
    input.BindOnPressed("up", [&y]() { y++; y %= 11; });

    meta_input.AddKeyBind(GLFW_KEY_ESCAPE, "quit");
    meta_input.AddKeyBind(GLFW_KEY_P, "quit");
    meta_input.AddKeyBind(GLFW_KEY_Q, "toggle_info");
    meta_input.AddMouseButtonBind(GLFW_MOUSE_BUTTON_LEFT, "mouse_select");
    meta_input.BindOnPressed("quit", [&window]() { glfwSetWindowShouldClose(window, GLFW_TRUE); });
    meta_input.BindOnPressed("toggle_info", [&show_info_panel]() { show_info_panel = !show_info_panel; });
    meta_input.BindOnPressed("mouse_select", [&dragging]() { dragging = true; });
    meta_input.BindOnReleased("mouse_select", [&dragging]() { dragging = false; });

    auto rect_hitbox = bifrost::GenRectHitbox({80.0f, 80.0f});
    glm::vec2 rect_pos = ui_camera.dimensions / 2.0f;

    /********************************
     * 
     * 
     *  MAIN LOOP
     * 
     * 
     * */
    while(!glfwWindowShouldClose(window))
    {
	   // UPDATE
    	double time = glfwGetTime();
        if (!show_info_panel)
            input.PollEvents(window);
        meta_input.PollEvents(window);

	   // RENDER
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Draw game
    	glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        bifrost::DrawDebugText(ui_camera, glm::vec2{10.0f, ui_camera.dimensions.y - (float)font_size}, (float)font_size, font_color, "ABCDEFGHIJKLMNOPQRSTUVWXYZ\nabcdefghijklmnopqrstuvwxyz\n1234567890-=!#%^*()_+[]{};':,.<>/?\\|~");

        bifrost::LineIntersectionResult line_hit{};
        if (dragging)
            line_hit = bifrost::GetLineIntersection(rect_hitbox, rect_pos, 0.0f, input.MousePressedAt, input.MouseAt);

        bool mouse_over = bifrost::ContainsPoint(rect_hitbox, rect_pos, 0.0f, meta_input.MouseAt);
        auto rect_color = (mouse_over || line_hit.hit) ? glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) : glm::vec4(1.0f);
        bifrost::DrawRectangle(ui_camera, rect_pos, glm::vec2(80.0f, 80.0f), dungeon_texture, glm::vec2(x * 17.f, y * 17.f), glm::vec2(16.f), rect_color);
        bifrost::DrawHitbox(ui_camera, rect_hitbox, rect_pos, 0.f, glm::vec3(0.f, 1.f, 0.f));

        auto tex_size = glm::vec2(12.f * 17.f, 11.f * 17.f);
        bifrost::DrawRectangle(ui_camera, tex_size / 2.f, tex_size, dungeon_texture);

        auto box_start = glm::vec2(x, y) * 17.f;
        auto box_end = box_start + glm::vec2(16.f);
        bifrost::DrawLine(ui_camera, glm::vec2(box_start.x, box_start.y), glm::vec2(box_start.x, box_end.y), 2.0f, glm::vec3(1.f, 0.f, 0.f));
        bifrost::DrawLine(ui_camera, glm::vec2(box_start.x, box_end.y), glm::vec2(box_end.x, box_end.y), 2.0f, glm::vec3(1.f, 0.f, 0.f));
        bifrost::DrawLine(ui_camera, glm::vec2(box_end.x, box_end.y), glm::vec2(box_end.x, box_start.y), 2.0f, glm::vec3(1.f, 0.f, 0.f));
        bifrost::DrawLine(ui_camera, glm::vec2(box_end.x, box_start.y), glm::vec2(box_start.x, box_start.y), 2.0f, glm::vec3(1.f, 0.f, 0.f));

        //bifrost::DrawDebugText(ui_camera, glm::vec2{10.0f}, (float)font_size, font_color, std::format("[{:.1f}s]", time));

        if (dragging)
        {
            glm::vec2 start = input.MousePressedAt;
            glm::vec2 end = input.MouseAt;
            bifrost::DrawLine(ui_camera, start, end, 2.0f, glm::vec3(1.0f));
        }
        
        // Draw info panel
        if (show_info_panel)
        {
            ImGui::Begin("INFO");
            ImGui::ColorEdit3("Background", &clear_color.x);
            ImGui::ColorEdit3("Font Color", &font_color.x);
            ImGui::DragInt("Font Size", &font_size, 12, 12, 120);
            ImGui::Text("OpenGL Version: %s", (char*)glGetString(GL_VERSION));
            screen_size = bifrost::GetScreenSize(*window);
            ImGui::Text("Resolution: %dx%d", screen_size.x, screen_size.y);
            ImGui::Text("ViewPort: %dx%d", (int)ui_camera.dimensions.x, (int)ui_camera.dimensions.y);
            if (ImGui::Button("RESET"))
            {
                font_size = 48;
                font_color = glm::vec4{1.0f};
                clear_color = glm::vec4{0.45f, 0.55f, 0.60f, 1.00f};
            }
            ImGui::End();  
        }
	
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    ma_sound_uninit(&sound);
    ma_engine_uninit(&engine);

    return 0;
}

namespace
{
void GlfwFramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    ui_camera = bifrost::GenUICamera(width, height);
}
   
void GlfwErrorCallback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}
}
