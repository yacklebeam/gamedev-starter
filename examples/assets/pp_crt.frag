#version 450 core

uniform sampler2D bifrost_tex;
uniform vec4 bifrost_color;

in vec2 texture_coords;
out vec4 fragment_color;

void main()
{
    // ── Barrel distortion ──────────────────────────────────────────────────
    // Remap UV to [-0.5, 0.5], apply radial scale, then back to [0, 1].
    // The multiplier grows toward the corners, pushing those UVs outside [0,1]
    // which produces the rounded black border of a CRT tube.
    vec2 uv = texture_coords - 0.5;
    float r2 = dot(uv, uv);
    uv *= 1.0 + r2 * 0.3;
    uv += 0.5;

    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
    {
        fragment_color = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    vec4 color = texture(bifrost_tex, uv) * bifrost_color;

    // ── Scanlines ──────────────────────────────────────────────────────────
    // Alternate brightness every other screen-space row.
    float scanline = mod(floor(gl_FragCoord.y), 2.0) < 1.0 ? 1.0 : 0.6;
    color.rgb *= scanline;

    // ── Vignette ───────────────────────────────────────────────────────────
    // Darken toward the edges to simulate the phosphor falloff on a curved tube.
    float vignette = 1.0 - dot(uv - 0.5, uv - 0.5) * 3.5;
    color.rgb *= max(vignette, 0.0);

    fragment_color = color;
}
