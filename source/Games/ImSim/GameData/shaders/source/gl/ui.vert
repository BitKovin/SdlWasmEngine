#version 300 es

layout (location = 0) in vec2 a_Position;  // usually your quad vertex positions
layout (location = 1) in vec2 a_TexCoord;  // original mesh texcoord (0..1 if simple quad)

uniform mat4 u_Model;
uniform mat4 u_Projection;

out vec2 v_TexCoord;   // pass-through, in case you still want direct UV sampling
out vec2 v_LocalUV;    // always normalized [0..1] across the quad

void main()
{
    // For 9-slice we assume the mesh's a_TexCoord is already in [0..1].
    // If that's the case, we can reuse it directly as localUV.
    v_LocalUV = a_TexCoord;

    // Forward original texcoord too (optional, in case some shaders use it)
    v_TexCoord = a_TexCoord;

    gl_Position = u_Projection * u_Model * vec4(a_Position, 0.0, 1.0);
}
