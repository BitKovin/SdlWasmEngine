#version 300 es

// Vertex attributes for the quad
layout(location = 0) in vec3 Position;           // Local position of the quad vertex.
layout(location = 2) in vec2 TextureCoordinate;    // Texture coordinate.

// Instanced attributes: instance model matrix (splitted into 4 vec4) and instance color.
layout(location = 10) in vec4 instanceModel0;
layout(location = 11) in vec4 instanceModel1;
layout(location = 12) in vec4 instanceModel2;
layout(location = 13) in vec4 instanceModel3;
layout(location = 14) in vec4 instanceColor;

uniform float brightness;

uniform mat4 projection;
uniform mat4 view;

uniform mat4 lightMatrix1;
uniform mat4 lightMatrix2;
uniform mat4 lightMatrix3;
uniform mat4 lightMatrix4;

out vec2 v_texcoord;
out vec4 v_color;
out vec3 v_normal;
out vec3 v_worldPosition;
out vec4 v_clipPosition;

out vec4 v_shadowCoords1;
out vec4 v_shadowCoords2;
out vec4 v_shadowCoords3;
out vec4 v_shadowCoords4;

uniform bool is_decal;

void main()
{
    // Reconstruct the complete model matrix from the instanced columns.
    mat4 model = mat4(instanceModel0, instanceModel1, instanceModel2, instanceModel3);

    // Compute world position using the instance model matrix.
    vec4 worldPosition = model * vec4(Position, 1.0);

    v_worldPosition = worldPosition.xyz;

    // Compute clip space position.
    gl_Position = projection * view * worldPosition;

    v_clipPosition = gl_Position;

    v_texcoord = TextureCoordinate;
    v_color = instanceColor * vec4(brightness,brightness,brightness, 1);

    v_normal = vec3(0,1,0);

    if(is_decal)
    {
        vec3 forward = normalize(model[2].xyz);
        v_normal = forward;
    }

    v_shadowCoords1 = lightMatrix1 * worldPosition;
    v_shadowCoords2 = lightMatrix2 * worldPosition;
    v_shadowCoords3 = lightMatrix3 * worldPosition;
    v_shadowCoords4 = lightMatrix4 * worldPosition;


}
