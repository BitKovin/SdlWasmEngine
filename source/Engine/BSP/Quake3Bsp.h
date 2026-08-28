#ifndef _QUAKE3BSP_H
#define _QUAKE3BSP_H

#include <cstring>
#include <map>
#include <string>
#include <vector>

#include <VertexData.h>

#include <IDrawMesh.h>

#include <glm.h>
#include <ShaderManager.h>
#include <BoundingBox.hpp>

// Texture abstraction – replaces raw GLuint texture handles
#include <Texture.hpp>

#include <cstddef>

#include "BSPModelDrawCommand.h"


#define FACE_POLYGON 1
#define MAX_TEXTURES 1000

#define MAP_SCALE 32.0f

// ─────────────────────────────────────────────────────────────────────────────
// BSP format identifiers
// ─────────────────────────────────────────────────────────────────────────────

// Magic bytes stored as little-endian uint32 for fast comparison
#define BSP_IDENT_IBSP  0x50534249u  // 'IBSP'
#define BSP_IDENT_RBSP  0x50534252u  // 'RBSP'
#define BSP_IDENT_FBSP  0x50534246u  // 'FBSP'

// IBSP versions
#define BSP_VERSION_Q3   46   // Quake 3 Arena / OpenArena / Xonotic / etc.
#define BSP_VERSION_WOLF 47   // Return to Castle Wolfenstein / Enemy Territory

// RBSP / FBSP version (Raven Software / qFusion)
#define BSP_VERSION_RAVEN  1

// Lightmap atlas sizes per format
#define BSP_LIGHTMAP_SIZE_IBSP  128   // IBSP: 128x128
#define BSP_LIGHTMAP_SIZE_FBSP  512   // FBSP: 512x512

// Maximum style slots per surface in RBSP/FBSP
#define BSP_MAX_LIGHTMAP_STYLES 4
#define LS_NORMAL  0x00   // always-on base style
#define LS_NONE    0xFF   // slot unused

// ─────────────────────────────────────────────────────────────────────────────
// BSP header / lump directory
// ─────────────────────────────────────────────────────────────────────────────

struct tBSPHeader {
    char    strID[4];  // 'IBSP', 'RBSP', or 'FBSP'
    int32_t version;   // 46, 47, or 1
};

struct tBSPLump {
    int32_t offset;
    int32_t length;
};



// ─────────────────────────────────────────────────────────────────────────────
// Shared geometry structs (identical across all formats)
// ─────────────────────────────────────────────────────────────────────────────

struct tBSPVertex {
    glm::vec3 vPosition;
    glm::vec2 vTextureCoord;
    glm::vec2 vLightmapCoord;
    glm::vec3 vNormal;
    uint8_t   color[4];
};

struct tBSPVertexRBSP {
    glm::vec3 vPosition;                          // 12
    glm::vec2 vTextureCoord;                      // 8
    glm::vec2 vLightmapCoord[BSP_MAX_LIGHTMAP_STYLES]; // 4*8 = 32
    glm::vec3 vNormal;                            // 12
    uint8_t   color[BSP_MAX_LIGHTMAP_STYLES][4];  // 4*4 = 16
};                                                // total = 80 bytes
static_assert(sizeof(tBSPVertexRBSP) == 80, "tBSPVertexRBSP must be 80 bytes");

struct tBSPTexture {
    char strName[64];
    int  flags;
    int  contents;
};

// ─────────────────────────────────────────────────────────────────────────────
// Face structs — IBSP (104 bytes) vs RBSP/FBSP (148 bytes)
// ─────────────────────────────────────────────────────────────────────────────

// IBSP face — single lightmap slot, no styles
struct tBSPFace {
    int       textureID;
    int       effect;
    int       type;
    int       startVertIndex;
    int       numOfVerts;
    int       startIndex;
    int       numOfIndices;
    int       lightmapID;       // single atlas page index
    int       lMapCorner[2];
    int       lMapSize[2];
    glm::vec3 lMapPos;
    glm::vec3 lMapVecs[2];
    glm::vec3 vNormal;
    int       size[2];
};
static_assert(sizeof(tBSPFace) == 104, "tBSPFace must be 104 bytes");

