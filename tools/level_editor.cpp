#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include <bifrost/bifrost.h>
#include <bifrost/bifrost_dungeon.h>

#include <stdio.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

// Dungeon spritesheet constants
static constexpr int TILE_SRC_SIZE  = 16;  // source pixels per tile
static constexpr int TILE_STRIDE    = 17;  // source stride (tile + 1px gap)
static constexpr int SHEET_COLS     = 12;
static constexpr int SHEET_ROWS     = 11;
static constexpr int TILE_DISPLAY   = 32;  // display pixels per tile at 1× zoom

static constexpr int LAYER_COUNT    = 3;
static constexpr int LAYER_GROUND   = 0;
static constexpr int LAYER_WALLS    = 1;
static constexpr int LAYER_OBJECTS  = 2;
static const char*   LAYER_SECTION[]  = { "ground", "walls", "objects" };
static const char*   LAYER_LABEL[]    = { "Ground", "Walls", "Objects" };

enum class Tool { Paint, Eyedropper };

struct TileCell
{
    int tile;     // -1 = empty
    int rotation; // 0-3 quarter-turns clockwise
};

struct LevelData
{
    int width  = 20;
    int height = 15;
    std::vector<TileCell> tiles[LAYER_COUNT]; // 0=ground, 1=walls, 2=objects

    void resize(int w, int h)
    {
        for (int l = 0; l < LAYER_COUNT; ++l)
        {
            std::vector<TileCell> next(w * h, {-1, 0});
            for (int y = 0; y < std::min(h, height); ++y)
                for (int x = 0; x < std::min(w, width); ++x)
                    next[y * w + x] = tiles[l][y * width + x];
            tiles[l] = std::move(next);
        }
        width  = w;
        height = h;
    }

    TileCell& at(int layer, int x, int y)       { return tiles[layer][y * width + x]; }
    TileCell  get(int layer, int x, int y) const { return tiles[layer][y * width + x]; }
};

// undo/redo
static std::vector<LevelData> undo_stack;
static std::vector<LevelData> redo_stack;

static void PushUndo(const LevelData& level)
{
    undo_stack.push_back(level);
    redo_stack.clear();
}

// File format:
//   [ground]
//   w,h
//   tile:rot,...  (one row per line)
//   [walls]
//   ...
//   [objects]
//   ...
//
// Old single-section files (no header) load into ground; walls/objects default empty.

static bool SaveLevel(const LevelData& level, const char* path)
{
    std::ofstream f(path);
    if (!f) return false;
    for (int l = 0; l < LAYER_COUNT; ++l)
    {
        f << "[" << LAYER_SECTION[l] << "]\n";
        f << level.width << "," << level.height << "\n";
        for (int y = 0; y < level.height; ++y)
        {
            for (int x = 0; x < level.width; ++x)
            {
                if (x) f << ",";
                TileCell c = level.get(l, x, y);
                f << c.tile << ":" << c.rotation;
            }
            f << "\n";
        }
    }
    return true;
}

static void ParseLayerRows(std::ifstream& f, LevelData& level, int layer_idx, int w, int h)
{
    level.tiles[layer_idx].assign(w * h, {-1, 0});
    for (int y = 0; y < h; ++y)
    {
        std::string line;
        if (!std::getline(f, line)) break;
        std::istringstream ss(line);
        std::string tok;
        for (int x = 0; x < w && std::getline(ss, tok, ','); ++x)
        {
            TileCell c = {-1, 0};
            size_t colon = tok.find(':');
            if (colon != std::string::npos)
            {
                c.tile     = std::stoi(tok.substr(0, colon));
                c.rotation = std::stoi(tok.substr(colon + 1));
            }
            else
            {
                c.tile = std::stoi(tok);
            }
            level.at(layer_idx, x, y) = c;
        }
    }
}

