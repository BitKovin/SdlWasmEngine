$input v_texcoord0, v_color0, v_normal, v_world, v_texcoord1, v_tangent, v_bitangent

#include <../bgfx_shader.sh>

SAMPLER2D(u_texture, 0);

// vec3 uniforms packed into vec4 — access .xyz.  Names kept identical.
uniform vec4 cameraPosition;

// bool uniforms packed into vec4 — .x > 0.5 == true.  Names kept identical.
uniform vec4 is_particle;
uniform vec4 is_decal;

uniform vec4 light_color;
uniform vec4 direct_light_color;
uniform vec4 direct_light_dir;

void main()
{
    vec3 color = vec3(0.17, 0.09, 0.09);
    float alpha = 0.75;

    gl_FragColor = vec4(color, alpha);
}
