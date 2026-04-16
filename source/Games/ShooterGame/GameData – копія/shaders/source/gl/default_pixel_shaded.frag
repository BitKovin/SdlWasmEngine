#version 300 es
precision highp float;
in vec2 v_texcoord;
in vec4 v_color;
in vec3 v_normal;
in vec3 v_worldPosition;
in vec4 v_clipPosition;

in vec4 v_shadowCoords1;
in vec4 v_shadowCoords2;
in vec4 v_shadowCoords3;
in vec4 v_shadowCoords4;

out vec4 FragColor;
uniform sampler2D u_texture;  // Changed from "texture" to avoid keyword conflict

uniform vec3 cameraPosition;

uniform highp sampler2DShadow shadowMap;
uniform highp sampler2D      shadowMapRaw; 
uniform highp sampler2DShadow shadowMapDetail;
uniform highp sampler2D      shadowMapDetailRaw; 
uniform int shadowMapSize;  // width, height of shadow map
uniform vec3 lightDirection;   
uniform float shadowDistance1;
uniform float shadowDistance2;
uniform float shadowDistance3;
uniform float shadowDistance4;
uniform float shadowRadius1;
uniform float shadowRadius2;
uniform float shadowRadius3;
uniform float shadowRadius4;
uniform float shaddowOffsetScale;

uniform bool is_particle;
uniform bool is_decal;

//#define USE_PCSS_DIRECT
#define USE_PCSS_INDIRECT

vec3 CalculateDirectionalLight();

void main() {
    vec4 texColor = texture(u_texture, v_texcoord)*v_color;

    vec3 color = texColor.rgb;
    float alpha = texColor.a;

    if(alpha <0.01)
        discard;

    vec3 ligthColor = CalculateDirectionalLight();

    color*=ligthColor;

    FragColor = vec4(color, alpha);
}

//—— 1) project + atlas remap → [0..1] ——
vec3 projectPC(vec4 sc, vec2 mapOffset) {
    vec3 pc = sc.xyz / sc.w * 0.5 + 0.5;
    pc.xy   = pc.xy * 0.5 + mapOffset * 0.5;
    return pc;
}

//—— 2) original bias calculation ——
float computeBias(
    float shadowDistance,
    int   pcfRadius,
    float biasScale,
    float NdotL
) {
    float worldTexel = (2.0 * shadowDistance) / float(shadowMapSize);
    float worldBase  = biasScale * worldTexel;
    float worldBias  = worldBase * (1.0 + (1.0 - NdotL));
    return worldBias / (shadowDistance + 1000.0) * (float(pcfRadius) + 1.0);
}

//—— 3a) fixed‑radius PCF — returns [0..1] ——
float computePCF(
    highp sampler2DShadow mapCmp,
    vec3            pc,
    vec2            texelSize,
    float           bias,
    int             radius,
    vec2            tileMin,
    vec2            tileMax
) {
    float sum = 0.0;
    float n   = 0.0;
    
    for (int xo = -radius; xo <= radius; ++xo) {
        for (int yo = -radius; yo <= radius; ++yo) {
            n += 1.0;
            vec2 uv = clamp(
                pc.xy + vec2(xo,yo) * texelSize,
                tileMin, tileMax
            );
            sum += texture(
                mapCmp,
                vec3(uv, pc.z - bias)
            );
        }
    }
    return sum / n;
}