static bool LoadLevel(LevelData& level, const char* path)
{
    std::ifstream f(path);
    if (!f) return false;

    std::string line;
    if (!std::getline(f, line)) return false;

    if (!line.empty() && line[0] == '[')
    {
        // New multi-section format
        bool dims_set = false;
        do
        {
            int layer_idx = -1;
            for (int l = 0; l < LAYER_COUNT; ++l)
            {
                std::string expected = std::string("[") + LAYER_SECTION[l] + "]";
                if (line == expected) { layer_idx = l; break; }
            }
            if (layer_idx < 0) return false;

            std::string dim_line;
            if (!std::getline(f, dim_line)) break;
            int w = 0, h = 0;
            if (sscanf(dim_line.c_str(), "%d,%d", &w, &h) != 2 || w <= 0 || h <= 0) return false;

            if (!dims_set)
            {
                level.width  = w;
                level.height = h;
                for (int l = 0; l < LAYER_COUNT; ++l)
                    level.tiles[l].assign(w * h, {-1, 0});
                dims_set = true;
            }

            ParseLayerRows(f, level, layer_idx, w, h);
        }
        while (std::getline(f, line) && !line.empty());
    }
    else
    {
        // Old format: first line is dimensions, data is ground only
        int w = 0, h = 0;
        if (sscanf(line.c_str(), "%d,%d", &w, &h) != 2 || w <= 0 || h <= 0) return false;
        level.width  = w;
        level.height = h;
        for (int l = 0; l < LAYER_COUNT; ++l)
            level.tiles[l].assign(w * h, {-1, 0});
        ParseLayerRows(f, level, LAYER_GROUND, w, h);
    }

    return true;
}

// Convert flat tile index to spritesheet column/row.
// The tile selector displays the sheet top-to-bottom, but the GL texture has
// y=0 at the bottom (OpenGL origin), so we invert the row here.
static glm::vec2 TileSourceOrigin(int idx)
{
    int col = idx % SHEET_COLS;
    int row = idx / SHEET_COLS;
    int gl_row = (SHEET_ROWS - 1) - row;
    return glm::vec2(col * TILE_STRIDE, gl_row * TILE_STRIDE);
}

namespace
{
    void GlfwErrorCallback(int error, const char* description)
    {
        fprintf(stderr, "GLFW Error %d: %s\n", error, description);
    }
}

