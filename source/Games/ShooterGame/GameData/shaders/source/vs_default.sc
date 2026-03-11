$input a_position, a_normal, a_texcoord0, a_texcoord2, a_indices, a_weight, a_color0
$output v_texcoord0, v_color0, v_normal, v_world

#include <bgfx_shader.sh>

uniform mat4 projection;
uniform mat4 view;
uniform mat4 world;

uniform mat4 lightMatrix1;
uniform mat4 lightMatrix2;
uniform mat4 lightMatrix3;
uniform mat4 lightMatrix4;

uniform vec4 brightness;
uniform vec4 viewmodelScaleFactor;
uniform vec4 isViewmodel;   // .x > 0.5 = true  (was bool)

#define MAX_BONES 128
uniform mat4 finalBonesMatrices[MAX_BONES];

mat3 mat4ToMat3(mat4 m)
{
    return mat3(m[0].xyz, m[1].xyz, m[2].xyz);
}

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

mat4 GetBoneTransforms(vec4 indices, vec4 weights)
{
    mat4 identity = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0);

    float sum = weights.x + weights.y + weights.z + weights.w;
    if (sum < 0.01)
        return identity;

    return
        finalBonesMatrices[int(indices.x)] * (weights.x / sum) +
        finalBonesMatrices[int(indices.y)] * (weights.y / sum) +
        finalBonesMatrices[int(indices.z)] * (weights.z / sum) +
        finalBonesMatrices[int(indices.w)] * (weights.w / sum);
}

void main()
{
    mat4 boneTrans      = GetBoneTransforms(a_indices, a_weight);
    mat4 vertWorldTrans = mul(world, boneTrans);

    vec4 worldPos = mul(vertWorldTrans, vec4(a_position, 1.0));
    v_world = worldPos;

    vec4 clipPos = mul(projection, mul(view, worldPos));
    if (isViewmodel.x > 0.5)
        clipPos.z *= 0.2 * viewmodelScaleFactor.x;
    gl_Position = clipPos;

    mat3 m3           = mat4ToMat3(vertWorldTrans);
    mat3 normalMatrix = transpose(inverseMat3(m3));
    if (determinant(m3) < 0.0)
        normalMatrix = -normalMatrix;

    v_normal    = normalize(mul(normalMatrix, a_normal));
    v_color0    = vec4(brightness.x, brightness.x, brightness.x, 1.0) * a_color0;
    v_texcoord0 = a_texcoord0;
}