// RBSP/FBSP face — four lightmap + style slots (148 bytes)
// Matches Raven Software's RBSP layout used in JK2/JKA/EF/SoF2
// and the qFusion FBSP variant used in Warsow/Warfork.
struct tBSPFaceRBSP {
    int     textureID;
    int     effect;
    int     type;
    int     startVertIndex;
    int     numOfVerts;
    int     startIndex;
    int     numOfIndices;
    uint8_t lightmapStyles[BSP_MAX_LIGHTMAP_STYLES]; // style index per slot (LS_NORMAL / LS_NONE)
    uint8_t vertexStyles[BSP_MAX_LIGHTMAP_STYLES];   // same for vertex-lit fallback
    int     lightmapNum[BSP_MAX_LIGHTMAP_STYLES];    // atlas page index per slot
    int     lMapCorner[BSP_MAX_LIGHTMAP_STYLES][2];  // pixel offsets per slot (x,y pairs)
    int     lMapSize[2];
    glm::vec3 lMapPos;
    glm::vec3 lMapVecs[2];
    glm::vec3 vNormal;
    int     size[2];
};
static_assert(sizeof(tBSPFaceRBSP) == 148, "tBSPFaceRBSP must be 148 bytes");

// ─────────────────────────────────────────────────────────────────────────────
// Lightmap image storage
// IBSP:  128×128 × 3 bytes = 49152 bytes per atlas
// FBSP:  512×512 × 3 bytes = 786432 bytes per atlas
// We use a flat byte vector to handle both sizes uniformly at runtime.
// ─────────────────────────────────────────────────────────────────────────────

struct tBSPLightmap {
    uint8_t imageBits[128][128][3]; // used for IBSP; for FBSP we use the raw vector below
};

// Generic lightmap data container used by the loader for both formats.
// 'pixels' always has exactly (size * size * 3) bytes.
struct tBSPLightmapData {
    int                   size   = 128;   // width and height (square)
    std::vector<uint8_t>  pixels;         // RGB, row-major

    tBSPLightmapData() = default;
    explicit tBSPLightmapData(int s) : size(s), pixels(s * s * 3, 0) {}
};

// ─────────────────────────────────────────────────────────────────────────────
// Remaining BSP structs (format-independent)
// ─────────────────────────────────────────────────────────────────────────────

struct tBSPPlane {
    glm::vec3 normal;
    float dist;
};

struct tBSPNode {
    int plane;
    int children[2];
    int mins[3];
    int maxs[3];
};

struct tBSPLeaf {
    int cluster;
    int area;
    int mins[3];
    int maxs[3];
    int leafface;
    int n_leaffaces;
    int leafbrush;
    int n_leafbrushes;
};

struct tBSPModel {
    float mins[3];
    float maxs[3];
    int face;
    int n_faces;
    int brush;
    int n_brushes;
};

struct tBSPBrush {
    int brushside;
    int n_brushsides;
    int texture;
};

struct tBSPBrushSide {
    int plane;
    int texture;
};

struct tBSPMeshVert {
    int offset;
};

struct tBSPEffect {
    char name[64];
    int brush;
    int unknown;
};

struct tBSPLightvol {
    uint8_t ambient[3];
    uint8_t directional[3];
    uint8_t dir[2];
};

// RBSP/FBSP has 18 lumps — add LIGHTARRAY at index 17
enum eLumps {
    kEntities = 0,
    kTextures = 1,
    kPlanes = 2,
    kNodes = 3,
    kLeafs = 4,
    kLeafFaces = 5,
    kLeafBrushes = 6,
    kModels = 7,
    kBrushes = 8,
    kBrushSides = 9,
    kVertices = 10,
    kIndices = 11,
    kShaders = 12,
    kFaces = 13,
    kLightmaps = 14,
    kLightVolumes = 15,
    kVisData = 16,
    kLightArray = 17,  // RBSP/FBSP only: uint16_t indices into kLightVolumes palette
    kMaxLumps = 18
};

// Correct 30-byte struct (matches bspGridPoint_t in q3map2 source)
struct tBSPLightvolRBSP {
    uint8_t ambient[BSP_MAX_LIGHTMAP_STYLES][3];     // 12
    uint8_t directional[BSP_MAX_LIGHTMAP_STYLES][3]; // 12
    uint8_t styles[BSP_MAX_LIGHTMAP_STYLES];         //  4
    uint8_t dir[2];                                  //  2
};                                                   // = 30 bytes
static_assert(sizeof(tBSPLightvolRBSP) == 30, "tBSPLightvolRBSP must be 30 bytes");
// total = 30 bytes

static_assert(sizeof(tBSPLightvolRBSP) == 30, "tBSPLightvolRBSP must be 30 bytes");

