$input v_texcoord0, v_color0, v_normal, v_world, v_texcoord1, v_tangent, v_bitangent

#include <bgfx_shader.sh>

void main() {

    vec3 finalColor = vec3(0.0, 0.0, 0.0);
    float alpha = 0.2;

    gl_FragColor = vec4(finalColor, alpha);
}