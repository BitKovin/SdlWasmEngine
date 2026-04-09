$input v_texcoord0, v_texcoord1, v_texcoord2, v_texcoord3, v_texcoord4, v_normal, v_color0, v_world

#include <bgfx_shader.sh>

SAMPLER2D(s_bspTexture,  0);
SAMPLER2D(s_bspLightmap, 1);
SAMPLER2D(s_bspLightmap1,2);
SAMPLER2D(s_bspLightmap2,3);
SAMPLER2D(s_bspLightmap3,4);

uniform vec4 cameraPosition;
uniform vec4 light_color;
uniform vec4 direct_light_color;
uniform vec4 direct_light_dir;
uniform vec4 fog_start;
uniform vec4 fog_end;
uniform vec4 fog_opacity;
uniform vec4 fog_color;
uniform vec4 useVertexLight;
uniform vec4 PointLightsNumber;
uniform vec4 brightness;
uniform vec4 numLightmapSlots;
uniform vec4 lmStyleColor0;
uniform vec4 lmStyleColor1;
uniform vec4 lmStyleColor2;
uniform vec4 lmStyleColor3;
uniform vec4 isRBSP;

#ifndef MAX_POINT_LIGHTS
    #define MAX_POINT_LIGHTS 16
#endif
uniform mat4 PointLights[MAX_POINT_LIGHTS];

vec3 CalculateSimplePointLight(int i, vec3 pixelPosition, vec3 normal)
{
    vec4 col0 = mtxGetColumn(PointLights[i], 0);
    vec4 col1 = mtxGetColumn(PointLights[i], 1);
    vec4 col2 = mtxGetColumn(PointLights[i], 2);
    vec3 lightPos = col0.xyz; float innerCone = col0.w;
    vec3 lightCol = col1.rgb; float radius    = col1.w;
    vec3 lightFwd = col2.xyz; float outerCone = col2.w;
    if (radius <= 0.0) return vec3_splat(0.0);
    vec3  lightVector     = lightPos - pixelPosition;
    float distanceToLight = length(lightVector);
    if (distanceToLight > radius) return vec3_splat(0.0);
    vec3  lightDir  = lightVector / distanceToLight;
    float dirFactor = smoothstep(outerCone, innerCone, dot(-lightDir, normalize(lightFwd)));
    if (dirFactor <= 0.001) return vec3_splat(0.0);
    vec3 n = normalize(normal);
    if (dot(n, lightDir) < 0.0) return vec3_splat(0.0);
    float dist    = max((radius - distanceToLight) / radius, 0.0);
    float intense = dist * max(dot(n, lightDir), 0.0);
    return lightCol * intense * dirFactor;
}

vec4 ApplyFog(vec4 fragColor, vec3 pixelPosition)
{
    float fragDistance = distance(pixelPosition, cameraPosition.xyz);
    float fogFactor    = clamp((fragDistance - fog_start.x) / (fog_end.x - fog_start.x), 0.0, 1.0) * fog_opacity.x;
    return vec4(mix(fragColor.rgb, fog_color.rgb, fogFactor), fragColor.a);
}

void main()
{
    vec4 o_texture = texture2D(s_bspTexture, v_texcoord0);

    vec3 o_lightmap = vec3_splat(0.0);

    if(isRBSP.x > 0.0)
    {

        vec3 vertexLightFactor = (useVertexLight.x > 0.0) ? v_color0.rgb : vec3_splat(1.0);

        // All slots share the same UV — they live in separate atlas textures
        int  slots     = int(numLightmapSlots.x);

        if (slots > 0) o_lightmap += texture2D(s_bspLightmap,  v_texcoord1).rgb * lmStyleColor0.rgb;
        if (slots > 1) o_lightmap += texture2D(s_bspLightmap1, v_texcoord2).rgb * lmStyleColor1.rgb;
        if (slots > 2) o_lightmap += texture2D(s_bspLightmap2, v_texcoord3).rgb * lmStyleColor2.rgb;
        if (slots > 3) o_lightmap += texture2D(s_bspLightmap3, v_texcoord4).rgb * lmStyleColor3.rgb;

        o_lightmap *= vertexLightFactor;

    }else
    {
        float vertexLightComp   = 4.04;
        vec3  vertexLightFactor = vec3(1.5, 1.5, 1.5);

    if (useVertexLight.x > 0.0)
        vertexLightFactor = vec3(vertexLightComp,vertexLightComp,vertexLightComp) * v_color0.rgb;

        vertexLightFactor *= 1.5;

        o_lightmap = texture2D(s_bspLightmap, v_texcoord1).rgb
                      * vertexLightFactor;

    }

    

    vec3  normal = normalize(v_normal);
    float dirDot = clamp(dot(normal, normalize(direct_light_dir.xyz)), 0.0, 1.0);
    o_lightmap  += mix(light_color.rgb, direct_light_color.rgb, dirDot);

    int numLights = min(MAX_POINT_LIGHTS, int(PointLightsNumber.x));
    for (int i = 0; i < numLights; i++)
        o_lightmap += CalculateSimplePointLight(i, v_world.xyz, normal);

    gl_FragColor = ApplyFog(
        o_texture * vec4(o_lightmap, 1.0) * vec4(brightness.x, brightness.x, brightness.x, 1.0),
        v_world.xyz);
}