struct tBSPVisData {
    int n_vecs;
    int sz_vecs;
    std::vector<uint8_t> vecs;
};

// ─────────────────────────────────────────────────────────────────────────────
// Render helpers
// ─────────────────────────────────────────────────────────────────────────────

struct RenderBuffers
{
    std::map<int, std::vector<VertexData>> v_faceVBOs;
    std::map<int, std::vector<uint32_t>>   v_faceIDXs;
    std::map<int, std::string>             texvec;

    // Per-face lightmap UV sets for all 4 style slots.
    // Slot 0 UVs are already baked into VertexData::ShadowMapCoords.
    // Slots 1-3 are stored here for shader multi-style blending.
    // Outer key = face index.  Inner array index = style slot (0-3).
    std::map<int, std::array<std::vector<glm::vec2>, BSP_MAX_LIGHTMAP_STYLES>> v_faceLightmapUVs;

    // Unified lightmap data (replaces the old tBSPLightmap vector)
    std::vector<tBSPLightmapData>          G_lightMaps;
    // Legacy field kept for compatibility (populated only for IBSP 128x128 maps)
    std::vector<tBSPLightmap>              G_lightMaps_Legacy;
};

struct CachedFaceTextureData
{
    int    textureId   = 0;
    string textureName = "";
    bool   isCube      = false;

    // Slot 0 is always the base lightmap (legacy path uses only this).
    // Slots 1-3 are additional style layers (RBSP/FBSP only).
    // Values are native GPU texture IDs (from m_lightmapTextures or fallback).
    int     lightmapId                             = 0;   // alias for lightmapIds[0]
    int     lightmapIds[BSP_MAX_LIGHTMAP_STYLES]   = {};  // all slots
    uint8_t lightmapStyles[BSP_MAX_LIGHTMAP_STYLES]= { LS_NONE, LS_NONE, LS_NONE, LS_NONE };
    int     numActiveSlots                         = 1;

    std::vector<int> animatedTextureFrames;

    bool   transparent = false;
    int    numOfIndices = 0;
};

struct LightVolPointData {
    glm::vec3 directColor = {};
    glm::vec3 ambientColor = {};
    glm::vec3 direction = {};

    friend LightVolPointData operator+(const LightVolPointData& a, const LightVolPointData& b) {
        return { a.directColor + b.directColor, a.ambientColor + b.ambientColor, a.direction + b.direction };
    }
    friend LightVolPointData operator-(const LightVolPointData& a, const LightVolPointData& b) {
        return { a.directColor - b.directColor, a.ambientColor - b.ambientColor, a.direction - b.direction };
    }
    friend LightVolPointData operator*(const LightVolPointData& v, float s) {
        return { v.directColor * s, v.ambientColor * s, v.direction };
    }
    friend LightVolPointData operator*(float s, const LightVolPointData& v) { return v * s; }
    friend LightVolPointData operator/(const LightVolPointData& v, float s) {
        return { v.directColor / s, v.ambientColor / s, v.direction };
    }
    LightVolPointData& operator+=(const LightVolPointData& o) {
        directColor += o.directColor; ambientColor += o.ambientColor; direction += o.direction; return *this;
    }
    LightVolPointData& operator-=(const LightVolPointData& o) {
        directColor -= o.directColor; ambientColor -= o.ambientColor; direction -= o.direction; return *this;
    }
    LightVolPointData& operator*=(float s) { directColor *= s; ambientColor *= s; direction *= s; return *this; }
    LightVolPointData& operator/=(float s) { directColor /= s; ambientColor /= s; direction /= s; return *this; }

    static LightVolPointData Lerp(const LightVolPointData& a, const LightVolPointData& b, float t) {
        LightVolPointData r;
        r.directColor  = glm::mix(a.directColor,  b.directColor,  t);
        r.ambientColor = glm::mix(a.ambientColor, b.ambientColor, t);
        r.direction    = glm::normalize(glm::slerp(a.direction, b.direction, t));
        return r;
    }
};

struct FaceRenderData {
    int               faceIndex;
    bool              useLightmap;
    LightVolPointData lightPointData;
    mat4              modelMatrix;
};

struct OpaqueModelVBO {
    bgfx::VertexBufferHandle vbo = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle  ibo = BGFX_INVALID_HANDLE;
    uint32_t                 IndexCount = 0;
};

