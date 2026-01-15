#version 300 es
precision highp float;

in vec2 g_TexCoord;
in vec2 g_LmapCoord;
in vec3 g_normal;
in vec4 g_color;

in vec4 g_world;

uniform vec3 cameraPosition;

//Texture samplers
uniform sampler2D s_bspTexture;
uniform sampler2D s_bspLightmap;
uniform vec3 light_color; 
uniform vec3 direct_light_color; 
uniform vec3 direct_light_dir; 

uniform float fog_start;
uniform float fog_end;
uniform float fog_opacity;
uniform vec3  fog_color;

uniform bool useVertexLight;

//final color
out vec4 FragColor;

#ifndef MAX_POINT_LIGHTS
    #define MAX_POINT_LIGHTS 16
#endif

uniform int PointLightsNumber;
uniform vec4 LightPositions[MAX_POINT_LIGHTS]; // xyz = position, w = inner cone (see note)
uniform vec3 LightColors[MAX_POINT_LIGHTS];
uniform float LightRadiuses[MAX_POINT_LIGHTS];
uniform vec4 LightDirections[MAX_POINT_LIGHTS]; // xyz = direction, w = outer cone (see note)

vec3 CalculateSimplePointLight(int i, vec3 pixelPosition, vec3 normal)
{
    // ensure normal is normalized
    normal = normalize(normal);

    // vector from pixel -> light
    vec3 lightVector = LightPositions[i].xyz - pixelPosition;
    float distanceToLight = length(lightVector);

    // early-out if outside light radius or invalid radius
    if (distanceToLight > LightRadiuses[i] || LightRadiuses[i] <= 0.0)
        return vec3(0.0);

    // normalized direction from pixel to light
    vec3 lightDir = lightVector / distanceToLight; // avoids computing length twice

    // dot between vector from light->pixel and the stored spotlight direction
    // note: -lightDir is from light to pixel (since lightDir is pixel->light)
    float lightDot = dot(-lightDir, normalize(LightDirections[i].xyz));

    float innerCone = LightPositions[i].w;
    float outerCone = LightDirections[i].w;

    // smooth transition between outer and inner cones
    float dirFactor = smoothstep(outerCone, innerCone, lightDot);

    if (dirFactor <= 0.001)
        return vec3(0.0);

    // backface / lighting check (preserve original logic)
    if (dot(normal, lightDir) < 0.0)
        return vec3(0.0);

    // simple radial attenuation (linear)
    float dist = max((LightRadiuses[i] - distanceToLight) / LightRadiuses[i], 0.0);
    float intense = dist; // original used just 'dist' (comment shows alternative)
    intense *= max(dot(normal, lightDir), 0.0);
    intense = max(intense, 0.0);    

    vec3 L = LightColors[i] * intense;

    return L * dirFactor;
}

vec4 ApplyFog(vec4 fragColor);

void main()
{
    vec4 o_texture = texture(s_bspTexture,  g_TexCoord);


    float vertexLightComp = 4.04;

    vec3 vertexLightFactor = vec3(1.5);

    if(useVertexLight)
    {
        vertexLightFactor = vertexLightComp * g_color.rgb;
    }

    vertexLightFactor *= 1.5f;

    vec3 o_lightmap = texture(s_bspLightmap, g_LmapCoord).rgb * light_color * vertexLightFactor;

    vec3 normal = normalize(g_normal);

    o_lightmap += clamp(dot(normal, normalize(direct_light_dir))*0.7 + 0.3,0.0,1.0) * vec3(direct_light_color) * 2.0 * (useVertexLight ? vertexLightComp : 1.0);

    for (int i = 0; i < min(MAX_POINT_LIGHTS, PointLightsNumber); i++)
	{
		o_lightmap += CalculateSimplePointLight(i, g_world.xyz, normal);
	}

    FragColor = ApplyFog(o_texture * vec4(o_lightmap,1) * 1.0);
}

vec4 ApplyFog(vec4 fragColor)
{
    float fragDistance = distance(g_world.xyz, cameraPosition);

    // Adjust for negative fog_start
    float start = max(fog_start, 0.0); // optional, to avoid weird behavior if you want
    float end = max(fog_end, 0.0001);  // avoid division by zero

    // Compute fog factor
    float fogFactor = (fragDistance - fog_start) / (fog_end - fog_start);

    // Clamp between 0 and 1
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    // Apply global fog opacity
    fogFactor *= fog_opacity;

    // Blend scene color with fog color
    return vec4(mix(fragColor.rgb, fog_color, fogFactor), fragColor.a);
}
