#version 450 core

uniform sampler2D bifrost_tex;
uniform vec4 bifrost_color;

in vec2 texture_coords;
out vec4 fragment_color;

void main()
{
    // Snap UVs to a coarse grid to simulate a lower-resolution display.
    // Adjust these to taste — smaller = chunkier pixels.
    const vec2 virtual_resolution = vec2(80.0, 60.0);

    vec2 pixel_size = 1.0 / virtual_resolution;
    vec2 uv = floor(texture_coords / pixel_size) * pixel_size + pixel_size * 0.5;

    fragment_color = texture(bifrost_tex, uv) * bifrost_color;
}
