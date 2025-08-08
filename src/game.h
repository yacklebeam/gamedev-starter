#pragma once

#include <GLFW/glfw3.h>

struct Context
{
    GLFWwindow *window;
    double frame_time;
    double time;  
};

void Init(const Context& context);
void Update(const Context& context);
void Render(const Context& context);
void Cleanup(const Context& context);