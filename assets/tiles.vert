#version 430 core

layout (location = 0) in vec2 v_Position;
layout (location = 1) in vec2 v_TexturePosition;
layout (location = 2) in vec2 offset;
layout (location = 3) in mat3 model;

uniform mat4 vp;

out vec2 v_Texture;

void main()
{
    v_Texture = v_TexturePosition;
    gl_Position = u_MVP * vec4(v_Position, 1.0);
}