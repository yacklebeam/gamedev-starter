cmake_minimum_required(VERSION 3.10)

set (CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(game-project)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT game)

add_subdirectory(externals/glfw)

add_executable(game
    src/main.cpp

    externals/bifrost/bifrost.cpp
    externals/bifrost/bifrost_input.cpp
    
    externals/miniaudio/miniaudio.c

    externals/imgui/backends/imgui_impl_glfw.cpp
    externals/imgui/backends/imgui_impl_opengl3.cpp
    externals/imgui/imgui.cpp
    externals/imgui/imgui_draw.cpp
    externals/imgui/imgui_tables.cpp
    externals/imgui/imgui_widgets.cpp
)

add_dependencies(game glfw)

source_group("imgui" FILES 
    externals/imgui/backends/imgui_impl_glfw.cpp
    externals/imgui/backends/imgui_impl_opengl3.cpp
    externals/imgui/imgui.cpp
    externals/imgui/imgui_draw.cpp
    externals/imgui/imgui_tables.cpp
    externals/imgui/imgui_widgets.cpp
)

source_group("bifrost" FILES 
    externals/bifrost/bifrost.cpp
    externals/bifrost/bifrost_input.cpp
)

source_group("miniaudio" FILES 
    externals/miniaudio/miniaudio.c
)

target_include_directories(game PUBLIC externals/imgui)
target_include_directories(game PUBLIC externals/glfw/include)
target_include_directories(game PUBLIC externals/glfw/deps)
target_include_directories(game PUBLIC externals)
target_include_directories(game PUBLIC src)

target_link_libraries(game PUBLIC glfw)
IF (WIN32)
    target_link_libraries(game PUBLIC opengl32)
    target_link_libraries(game PUBLIC gdi32)
    target_link_libraries(game PUBLIC shell32)
    target_link_options(game PRIVATE "/SUBSYSTEM:WINDOWS")
ENDIF()

add_custom_command(TARGET game PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets/ $<TARGET_FILE_DIR:game>
)
