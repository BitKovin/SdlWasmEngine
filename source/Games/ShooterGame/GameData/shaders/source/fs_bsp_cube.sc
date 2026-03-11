$input v_texcoord0, v_texcoord1, v_normal, v_color0, v_world

#include <bgfx_shader.sh>

SAMPLERCUBE(s_bspTexture, 0);

uniform vec4 cameraPosition;

void main()
{
    vec3 dir = normalize(v_world.xyz - cameraPosition.xyz) * vec3(-1.0, 1.0, 1.0);
    gl_FragColor = textureCube(s_bspTexture, dir);
}