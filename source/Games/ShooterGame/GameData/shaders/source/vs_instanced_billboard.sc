$input a_position, a_texcoord0, i_data0, i_data1, i_data2, i_data3, i_data4
$output v_texcoord0, v_color0, v_normal, v_world, v_texcoord1, v_tangent, v_bitangent

#include <bgfx_shader.sh>

uniform mat4 projection;
uniform mat4 view;
uniform vec4 brightness;
uniform vec4 is_decal; // .x > 0.5 = true

void main()
{
    // Reconstruct model matrix from per-instance registers
    mat4 model = mtxFromCols(i_data0, i_data1, i_data2, i_data3);

    vec4 worldPosition = mul(model, vec4(a_position, 1.0));
    v_world = worldPosition;

    gl_Position = mul(projection, mul(view, worldPosition));

    v_texcoord0 = a_texcoord0;
    v_texcoord1 = a_texcoord0;

    v_color0 = i_data4 * vec4(brightness.x, brightness.x, brightness.x, 1.0);

    if (is_decal.x > 0.5)
        v_normal = normalize(model[2].xyz);
    else
        v_normal = vec3(0.0, 1.0, 0.0);

    v_tangent = vec3(0.0,0.0,0.0);
    v_bitangent = vec3(0.0,0.0,0.0);

}