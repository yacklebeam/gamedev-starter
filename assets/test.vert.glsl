#version 450 core
layout (location = 0) in vec2 position;
out vec2 texture_coords;
uniform mat4 mvp;
void main()
{
    gl_Position = mvp * vec4(position, 0.0, 1.0);
    texture_coords = position + vec2(0.5);
}