struct MergedModelFacesData {
    // Offsets/counts into CQuake3BSP::m_worldVBO / m_worldIBO — the whole
    // map shares one vertex buffer and one index buffer; each merged group
    // is just a sub-range of it, selected at draw time via setVertexBuffer/
    // setIndexBuffer start+count parameters.
    uint32_t vertexOffset = 0;
    uint32_t vertexCount  = 0;
    uint32_t indexOffset  = 0;
    uint32_t IndexCount   = 0;
    uint32   referenceFace = 0;
    uint32   uId           = 0;
    BoundingBox bounds;
};

class BSPModelRef;

// ─────────────────────────────────────────────────────────────────────────────
// BSPPortal
// ─────────────────────────────────────────────────────────────────────────────

struct BSPPortal
{
    int                    planeIndex;
    std::vector<glm::vec3> vertices;
    int                    frontChild;
    int                    backChild;
    int                    frontLeafIndex = -1;
    int                    backLeafIndex  = -1;
    glm::vec3              center;
    BoundingBox            bounds;

    bool IsLeafPortal()  const { return frontLeafIndex >= 0 && backLeafIndex >= 0; }
    int  FrontLeaf()     const { return frontLeafIndex; }
    int  BackLeaf()      const { return backLeafIndex; }
    int  OtherLeaf(int knownLeaf) const {
        if (frontLeafIndex == knownLeaf) return backLeafIndex;
        if (backLeafIndex  == knownLeaf) return frontLeafIndex;
        return -1;
    }
    int FrontCluster(const std::vector<tBSPLeaf>& leafs) const {
        return (frontLeafIndex >= 0 && frontLeafIndex < (int)leafs.size())
            ? leafs[frontLeafIndex].cluster : -1;
    }
    int BackCluster(const std::vector<tBSPLeaf>& leafs) const {
        return (backLeafIndex >= 0 && backLeafIndex < (int)leafs.size())
            ? leafs[backLeafIndex].cluster : -1;
    }
    float Area() const {
        if (vertices.size() < 3) return 0.f;
        float area = 0.f;
        const glm::vec3& o = vertices[0];
        for (size_t i = 1; i + 1 < vertices.size(); ++i)
            area += glm::length(glm::cross(vertices[i] - o, vertices[i+1] - o));
        return area * 0.5f;
    }
};



// ─────────────────────────────────────────────────────────────────────────────
// CQuake3BSP
// ─────────────────────────────────────────────────────────────────────────────

class CQuake3BSP : public IDrawMesh
{
public:
    CQuake3BSP();
    ~CQuake3BSP();

    bool LoadBSP(const char* filename);

    // ── Format detection (set during LoadBSP) ─────────────────────────────────
    bool    m_isFBSP        = false;  // true for FBSP (qFusion/Warsow)
    bool    m_isRBSP        = false;  // true for RBSP or FBSP (Raven-derived)
    int     m_lightmapSize  = BSP_LIGHTMAP_SIZE_IBSP;  // 128 or 512
    int     m_bspVersion    = 0;
    uint32_t m_bspIdent     = 0;

    int m_numOfVerts;
    int count;
    int indcount;
    int tcoordcount;

    char   tname[MAX_TEXTURES][64];
    int    textureID;

    std::vector<std::shared_ptr<Texture>> m_faceTextures;

    string filePath = "";

    void GenerateTexture();
    void GenerateLightmap();
    bool RenderMergedFace(int index, bool lightmap, LightVolPointData lightData, mat4 model);

    void BuildVBO();
    void CreateVBO_Patch(int index);
    void CreateVBO(int m_numOfFaces);
    void FillExtraLightmapUVs();
    void BSPDebug(int index);
    void CreateIndices(int index);

    std::string GetLightMapFilePathFromId(int id, const std::string& filePath);

    void PreloadFace(int index);
    void PreloadFaces();

    void BuildMergedModels();

    int m_numOfFaces;
    int m_numOfIndices;
    int m_numOfTextures;
    int m_numOfLightmaps;
    int numVisibleFaces;
    int skipindices;

    static inline vec3 lightVolGridSize = vec3(64, 64, 128);

    std::vector<std::shared_ptr<Texture>> m_lightmapTextures;
    std::shared_ptr<Texture> m_missingLightmap;
    std::shared_ptr<Texture> m_whiteLightmap;