//—— 3b) PCSS — returns [0..1] ——
float computePCSS(
    sampler2D       mapRaw,
    highp sampler2DShadow mapCmp,
    vec3            pc,
    vec2            texelSize,
    float           baseBias,
    int             pcfRadius,
    float           shadowDistance,
    vec2            tileMin,
    vec2            tileMax,
    float           lightRadius,
    float           hideStart,
    float           hideEnd
) {
    // 3b.1) Blocker search (3×3)
    const int br = 3;
    float sumB = 0.0;
    int   cntB = 0;
    for (int xo = -br; xo <= br; ++xo) {
        for (int yo = -br; yo <= br; ++yo) {
            vec2 uv = clamp(
                pc.xy + vec2(xo, yo) * texelSize * length(vec2(xo, yo)) * 5.0,
                tileMin, tileMax
            );
            float d = texture(mapRaw, uv).r;
            if (d < pc.z - baseBias) {
                sumB += d;
                cntB++;
            }
        }
    }
    if (cntB == 0) return 1.0; // No blockers, fully lit

    float avgB = sumB / float(cntB);

    // 3b.2) Linearize into world-space [0..maxDepth]
    float maxDepth = shadowDistance + 1000.0;
    float rWS = pc.z * maxDepth;
    float bWS = avgB * maxDepth;

    // 3b.3) Penumbra width in world units
    float penWS = lightRadius * (rWS - bWS) / bWS;

    // 3b.4) To texel radius
    float penUV = penWS / (2.0 * shadowDistance);
    float rTex = penUV * float(shadowMapSize);
    float maxR = float(pcfRadius) * 8.0;
    rTex = clamp(rTex, 2.0, maxR);
    int r = int(ceil(rTex));

    // 3b.5) Dynamic bias
    float dratio = (rWS - bWS) / bWS;
    float dynBias = baseBias * max(dratio, 1.0);

    // 3b.6) PCF compare pass
    float sumS = 0.0;
    int   cntS = 0;
    for (int xo = -r; xo <= r; ++xo) {
        for (int yo = -r; yo <= r; ++yo) {
            vec2 uv = clamp(
                pc.xy + vec2(xo, yo) * texelSize,
                tileMin, tileMax
            );
            sumS += texture(
                mapCmp,
                vec3(uv, pc.z - dynBias)
            );
            cntS++;
        }
    }

    float pcssFactor = sumS / float(cntS);

    // Apply fade based on distance to caster
    float depthDiffWS = rWS - bWS;
    float fadeT = clamp((depthDiffWS - hideStart) / (hideEnd - hideStart), 0.0, 1.0);
    fadeT = smoothstep(0.0, 1.0, fadeT);
    float fadedPcssFactor = mix(pcssFactor, 1.0, fadeT);

    return fadedPcssFactor;
}

//—— your entry point (header unchanged) ——
vec2 GetDirectionalShadow(
    vec2  mapOffset,
    float shadowDistance,
    vec4  shadowCoords,
    int   pcfRadius,
    float biasScale
) {



    // 1) project
    vec3 pc = projectPC(shadowCoords, mapOffset);

    // 2) prep bias & NdotL & tile bounds
    float NdotL    = clamp(dot(normalize(v_normal),
                               normalize(lightDirection)),
                           0.0, 1.0);
    float baseBias = computeBias(shadowDistance, pcfRadius, biasScale, NdotL);
    vec2  texelSz  = vec2(0.5) / float(shadowMapSize);
    vec2  tileMin  = mapOffset * 0.5;
    vec2  tileMax  = tileMin + vec2(0.5);

    // 3) direct pass
    float direct;
    #ifdef USE_PCSS_DIRECT
    direct = computePCSS(
        shadowMapRaw, shadowMap,
        pc, texelSz,
        baseBias,
        pcfRadius,
        shadowDistance,
        tileMin, tileMax,
    );
    #else

    direct = computePCF(
        shadowMap,
        pc, texelSz,
        baseBias,
        pcfRadius,
        tileMin, tileMax
    );
    

    #endif

    float directNoDetail = direct;

    // 4) indirect pass
    float indirect;
    #ifdef USE_PCSS_INDIRECT
    

        indirect = computePCSS(
            shadowMapDetailRaw, shadowMapDetail,
            pc, texelSz,
            baseBias,
            pcfRadius,
            shadowDistance,
            tileMin, tileMax, 
            mix(200.0, 40.0, directNoDetail),
            0.0,mix(5.0, 50.0, directNoDetail)
        );
    


    #else
        float sampleScale = mix(4.0, 1.0, directNoDetail);
        int   ir          = int(ceil(float(pcfRadius) * sampleScale));
        float indirectBias= baseBias; // no over‑scale
        indirect = computePCF(
            shadowMapDetail,
            pc,
            texelSz * sampleScale,
            indirectBias,
            ir,
            tileMin, tileMax
        );
    #endif

    // 5) mix exactly like your original PCF
    float finalDirect   = mix(0.0, directNoDetail, indirect);
    float finalIndirect = mix(indirect * 0.7 + 0.3, 1.0, directNoDetail);

    return vec2(finalDirect, finalIndirect);
}

