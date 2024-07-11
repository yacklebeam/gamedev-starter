#pragma once

#include <GLFW/glfw3.h>

struct Context
{
    GLFWwindow *window;
    double frame_time;
    double time;  
};

void Init(Context *context);
void Update(Context *context);
void Render(Context *context);
void Cleanup(Context *context);