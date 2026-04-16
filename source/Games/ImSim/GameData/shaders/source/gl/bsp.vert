#version 300 es

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TextureCoordinate;
layout(location = 8) in vec4 Color; // Lightmap coords
layout(location = 9) in vec2 ShadowMapCoords; // Lightmap coords


out vec2 g_TexCoord;
out vec2 g_LmapCoord;
out vec3 g_normal;
out vec4 g_color;
out vec4 g_world;


uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;



void main()
{

    g_world = model * vec4(Position, 1.0f);

    gl_Position = projection * view * g_world;
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    g_normal = normalize(normalMatrix * Normal);

    g_TexCoord = vec2(TextureCoordinate.x, TextureCoordinate.y);
    g_LmapCoord = vec2(ShadowMapCoords.x, ShadowMapCoords.y);

    g_color = Color;
}