vec3 CalculateDirectionalLight()
{
    // 1) Basic N·L
    float directionPower = is_particle
        ? 1.0
        : clamp(dot(-v_normal, lightDirection), 0.0, 1.0);

    float indirectPower = dot(-v_normal, vec3(0, -1, 0)) / 2.0f + 0.5f;

    if(is_particle)
    {
        indirectPower = 1.0;
    }

    
    indirectPower = mix(0.2,0.35,indirectPower);

    // 2) Distance from the camera
    float dist = v_clipPosition.z;

    
    float biasScales[4]        = float[]( 1.0,
                                      1.0,
                                      1.0,
                                      1.0 );


    // Cascade parameters (unchanged)
    float cascadeDist[4]   = float[](shadowDistance1, shadowDistance2, shadowDistance3, shadowDistance4);
    float cascadeRadius[4] = float[](shadowRadius1,   shadowRadius2,   shadowRadius3,   shadowRadius4);
    vec2  cascadeOff[4]    = vec2[](vec2(0,0), vec2(1,0), vec2(0,1), vec2(1,1));
    vec4  cascadeCoords[4] = vec4[](v_shadowCoords1, v_shadowCoords2, v_shadowCoords3, v_shadowCoords4);
    int   cascadePCF[4]    = int[](2, 1, 1, 1);

    vec2 shadow = vec2(1.0);
    float blendPercent = 0.15; // 15% overlap for all cascades

    if(dist < cascadeDist[3]) {
        bool blended = false;

        // Blend between cascades 0–1, 1–2, 2–3
        for (int i = 0; i < 3; ++i) {
            float prevDist   = (i == 0) ? 0.0 : cascadeDist[i-1];
            float depthSpan  = cascadeDist[i] - prevDist;
            float fadeSize   = depthSpan * blendPercent;
            float fadeStart  = cascadeDist[i] - fadeSize;
            float fadeEnd    = cascadeDist[i];

            if (dist >= fadeStart && dist <= fadeEnd) {
                float blendF = smoothstep(fadeStart, fadeEnd, dist);

                // Sample both cascades using their correct radii
                vec2 sh0 = GetDirectionalShadow(
                    cascadeOff[i],
                    cascadeRadius[i],
                    cascadeCoords[i],
                    cascadePCF[i],
                    biasScales[i]
                );
                vec2 sh1 = GetDirectionalShadow(
                    cascadeOff[i+1],
                    cascadeRadius[i+1],
                    cascadeCoords[i+1],
                    cascadePCF[i+1],
                    biasScales[i+1]
                );
                shadow = mix(sh0, sh1, blendF);
                blended = true;
                break;
            }
        }

        // If not in a fade region, pick the matching cascade
        if (!blended) {
            int idx = 0;
            for (int i = 0; i < 4; ++i) {
                if (dist <= cascadeDist[i]) {
                    idx = i;
                    break;
                }
            }
            shadow = GetDirectionalShadow(
                cascadeOff[idx],
                cascadeRadius[idx],
                cascadeCoords[idx],
                cascadePCF[idx],
                biasScales[idx]
            );
        }
    }

    

    directionPower *= shadow.x;
    indirectPower *= shadow.y;

    return mix(vec3(indirectPower), vec3(1.0), directionPower);
}