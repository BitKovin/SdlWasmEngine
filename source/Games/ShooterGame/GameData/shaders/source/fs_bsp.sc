$input v_texcoord0, v_texcoord1, v_normal, v_color0, v_world

#include <bgfx_shader.sh>

SAMPLER2D(s_bspTexture,  0);
SAMPLER2D(s_bspLightmap, 1);

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

#ifndef MAX_POINT_LIGHTS
    #define MAX_POINT_LIGHTS 16
#endif

uniform mat4 PointLights[MAX_POINT_LIGHTS];

vec3 CalculateSimplePointLight(int i, vec3 pixelPosition, vec3 normal)
{
    vec4 col0 = PointLights[i][0];
    vec4 col1 = PointLights[i][1];
    vec4 col2 = PointLights[i][2];

    vec3  lightPos  = col0.xyz;
    float innerCone = col0.w;
    vec3  lightCol  = col1.rgb;
    float radius    = col1.w;
    vec3  lightFwd  = col2.xyz;
    float outerCone = col2.w;

    if (radius <= 0.0)
        return vec3(0.0, 0.0, 0.0);

    normal = normalize(normal);

    vec3  lightVector     = lightPos - pixelPosition;
    float distanceToLight = length(lightVector);

    if (distanceToLight > radius)
        return vec3(0.0, 0.0, 0.0);

    vec3  lightDir  = lightVector / distanceToLight;
    float lightDot  = dot(-lightDir, normalize(lightFwd));
    float dirFactor = smoothstep(outerCone, innerCone, lightDot);

    if (dirFactor <= 0.001)
        return vec3(0.0, 0.0, 0.0);
    if (dot(normal, lightDir) < 0.0)
        return vec3(0.0, 0.0, 0.0);

    float dist    = max((radius - distanceToLight) / radius, 0.0);
    float intense = dist * max(dot(normal, lightDir), 0.0);

    return lightCol * intense * dirFactor;
}

vec4 ApplyFog(vec4 fragColor, vec3 pixelPosition)
{
    float fragDistance = distance(pixelPosition, cameraPosition.xyz);
    float fogFactor    = (fragDistance - fog_start.x) / (fog_end.x - fog_start.x);
    fogFactor          = clamp(fogFactor, 0.0, 1.0) * fog_opacity.x;
    return vec4(mix(fragColor.rgb, fog_color.rgb, fogFactor), fragColor.a);
}

void main()
{
    vec4 o_texture = texture2D(s_bspTexture, v_texcoord0);

    float vertexLightComp   = 4.04;
    vec3  vertexLightFactor = vec3(1.5, 1.5, 1.5);

    if (useVertexLight.x > 0.0)
        vertexLightFactor = vec3(vertexLightComp,vertexLightComp,vertexLightComp) * v_color0.rgb;

    vertexLightFactor *= 1.5;

    vec3 o_lightmap = texture2D(s_bspLightmap, v_texcoord1).rgb
                      * light_color.rgb
                      * vertexLightFactor;

    vec3  normal  = normalize(v_normal);
    float dirDot  = clamp(dot(normal, normalize(direct_light_dir.xyz)) * 0.7 + 0.3, 0.0, 1.0);
    float vlScale = (useVertexLight.x > 0.0) ? vertexLightComp : 1.0;
    o_lightmap   += dirDot * direct_light_color.rgb * 2.0 * vlScale;

    int numLights = min(MAX_POINT_LIGHTS, int(PointLightsNumber.x));
    for (int i = 0; i < numLights; i++)
        o_lightmap += CalculateSimplePointLight(i, v_world.xyz, normal);

    gl_FragColor = ApplyFog(o_texture * vec4(o_lightmap, 1.0), v_world.xyz);
}