int main()
{
    glfwSetErrorCallback(GlfwErrorCallback);
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Level Editor", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    gladLoadGL(glfwGetProcAddress);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450");

    glEnable(GL_MULTISAMPLE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    auto dungeon_texture = bifrost::GetDungeonTexture();

    // Editor state
    LevelData level;
    for (int l = 0; l < LAYER_COUNT; ++l)
        level.tiles[l].assign(level.width * level.height, {-1, 0});

    Tool      active_tool     = Tool::Paint;
    int       selected_tile   = 0;
    int       brush_rotation  = 0; // 0-3 quarter-turns clockwise; R key cycles
    int       active_layer    = LAYER_GROUND;
    bool      layer_visible[LAYER_COUNT] = { true, true, true };
    float     zoom            = 1.0f;
    glm::vec2 pan_offset      = {0.0f, 0.0f};

    // Middle-mouse pan state
    bool      panning        = false;
    glm::vec2 pan_mouse_start{};
    glm::vec2 pan_offset_start{};

    // Per-stroke undo: separate tracking for left (paint) and right (erase)
    bool      stroke_l = false;
    bool      stroke_r = false;
    LevelData stroke_snapshot;

    // Resize UI state
    int resize_w = level.width;
    int resize_h = level.height;

    // File path
    char file_path[512] = "level.csv";

    // Status message
    char status_msg[128] = "";

    // Right panel width
    static constexpr float PANEL_W = 230.0f;

    // Framebuffer for level viewport
    int fb_w = 0, fb_h = 0;
    bifrost::Framebuffer level_fb{};
    bool fb_valid = false;

    // Framebuffer for 128×128 tile preview widget
    bifrost::Framebuffer preview_fb = bifrost::GenFramebuffer(128, 128);

    auto RebuildFramebuffer = [&](int w, int h)
    {
        if (fb_valid)
        {
            glDeleteFramebuffers(1, &level_fb.id);
            glDeleteTextures(1, &level_fb.texture_id);
        }
        level_fb  = bifrost::GenFramebuffer(w, h);
        fb_w      = w;
        fb_h      = h;
        fb_valid  = true;
    };

    // Convert viewport mouse position to level tile coordinates
    auto MouseToTile = [&](glm::vec2 mouse_in_viewport) -> glm::ivec2
    {
        float tile_px = TILE_DISPLAY * zoom;
        float lx = (mouse_in_viewport.x - pan_offset.x) / tile_px;
        float ly = (mouse_in_viewport.y - pan_offset.y) / tile_px;
        return { (int)lx, (int)ly };
    };

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Keyboard shortcuts (when viewport not captured by ImGui text)
        if (!io.WantCaptureKeyboard)
        {
            if (ImGui::IsKeyPressed(ImGuiKey_Z, false) && io.KeyCtrl)
            {
                if (!undo_stack.empty())
                {
                    redo_stack.push_back(level);
                    level = undo_stack.back();
                    undo_stack.pop_back();
                }
            }
            if (ImGui::IsKeyPressed(ImGuiKey_Y, false) && io.KeyCtrl)
            {
                if (!redo_stack.empty())
                {
                    undo_stack.push_back(level);
                    level = redo_stack.back();
                    redo_stack.pop_back();
                }
            }
            if (ImGui::IsKeyPressed(ImGuiKey_R, false))
                brush_rotation = (brush_rotation + 1) % 4;

            // 1/2/3 to switch active layer
            if (ImGui::IsKeyPressed(ImGuiKey_1, false)) active_layer = LAYER_GROUND;
            if (ImGui::IsKeyPressed(ImGuiKey_2, false)) active_layer = LAYER_WALLS;
            if (ImGui::IsKeyPressed(ImGuiKey_3, false)) active_layer = LAYER_OBJECTS;
        }

        // ImGui uses display coordinates (from glfwGetWindowSize), not framebuffer pixels.
        // Always derive layout dimensions from io.DisplaySize to stay consistent.
        float viewport_w = io.DisplaySize.x - PANEL_W;
        float viewport_h = io.DisplaySize.y;

        // The level framebuffer matches the viewport in display-pixel space.
        // On HiDPI systems the GL framebuffer is larger, but we keep the FBO at
        // display resolution and let ImGui scale the Image as needed.
        if (!fb_valid || (int)viewport_w != fb_w || (int)viewport_h != fb_h)
            RebuildFramebuffer((int)viewport_w, (int)viewport_h);

        // Track actual GL framebuffer size for glViewport on the default target.
        int win_fb_w, win_fb_h;
        glfwGetFramebufferSize(window, &win_fb_w, &win_fb_h);

        // ---- Render level to framebuffer ----
        glBindFramebuffer(GL_FRAMEBUFFER, level_fb.id);
        glViewport(0, 0, fb_w, fb_h);
        glClearColor(0.18f, 0.18f, 0.18f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        {
            bifrost::Camera2d cam = bifrost::GenUICamera(fb_w, fb_h);
            float tile_px = TILE_DISPLAY * zoom;

            // Checkerboard background
            for (int ty = 0; ty < level.height; ++ty)
            {
                for (int tx = 0; tx < level.width; ++tx)
                {
                    float cx = pan_offset.x + tx * tile_px + tile_px * 0.5f;
                    float cy = (float)fb_h - (pan_offset.y + ty * tile_px + tile_px * 0.5f);
                    glm::vec4 bg = ((tx + ty) % 2 == 0)
                        ? glm::vec4(0.25f, 0.25f, 0.25f, 1.f)
                        : glm::vec4(0.22f, 0.22f, 0.22f, 1.f);
                    bifrost::DrawRectangle(cam, {cx, cy}, {tile_px, tile_px}, bg);
                }
            }

            // Draw layers ground → walls → objects, skipping hidden ones
            for (int l = 0; l < LAYER_COUNT; ++l)
            {
                if (!layer_visible[l]) continue;
                for (int ty = 0; ty < level.height; ++ty)
                {
                    for (int tx = 0; tx < level.width; ++tx)
                    {
                        TileCell cell = level.get(l, tx, ty);
                        if (cell.tile < 0) continue;

                        float cx = pan_offset.x + tx * tile_px + tile_px * 0.5f;
                        float cy = (float)fb_h - (pan_offset.y + ty * tile_px + tile_px * 0.5f);
                        glm::vec2 src = TileSourceOrigin(cell.tile);
                        bifrost::DrawRectangle(cam, {cx, cy}, {tile_px, tile_px},
                            cell.rotation * 90.0f,
                            dungeon_texture, src, glm::vec2(TILE_SRC_SIZE));
                    }
                }
            }

            // Grid lines
            for (int tx = 0; tx <= level.width; ++tx)
            {
                float x = pan_offset.x + tx * tile_px;
                float y0 = (float)fb_h - pan_offset.y;
                float y1 = (float)fb_h - (pan_offset.y + level.height * tile_px);
                bifrost::DrawLine(cam, {x, y0}, {x, y1}, 1.0f, glm::vec3(0.35f));
            }
            for (int ty = 0; ty <= level.height; ++ty)
            {
                float y  = (float)fb_h - (pan_offset.y + ty * tile_px);
                float x0 = pan_offset.x;
                float x1 = pan_offset.x + level.width * tile_px;
                bifrost::DrawLine(cam, {x0, y}, {x1, y}, 1.0f, glm::vec3(0.35f));
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, win_fb_w, win_fb_h);

        // ---- ImGui: level viewport child window ----
        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize({viewport_w, viewport_h});
        ImGui::Begin("##viewport", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus);

        // Display framebuffer texture (ImGui expects top-left origin; OpenGL texture is flipped)
        ImVec2 img_pos  = ImGui::GetCursorScreenPos();
        ImGui::Image((ImTextureID)(intptr_t)level_fb.texture_id,
                     {viewport_w, viewport_h},
                     {0, 1}, {1, 0}); // flip UV vertically

        // Mouse interaction within viewport image
        bool hovered = ImGui::IsItemHovered();
        ImVec2 mouse_abs = io.MousePos;
        glm::vec2 mouse_in_vp = { mouse_abs.x - img_pos.x, mouse_abs.y - img_pos.y };

        // Scroll to zoom
        if (hovered && io.MouseWheel != 0.0f)
        {
            float old_zoom = zoom;
            zoom = std::clamp(zoom * (1.0f + io.MouseWheel * 0.1f), 0.1f, 8.0f);
            // Zoom toward mouse cursor
            float scale = zoom / old_zoom;
            pan_offset.x = mouse_in_vp.x - scale * (mouse_in_vp.x - pan_offset.x);
            pan_offset.y = mouse_in_vp.y - scale * (mouse_in_vp.y - pan_offset.y);
        }

        // Middle-mouse pan
        if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Middle))
        {
            panning         = true;
            pan_mouse_start = mouse_in_vp;
            pan_offset_start = pan_offset;
        }
        if (panning)
        {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
            {
                pan_offset = pan_offset_start + (mouse_in_vp - pan_mouse_start);
            }
            else
            {
                panning = false;
            }
        }

        // Paint / Erase / Eyedropper — hidden active layer blocks all edits
        bool can_paint = layer_visible[active_layer];
        if (hovered)
        {
            glm::ivec2 tile = MouseToTile(mouse_in_vp);
            bool in_bounds  = tile.x >= 0 && tile.x < level.width &&
                              tile.y >= 0 && tile.y < level.height;

            // Left click: paint or eyedrop
            if (active_tool == Tool::Eyedropper)
            {
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && in_bounds)
                {
                    TileCell picked = level.get(active_layer, tile.x, tile.y);
                    if (picked.tile >= 0)
                    {
                        selected_tile  = picked.tile;
                        brush_rotation = picked.rotation;
                        active_tool    = Tool::Paint;
                    }
                }
            }
            else
            {
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && in_bounds && can_paint)
                {
                    stroke_snapshot = level;
                    stroke_l        = true;
                }
                if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && in_bounds && can_paint)
                    level.at(active_layer, tile.x, tile.y) = { selected_tile, brush_rotation };
                if (stroke_l && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
                {
                    PushUndo(stroke_snapshot);
                    stroke_l = false;
                }
            }

            // Right click: always erase (on active layer)
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && in_bounds && can_paint)
            {
                stroke_snapshot = level;
                stroke_r        = true;
            }
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right) && in_bounds && can_paint)
                level.at(active_layer, tile.x, tile.y) = { -1, 0 };
            if (stroke_r && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
            {
                PushUndo(stroke_snapshot);
                stroke_r = false;
            }
        }

        ImGui::End(); // viewport

        // ---- Right panel ----
        ImGui::SetNextWindowPos({viewport_w, 0});
        ImGui::SetNextWindowSize({PANEL_W, viewport_h});
        ImGui::Begin("##panel", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoBringToFrontOnFocus);

        // Tool buttons (left=paint, right=erase always; eyedropper overrides left click)
        ImGui::Text("LMB: Paint  RMB: Erase");
        if (ImGui::RadioButton("Paint",    active_tool == Tool::Paint))      active_tool = Tool::Paint;
        ImGui::SameLine();
        if (ImGui::RadioButton("Eyedrop",  active_tool == Tool::Eyedropper)) active_tool = Tool::Eyedropper;

        ImGui::Separator();

        // Layer selector — stacked to mirror render order: objects on top, ground on bottom
        // Eye checkbox | radio button, displayed top-to-bottom as: Objects / Walls / Ground
        ImGui::Text("Layer  (1=Ground 2=Walls 3=Objects)");
        for (int display_i = LAYER_COUNT - 1; display_i >= 0; --display_i)
        {
            ImGui::Checkbox(("##vis" + std::to_string(display_i)).c_str(), &layer_visible[display_i]);
            ImGui::SameLine();
            if (ImGui::RadioButton(LAYER_LABEL[display_i], active_layer == display_i))
                active_layer = display_i;
            if (!layer_visible[display_i])
            {
                ImGui::SameLine();
                ImGui::TextDisabled("(hidden)");
            }
        }

        ImGui::Separator();

        // Tile selector: full spritesheet as ImGui::Image with overlay
        ImGui::Text("Tile");
        float sheet_display_w = (float)(SHEET_COLS * TILE_STRIDE);
        float sheet_display_h = (float)(SHEET_ROWS * TILE_STRIDE);
        float avail_w         = ImGui::GetContentRegionAvail().x;
        float sheet_scale     = avail_w / sheet_display_w;
        float scaled_w        = avail_w;
        float scaled_h        = sheet_display_h * sheet_scale;

        ImVec2 sheet_pos = ImGui::GetCursorScreenPos();
        ImGui::Image((ImTextureID)(intptr_t)dungeon_texture.id,
                     {scaled_w, scaled_h},
                     {0, 1}, {1, 0}); // flip UV: OpenGL bottom-left → ImGui top-left

        // Determine preview tile: hovered tile in sheet, or selected tile if not hovering
        int preview_tile = selected_tile;
        if (ImGui::IsItemHovered())
        {
            ImVec2 mp = io.MousePos;
            int hov_col = (int)((mp.x - sheet_pos.x) / (TILE_STRIDE * sheet_scale));
            int hov_row = (int)((mp.y - sheet_pos.y) / (TILE_STRIDE * sheet_scale));
            hov_col = std::clamp(hov_col, 0, SHEET_COLS - 1);
            hov_row = std::clamp(hov_row, 0, SHEET_ROWS - 1);
            preview_tile = hov_row * SHEET_COLS + hov_col;
        }

        // Highlight selected tile
        {
            int sel_col = selected_tile % SHEET_COLS;
            int sel_row = selected_tile / SHEET_COLS;
            float cx = sheet_pos.x + sel_col * TILE_STRIDE * sheet_scale;
            float cy = sheet_pos.y + sel_row * TILE_STRIDE * sheet_scale;
            float cs = TILE_STRIDE * sheet_scale;
            ImDrawList* dl = ImGui::GetWindowDrawList();
            dl->AddRect({cx, cy}, {cx + cs, cy + cs}, IM_COL32(255, 80, 80, 255), 0.f, 0, 2.f);
        }

        // Click to select tile
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            ImVec2 mp = io.MousePos;
            int col = (int)((mp.x - sheet_pos.x) / (TILE_STRIDE * sheet_scale));
            int row = (int)((mp.y - sheet_pos.y) / (TILE_STRIDE * sheet_scale));
            col = std::clamp(col, 0, SHEET_COLS - 1);
            row = std::clamp(row, 0, SHEET_ROWS - 1);
            selected_tile = row * SHEET_COLS + col;
            active_tool   = Tool::Paint;
        }

        ImGui::Separator();

        // ---- Render tile preview to 128×128 FBO ----
        {
            bifrost::Camera2d prev_cam = bifrost::GenUICamera(128, 128);
            glBindFramebuffer(GL_FRAMEBUFFER, preview_fb.id);
            glViewport(0, 0, 128, 128);
            glClearColor(0.18f, 0.18f, 0.18f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            if (preview_tile >= 0)
            {
                glm::vec2 src = TileSourceOrigin(preview_tile);
                bifrost::DrawRectangle(prev_cam, {64.0f, 64.0f}, {96.0f, 96.0f},
                    brush_rotation * 90.0f,
                    dungeon_texture, src, glm::vec2(TILE_SRC_SIZE));
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, win_fb_w, win_fb_h);
        }

        // Preview widget
        ImGui::Text("Preview");
        ImGui::Image((ImTextureID)(intptr_t)preview_fb.texture_id,
                     {128.0f, 128.0f}, {0, 1}, {1, 0});
        static const char* rot_labels[] = { "0deg", "90deg", "180deg", "270deg" };
        ImGui::Text("Rotation: %s  (R to cycle)", rot_labels[brush_rotation]);

        ImGui::Separator();

        // Level resize
        ImGui::Text("Size");
        ImGui::SetNextItemWidth(60);
        ImGui::InputInt("W##rw", &resize_w);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(60);
        ImGui::InputInt("H##rh", &resize_h);
        resize_w = std::clamp(resize_w, 1, 512);
        resize_h = std::clamp(resize_h, 1, 512);
        if (ImGui::Button("Resize"))
        {
            PushUndo(level);
            level.resize(resize_w, resize_h);
        }

        ImGui::Separator();

        // File I/O
        ImGui::Text("File");
        ImGui::SetNextItemWidth(PANEL_W - 16);
        ImGui::InputText("##path", file_path, sizeof(file_path));
        if (ImGui::Button("Save"))
        {
            if (SaveLevel(level, file_path))
                snprintf(status_msg, sizeof(status_msg), "Saved: %s", file_path);
            else
                snprintf(status_msg, sizeof(status_msg), "Save failed!");
        }
        ImGui::SameLine();
        if (ImGui::Button("Load"))
        {
            LevelData tmp;
            if (LoadLevel(tmp, file_path))
            {
                PushUndo(level);
                level     = tmp;
                resize_w  = level.width;
                resize_h  = level.height;
                snprintf(status_msg, sizeof(status_msg), "Loaded: %s", file_path);
            }
            else
            {
                snprintf(status_msg, sizeof(status_msg), "Load failed!");
            }
        }
        if (status_msg[0])
            ImGui::TextUnformatted(status_msg);

        ImGui::Separator();

        // Zoom display
        ImGui::Text("Zoom: %.2fx", zoom);
        ImGui::Text("Undo: %d  Redo: %d", (int)undo_stack.size(), (int)redo_stack.size());

        ImGui::End(); // panel

        // ---- Final render ----
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    if (fb_valid)
    {
        glDeleteFramebuffers(1, &level_fb.id);
        glDeleteTextures(1, &level_fb.texture_id);
    }
    glDeleteFramebuffers(1, &preview_fb.id);
    glDeleteTextures(1, &preview_fb.texture_id);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
