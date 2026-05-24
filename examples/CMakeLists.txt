cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

project(bifrost-examples)

set(ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

add_subdirectory(${ROOT}/externals/glfw ${CMAKE_BINARY_DIR}/glfw)

set(BIFROST_SRC ${ROOT}/externals/bifrost)

set(IMGUI_SRC ${ROOT}/externals/imgui)
set(IMGUI_SRCS
    ${IMGUI_SRC}/backends/imgui_impl_glfw.cpp
    ${IMGUI_SRC}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_SRC}/imgui.cpp
    ${IMGUI_SRC}/imgui_draw.cpp
    ${IMGUI_SRC}/imgui_tables.cpp
    ${IMGUI_SRC}/imgui_widgets.cpp
)

function(add_example name)
    set(extra_sources "")
    foreach(mod IN LISTS ARGN)
        list(APPEND extra_sources ${BIFROST_SRC}/${mod}.cpp)
    endforeach()

    add_executable(${name} ${name}.cpp ${BIFROST_SRC}/bifrost.cpp ${extra_sources})
    add_dependencies(${name} glfw)
    target_include_directories(${name} PUBLIC
        ${ROOT}/externals
        ${ROOT}/externals/glfw/include
        ${ROOT}/externals/glfw/deps
        ${ROOT}/externals/imgui
    )
    target_link_libraries(${name} PUBLIC glfw)
    IF (WIN32)
        target_link_libraries(${name} PUBLIC opengl32 gdi32 shell32)
    ENDIF()
endfunction()

add_example(basic)
add_example(input bifrost_input)
add_example(dungeon bifrost_input bifrost_dungeon)
add_example(collision bifrost_input bifrost_collision)

add_executable(imgui imgui.cpp ${BIFROST_SRC}/bifrost.cpp ${IMGUI_SRCS})
add_dependencies(imgui glfw)
target_include_directories(imgui PUBLIC
    ${ROOT}/externals
    ${ROOT}/externals/glfw/include
    ${ROOT}/externals/glfw/deps
    ${ROOT}/externals/imgui
    ${ROOT}/externals/imgui/backends
)
target_link_libraries(imgui PUBLIC glfw)
IF (WIN32)
    target_link_libraries(imgui PUBLIC opengl32 gdi32 shell32)
ENDIF()
