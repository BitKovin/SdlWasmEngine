#version 300 es
precision highp float;

in vec2 g_TexCoord;
in vec2 g_LmapCoord;
in vec3 g_normal;
in vec4 g_color;
in vec4 g_world;

uniform vec3 cameraPosition;

//Texture samplers
uniform samplerCube s_bspTexture;

//final color
out vec4 FragColor;

void main()
{
    vec4 o_texture  = texture(s_bspTexture, normalize(g_world.xyz - cameraPosition) * vec3(-1.0,1.0,1.0));

    FragColor = o_texture;
}
