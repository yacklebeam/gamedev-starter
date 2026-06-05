#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <bifrost/bifrost.h>
#include <bifrost/bifrost_input.h>

int main()
{
    // Create the window and initialize renderer
    bifrost::EnableDefaultRendererOptions();
    auto window = bifrost::Initialize(800, 600, "postprocessing example");
    auto screen_size = bifrost::GetScreenSize(window);
    glViewport(0, 0, screen_size.x, screen_size.y);
    auto camera = bifrost::GenUICamera(screen_size.x, screen_size.y);

    // Custom OpenGL setup beyond defaults
    glfwSetWindowUserPointer(window, &camera);
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);
        auto camera = (bifrost::Camera2d*)glfwGetWindowUserPointer(window);
        *camera = bifrost::GenUICamera(width, height);
    });
    glfwWindowHint(GLFW_SAMPLES, 4);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    // Establish game state
    bool grayscale_on = false;
    bool pixelate_on  = false;
    bool crt_on       = false;
    float angle = 0.0f;

    // Generate RenderPasses and Framebuffer for initial rendering 
    auto grayscale = bifrost::GenRenderPass(screen_size.x, screen_size.y, "assets/pp_grayscale.frag");
    auto pixelate  = bifrost::GenRenderPass(screen_size.x, screen_size.y, "assets/pp_pixelate.frag");
    auto crt       = bifrost::GenRenderPass(screen_size.x, screen_size.y, "assets/pp_crt.frag");
    auto framebuffer = bifrost::GenFramebuffer(screen_size.x, screen_size.y);

    // Setup default input handling
    bifrost::InputHandler input{};
    input.AddKeyBind(GLFW_KEY_1, "toggle_1");
    input.AddKeyBind(GLFW_KEY_2, "toggle_2");
    input.AddKeyBind(GLFW_KEY_3, "toggle_3");
    input.AddKeyBind(GLFW_KEY_ESCAPE, "quit");
    input.BindOnPressed("quit", [&window]() { glfwSetWindowShouldClose(window, GLFW_TRUE); });
    input.BindOnPressed("toggle_1", [&grayscale_on]() { grayscale_on = !grayscale_on; });
    input.BindOnPressed("toggle_2", [&pixelate_on]() { pixelate_on = !pixelate_on; });
    input.BindOnPressed("toggle_3", [&crt_on]() { crt_on = !crt_on; });

    // Game main loop
    while (!glfwWindowShouldClose(window))
    {
        // Run 1 step of game simulation
        {
            input.PollEvents(window);
            angle += 0.5f;
        }

        // Render the game scene, captured to a framebuffer
        {
            BIFROST_ACTIVATE_FRAMEBUFFER(framebuffer);

            glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            bifrost::DrawRectangle(camera, camera.dimensions * 0.5f,                           glm::vec2(150.0f), angle,        glm::vec3(1.0f, 0.4f, 0.2f));
            bifrost::DrawRectangle(camera, camera.dimensions * 0.5f + glm::vec2(120.0f, 0.0f), glm::vec2(80.0f),  angle * 1.5f, glm::vec3(0.2f, 0.6f, 1.0f));
            bifrost::DrawRectangle(camera, camera.dimensions * 0.5f - glm::vec2(120.0f, 0.0f), glm::vec2(80.0f),  angle * 2.0f, glm::vec3(0.4f, 1.0f, 0.4f));
        }

        // Capture the framebuffer texture for final rendering
        {
            auto main_scene_texture = framebuffer.texture;
            
            // Run (optional) post processing shaders
            if (grayscale_on) main_scene_texture = grayscale.Run(main_scene_texture);
            if (pixelate_on)  main_scene_texture = pixelate.Run(main_scene_texture);
            if (crt_on)       main_scene_texture = crt.Run(main_scene_texture);

            // Blit scene to default framebuffer
            bifrost::DrawToScreen(main_scene_texture);

            // Draw any parts of the scene that are drawn after post processing effects
            bifrost::DrawDebugText(camera, glm::vec2(10.0f, camera.dimensions.y - 32.0f), 24.0f, "postprocessing example");
            bifrost::DrawDebugText(camera, glm::vec2(10.0f, 10.0f), 24.0f, glm::vec3(0.8f, 0.8f, 0.8f),
                "1: grayscale [%s]  2: pixelate [%s]  3: CRT [%s]",
                grayscale_on ? "ON" : "OFF",
                pixelate_on  ? "ON" : "OFF",
                crt_on        ? "ON" : "OFF");

        }

        glfwSwapBuffers(window);
    }

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
