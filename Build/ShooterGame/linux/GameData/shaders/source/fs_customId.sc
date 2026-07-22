$input v_texcoord0, v_color0, v_normal, v_world, v_texcoord1, v_tangent, v_bitangent

#include <bgfx_shader.sh>

SAMPLER2D(u_texture, 0);

uniform vec4 customId; // .x used

vec3 encodeId24(float id)
{
    // Clamp safely in float domain
    id = clamp(id, 0.0, 16777215.0);

    float r = floor(id / 65536.0);
    float g = floor((id - r * 65536.0) / 256.0);
    float b = floor(id - r * 65536.0 - g * 256.0);

    return vec3(r, g, b) / 255.0;
}

void main()
{
    vec4 texColor = texture2D(u_texture, v_texcoord0) * v_color0;

    if (texColor.a < 0.99)
    {
        discard;
    }

    float id = floor(customId.x + 0.5);

    gl_FragColor = vec4(encodeId24(id), 1.0);
}