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
uniform vec3 fog_color;

uniform float useVertexLight;

//final color
out vec4 FragColor;

#ifndef MAX_POINT_LIGHTS
    #define MAX_POINT_LIGHTS 16
#endif

uniform float PointLightsNumber;

uniform mat4 PointLights[MAX_POINT_LIGHTS];

vec3 CalculateSimplePointLight(int i, vec3 pixelPosition, vec3 normal) {
    // unpack columns
    vec4 col0 = PointLights[i][0];
    vec4 col1 = PointLights[i][1];
    vec4 col2 = PointLights[i][2];

    vec3 lightPos = col0.xyz;
    float innerCone = col0.w;

    vec3 lightCol = col1.rgb;
    float radius = col1.w;

    vec3 lightFwd = col2.xyz;
    float outerCone = col2.w;

    // early-out on bad radius
    if(radius <= 0.0f)
        return vec3(0.0f);

    normal = normalize(normal);

    vec3 lightVector = lightPos - pixelPosition;
    float distanceToLight = length(lightVector);

    if(distanceToLight > radius)
        return vec3(0.0f);

    vec3 lightDir = lightVector / distanceToLight;

    float lightDot = dot(-lightDir, normalize(lightFwd));
    float dirFactor = smoothstep(outerCone, innerCone, lightDot);

    if(dirFactor <= 0.001f)
        return vec3(0.0f);
    if(dot(normal, lightDir) < 0.0f)
        return vec3(0.0f);

    float dist = max((radius - distanceToLight) / radius, 0.0f);
    float intense = dist * max(dot(normal, lightDir), 0.0f);

    return lightCol * intense * dirFactor;
}

vec4 ApplyFog(vec4 fragColor);

void main() {
    vec4 o_texture = texture(s_bspTexture, g_TexCoord);

    float vertexLightComp = 4.04f;

    vec3 vertexLightFactor = vec3(1.5f);

    if(useVertexLight > 0.0f) {
        vertexLightFactor = vertexLightComp * g_color.rgb;
    }

    vertexLightFactor *= 1.5f;

    vec3 o_lightmap = texture(s_bspLightmap, g_LmapCoord).rgb * light_color * vertexLightFactor;

    vec3 normal = normalize(g_normal);

    o_lightmap += clamp(dot(normal, normalize(direct_light_dir)) * 0.7f + 0.3f, 0.0f, 1.0f) * vec3(direct_light_color) * 2.0f * ((useVertexLight > 0.0f) ? vertexLightComp : 1.0f);

    for(int i = 0; i < min(MAX_POINT_LIGHTS, int(PointLightsNumber)); i++) {
        o_lightmap += CalculateSimplePointLight(i, g_world.xyz, normal);
    }

    FragColor = ApplyFog(o_texture * vec4(o_lightmap, 1) * 1.0f);
}

vec4 ApplyFog(vec4 fragColor) {
    float fragDistance = distance(g_world.xyz, cameraPosition);

    // Adjust for negative fog_start
    float start = max(fog_start, 0.0f); // optional, to avoid weird behavior if you want
    float end = max(fog_end, 0.0001f);  // avoid division by zero

    // Compute fog factor
    float fogFactor = (fragDistance - fog_start) / (fog_end - fog_start);

    // Clamp between 0 and 1
    fogFactor = clamp(fogFactor, 0.0f, 1.0f);

    // Apply global fog opacity
    fogFactor *= fog_opacity;

    // Blend scene color with fog color
    return vec4(mix(fragColor.rgb, fog_color, fogFactor), fragColor.a);
}
