#version 300 es
precision highp float;

in vec2 v_LocalUV;            // 0..1 across the element quad
out vec4 FragColor;

uniform sampler2D u_Texture;
uniform vec4 u_Color;

// ----------------- configuration (pixel counts) -----------------
const int SLICE_LEFT   = 16;
const int SLICE_RIGHT  = 16;
const int SLICE_TOP    = 16;   // distance from top edge (in px)
const int SLICE_BOTTOM = 16;

const int TILE_CENTER = 1;     // 0 = stretch center, 1 = tile center
const float EPS = 1e-6;
// ----------------------------------------------------------------

// estimate element size in pixels using GPU derivatives
vec2 estimateElementSizePx()
{
    float du_dx = abs(dFdx(v_LocalUV.x));
    float du_dy = abs(dFdy(v_LocalUV.x));
    float dv_dx = abs(dFdx(v_LocalUV.y));
    float dv_dy = abs(dFdy(v_LocalUV.y));

    float du = max(du_dx, du_dy);
    float dv = max(dv_dx, dv_dy);

    float wpx = 1.0 / max(du, EPS);
    float hpx = 1.0 / max(dv, EPS);

    return vec2(wpx, hpx);
}

// fetch a bilinearly-filtered color from integer texel coordinates inside a region.
// regionStart = starting texel index (inclusive), regionSize = number of texels in that axis.
vec4 fetchBilinearInRegion(ivec2 texSize,
                           float tex_f_x, int regionStartX, int regionSizeX,
                           float tex_f_y, int regionStartY, int regionSizeY)
{
    int regionEndX = regionStartX + regionSizeX - 1;
    int regionEndY = regionStartY + regionSizeY - 1;

    // raw floor (may be out of region - we'll clamp indices)
    int ix_raw = int(floor(tex_f_x));
    int iy_raw = int(floor(tex_f_y));

    int ix0 = clamp(ix_raw, regionStartX, regionEndX);
    int iy0 = clamp(iy_raw, regionStartY, regionEndY);

    int ix1 = ix0 + 1;
    if (ix1 > regionEndX) ix1 = regionEndX;
    int iy1 = iy0 + 1;
    if (iy1 > regionEndY) iy1 = regionEndY;

    // fractional part relative to raw floor. If region is only 1 texel wide, force 0.
    float fx = tex_f_x - float(ix_raw);
    if (regionSizeX <= 1 || ix0 == ix1) fx = 0.0;
    float fy = tex_f_y - float(iy_raw);
    if (regionSizeY <= 1 || iy0 == iy1) fy = 0.0;

    // fetch 4 texels (safe: they are clamped inside region)
    vec4 c00 = texelFetch(u_Texture, ivec2(ix0, iy0), 0);
    vec4 c10 = texelFetch(u_Texture, ivec2(ix1, iy0), 0);
    vec4 c01 = texelFetch(u_Texture, ivec2(ix0, iy1), 0);
    vec4 c11 = texelFetch(u_Texture, ivec2(ix1, iy1), 0);

    vec4 cx0 = mix(c00, c10, fx);
    vec4 cx1 = mix(c01, c11, fx);
    return mix(cx0, cx1, fy);
}