    int*              m_pIndices;
    tBSPVertex*       m_pVerts;
    tBSPVertexRBSP*   m_pVertsRBSP;  // raw on-disk RBSP/FBSP verts (nullptr for IBSP)
    tBSPFace*         m_pFaces;      // always populated (converted from RBSP if needed)
    tBSPFaceRBSP*     m_pFacesRBSP;  // raw on-disk RBSP/FBSP faces (nullptr for IBSP)

    // Whole-map shared buffers: every merged face group is a sub-range of
    // these, addressed via MergedModelFacesData::vertexOffset/indexOffset.
    bgfx::VertexBufferHandle m_worldVBO = BGFX_INVALID_HANDLE;
    bgfx::IndexBufferHandle  m_worldIBO = BGFX_INVALID_HANDLE;

    RenderBuffers  Rbuffers;
    tBSPTexture*   pTextures;
    tBSPLightmap*  pLightmaps;  // legacy 128x128 raw data (nullptr for FBSP)

    std::string entities;
    std::vector<tBSPPlane>     planes;
    std::vector<tBSPNode>      nodes;
    std::vector<tBSPLeaf>      leafs;
    std::vector<int>           leafFaces;
    std::vector<int>           leafBrushes;
    std::vector<tBSPModel>     models;
    std::vector<tBSPBrush>     brushes;
    std::vector<tBSPBrushSide> brushSides;
    std::vector<tBSPMeshVert>  meshVerts;
    std::vector<tBSPEffect>    effects;

    std::vector<tBSPLightvol>     lightVols;
    std::vector<tBSPLightvol>     lightVolPalette;
    std::vector<uint32_t>         lightVolIndices;

    // Full 4-slot lightvol data for RBSP/FBSP. Indexed by the same lightVolIndices.
    std::vector<tBSPLightvolRBSP> lightVolPaletteRBSP;

    std::vector<OpaqueModelVBO>      opaqueVBOs;
    std::vector<MergedModelFacesData> mergedFacesData;
    vector<int>                       mergedFacesMapping;

    CachedFaceTextureData* cachedFaces;

    std::vector<BoundingBox> faceBounds;

    tBSPVisData visData;

    vector<FaceRenderData> facesToDrawTransparent;

    glm::vec3 originalMins;
    glm::vec3 originalMaxs;

    std::vector<BSPPortal>              portals;
    int                                 m_numClusters = 0;
    std::vector<std::vector<int>>       m_clusterToPortals;
    std::vector<std::vector<int>>       m_portalAdjacency;

    std::vector<VertexData> GetFaceVertices(int faceId);
    std::vector<uint32_t>   GetFaceIndices(int faceId);

    inline tBSPLightvol GetLightVol(int i) const {
        if (i < 0 || i >= (int)lightVolIndices.size()) {
            tBSPLightvol fallback{};
            fallback.ambient[0] = fallback.ambient[1] = fallback.ambient[2] = 76; // ~0.3
            return fallback;
        }
        uint32_t idx = lightVolIndices[i];
        if (idx >= lightVolPalette.size()) {
            tBSPLightvol fallback{};
            return fallback;
        }
        return lightVolPalette[idx];
    }

    inline tBSPLightvolRBSP GetLightVolRBSP(int i) const {
        if (i < 0 || i >= (int)lightVolIndices.size()) return tBSPLightvolRBSP{};
        uint32_t idx = lightVolIndices[i];
        if (!lightVolPaletteRBSP.empty() && idx < lightVolPaletteRBSP.size())
            return lightVolPaletteRBSP[idx];
        // Fallback: wrap IBSP data
        tBSPLightvolRBSP r{};
        if (idx < lightVolPalette.size()) {
            const tBSPLightvol& v = lightVolPalette[idx];
            for (int c = 0; c < 3; ++c) {
                r.ambient[0][c] = v.ambient[c];
                r.directional[0][c] = v.directional[c];
            }
            r.styles[0] = LS_NORMAL;
            r.dir[0] = v.dir[0];
            r.dir[1] = v.dir[1];
        }
        return r;
    }

    // Returns the GPU texture native ID for a given face's lightmap style slot.
    // Slot 0 = base (always-on). Slots 1-3 = switchable styles (RBSP/FBSP only).
    int GetFaceLightmapId(int faceIndex, int slot = 0) const;

    bool CheckLightProbeAcess(const glm::vec3& position, const glm::vec3& volPosition);

    LightVolPointData GetLightvolColorPoint(const glm::vec3& position, bool wallCheck = false);
    LightVolPointData GetLightvolColor(const glm::vec3& position, bool wallCheck = false);

