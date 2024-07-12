#version 430 core

in vec2 v_Texture;

uniform sampler2D u_Texture;
uniform vec4 u_Color;
uniform float u_Radius;

out vec4 v_FragColor;

float map(float v, float a, float b, float x, float y)
{
    float t = (v - a) / (b - a);
    return mix(x, y, t);
}

float ddxy(vec2 v)
{
    return dFdx(v.x) + dFdy(v.y);
}

void main()
{
    vec4 TextureColor = texture(u_Texture, v_Texture);

    v_FragColor.a = TextureColor.a;
    v_FragColor.rgb = u_Color.a * u_Color.rgb + (1 - u_Color.a) * TextureColor.rgb;

    if (u_Radius > 0)
    {
        vec2 uv = vec2(map(v_Texture.x, 0, 1, -1, 1), map(v_Texture.y, 0, 1, -1, 1));
        float d = (uv.x * uv.x + uv.y * uv.y) - u_Radius;
        d = d / ddxy(uv);
        d = 1 - d;
        v_FragColor.a = d * v_FragColor.a;
    }
}