void main()
{
    // texture size in texels
    ivec2 ts = textureSize(u_Texture, 0);
    int texW = ts.x;
    int texH = ts.y;

    // integer texel sizes for regions
    int leftTex  = clamp(SLICE_LEFT, 0, texW);
    int rightTex = clamp(SLICE_RIGHT, 0, texW - leftTex);
    int topTex   = clamp(SLICE_TOP, 0, texH);
    int botTex   = clamp(SLICE_BOTTOM, 0, texH - topTex);

    int centerTexW = max(1, texW - leftTex - rightTex);
    int centerTexH = max(1, texH - topTex - botTex);

    // element pixel size (approx)
    vec2 elemPx = estimateElementSizePx();
    float elemW = max(1.0, elemPx.x);
    float elemH = max(1.0, elemPx.y);

    // scale borders down if the element is smaller than border sum (avoid overlap)
    float needW = float(leftTex + rightTex);
    float needH = float(topTex + botTex);
    float sx = 1.0;
    if (needW > EPS && elemW < needW) sx = elemW / needW;
    float sy = 1.0;
    if (needH > EPS && elemH < needH) sy = elemH / needH;

    float leftPxF   = float(leftTex)  * sx;
    float rightPxF  = float(rightTex) * sx;
    float topPxF    = float(topTex)   * sy;
    float botPxF    = float(botTex)   * sy;

    float xL = leftPxF / elemW;
    float xR = rightPxF / elemW;
    float yB = botPxF  / elemH;
    float yT = topPxF  / elemH;

    // center element region in pixels
    float centerElemWpx = max(1.0, elemW - leftPxF - rightPxF);
    float centerElemHpx = max(1.0, elemH - topPxF - botPxF);

    float repeatsX = float(centerElemWpx) / float(centerTexW);
    float repeatsY = float(centerElemHpx) / float(centerTexH);
    bool tileX = (TILE_CENTER != 0) && (repeatsX > 1.0);
    bool tileY = (TILE_CENTER != 0) && (repeatsY > 1.0);

    // ---- compute target texel-space coordinates (floating) for this fragment ----
    // We'll compute tex_f_x in the texture's texel index space (0..texW-1)
    float tex_f_x;
    int regionStartX;
    int regionSizeX;

    if (v_LocalUV.x < xL)
    {
        // left slice maps to texels [0 .. leftTex-1]
        float tx = v_LocalUV.x / max(xL, EPS); // 0..1 within left slice
        regionStartX = 0;
        regionSizeX = leftTex;
        tex_f_x = tx * float(regionSizeX) - 0.5 + float(regionStartX);
    }
    else if (v_LocalUV.x > 1.0 - xR)
    {
        // right slice maps to texels [left+center .. texW-1]
        float tx = (v_LocalUV.x - (1.0 - xR)) / max(xR, EPS);
        regionStartX = leftTex + centerTexW;
        regionSizeX = rightTex > 0 ? rightTex : 1;
        tex_f_x = tx * float(regionSizeX) - 0.5 + float(regionStartX);
    }
    else
    {
        // center horizontal
        float centerWidth01 = max(EPS, 1.0 - xL - xR);
        float t = (v_LocalUV.x - xL) / centerWidth01; // 0..1 across center
        if (tileX) t = fract(t * repeatsX);
        regionStartX = leftTex;
        regionSizeX = centerTexW;
        tex_f_x = t * float(regionSizeX) - 0.5 + float(regionStartX);
    }

    float tex_f_y;
    int regionStartY;
    int regionSizeY;

    if (v_LocalUV.y < yB)
    {
        // bottom slice: texels [0 .. botTex-1]
        float ty = v_LocalUV.y / max(yB, EPS);
        regionStartY = 0;
        regionSizeY = botTex > 0 ? botTex : 1;
        tex_f_y = ty * float(regionSizeY) - 0.5 + float(regionStartY);
    }
    else if (v_LocalUV.y > 1.0 - yT)
    {
        // top slice: texels [bot + center .. texH-1]
        float ty = (v_LocalUV.y - (1.0 - yT)) / max(yT, EPS);
        regionStartY = botTex + centerTexH;
        regionSizeY = topTex > 0 ? topTex : 1;
        tex_f_y = ty * float(regionSizeY) - 0.5 + float(regionStartY);
    }
    else
    {
        // center vertical
        float centerHeight01 = max(EPS, 1.0 - yB - yT);
        float t = (v_LocalUV.y - yB) / centerHeight01;
        if (tileY) t = fract(t * repeatsY);
        regionStartY = botTex;
        regionSizeY = centerTexH;
        tex_f_y = t * float(regionSizeY) - 0.5 + float(regionStartY);
    }

    // fetch bilinear inside the region (no sampling outside the region)
    vec4 color = fetchBilinearInRegion(ts, tex_f_x, regionStartX, regionSizeX,
                                           tex_f_y, regionStartY, regionSizeY);

    FragColor = color * u_Color;
}