    // Casts a ray from 'start' to 'end' (engine Y-up world space, BSP units)
    // against worldspawn (model 0) faces that carry a lightmap.
    // Returns the bilinearly-filtered RGB lightmap colour at the first hit,
    // or vec3(0) if the ray misses all lit geometry.
    glm::vec3 LinetraceLightmapColor(glm::vec3 start, glm::vec3 end);

    int  FindClusterAtPosition(glm::vec3 cameraPos);
    bool IsClusterVisible(int sourceCluster, int testCluster);

    glm::vec3 GetStyleColor(uint8_t style) const;

    void DrawForward(mat4x4 view, mat4x4 projection);

    void RenderBSP(const glm::vec3& cameraPos, tBSPModel& model,
        mat4 modelMatrix, bool useClusterVis, bool lightmap);

    void RenderTransparentFaces();
    bool IsFaceTransparent(int index);

    vector<BSPModelRef> GetAllModelRefs();

    void BuildStaticOpaqueObstacles();
    void LoadToLevel();

    int GetFaceTextureNativeId(int cachedTextureId) const;
    int GetLightmapNativeId(int lightmapSlot) const;
    int GetWhiteLightmapNativeId() const;

    void LoadPortalsFromPRT(const std::string& content);
    void BuildClusterToPortalsMap();

    std::vector<vec3> FindPath(vec3 start, vec3 target, int smoothSubdiv = 0, float smoothAlpha = 0.0f);

private:
    // Converts a raw on-disk RBSP/FBSP face into the canonical tBSPFace.
    // Slot 0 of the 4-element arrays maps to the existing single-slot fields.
    // The raw face is also kept in m_pFacesRBSP for multi-style access.
    static tBSPFace ConvertRBSPFace(const tBSPFaceRBSP& src);

    // Samples the lightmap colour at a world-space hit point on a given face.
    // For embedded lightmaps (G_lightMaps) performs bilinear filtering on raw
    // CPU pixel data.  For external lightmaps (per-face TGA files) it delegates
    // to Texture::SampleRGB() — a placeholder that must be implemented in
    // Texture.hpp / Texture.cpp to perform CPU-side texel lookup.
    //
    // hitPos : engine Y-up world space, BSP units (same frame as v_faceVBOs).
    glm::vec3 SampleLightmapFace(int faceIndex, const glm::vec3& hitPos);

    void PrecomputeFaceAABBs();

    struct FaceAABB { glm::vec3 mn, mx; };
    std::vector<FaceAABB> m_faceAABBs;   // built once after BSP load
    
    static constexpr int kBezierTessLevel = 10;

};


// ─────────────────────────────────────────────────────────────────────────────
// BSPModelRef
// ─────────────────────────────────────────────────────────────────────────────

class BSPModelRef : public IDrawMesh
{
public:
    CQuake3BSP* bsp = nullptr;
    int         id  = -1;
    tBSPModel&  model;

    bool Static           = true;
    bool useBspVisibility = false;

    vec3 Position    = vec3(0);
    vec3 Rotation    = vec3(0);
    vec3 Scale       = vec3(1);
    vec3 avgPosition = vec3(0);

    mat4        finalWorldMatrix;
    BoundingBox bounds;

    BoundingBox GetTransformedBounds();
    mat4        GetWorldMatrix();

	BoundingBox GetBoundingBox() override { return GetTransformedBounds(); }

    BSPModelRef(CQuake3BSP* bsp_ptr, int model_id, tBSPModel& model_ref);
    ~BSPModelRef();

    void  BuildVisBlocker();
    float GetDistanceToCamera();
    void  CalculateAveragePosition();

    bool IsCameraVisible();
    bool IsInFrustrum(Frustum frustrum);
    bool IsBspVisible();
    bool CheckPointBspVisible(int cameraCluster, vec3 position);

    vector<tBSPFace> GetFaces();

    vector<MeshUtils::PositionVerticesIndices> GetNavObstacleMeshes();
    vector<VertexData> GetVertices(bool collisionOnly = false, bool opaqueOnly = false);
    vector<uint32_t>   GetIndices(bool collisionOnly = false, bool opaqueOnly = false);

    void FinalizeFrameData();

    void CollectDrawCommands(std::vector<IDrawCommand*>& outCommands) override
    {
        outCommands.push_back(&drawCommand);
    }

private:
    BSPModelDrawCommand drawCommand;
};

#endif
