$input a_position, a_color0, a_indices, a_weight
$output v_color0

#include <bgfx_shader.sh>

uniform mat4 u_mvp;
uniform mat4 finalBonesMatrices[128];

mat4 GetBoneTransform(vec4 indices, vec4 weights)
{
    const mat4 identity = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );

    float sum = weights.x + weights.y + weights.z + weights.w;
    if (sum < 0.0001)
        return identity;

    weights /= sum;

    mat4 skin = mat4(
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 0.0
    );

    int i0 = int(indices.x);
    int i1 = int(indices.y);
    int i2 = int(indices.z);
    int i3 = int(indices.w);


    skin += finalBonesMatrices[i0] * weights.x;
    skin += finalBonesMatrices[i1] * weights.y;
    skin += finalBonesMatrices[i2] * weights.z;
    skin += finalBonesMatrices[i3] * weights.w;

    return skin;
}

void main()
{
    mat4 skinMatrix = GetBoneTransform(a_indices, a_weight);
    vec4 skinnedPos = mul(skinMatrix, vec4(a_position, 1.0));
    gl_Position = mul(u_mvp, skinnedPos);
    v_color0 = a_color0;
}
