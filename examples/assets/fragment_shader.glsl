#version 450 core
uniform vec4 bifrost_color;
out vec4 fragment_color;
void main()
{
    fragment_color = bifrost_color;
}