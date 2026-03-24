$input a_position, a_normal, a_texcoord0, a_texcoord2, a_color0
$output v_texcoord0, v_color0, v_normal, v_world, v_texcoord1

#include <bgfx_shader.sh>

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Extract upper-left 3x3 from a mat4
mat3 mat4ToMat3(mat4 m)
{
    return mat3(m[0].xyz, m[1].xyz, m[2].xyz);
}

// Analytic inverse of a 3x3 matrix via adjugate
mat3 inverseMat3(mat3 m)
{
    float a = m[0][0], b = m[0][1], c = m[0][2];
    float d = m[1][0], e = m[1][1], f = m[1][2];
    float g = m[2][0], h = m[2][1], k = m[2][2];

    float det = a*(e*k - f*h) - b*(d*k - f*g) + c*(d*h - e*g);

    mat3 adj = mat3(
         (e*k - f*h), -(b*k - c*h),  (b*f - c*e),
        -(d*k - f*g),  (a*k - c*g), -(a*f - c*d),
         (d*h - e*g), -(a*h - b*g),  (a*e - b*d)
    );

    return adj * (1.0 / det);
}

void main()
{
    v_world     = mul(model, vec4(a_position, 1.0));
    gl_Position = mul(projection, mul(view, v_world));

    mat3 normalMatrix = transpose(inverseMat3(mat4ToMat3(model)));
    v_normal    = normalize(mul(normalMatrix, a_normal));

    v_texcoord0 = a_texcoord0;
    v_texcoord1 = a_texcoord2;
    v_color0    = a_color0;
}
