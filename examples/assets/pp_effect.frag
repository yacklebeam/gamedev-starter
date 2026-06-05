#version 450 core

uniform sampler2D bifrost_tex;
uniform vec4 bifrost_color;

in vec2 texture_coords;
out vec4 fragment_color;

void main()
{
    vec4 color = texture(bifrost_tex, texture_coords) * bifrost_color;

    // Greyscale
    float grey = dot(color.rgb, vec3(0.299, 0.587, 0.114));

    // Vignette — darken towards corners
    vec2 uv = texture_coords - vec2(0.5);
    float vignette = 1.0 - dot(uv, uv) * 2.5;

    fragment_color = vec4(vec3(grey) * max(vignette, 0.0), color.a);
}
