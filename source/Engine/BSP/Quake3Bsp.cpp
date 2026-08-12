#include "Quake3Bsp.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include <DebugDraw.hpp>
#include <AssetRegistry.h>
#include <Camera.h>
#include <Level.hpp>
#include <MapParser.h>
#include <LevelObjectFactory.h>
#include <Renderer/Renderer.h>
#include <EngineMain.h>
#include <FileSystem/FileSystem.h>
#include <LightSystem/LightManager.h>

#include <Renderer/Abstractions/ViewIdManager.h>

#include <BgfxStateManager.h>

#include <ShaderManager.h>
#include <Physics.h>

#include <Logger.hpp>

#ifndef _MSC_VER
#  define strcpy_s strcpy
#  define strcat_s strcat
#endif


// ─────────────────────────────────────────────────────────────────────────────
// Construction / destruction
// ─────────────────────────────────────────────────────────────────────────────

CQuake3BSP::CQuake3BSP()
    : m_numOfVerts(0), m_numOfFaces(0), m_numOfIndices(0),
    m_numOfTextures(0), m_numOfLightmaps(0),
    numVisibleFaces(0), skipindices(0),
    m_pVerts(nullptr), m_pVertsRBSP(nullptr),
    m_pFaces(nullptr), m_pFacesRBSP(nullptr),
    m_pIndices(nullptr),
    pTextures(nullptr), pLightmaps(nullptr), cachedFaces(nullptr),
    textureID(0), count(0), indcount(0), tcoordcount(0)
{}

CQuake3BSP::~CQuake3BSP()
{
    if (filePath.empty()) return;

    for (auto& modelVBO : opaqueVBOs)
    {
        if (bgfx::isValid(modelVBO.vbo)) bgfx::destroy(modelVBO.vbo);
        if (bgfx::isValid(modelVBO.ibo)) bgfx::destroy(modelVBO.ibo);
    }

    for (auto& mergedModel : mergedFacesData)
    {
        if (bgfx::isValid(mergedModel.vbo)) bgfx::destroy(mergedModel.vbo);
        if (bgfx::isValid(mergedModel.ibo)) bgfx::destroy(mergedModel.ibo);
    }

    delete[] m_pVerts;
    delete[] m_pVertsRBSP;
    delete[] m_pFaces;
    delete[] m_pFacesRBSP;
    delete[] m_pIndices;
    delete[] pTextures;
    delete[] pLightmaps;
    delete[] cachedFaces;

    // m_lightmapTextures, m_missingLightmap, m_whiteLightmap, m_faceTextures
    // are all shared_ptr<Texture> – destroyed automatically.
}

// ─────────────────────────────────────────────────────────────────────────────
// LoadBSP
// ─────────────────────────────────────────────────────────────────────────────

// ─────────────────────────────────────────────────────────────────────────────
// ConvertRBSPFace
// Converts a 148-byte RBSP/FBSP face to the canonical 104-byte tBSPFace.
// Slot 0 of each 4-element array is the base (always-on) style and maps
// directly to the existing single-slot fields used by the rest of the engine.
// ─────────────────────────────────────────────────────────────────────────────
/*static*/ tBSPFace CQuake3BSP::ConvertRBSPFace(const tBSPFaceRBSP& s)
{
    tBSPFace d{};
    d.textureID = s.textureID;
    d.effect = s.effect;
    d.type = s.type;
    d.startVertIndex = s.startVertIndex;
    d.numOfVerts = s.numOfVerts;
    d.startIndex = s.startIndex;
    d.numOfIndices = s.numOfIndices;
    // Base lightmap is always slot 0 (LS_NORMAL).
    // If the slot is unused (LS_NONE) fall back to -1 (no lightmap).
    d.lightmapID = (s.lightmapStyles[0] != LS_NONE) ? s.lightmapNum[0] : -1;

    if (s.lightmapStyles[1] != LS_NONE)
        Logger::Log("");

    d.lMapCorner[0] = s.lMapCorner[0][0];
    d.lMapCorner[1] = s.lMapCorner[0][1];
    d.lMapSize[0] = s.lMapSize[0];
    d.lMapSize[1] = s.lMapSize[1];
    d.lMapPos = s.lMapPos;
    d.lMapVecs[0] = s.lMapVecs[0];
    d.lMapVecs[1] = s.lMapVecs[1];
    d.vNormal = s.vNormal;
    d.size[0] = s.size[0];
    d.size[1] = s.size[1];
    return d;
}

bool CQuake3BSP::LoadBSP(const char* filename)
{
    if (!filename) {
        printf("ERROR:: You must specify BSP file as parameter\n");
        return false;
    }

    std::vector<uint8_t> fileData;
    try {
        fileData = FileSystemEngine::ReadFileBinary(filename);
    }
    catch (const std::exception& e) {
        printf("ERROR:: cannot read BSP file '%s': %s\n", filename, e.what());
        return false;
    }

    const uint8_t* base = fileData.data();
    size_t         totalSize = fileData.size();

    if (totalSize < sizeof(tBSPHeader) + sizeof(tBSPLump) * kMaxLumps) {
        printf("ERROR:: BSP too small\n");
        return false;
    }

    tBSPHeader header;
    memcpy(&header, base, sizeof(header));

    // ── Format detection ──────────────────────────────────────────────────────
    uint32_t ident = 0;
    memcpy(&ident, header.strID, 4);
    m_bspIdent = ident;
    m_bspVersion = header.version;

    if (ident == BSP_IDENT_FBSP) {
        m_isFBSP = true;
        m_isRBSP = true;
        m_lightmapSize = BSP_LIGHTMAP_SIZE_FBSP;
        printf("BSP format: FBSP v%d (qFusion/Warsow) — 512x512 lightmaps, 4-slot styles\n",
            header.version);
    }
    else if (ident == BSP_IDENT_RBSP) {
        m_isFBSP = false;
        m_isRBSP = true;
        m_lightmapSize = BSP_LIGHTMAP_SIZE_IBSP;
        printf("BSP format: RBSP v%d (Raven Software) — 128x128 lightmaps, 4-slot styles\n",
            header.version);
    }
    else if (ident == BSP_IDENT_IBSP) {
        m_isFBSP = false;
        m_isRBSP = false;
        m_lightmapSize = BSP_LIGHTMAP_SIZE_IBSP;
        printf("BSP format: IBSP v%d (%s) — 128x128 lightmaps\n",
            header.version,
            header.version == BSP_VERSION_WOLF ? "RTCW/ET" : "Quake 3");
    }
    else {
        char id[5] = {};
        memcpy(id, header.strID, 4);
        printf("ERROR:: Unknown BSP magic '%.4s' (0x%08X)\n", id, ident);
        return false;
    }

    tBSPLump lumps[kMaxLumps];
    memcpy(lumps, base + sizeof(header), sizeof(lumps));

    filePath = filename;

    auto checkLump = [&](int idx, size_t elemSize, size_t& outCount) -> bool {
        const auto& L = lumps[idx];
        if (L.offset < 0 || L.length < 0 ||
            size_t(L.offset) + size_t(L.length) > totalSize ||
            (L.length % elemSize) != 0)
        {
            outCount = 0;
            return false;
        }
        outCount = L.length / elemSize;
        return true;
        };

    // Entities (text)
    {
        size_t cnt = 0;
        if (checkLump(kEntities, 1, cnt) && cnt > 0) {
            const char* ptr = reinterpret_cast<const char*>(base + lumps[kEntities].offset);
            entities.assign(ptr, cnt);
        }
    }

    // Planes
    {
        size_t cnt = 0;
        if (checkLump(kPlanes, sizeof(tBSPPlane), cnt)) {
            planes.resize(cnt);
            memcpy(planes.data(), base + lumps[kPlanes].offset, cnt * sizeof(tBSPPlane));
            for (auto& p : planes) {
                float oldY = p.normal.y;
                p.normal.y = p.normal.z;
                p.normal.z = -oldY;
            }
        }
    }

    // Nodes
    {
        size_t cnt = 0;
        if (checkLump(kNodes, sizeof(tBSPNode), cnt)) {
            nodes.resize(cnt);
            memcpy(nodes.data(), base + lumps[kNodes].offset, cnt * sizeof(tBSPNode));
        }
    }

    // Leafs
    {
        size_t cnt = 0;
        if (checkLump(kLeafs, sizeof(tBSPLeaf), cnt)) {
            leafs.resize(cnt);
            memcpy(leafs.data(), base + lumps[kLeafs].offset, cnt * sizeof(tBSPLeaf));
            for (auto& lf : leafs) {
                float t = lf.mins[1];
                lf.mins[1] = lf.mins[2];
                lf.mins[2] = -t;
                t = lf.maxs[1];
                lf.maxs[1] = lf.maxs[2];
                lf.maxs[2] = -t;
            }
        }
    }

    // LeafFaces
    {
        size_t cnt = 0;
        if (checkLump(kLeafFaces, sizeof(int), cnt)) {
            leafFaces.resize(cnt);
            memcpy(leafFaces.data(), base + lumps[kLeafFaces].offset, cnt * sizeof(int));
        }
    }

    // LeafBrushes
    {
        size_t cnt = 0;
        if (checkLump(kLeafBrushes, sizeof(int), cnt)) {
            leafBrushes.resize(cnt);
            memcpy(leafBrushes.data(), base + lumps[kLeafBrushes].offset, cnt * sizeof(int));
        }
    }

    // Models + original bounds
    {
        size_t cnt = 0;
        if (checkLump(kModels, sizeof(tBSPModel), cnt)) {
            models.resize(cnt);
            memcpy(models.data(), base + lumps[kModels].offset, cnt * sizeof(tBSPModel));

            if (!models.empty()) {
                originalMins = glm::vec3(models[0].mins[0], models[0].mins[1], models[0].mins[2]);
                originalMaxs = glm::vec3(models[0].maxs[0], models[0].maxs[1], models[0].maxs[2]);
            }
            for (auto& m : models) {
                float t = m.mins[1];
                m.mins[1] = m.mins[2];
                m.mins[2] = -t;
                t = m.maxs[1];
                m.maxs[1] = m.maxs[2];
                m.maxs[2] = -t;
            }
        }
    }

    // Brushes & BrushSides
    {
        size_t cntB = 0, cntBS = 0;
        if (checkLump(kBrushes, sizeof(tBSPBrush), cntB)) {
            brushes.resize(cntB);
            memcpy(brushes.data(), base + lumps[kBrushes].offset, cntB * sizeof(tBSPBrush));
        }
        if (checkLump(kBrushSides, sizeof(tBSPBrushSide), cntBS)) {
            brushSides.resize(cntBS);
            memcpy(brushSides.data(), base + lumps[kBrushSides].offset, cntBS * sizeof(tBSPBrushSide));
        }
    }

    // MeshVerts
    {
        size_t cnt = 0;
        if (checkLump(kIndices, sizeof(tBSPMeshVert), cnt)) {
            meshVerts.resize(cnt);
            memcpy(meshVerts.data(), base + lumps[kIndices].offset, cnt * sizeof(tBSPMeshVert));
        }
    }

    // Effects
    {
        size_t cnt = 0;
        if (checkLump(kShaders, sizeof(tBSPEffect), cnt)) {
            effects.resize(cnt);
            memcpy(effects.data(), base + lumps[kShaders].offset, cnt * sizeof(tBSPEffect));
        }
    }

    {
        if (m_isRBSP) {
            // Lump 15: palette of bspGridPoint_t (already deduplicated by q3map2)
            const auto& LP = lumps[kLightVolumes];
            const size_t paletteCount = (LP.offset >= 0 && LP.length > 0
                && size_t(LP.offset) + size_t(LP.length) <= totalSize)
                ? size_t(LP.length) / sizeof(tBSPLightvolRBSP) : 0;

            if (paletteCount > 0) {
                const uint8_t* ptr = base + LP.offset;
                lightVolPaletteRBSP.resize(paletteCount);
                memcpy(lightVolPaletteRBSP.data(), ptr, paletteCount * sizeof(tBSPLightvolRBSP));

                // Build the canonical IBSP palette from it (sum all active style slots)
                lightVolPalette.resize(paletteCount);
                for (size_t j = 0; j < paletteCount; ++j) {
                    const tBSPLightvolRBSP& r = lightVolPaletteRBSP[j];
                    tBSPLightvol& lv = lightVolPalette[j];
                    uint32_t a0 = 0, a1 = 0, a2 = 0, d0 = 0, d1 = 0, d2 = 0;
                    for (int s = 0; s < BSP_MAX_LIGHTMAP_STYLES; ++s) {
                        if (r.styles[s] == LS_NONE && s > 0) continue;
                        a0 += r.ambient[s][0]; a1 += r.ambient[s][1]; a2 += r.ambient[s][2];
                        d0 += r.directional[s][0]; d1 += r.directional[s][1]; d2 += r.directional[s][2];
                    }
                    lv.ambient[0] = (uint8_t)std::min(255u, a0);
                    lv.ambient[1] = (uint8_t)std::min(255u, a1);
                    lv.ambient[2] = (uint8_t)std::min(255u, a2);
                    lv.directional[0] = (uint8_t)std::min(255u, d0);
                    lv.directional[1] = (uint8_t)std::min(255u, d1);
                    lv.directional[2] = (uint8_t)std::min(255u, d2);
                    lv.dir[0] = r.dir[0];
                    lv.dir[1] = r.dir[1];
                }
            }

            // Lump 17: per-cell uint16_t indices into the palette
            const auto& LA = lumps[kLightArray];
            const size_t indexCount = (LA.offset >= 0 && LA.length > 0
                && size_t(LA.offset) + size_t(LA.length) <= totalSize)
                ? size_t(LA.length) / sizeof(uint16_t) : 0;

            lightVolIndices.resize(indexCount);
            if (indexCount > 0) {
                const uint16_t* src = reinterpret_cast<const uint16_t*>(base + LA.offset);
                for (size_t j = 0; j < indexCount; ++j)
                    lightVolIndices[j] = src[j];
            }

            Logger::Log("RBSP LightGrid: palette=" + std::to_string(paletteCount) +
                " cells=" + std::to_string(indexCount));

        }
        else {
            // IBSP: lump 15 is a flat per-cell array, no index lump
            size_t cnt = 0;
            if (checkLump(kLightVolumes, sizeof(tBSPLightvol), cnt)) {
                const uint8_t* ptr = base + lumps[kLightVolumes].offset;
                lightVolPalette.resize(cnt);
                memcpy(lightVolPalette.data(), ptr, cnt * sizeof(tBSPLightvol));
                lightVolIndices.resize(cnt);
                std::iota(lightVolIndices.begin(), lightVolIndices.end(), 0u);
            }
        }
    }

    // VisData
    if (lumps[kVisData].length >= 2 * (int)sizeof(int)) {
        size_t off = lumps[kVisData].offset;
        visData.n_vecs = *reinterpret_cast<const int*>(base + off);
        visData.sz_vecs = *reinterpret_cast<const int*>(base + off + sizeof(int));
        size_t totalVis = size_t(visData.n_vecs) * visData.sz_vecs;
        visData.vecs.resize(totalVis);
        memcpy(visData.vecs.data(), base + off + 2 * sizeof(int), totalVis);
    }

    // Vertices (Y-up swap)
    {
        if (m_isRBSP) {
            // FBSP/RBSP: 80-byte on-disk vertex with 4 lightmap slots
            size_t cnt = 0;
            if (checkLump(kVertices, sizeof(tBSPVertexRBSP), cnt)) {
                m_numOfVerts = static_cast<int>(cnt);
                m_pVerts = new tBSPVertex[cnt];
                m_pVertsRBSP = new tBSPVertexRBSP[cnt];
                const uint8_t* ptr = base + lumps[kVertices].offset;
                for (size_t i = 0; i < cnt; ++i) {
                    tBSPVertexRBSP raw{};
                    memcpy(&raw, ptr + i * sizeof(tBSPVertexRBSP), sizeof(tBSPVertexRBSP));

                    // Y-up swap on position and normal
                    float t = raw.vPosition.y;
                    raw.vPosition.y = raw.vPosition.z;
                    raw.vPosition.z = -t;
                    t = raw.vNormal.y;
                    raw.vNormal.y = raw.vNormal.z;
                    raw.vNormal.z = -t;

                    // Keep the full raw vertex for multi-slot UV access
                    m_pVertsRBSP[i] = raw;

                    // Canonical IBSP-shaped vertex (slot 0 only)
                    m_pVerts[i].vPosition = raw.vPosition;
                    m_pVerts[i].vTextureCoord = raw.vTextureCoord;
                    m_pVerts[i].vLightmapCoord = raw.vLightmapCoord[0];
                    m_pVerts[i].vNormal = raw.vNormal;
                    m_pVerts[i].color[0] = raw.color[0][0];
                    m_pVerts[i].color[1] = raw.color[0][1];
                    m_pVerts[i].color[2] = raw.color[0][2];
                    m_pVerts[i].color[3] = raw.color[0][3];
                }
            }
        }
        else {
            // IBSP: 44-byte on-disk vertex (original path)
            size_t cnt = 0;
            if (checkLump(kVertices, sizeof(tBSPVertex), cnt)) {
                m_numOfVerts = static_cast<int>(cnt);
                m_pVerts = new tBSPVertex[cnt];
                const uint8_t* ptr = base + lumps[kVertices].offset;
                for (size_t i = 0; i < cnt; ++i) {
                    memcpy(&m_pVerts[i], ptr + i * sizeof(tBSPVertex), sizeof(tBSPVertex));
                    float t = m_pVerts[i].vPosition.y;
                    m_pVerts[i].vPosition.y = m_pVerts[i].vPosition.z;
                    m_pVerts[i].vPosition.z = -t;
                    t = m_pVerts[i].vNormal.y;
                    m_pVerts[i].vNormal.y = m_pVerts[i].vNormal.z;
                    m_pVerts[i].vNormal.z = -t;
                }
            }
        }
    }

    // Indices, Faces, Textures, Lightmaps
    {
        size_t cntIdx = 0;
        if (checkLump(kIndices, sizeof(int), cntIdx)) {
            m_numOfIndices = static_cast<int>(cntIdx);
            m_pIndices = new int[cntIdx];
            memcpy(m_pIndices, base + lumps[kIndices].offset, cntIdx * sizeof(int));
        }

        // ── Faces: choose the right on-disk struct size ────────────────────────
        // IBSP: tBSPFace      = 104 bytes (single lightmap slot, no styles)
        // RBSP/FBSP: tBSPFaceRBSP = 148 bytes (4 lightmap + style slots)
        // We always store the canonical tBSPFace internally; RBSP faces are
        // converted via ConvertRBSPFace() which maps slot 0 → the single slot.
        {
            const size_t onDiskSize = m_isRBSP ? sizeof(tBSPFaceRBSP)
                : sizeof(tBSPFace);
            const auto& L = lumps[kFaces];
            if (L.offset >= 0 && L.length >= 0 &&
                size_t(L.offset) + size_t(L.length) <= totalSize &&
                L.length % onDiskSize == 0)
            {
                const size_t cnt = L.length / onDiskSize;
                const uint8_t* ptr = base + L.offset;
                m_numOfFaces = static_cast<int>(cnt);
                m_pFaces = new tBSPFace[cnt];

                if (m_isRBSP) {
                    m_pFacesRBSP = new tBSPFaceRBSP[cnt];
                    for (size_t fi = 0; fi < cnt; ++fi) {
                        memcpy(&m_pFacesRBSP[fi], ptr + fi * sizeof(tBSPFaceRBSP),
                            sizeof(tBSPFaceRBSP));
                        m_pFaces[fi] = ConvertRBSPFace(m_pFacesRBSP[fi]);
                    }
                }
                else {
                    memcpy(m_pFaces, ptr, cnt * sizeof(tBSPFace));
                }
            }
        }

        size_t cntTex = 0;
        if (checkLump(kTextures, sizeof(tBSPTexture), cntTex)) {
            m_numOfTextures = static_cast<int>(cntTex);
            pTextures = new tBSPTexture[cntTex];
            memcpy(pTextures, base + lumps[kTextures].offset, cntTex * sizeof(tBSPTexture));
            for (int i = 0; i < m_numOfTextures; ++i) {
                strcpy_s(tname[i], pTextures[i].strName);
                strcat_s(tname[i], ".jpg");
            }
        }

        // ── Lightmaps: atlas size differs per format ───────────────────────────
        // IBSP / RBSP: 128×128 × 3 = 49152 bytes per atlas
        // FBSP:        512×512 × 3 = 786432 bytes per atlas
        {
            const int    lmSz = m_lightmapSize;
            const size_t bytesPerLM = size_t(lmSz) * lmSz * 3;
            const auto& L = lumps[kLightmaps];

            if (L.offset >= 0 && L.length >= 0 &&
                size_t(L.offset) + size_t(L.length) <= totalSize &&
                bytesPerLM > 0 && L.length % bytesPerLM == 0)
            {
                const size_t cnt = L.length / bytesPerLM;
                m_numOfLightmaps = static_cast<int>(cnt);
                const uint8_t* ptr = base + L.offset;

                if (!m_isFBSP) {
                    // Legacy path: keep the tBSPLightmap raw array for compat
                    pLightmaps = new tBSPLightmap[cnt];
                    memcpy(pLightmaps, ptr, cnt * sizeof(tBSPLightmap));
                    for (int i = 0; i < m_numOfLightmaps; ++i)
                        Rbuffers.G_lightMaps_Legacy.push_back(pLightmaps[i]);
                }

                // Always populate the unified G_lightMaps container
                Rbuffers.G_lightMaps.reserve(cnt);
                for (size_t i = 0; i < cnt; ++i) {
                    tBSPLightmapData lmd(lmSz);
                    memcpy(lmd.pixels.data(), ptr + i * bytesPerLM, bytesPerLM);
                    Rbuffers.G_lightMaps.push_back(std::move(lmd));
                }
            }
        }
    }

    printf("BSP loaded successfully: %s\n", filename);
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// BuildVBO / CreateVBO / CreateIndices / CreateRenderBuffers
// ─────────────────────────────────────────────────────────────────────────────

void CQuake3BSP::BuildVBO()
{
    for (int index = 0; index < m_numOfFaces; index++) {
        tBSPFace* pFace = &m_pFaces[index];

        if (pFace->type == FACE_POLYGON)
            skipindices += pFace->numOfIndices;

        CreateVBO(index);
        CreateIndices(index);
        CreateRenderBuffers(index);
        numVisibleFaces++;
    }

    // Fill extra lightmap UV slots (1-3) for RBSP/FBSP faces now that all
    // faces and their raw tBSPVertexRBSP data are available.
    FillExtraLightmapUVs();
}

// Populates v_faceLightmapUVs[face][1..3] from the raw RBSP vertex data.
// Slot 0 is already set in CreateVBO. This is a no-op for IBSP.
void CQuake3BSP::FillExtraLightmapUVs()
{
    if (!m_pFacesRBSP || !m_pVertsRBSP) return;

    for (int fi = 0; fi < m_numOfFaces; ++fi) {
        const tBSPFaceRBSP& rf = m_pFacesRBSP[fi];
        auto& uvSets = Rbuffers.v_faceLightmapUVs[fi];

        for (int slot = 1; slot < BSP_MAX_LIGHTMAP_STYLES; ++slot) {
            if (rf.lightmapStyles[slot] == LS_NONE) continue; // unused slot
            uvSets[slot].resize(rf.numOfVerts);
            for (int v = 0; v < rf.numOfVerts; ++v) {
                uvSets[slot][v] = m_pVertsRBSP[rf.startVertIndex + v].vLightmapCoord[slot];
            }
        }
    }
}

// ── Bezier helpers (file-scope) ───────────────────────────────────────────

static void BezierBasis(float t, float b[3])
{
    const float inv = 1.0f - t;
    b[0] = inv * inv;
    b[1] = 2.0f * t * inv;
    b[2] = t * t;
}

// Evaluates one point on a biquadratic Bezier patch.
//
//  cp   – row-major 3×3 base vertices  (row=j/t-axis, col=i/s-axis → cp[j*3+i])
//  rcp  – row-major 3×3 RBSP vertices, or nullptr if not an RBSP map
//  s, t – surface parameters in [0, 1]
static VertexData EvalBezierVertex(const tBSPVertex      cp[9],
    const tBSPVertexRBSP* rcp,
    float s, float t)
{
    // Two-pass biquadratic as id Software implements it in tr_curve.c.
    // The outer pass (t) sweeps along rows of the control grid;
    // the inner pass (s) sweeps along the interpolated column result.
    // These are NOT interchangeable — swapping them produces warped UVs.

    // ── Helper: linearly blend two BSP vertices ───────────────────────────
    auto lerpVert = [](const tBSPVertex& a, const tBSPVertex& b, float f)
        -> tBSPVertex
        {
            tBSPVertex r;
            r.vPosition = glm::mix(a.vPosition, b.vPosition, f);
            r.vNormal = glm::mix(a.vNormal, b.vNormal, f);
            r.vTextureCoord = glm::mix(a.vTextureCoord, b.vTextureCoord, f);
            r.vLightmapCoord = glm::mix(a.vLightmapCoord, b.vLightmapCoord, f);
            for (int c = 0; c < 4; c++)
                r.color[c] = static_cast<unsigned char>(
                    glm::mix((float)a.color[c], (float)b.color[c], f));
            return r;
        };
    auto lerpRBSP = [](const tBSPVertexRBSP& a, const tBSPVertexRBSP& b, float f)
        -> tBSPVertexRBSP
        {
            tBSPVertexRBSP r;
            for (int k = 0; k < 4; k++)
                r.vLightmapCoord[k] = glm::mix(a.vLightmapCoord[k],
                    b.vLightmapCoord[k], f);
            return r;
        };

    // ── Pass 1: evaluate each control row at parameter t ──────────────────
    // cp layout: cp[row*3 + col], row=0..2, col=0..2
    // For each row, evaluate the 1D quadratic Bezier along t.
    // This produces 3 intermediate points (one per control row).
    tBSPVertex    mid[3];
    tBSPVertexRBSP midR[3];

    for (int row = 0; row < 3; row++) {
        const tBSPVertex& c0 = cp[row * 3 + 0];
        const tBSPVertex& c1 = cp[row * 3 + 1];
        const tBSPVertex& c2 = cp[row * 3 + 2];

        // Quadratic de Casteljau at t:  lerp(lerp(c0,c1,t), lerp(c1,c2,t), t)
        tBSPVertex a = lerpVert(c0, c1, t);
        tBSPVertex b = lerpVert(c1, c2, t);
        mid[row] = lerpVert(a, b, t);

        if (rcp) {
            const tBSPVertexRBSP& r0 = rcp[row * 3 + 0];
            const tBSPVertexRBSP& r1 = rcp[row * 3 + 1];
            const tBSPVertexRBSP& r2 = rcp[row * 3 + 2];
            tBSPVertexRBSP ra = lerpRBSP(r0, r1, t);
            tBSPVertexRBSP rb = lerpRBSP(r1, r2, t);
            midR[row] = lerpRBSP(ra, rb, t);
        }
    }

    // ── Pass 2: evaluate the 3 intermediate points at parameter s ─────────
    tBSPVertex a = lerpVert(mid[0], mid[1], s);
    tBSPVertex b = lerpVert(mid[1], mid[2], s);
    tBSPVertex final = lerpVert(a, b, s);

    VertexData vd{};
    vd.Position = final.vPosition;
    vd.Normal = glm::length(final.vNormal) > 1e-6f
        ? glm::normalize(final.vNormal)
        : glm::vec3(0.f, 0.f, 1.f);
    vd.TextureCoordinate = final.vTextureCoord;
    vd.ShadowMapCoords = final.vLightmapCoord;
    vd.Color = glm::vec4(final.color[0] / 255.f, final.color[1] / 255.f,
        final.color[2] / 255.f, final.color[3] / 255.f);

    if (rcp) {
        tBSPVertexRBSP ra = lerpRBSP(midR[0], midR[1], s);
        tBSPVertexRBSP rb = lerpRBSP(midR[1], midR[2], s);
        tBSPVertexRBSP rf;
        for (int k = 0; k < 4; k++)
            rf.vLightmapCoord[k] = glm::mix(ra.vLightmapCoord[k],
                rb.vLightmapCoord[k], s);

        vd.Tangent = glm::vec3(rf.vLightmapCoord[1], 0.f);
        vd.BiTangent = glm::vec3(rf.vLightmapCoord[2], 0.f);
        vd.TextureCoordinate2 = rf.vLightmapCoord[3];
    }
    return vd;
}

// ── Type-2: Bezier patch ──────────────────────────────────────────────────
//
//  Writes both Rbuffers.v_faceVBOs and Rbuffers.v_faceIDXs for this face,
//  so CreateIndices() must be a no-op when called afterward.
//
//  All uvSets slots are populated here because FillExtraLightmapUVs()
//  maps 1-to-1 onto raw BSP vertex indices and cannot run post-tessellation.
void CQuake3BSP::CreateVBO_Patch(int index)
{
    tBSPFace* pFace = &m_pFaces[index];
    auto& vertices = Rbuffers.v_faceVBOs[index];
    auto& indices = Rbuffers.v_faceIDXs[index];
    auto& uvSets = Rbuffers.v_faceLightmapUVs[index];
    for (auto& uv : uvSets) uv.clear();

    const int gridW = pFace->size[0];
    const int gridH = pFace->size[1];

    // Sanity-check: grid dimensions must be odd and ≥ 3
    if (gridW < 3 || gridH < 3 || (gridW & 1) == 0 || (gridH & 1) == 0) {
        faceBounds.push_back(BoundingBox{});
        return;
    }

    const int patchesX = (gridW - 1) / 2;
    const int patchesY = (gridH - 1) / 2;
    const int L = std::max(1, kBezierTessLevel);
    const int stride = L + 1;   // vertex columns per tessellated row

    for (int py = 0; py < patchesY; py++) {
        for (int px = 0; px < patchesX; px++) {
            // ── Collect 3×3 control points (row-major, j=row, i=col) ──────
            tBSPVertex     cpBase[9];
            tBSPVertexRBSP cpRBSP[9];

            for (int cy = 0; cy < 3; cy++) {
                for (int cx = 0; cx < 3; cx++) {
                    const int src = pFace->startVertIndex
                        + (py * 2 + cy) * gridW
                        + (px * 2 + cx);
                    cpBase[cy * 3 + cx] = m_pVerts[src];
                    if (m_pVertsRBSP)
                        cpRBSP[cy * 3 + cx] = m_pVertsRBSP[src];
                }
            }

            const tBSPVertexRBSP* rbspPtr = m_pVertsRBSP ? cpRBSP : nullptr;
            const uint32_t base = static_cast<uint32_t>(vertices.size());

            // ── Tessellated vertex grid: (L+1) × (L+1) points ────────────
            for (int j = 0; j <= L; j++) {
                const float t = static_cast<float>(j) / static_cast<float>(L);
                for (int i = 0; i <= L; i++) {
                    const float s = static_cast<float>(i) / static_cast<float>(L);
                    VertexData vd = EvalBezierVertex(cpBase, rbspPtr, s, t);
                    vertices.push_back(vd);

                    uvSets[0].push_back(vd.ShadowMapCoords);
                    if (rbspPtr) {
                        uvSets[1].push_back(glm::vec2(vd.Tangent));
                        uvSets[2].push_back(glm::vec2(vd.BiTangent));
                        uvSets[3].push_back(vd.TextureCoordinate2);
                    }
                }
            }

            // ── Indices: two triangles per quad, CCW → CW (swap i1/i2) ───
            //
            //  Grid layout (j = row, i = col):
            //    v00 ── v10
            //     │  ╲   │
            //    v01 ── v11
            //
            //  CCW pair → flip last two indices → CW pair
            for (int j = 0; j < L; j++) {
                for (int i = 0; i < L; i++) {
                    const uint32_t v00 = base + static_cast<uint32_t>(j * stride + i);
                    const uint32_t v10 = base + static_cast<uint32_t>(j * stride + i + 1);
                    const uint32_t v01 = base + static_cast<uint32_t>((j + 1) * stride + i);
                    const uint32_t v11 = base + static_cast<uint32_t>((j + 1) * stride + i + 1);

                    indices.push_back(v00); indices.push_back(v11); indices.push_back(v10);
                    indices.push_back(v00); indices.push_back(v01); indices.push_back(v11);
                }
            }
        }
    }

    auto bounds = BoundingBox::FromVertices(vertices);
    bounds.Min -= vec3(0.1f);
    bounds.Max += vec3(0.1f);
    faceBounds.push_back(bounds);
}

// ── CreateVBO ─────────────────────────────────────────────────────────────

void CQuake3BSP::CreateVBO(int index)
{
    tBSPFace* pFace = &m_pFaces[index];

    if (pFace->type == 2) {
        // Patch: vertices, indices, UV sets, and bounds all handled here
        CreateVBO_Patch(index);
        return;
    }
    if (pFace->type == 4) {
        // Billboard: no renderable geometry; push sentinel bounds to keep
        // faceBounds index-aligned with m_pFaces
        const glm::vec3 p = m_pVerts[pFace->startVertIndex].vPosition;
        BoundingBox bb;
        bb.Min = p - glm::vec3(1.f);
        bb.Max = p + glm::vec3(1.f);
        faceBounds.push_back(bb);
        return;
    }
    // Types 1 (polygon) and 3 (mesh) both use vertex[] + meshvert[] indices
    // and are handled identically below.

    auto& vertices = Rbuffers.v_faceVBOs[index];
    auto& uvSets = Rbuffers.v_faceLightmapUVs[index];
    for (auto& uv : uvSets) uv.clear();

    for (int v = 0; v < pFace->numOfVerts; v++) {
        const tBSPVertex& bspVert = m_pVerts[pFace->startVertIndex + v];
        VertexData vd;

        vd.Position = bspVert.vPosition;
        vd.Normal = bspVert.vNormal;
        vd.TextureCoordinate = bspVert.vTextureCoord;
        vd.ShadowMapCoords = bspVert.vLightmapCoord; // slot 0

        vd.Color = glm::vec4(
            bspVert.color[0] / 255.0f,
            bspVert.color[1] / 255.0f,
            bspVert.color[2] / 255.0f,
            bspVert.color[3] / 255.0f);

        if (m_pVertsRBSP) {
            const tBSPVertexRBSP& rv = m_pVertsRBSP[pFace->startVertIndex + v];
            vd.Tangent = glm::vec3(rv.vLightmapCoord[1], 0.0f); // slot 1
            vd.BiTangent = glm::vec3(rv.vLightmapCoord[2], 0.0f); // slot 2
            vd.TextureCoordinate2 = glm::vec2(rv.vLightmapCoord[3]); // slot 3
        }

        vertices.push_back(vd);
        uvSets[0].push_back(bspVert.vLightmapCoord);
        // Slots 1-3 filled by FillExtraLightmapUVs() (not applicable to patches)
    }

    auto bounds = BoundingBox::FromVertices(vertices);
    bounds.Min -= vec3(0.1f);
    bounds.Max += vec3(0.1f);
    faceBounds.push_back(bounds);
}

// ── CreateIndices ─────────────────────────────────────────────────────────

void CQuake3BSP::CreateIndices(int index)
{
    tBSPFace* pFace = &m_pFaces[index];

    // Patch indices are already written by CreateVBO_Patch;
    // billboards have no geometry.
    if (pFace->type == 2 || pFace->type == 4) return;

    // Types 1 and 3: meshverts are offsets from startVertIndex
    int  start = pFace->startIndex;
    int  count = pFace->numOfIndices;
    auto& out = Rbuffers.v_faceIDXs[index];

    out.reserve(count);

    for (int j = 0; j < count; j += 3) {
        unsigned short i0 = m_pIndices[start + j + 0];
        unsigned short i1 = m_pIndices[start + j + 1];
        unsigned short i2 = m_pIndices[start + j + 2];
        // Flip winding order (CCW → CW)
        out.push_back(i0);
        out.push_back(i2);
        out.push_back(i1);
    }
}

// ── CreateRenderBuffers ───────────────────────────────────────────────────

void CQuake3BSP::CreateRenderBuffers(int index)
{
    const auto& vertices = Rbuffers.v_faceVBOs[index];
    const auto& indices = Rbuffers.v_faceIDXs[index];

    // Billboards and degenerate faces produce no geometry
    if (vertices.empty() || indices.empty()) return;

    auto& fb = FB_array.FB_Idx[index];

    const bgfx::Memory* vMem = bgfx::copy(
        vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(VertexData)));
    fb.VBO = bgfx::createVertexBuffer(vMem, VertexData::Declaration());

    const bgfx::Memory* iMem = bgfx::copy(
        indices.data(), static_cast<uint32_t>(indices.size() * sizeof(uint32_t)));
    fb.EBO = bgfx::createIndexBuffer(iMem, BGFX_BUFFER_INDEX32);
    fb.IndexCount = static_cast<uint32_t>(indices.size());
}

// ─────────────────────────────────────────────────────────────────────────────
// GenerateLightmap
// Replaces direct glGenTextures / glTexImage2D calls with Texture construction.
// ─────────────────────────────────────────────────────────────────────────────

void CQuake3BSP::GenerateLightmap()
{
    // ── Missing lightmap (grey 2×2 RGBA) ─────────────────────────────────────
    {
        const uint8_t v = 255;
        uint8_t pixels[16] = {
            v,v,v,v,  v,v,v,v,
            v,v,v,v,  v,v,v,v
        };
        m_missingLightmap = std::make_shared<Texture>(pixels, 2, 2, bgfx::TextureFormat::RGBA8, false);
        m_missingLightmap->setName("Missing Lightmap");
    }

    // ── White (black) lightmap (2×2 RGBA) ────────────────────────────────────
    {
        uint8_t pixels[16] = { 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };
        m_whiteLightmap = std::make_shared<Texture>(pixels, 2, 2, bgfx::TextureFormat::RGBA8, false);
        m_whiteLightmap->setName("Black Lightmap");
    }

    // ── Per-BSP lightmap textures ─────────────────────────────────────────────
    // G_lightMaps holds tBSPLightmapData with a runtime .size field (128 for
    // IBSP/RBSP, 512 for FBSP) so the GPU upload path is identical for both.
    m_lightmapTextures.reserve(Rbuffers.G_lightMaps.size());
    for (size_t i = 0; i < Rbuffers.G_lightMaps.size(); ++i)
    {
        const tBSPLightmapData& lmd = Rbuffers.G_lightMaps[i];

        auto tex = std::make_shared<Texture>(
            lmd.pixels.data(),
            lmd.size, lmd.size,
            bgfx::TextureFormat::RGB8,
            true /*generateMipmaps*/);
        tex->setName("BSP Lightmap " + std::to_string(i) +
            (m_isFBSP ? " (FBSP 512)" : " (128)"));
        m_lightmapTextures.push_back(std::move(tex));
    }

    printf("GenerateLightmap: uploaded %zu lightmap atlas(es) at %dx%d (%s)\n",
        m_lightmapTextures.size(),
        m_lightmapSize, m_lightmapSize,
        m_isFBSP ? "FBSP" : (m_isRBSP ? "RBSP" : "IBSP"));
}

void CQuake3BSP::GenerateTexture()
{
    // Intentionally empty – face textures are loaded on-demand in PreloadFace.
}

// ─────────────────────────────────────────────────────────────────────────────
// Native-ID helpers (keep all getID() calls in one place)
// ─────────────────────────────────────────────────────────────────────────────

int CQuake3BSP::GetLightmapNativeId(int lightmapSlot) const
{
    if (lightmapSlot >= 0 && lightmapSlot < (int)m_lightmapTextures.size())
    {
        const auto& tex = m_lightmapTextures[lightmapSlot];
        if (tex && tex->valid)
            return (int)tex->getID();
    }
    return m_missingLightmap ? (int)m_missingLightmap->getID() : 0;
}

int CQuake3BSP::GetWhiteLightmapNativeId() const
{
    return m_whiteLightmap ? (int)m_whiteLightmap->getID() : 0;
}

int CQuake3BSP::GetFaceLightmapId(int faceIndex, int slot) const
{
    if (faceIndex < 0 || faceIndex >= m_numOfFaces) return GetLightmapNativeId(-1);
    if (slot < 0 || slot >= BSP_MAX_LIGHTMAP_STYLES)  return GetLightmapNativeId(-1);
    if (!cachedFaces) return GetLightmapNativeId(-1);
    return cachedFaces[faceIndex].lightmapIds[slot];
}

int CQuake3BSP::GetFaceTextureNativeId(int cachedTextureId) const
{
    // cachedTextureId is stored as the raw GL texture id obtained from
    // AssetRegistry at PreloadFace time.  We return it unchanged; this helper
    // exists so callers have a named interface rather than using the raw int.
    return cachedTextureId;
}

// ─────────────────────────────────────────────────────────────────────────────
// PreloadFace / PreloadFaces
// ─────────────────────────────────────────────────────────────────────────────

void CQuake3BSP::PreloadFace(int index)
{
    // Resolve texture name from either face type
    const char* rawTexName = nullptr;
    int numOfIndices = 0;

    if (m_isFBSP && m_pFacesRBSP) {
        rawTexName = pTextures[m_pFacesRBSP[index].textureID].strName;
        numOfIndices = m_pFacesRBSP[index].numOfIndices;
    }
    else {
        rawTexName = pTextures[m_pFaces[index].textureID].strName;
        numOfIndices = m_pFaces[index].numOfIndices;
    }

    string textureName(rawTexName);
    int nameL = (int)textureName.length();

    bool isAnimated = false;
    string directory;      // e.g. "textures/lq_liquid/"
    string baseFilename;   // e.g. "water"
    string animPrefix = ""; // "+" or "plus_"
    int currentFrame = 0;

    if (!textureName.empty()) {
        // Split into directory + filename (supports both / and \ for cross-platform)
        size_t lastSlash = textureName.find_last_of("/\\");
        directory = (lastSlash != string::npos) ? textureName.substr(0, lastSlash + 1) : "";
        string filename = (lastSlash != string::npos) ? textureName.substr(lastSlash + 1) : textureName;

        // Parse filename only for animation markers
        if (!filename.empty()) {
            size_t prefixLen = 0;

            // Detect prefix
            if (filename[0] == '+') {
                animPrefix = "+";
                prefixLen = 1;
            }
            else if (filename.size() > 5 && filename.substr(0, 5) == "plus_") {
                animPrefix = "plus_";
                prefixLen = 5;
            }

            // If a valid prefix was found and there are characters following it
            if (prefixLen > 0 && prefixLen < filename.size()) {
                char frameChar = filename[prefixLen];
                bool validFrame = false;

                // Parse the single-character frame index (0-9, a-z, A-Z)
                if (isdigit(static_cast<unsigned char>(frameChar))) {
                    currentFrame = frameChar - '0';
                    validFrame = true;
                }
                else if (islower(static_cast<unsigned char>(frameChar))) {
                    currentFrame = frameChar - 'a' + 10;
                    validFrame = true;
                }
                else if (isupper(static_cast<unsigned char>(frameChar))) {
                    currentFrame = frameChar - 'A' + 10;
                    validFrame = true;
                }

                // If it is a valid frame and there is a base filename remaining
                if (validFrame && (prefixLen + 1) < filename.size()) {
                    baseFilename = filename.substr(prefixLen + 1);
                    isAnimated = true;
                }
            }
        }
    }
    bool isCube = nameL > 5
        && textureName[nameL - 1] == 'e' && textureName[nameL - 2] == 'b'
        && textureName[nameL - 3] == 'u' && textureName[nameL - 4] == 'c';

    std::string basePath = "GameData/" + textureName;
    if (isCube) {
        auto parts = StringHelper::Split(basePath, '/');
        basePath = "GameData/env/" + parts.back();
    }

    std::vector<std::string> extensions = { ".png", ".jpg", ".jpeg" };

    int faceTexture = m_whiteLightmap->getID();

    bool transparentPixels = false;

    if (isCube) {
        CubemapTexture* tex = nullptr;

        for (const auto& ext : extensions) {
            tex = AssetRegistry::GetTextureCubeFromFile(basePath + ext);
            if (tex && tex->valid) {
                faceTexture = tex->getID();
                break;
            }
        }
    }
    else {
        Texture* tex = nullptr;

        for (const auto& ext : extensions) {
            tex = AssetRegistry::GetTextureFromFile(basePath + ext);
            if (tex && tex->valid) {
                faceTexture = tex->getID();
                transparentPixels = tex->transparent;
                break;
            }
        }
    }


    CachedFaceTextureData data;
    data.isCube = isCube;
    data.textureId = faceTexture;
    data.textureName = textureName;
    data.transparent = textureName.ends_with("_t") || transparentPixels;
    data.numOfIndices = numOfIndices;
    data.numActiveSlots = 1;

    // Helper: resolve a lightmap atlas index to a native GPU texture ID
    auto resolveLM = [&](int lmAtlasIndex) -> int {
        if (m_numOfLightmaps > 0)
            return GetLightmapNativeId(lmAtlasIndex);
        if (isCube)
            return 0;
        string path = GetLightMapFilePathFromId(lmAtlasIndex, filePath);
        if (path.empty()) return GetLightmapNativeId(-1);
        auto tex = AssetRegistry::GetTextureFromFile(path);
        return (tex && tex->getID() != 0) ? (int)tex->getID() : GetLightmapNativeId(-1);
        };

    if (m_isFBSP && m_pFacesRBSP) {
        const tBSPFaceRBSP& rf = m_pFacesRBSP[index];

        // Slot 0
        bool slot0active = (rf.lightmapStyles[0] != LS_NONE) && (rf.lightmapNum[0] >= 0);
        data.lightmapStyles[0] = rf.lightmapStyles[0];
        data.lightmapIds[0] = slot0active ? resolveLM(rf.lightmapNum[0]) : GetLightmapNativeId(-1);
        data.lightmapId = data.lightmapIds[0];

        // Slots 1-3
        for (int slot = 1; slot < BSP_MAX_LIGHTMAP_STYLES; ++slot) {
            data.lightmapStyles[slot] = rf.lightmapStyles[slot];
            bool active = (rf.lightmapStyles[slot] != LS_NONE) && (rf.lightmapNum[slot] >= 0);
            if (active) {
                data.lightmapIds[slot] = resolveLM(rf.lightmapNum[slot]);
                ++data.numActiveSlots;
            }
            else {
                data.lightmapIds[slot] = GetLightmapNativeId(-1);
            }
        }
    }
    else {
        // IBSP — single slot only
        data.lightmapStyles[0] = LS_NORMAL;
        data.lightmapIds[0] = resolveLM(m_pFaces[index].lightmapID);
        data.lightmapId = data.lightmapIds[0];
        for (int slot = 1; slot < BSP_MAX_LIGHTMAP_STYLES; ++slot) {
            data.lightmapStyles[slot] = LS_NONE;
            data.lightmapIds[slot] = GetLightmapNativeId(-1);
        }
    }

    if (isAnimated && !baseFilename.empty() && !isCube) {
        std::vector<int> animFrames;
        const int MAX_FRAMES = 36;  // Single-char frames max out at 36 (0-9 + a-z)

        for (int f = 0; f < MAX_FRAMES; ++f) {
            // Map frame index to character: 0-9 -> '0'-'9', 10-35 -> 'a'-'z'
            char frameChar = (f < 10) ? ('0' + f) : ('a' + (f - 10));

            string frameFilename = animPrefix + frameChar + baseFilename;
            string frameFullTextureName = directory + frameFilename;
            string framePath = "GameData/" + frameFullTextureName + ".png";

            auto frameTex = AssetRegistry::GetTextureFromFile(framePath);
            if (frameTex == nullptr || !frameTex->valid) {
                // Try .jpg if .png failed
                framePath = "GameData/" + frameFullTextureName + ".jpg";
                frameTex = AssetRegistry::GetTextureFromFile(framePath);
            }

            if (!frameTex || frameTex->getID() == 0) {
                break;  // Stop at first missing frame (standard engine behavior)
            }
            animFrames.push_back((int)frameTex->getID());
        }

        if (!animFrames.empty()) {
            data.animatedTextureFrames = std::move(animFrames);
        }
    }

    cachedFaces[index] = data;
}

void CQuake3BSP::PreloadFaces()
{
    cachedFaces = new CachedFaceTextureData[m_numOfFaces];
    for (int i = 0; i < m_numOfFaces; i++)
        PreloadFace(i);

    PrecomputeFaceAABBs();
}

// ─────────────────────────────────────────────────────────────────────────────
// BuildMergedModels
// ─────────────────────────────────────────────────────────────────────────────

void CQuake3BSP::BuildMergedModels()
{
    uint32 mId = static_cast<uint32>(-1);

    unordered_map<string, vector<int>> facesMap;

    for (const auto& model : models)
    {
        mId++;
        string modelId = to_string(mId);

        for (int i = model.face; i < model.face + model.n_faces; i++)
        {
            string texId = to_string(m_pFaces[i].textureID);
            string lightmapId = to_string(m_pFaces[i].lightmapID);


            // Also key on style slots 1-3 for FBSP
            string styleKey = "";
            if (m_pFacesRBSP) {
                const tBSPFaceRBSP& rf = m_pFacesRBSP[i];
                for (int s = 1; s < BSP_MAX_LIGHTMAP_STYLES; ++s) 
                {
                    styleKey += "|" + to_string(rf.lightmapStyles[s]);
                    styleKey += ":" + to_string(rf.lightmapNum[s]);
                }
            }

            string finalStr = modelId + "|" + texId + "|" + lightmapId + styleKey;

            if (facesMap.find(finalStr) == facesMap.end())
                facesMap[finalStr] = vector<int>();

            facesMap[finalStr].push_back(i);
        }
    }

    mergedFacesMapping.resize(m_numOfFaces);

    for (const auto& keyPair : facesMap)
    {
        vector<MeshUtils::VerticesIndices> facesMeshes;
        facesMeshes.reserve(keyPair.second.size());

        for (int i : keyPair.second)
        {
            MeshUtils::VerticesIndices mesh;
            mesh.vertices = GetFaceVertices(i);
            mesh.indices = GetFaceIndices(i);
            facesMeshes.push_back(mesh);
        }

        auto mergedMesh = MeshUtils::MergeMeshes(facesMeshes);

        MergedModelFacesData data;

        {
            const bgfx::Memory* vMem = bgfx::copy(
                mergedMesh.vertices.data(),
                static_cast<uint32_t>(mergedMesh.vertices.size() * sizeof(VertexData)));
            data.vbo = bgfx::createVertexBuffer(vMem, VertexData::Declaration());

            const bgfx::Memory* iMem = bgfx::copy(
                mergedMesh.indices.data(),
                static_cast<uint32_t>(mergedMesh.indices.size() * sizeof(uint32_t)));
            data.ibo = bgfx::createIndexBuffer(iMem, BGFX_BUFFER_INDEX32);
            data.IndexCount = static_cast<uint32_t>(mergedMesh.indices.size());
        }

        data.referenceFace = keyPair.second[0];
        data.uId = (uint32)mergedFacesData.size();

        data.bounds = BoundingBox::FromVertices(mergedMesh.vertices)
            .Transform(glm::scale(vec3(1.0f / MAP_SCALE)));

        mergedFacesData.push_back(data);

        for (int i : keyPair.second)
            mergedFacesMapping[i] = data.uId;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Light volume helpers
// ─────────────────────────────────────────────────────────────────────────────

glm::vec3 computeLightDirection(const unsigned char vol_dir[2])
{
    float pitch = glm::radians((static_cast<float>(vol_dir[0]) / 255.0f) * 180.0f);
    float yaw = glm::radians((static_cast<float>(vol_dir[1]) / 255.0f) * 360.0f);

    float x = sinf(pitch) * cosf(yaw);
    float y = sinf(pitch) * sinf(yaw);
    float z = cosf(pitch);

    // Quake Z-up → engine Y-up: (x, y, z) → (x, z, -y)
    return glm::normalize(glm::vec3(x, z, -y));
}

std::vector<VertexData> CQuake3BSP::GetFaceVertices(int faceId)
{
    return Rbuffers.v_faceVBOs[faceId];
}

std::vector<uint32_t> CQuake3BSP::GetFaceIndices(int faceId)
{
    return Rbuffers.v_faceIDXs[faceId];
}

bool CQuake3BSP::CheckLightProbeAcess(const glm::vec3& position, const glm::vec3& volPosition)
{
    if (FindClusterAtPosition(volPosition) < 0) return false;

    float maxDimension = std::max(std::max(lightVolGridSize.x, lightVolGridSize.x), lightVolGridSize.z);
    return Physics::LineTrace(position, volPosition,
        BodyType::WorldOpaque).hasHit == false;
}

LightVolPointData CQuake3BSP::GetLightvolColorPoint(const glm::vec3& position, bool wallCheck)
{
    if (lightVolIndices.empty())
    {
        vec3 lightmapColor = LinetraceLightmapColor(position, position - vec3(0, MAP_SCALE * 10, 0));

        LightVolPointData data;
        data.ambientColor = lightmapColor / 2.0f;
        data.directColor = lightmapColor;
        data.direction = vec3(0,1,0);
        return data;
    }

    // Engine Y-up → Quake Z-up
    glm::vec3 pos_quake(position.x, -position.z, position.y);

    glm::vec3 modelMins = originalMins;
    glm::vec3 modelMaxs = originalMaxs;

    glm::ivec3 lightVolGridDims(
        static_cast<int>(std::floor(modelMaxs.x / lightVolGridSize.x) -
            std::ceil(modelMins.x / lightVolGridSize.x) + 1),
        static_cast<int>(std::floor(modelMaxs.y / lightVolGridSize.y) -
            std::ceil(modelMins.y / lightVolGridSize.y) + 1),
        static_cast<int>(std::floor(modelMaxs.z / lightVolGridSize.z) -
            std::ceil(modelMins.z / lightVolGridSize.z) + 1));

    float fx = (pos_quake.x - modelMins.x) / lightVolGridSize.x;
    float fy = (pos_quake.y - modelMins.y) / lightVolGridSize.y;
    float fz = (pos_quake.z - modelMins.z) / lightVolGridSize.z;

    int nx = static_cast<int>(std::floor(fx));
    int ny = static_cast<int>(std::floor(fy));
    int nz = static_cast<int>(std::floor(fz));

    fx -= nx;
    fy -= ny;
    fz -= nz;

    int nx0 = glm::clamp(nx, 0, lightVolGridDims.x - 1);
    int ny0 = glm::clamp(ny, 0, lightVolGridDims.y - 1);
    int nz0 = glm::clamp(nz, 0, lightVolGridDims.z - 1);
    int nx1 = glm::clamp(nx + 1, 0, lightVolGridDims.x - 1);
    int ny1 = glm::clamp(ny + 1, 0, lightVolGridDims.y - 1);
    int nz1 = glm::clamp(nz + 1, 0, lightVolGridDims.z - 1);

    auto getGridEnginePos = [&](int x, int y, int z) -> glm::vec3 {
        glm::vec3 quake = modelMins + glm::vec3(x, y, z) * lightVolGridSize;
        return glm::vec3(quake.x, quake.z, -quake.y);
        };

    auto getLightVolData = [&](int x, int y, int z)
        -> std::tuple<glm::vec3, glm::vec3, glm::vec3>
        {
            int idx = z * (lightVolGridDims.x * lightVolGridDims.y) + y * lightVolGridDims.x + x;
            const tBSPLightvolRBSP& vol = GetLightVolRBSP(idx);

            // Accumulate all active style slots weighted by their animated style colour.
            // For IBSP the RBSP wrapper already put everything in slot 0 with style
            // LS_NORMAL, so the loop still produces the correct single-slot result.
            glm::vec3 ambient(0.0f);
            glm::vec3 directional(0.0f);

			float styleWeight = 0.6f; // empirically chosen to match in-engine brightness

            if (m_isRBSP == false)
                styleWeight = 1.25f;



            for (int s = 0; s < BSP_MAX_LIGHTMAP_STYLES; ++s) {
                if (vol.styles[s] == LS_NONE) continue;
                const glm::vec3 styleColor = GetStyleColor(vol.styles[s]);
                if (styleColor == glm::vec3(0.0f)) continue; // style is off – skip

				float ambientFactor = 1.0f;
                float directionalFactor = 1.0f;

                if (s > 0)
                {
                    auto maxDirectionalLight = std::max(std::max(vol.directional[s][0], vol.directional[s][1]), vol.directional[s][2]);

					ambientFactor = std::min(maxDirectionalLight / 255.0f * 4.0f, 1.0f);
                    directionalFactor = 0.8f;
                }


                ambient += glm::vec3(
                    vol.ambient[s][0] / 255.0f,
                    vol.ambient[s][1] / 255.0f,
                    vol.ambient[s][2] / 255.0f) * styleColor * styleWeight * ambientFactor;
                directional += glm::vec3(
                    vol.directional[s][0] / 255.0f,
                    vol.directional[s][1] / 255.0f,
                    vol.directional[s][2] / 255.0f) * styleColor * styleWeight * directionalFactor;



            }
            return { ambient, directional, computeLightDirection(vol.dir) };
        };

    auto [amb000, dir000, vec000] = getLightVolData(nx0, ny0, nz0);
    auto [amb100, dir100, vec100] = getLightVolData(nx1, ny0, nz0);
    auto [amb010, dir010, vec010] = getLightVolData(nx0, ny1, nz0);
    auto [amb110, dir110, vec110] = getLightVolData(nx1, ny1, nz0);
    auto [amb001, dir001, vec001] = getLightVolData(nx0, ny0, nz1);
    auto [amb101, dir101, vec101] = getLightVolData(nx1, ny0, nz1);
    auto [amb011, dir011, vec011] = getLightVolData(nx0, ny1, nz1);
    auto [amb111, dir111, vec111] = getLightVolData(nx1, ny1, nz1);

    bool valid000 = true, valid100 = true, valid010 = true, valid110 = true;
    bool valid001 = true, valid101 = true, valid011 = true, valid111 = true;

    if (wallCheck) {
        valid000 = CheckLightProbeAcess(position / MAP_SCALE, getGridEnginePos(nx0, ny0, nz0) / MAP_SCALE);
        valid010 = CheckLightProbeAcess(position / MAP_SCALE, getGridEnginePos(nx0, ny1, nz0) / MAP_SCALE);
        valid100 = CheckLightProbeAcess(position / MAP_SCALE, getGridEnginePos(nx1, ny0, nz0) / MAP_SCALE);
        valid110 = CheckLightProbeAcess(position / MAP_SCALE, getGridEnginePos(nx1, ny1, nz0) / MAP_SCALE);
        valid001 = CheckLightProbeAcess(position / MAP_SCALE, getGridEnginePos(nx0, ny0, nz1) / MAP_SCALE);
        valid101 = CheckLightProbeAcess(position / MAP_SCALE, getGridEnginePos(nx1, ny0, nz1) / MAP_SCALE);
        valid011 = CheckLightProbeAcess(position / MAP_SCALE, getGridEnginePos(nx0, ny1, nz1) / MAP_SCALE);
        valid111 = CheckLightProbeAcess(position / MAP_SCALE, getGridEnginePos(nx1, ny1, nz1) / MAP_SCALE);

        if (valid000 || valid010 || valid100 || valid110 || valid001 || valid101 || valid011 || valid111)
        {

        }
        else
        {
            return GetLightvolColorPoint(position, false);
        }

    }

    struct LightData { glm::vec3 amb, dir_color, dir_vec; };
    LightData datas[8] = {
        {amb000, dir000, vec000}, {amb100, dir100, vec100},
        {amb010, dir010, vec010}, {amb110, dir110, vec110},
        {amb001, dir001, vec001}, {amb101, dir101, vec101},
        {amb011, dir011, vec011}, {amb111, dir111, vec111}
    };
    bool valids[8] = { valid000, valid100, valid010, valid110,
                        valid001, valid101, valid011, valid111 };
    float weights[8] = {
        (1 - fx) * (1 - fy) * (1 - fz), fx * (1 - fy) * (1 - fz),
        (1 - fx) * fy * (1 - fz),  fx * fy * (1 - fz),
        (1 - fx) * (1 - fy) * fz,   fx * (1 - fy) * fz,
        (1 - fx) * fy * fz,    fx * fy * fz
    };

    glm::vec3 sum_amb(0), sum_dir_color(0), sum_dir_vec(0);
    float     total_weight = 0.0f;
    for (int i = 0; i < 8; ++i) {
        if (valids[i]) {
            float w = weights[i];
            sum_amb += datas[i].amb * w;
            sum_dir_color += datas[i].dir_color * w;
            sum_dir_vec += datas[i].dir_vec * w;
            total_weight += w;
        }
    }

    glm::vec3 ambient, directional, dir_engine;
    if (total_weight > 0.0001f) {
        ambient = sum_amb / total_weight;
        directional = sum_dir_color / total_weight;
        dir_engine = glm::normalize(sum_dir_vec / total_weight);
    }
    else {
        // Fallback: nearest valid sample within a search radius
        const float SEARCH_RADIUS = 128.0f;
        const float SEARCH_RADIUS_SQR = SEARCH_RADIUS * SEARCH_RADIUS;

        int rx = static_cast<int>(std::ceil(SEARCH_RADIUS / lightVolGridSize.x));
        int ry = static_cast<int>(std::ceil(SEARCH_RADIUS / lightVolGridSize.y));
        int rz = static_cast<int>(std::ceil(SEARCH_RADIUS / lightVolGridSize.z));

        int cx = glm::clamp(nx, 0, lightVolGridDims.x - 1);
        int cy = glm::clamp(ny, 0, lightVolGridDims.y - 1);
        int cz = glm::clamp(nz, 0, lightVolGridDims.z - 1);

        int bestIdx = -1;
        float bestDist2 = std::numeric_limits<float>::max();
        LightData bestData{ glm::vec3(0), glm::vec3(0), glm::vec3(0) };

        int x0 = glm::max(0, cx - rx), x1 = glm::min(lightVolGridDims.x - 1, cx + rx);
        int y0 = glm::max(0, cy - ry), y1 = glm::min(lightVolGridDims.y - 1, cy + ry);
        int z0 = glm::max(0, cz - rz), z1 = glm::min(lightVolGridDims.z - 1, cz + rz);

        for (int z = z0; z <= z1; ++z) {
            for (int y = y0; y <= y1; ++y) {
                for (int x = x0; x <= x1; ++x) {
                    glm::vec3 engPos = getGridEnginePos(x, y, z);
                    float d2 = glm::distance2(position, engPos);
                    if (d2 > SEARCH_RADIUS_SQR) continue;

                    if (wallCheck &&
                        !CheckLightProbeAcess(position / MAP_SCALE, engPos / MAP_SCALE))
                        continue;

                    int idx = z * (lightVolGridDims.x * lightVolGridDims.y) +
                        y * lightVolGridDims.x + x;
                    const tBSPLightvolRBSP& vol = GetLightVolRBSP(idx);
                    glm::vec3 fbAmb(0.0f), fbDir(0.0f);
                    for (int s = 0; s < BSP_MAX_LIGHTMAP_STYLES; ++s) {
                        if (vol.styles[s] == LS_NONE) continue;
                        const glm::vec3 sc = GetStyleColor(vol.styles[s]);
                        if (sc == glm::vec3(0.0f)) continue;
                        fbAmb += glm::vec3(vol.ambient[s][0] / 255.0f,
                            vol.ambient[s][1] / 255.0f,
                            vol.ambient[s][2] / 255.0f) * sc;
                        fbDir += glm::vec3(vol.directional[s][0] / 255.0f,
                            vol.directional[s][1] / 255.0f,
                            vol.directional[s][2] / 255.0f) * sc;
                    }
                    LightData ld{ fbAmb, fbDir, computeLightDirection(vol.dir) };

                    if (d2 < bestDist2) {
                        bestDist2 = d2;
                        bestData = ld;
                        bestIdx = idx;
                    }
                }
            }
        }

        if (bestIdx >= 0) {
            ambient = bestData.amb;
            directional = bestData.dir_color;
            dir_engine = glm::length2(bestData.dir_vec) > 1e-6f
                ? glm::normalize(bestData.dir_vec)
                : glm::vec3(0, -1, 0);
        }
        else {
            ambient = vec3(0.3f);
            directional = vec3(0);
            dir_engine = vec3(0, -1, 0);
        }
    }

    return LightVolPointData{ directional, ambient, dir_engine };
}

LightVolPointData CQuake3BSP::GetLightvolColor(const glm::vec3& position, bool wallCheck)
{
    if (lightVolIndices.empty())
        return { vec3(0), vec3(0.3f), vec3(0, -1, 0) };

    auto data = GetLightvolColorPoint(position);
    auto centerData = data;
    float radius = lightVolGridSize.x;

    data += GetLightvolColorPoint(position + vec3(radius, 0, 0), wallCheck);
    data += GetLightvolColorPoint(position + vec3(-radius, 0, 0), wallCheck);
    data += GetLightvolColorPoint(position + vec3(0, 0, radius), wallCheck);
    data += GetLightvolColorPoint(position + vec3(0, 0, -radius), wallCheck);
    data /= 5.0f;

    centerData.direction = normalize(data.direction);
    return centerData;
}

glm::vec3 CQuake3BSP::SampleLightmapFace(int faceIndex, const glm::vec3& hitPos)
{
    const tBSPFace& face = m_pFaces[faceIndex];
    const auto& verts = Rbuffers.v_faceVBOs[faceIndex];
    const auto& indices = Rbuffers.v_faceIDXs[faceIndex];

    if (verts.empty() || indices.size() < 3) return glm::vec3(0.0f);

    // ── Find Barycentric Coordinates for the hit point ────────────
    bool found = false;
    float bU = 0.0f, bV = 0.0f, bW = 0.0f;
    int hitIdx0 = 0, hitIdx1 = 0, hitIdx2 = 0;

    for (size_t i = 0; i + 2 < indices.size(); i += 3)
    {
        hitIdx0 = indices[i + 0];
        hitIdx1 = indices[i + 1];
        hitIdx2 = indices[i + 2];

        glm::vec3 A = verts[hitIdx0].Position;
        glm::vec3 B = verts[hitIdx1].Position;
        glm::vec3 C = verts[hitIdx2].Position;

        glm::vec3 e0 = B - A;
        glm::vec3 e1 = C - A;
        glm::vec3 e2 = hitPos - A;

        float d00 = glm::dot(e0, e0);
        float d01 = glm::dot(e0, e1);
        float d11 = glm::dot(e1, e1);
        float d20 = glm::dot(e2, e0);
        float d21 = glm::dot(e2, e1);

        float denom = d00 * d11 - d01 * d01;
        if (fabsf(denom) < 1e-8f) continue; // Degenerate triangle

        bV = (d11 * d20 - d01 * d21) / denom;
        bW = (d00 * d21 - d01 * d20) / denom;
        bU = 1.0f - bV - bW;

        // Expand tolerance slightly for floating point inaccuracies from raycast hits
        if (bU >= -1e-4f && bV >= -1e-4f && bW >= -1e-4f)
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        // Fallback: If precision issues caused all tests to fail, use the closest vertex
        float minDist2 = std::numeric_limits<float>::max();
        for (size_t i = 0; i < verts.size(); ++i)
        {
            float d2 = glm::distance2(hitPos, verts[i].Position);
            if (d2 < minDist2)
            {
                minDist2 = d2;
                hitIdx0 = hitIdx1 = hitIdx2 = i;
                bU = 1.0f; bV = 0.0f; bW = 0.0f;
            }
        }
    }

    // ── Helper: Sample a single lightmap slot ────────────
    auto sampleAtlas = [&](int lmAtlasIndex, glm::vec2 uv) -> glm::vec3 {
        if (lmAtlasIndex < 0) return glm::vec3(0.0f);

        // Embedded CPU lightmap
        if (lmAtlasIndex < static_cast<int>(Rbuffers.G_lightMaps.size()))
        {
            const tBSPLightmapData& lmd = Rbuffers.G_lightMaps[lmAtlasIndex];
            const int sz = lmd.size;

            const float fs = uv.x * static_cast<float>(sz);
            const float ft = uv.y * static_cast<float>(sz);

            const int x0 = glm::clamp(static_cast<int>(floorf(fs)), 0, sz - 1);
            const int y0 = glm::clamp(static_cast<int>(floorf(ft)), 0, sz - 1);
            const int x1 = glm::clamp(x0 + 1, 0, sz - 1);
            const int y1 = glm::clamp(y0 + 1, 0, sz - 1);

            const float fx = fs - floorf(fs);
            const float fy = ft - floorf(ft);

            auto texel = [&](int px, int py) -> glm::vec3 {
                const int base = (py * sz + px) * 3;
                return glm::vec3(
                    lmd.pixels[base + 0] / 255.0f,
                    lmd.pixels[base + 1] / 255.0f,
                    lmd.pixels[base + 2] / 255.0f);
                };

            return glm::mix(
                glm::mix(texel(x0, y0), texel(x1, y0), fx),
                glm::mix(texel(x0, y1), texel(x1, y1), fx),
                fy);
        }

        // External lightmap: delegate to Texture::SampleRGB
        if (lmAtlasIndex < static_cast<int>(m_lightmapTextures.size()))
        {
            const auto& tex = m_lightmapTextures[lmAtlasIndex];
            if (tex && tex->valid) return tex->SampleRGB(uv.x, uv.y);
        }

        const std::string path = GetLightMapFilePathFromId(lmAtlasIndex, filePath);
        if (!path.empty())
        {
            Texture* tex = AssetRegistry::GetTextureFromFile(path);
            if (tex && tex->valid) return tex->SampleRGB(uv.x, uv.y);
        }

        return glm::vec3(0.0f);
        };

    // ── Accumulate Light Styles ────────────
    glm::vec3 finalColor(0.0f);

    for (int slot = 0; slot < BSP_MAX_LIGHTMAP_STYLES; ++slot)
    {
        uint8_t style = LS_NONE;
        int lmIndex = -1;

        if (m_isRBSP && m_pFacesRBSP)
        {
            style = m_pFacesRBSP[faceIndex].lightmapStyles[slot];
            lmIndex = m_pFacesRBSP[faceIndex].lightmapNum[slot];
        }
        else
        {
            if (slot == 0) {
                style = LS_NORMAL;
                lmIndex = face.lightmapID;
            }
        }

        if (style == LS_NONE || lmIndex < 0) continue;

        glm::vec3 styleColor = GetStyleColor(style);
        if (styleColor == glm::vec3(0.0f)) continue; // Style is dark, skip sampling

        // Get the UV array for this specific slot. If empty, fall back to slot 0.
        const auto& uvSets = Rbuffers.v_faceLightmapUVs[faceIndex];
        const auto& activeUVSet = uvSets[slot].empty() ? uvSets[0] : uvSets[slot];

        // Safety check to ensure we don't read out of bounds
        if (hitIdx0 >= activeUVSet.size() || hitIdx1 >= activeUVSet.size() || hitIdx2 >= activeUVSet.size())
            continue;

        // Interpolate UVs using our earlier calculated Barycentric weights
        glm::vec2 lightmapUV = bU * activeUVSet[hitIdx0] +
            bV * activeUVSet[hitIdx1] +
            bW * activeUVSet[hitIdx2];

        glm::vec3 sampledColor = sampleAtlas(lmIndex, lightmapUV);

        finalColor += sampledColor * styleColor;
    }

    return finalColor;
}

void CQuake3BSP::PrecomputeFaceAABBs()
{
    m_faceAABBs.resize(m_numOfFaces);
    for (int fi = 0; fi < m_numOfFaces; ++fi)
    {
        glm::vec3 mn(1e30f), mx(-1e30f);
        for (const auto& v : Rbuffers.v_faceVBOs[fi])
        {
            mn = glm::min(mn, v.Position);
            mx = glm::max(mx, v.Position);
        }
        // Expand slightly to avoid fp gaps at shared edges
        m_faceAABBs[fi] = { mn - 0.5f, mx + 0.5f };
    }
}

// ── Fast slab AABB test ───────────────────────────────────────────────────────
static inline bool RayAABB(const glm::vec3& orig, const glm::vec3& invDir,
    const glm::vec3& mn, const glm::vec3& mx,
    float maxT)
{
    const glm::vec3 t0 = (mn - orig) * invDir;
    const glm::vec3 t1 = (mx - orig) * invDir;
    const float tEnter = glm::compMax(glm::min(t0, t1));
    const float tExit = glm::compMin(glm::max(t0, t1));
    return tExit >= tEnter && tExit >= 1e-4f && tEnter <= maxT;
}

// ─────────────────────────────────────────────────────────────────────────────
glm::vec3 CQuake3BSP::LinetraceLightmapColor(glm::vec3 start, glm::vec3 end)
{
    if (models.empty()) return glm::vec3(0.0f);

    const glm::vec3 rayDir = end - start;
    const float     rayLen = glm::length(rayDir);
    if (rayLen < 1e-6f) return glm::vec3(0.0f);
    const glm::vec3 rayDirN = rayDir / rayLen;

    const glm::vec3 invDir(
        1.0f / (fabsf(rayDirN.x) > 1e-9f ? rayDirN.x : 1e-9f),
        1.0f / (fabsf(rayDirN.y) > 1e-9f ? rayDirN.y : 1e-9f),
        1.0f / (fabsf(rayDirN.z) > 1e-9f ? rayDirN.z : 1e-9f));

    static std::mutex            s_mutex;
    static std::vector<uint32_t> faceStamp;
    static uint32_t              stampGen = 0;
    static std::vector<int>      candidates;
    static glm::vec3             cachedPos = glm::vec3(1e30f);
    static int                   cachedCluster = -1;

    std::lock_guard<std::mutex> lock(s_mutex);

    if ((int)faceStamp.size() != m_numOfFaces)
    {
        faceStamp.assign(m_numOfFaces, 0);
        stampGen = 0;
    }
    const uint32_t thisStamp = ++stampGen;
    candidates.clear();

    const glm::vec3 scaledStart = start / MAP_SCALE;
    if (scaledStart != cachedPos)
    {
        cachedCluster = FindClusterAtPosition(scaledStart);
        cachedPos = scaledStart;
    }
    const int  startCluster = cachedCluster;
    const bool hasVis = (startCluster >= 0 && !visData.vecs.empty());

    const tBSPModel& world = models[0];
    const int        worldFaceEnd = world.face + world.n_faces;

    for (const tBSPLeaf& leaf : leafs)
    {
        if (leaf.cluster < 0)                                        continue;
        if (hasVis && !IsClusterVisible(startCluster, leaf.cluster)) continue;

        for (int i = 0; i < leaf.n_leaffaces; ++i)
        {
            const int fi = leafFaces[leaf.leafface + i];
            if ((unsigned)fi >= (unsigned)m_numOfFaces) continue;
            if (fi < world.face || fi >= worldFaceEnd)  continue;
            if (m_pFaces[fi].lightmapID < 0)            continue;
            if (faceStamp[fi] == thisStamp)             continue;

            faceStamp[fi] = thisStamp;
            candidates.push_back(fi);
        }
    }

    float     bestT = rayLen;
    int       bestFace = -1;
    glm::vec3 bestHit = glm::vec3(0.0f);

    for (const int fi : candidates)
    {
        if (!m_faceAABBs.empty())
        {
            const FaceAABB& box = m_faceAABBs[fi];
            if (!RayAABB(start, invDir, box.mn, box.mx, bestT))
                continue;
        }

        const auto& verts = Rbuffers.v_faceVBOs[fi];
        const auto& indices = Rbuffers.v_faceIDXs[fi];
        if (verts.empty() || indices.size() < 3) continue;

        const size_t triCount = indices.size() / 3;
        for (size_t i = 0; i < triCount; ++i)
        {
            const glm::vec3& v0 = verts[indices[i * 3 + 0]].Position;
            const glm::vec3& v1 = verts[indices[i * 3 + 1]].Position;
            const glm::vec3& v2 = verts[indices[i * 3 + 2]].Position;

            const glm::vec3 e1 = v1 - v0;
            const glm::vec3 e2 = v2 - v0;
            const glm::vec3 h = glm::cross(rayDirN, e2);
            const float     a = glm::dot(e1, h);
            if (fabsf(a) < 1e-7f) continue;

            const float     f = 1.0f / a;
            const glm::vec3 sv = start - v0;
            const float     u = f * glm::dot(sv, h);
            if (u < 0.0f || u > 1.0f) continue;

            const glm::vec3 q = glm::cross(sv, e1);
            const float     v = f * glm::dot(rayDirN, q);
            if (v < 0.0f || u + v > 1.0f) continue;

            const float t = f * glm::dot(e2, q);
            if (t < 1e-4f || t >= bestT) continue;

            bestT = t;
            bestFace = fi;
            bestHit = start + rayDirN * t;
        }
    }

    if (bestFace < 0) return glm::vec3(0.0f);
    return SampleLightmapFace(bestFace, bestHit);
}

// ─────────────────────────────────────────────────────────────────────────────
// Cluster / visibility
// ─────────────────────────────────────────────────────────────────────────────

int CQuake3BSP::FindClusterAtPosition(glm::vec3 cameraPos)
{
    cameraPos *= MAP_SCALE;

    int nodeIndex = 0;
    int depth = 0;
    while (nodeIndex >= 0 && depth < 200) {
        const tBSPNode& node = nodes[nodeIndex];
        const tBSPPlane& plane = planes[node.plane];
        float dist = glm::dot(cameraPos, plane.normal) - plane.dist;

        int childIndex = dist >= 0 ? 0 : 1;
        int nextChild = node.children[childIndex];
        if (nextChild < 0) {
            int leafIndex = -nextChild - 1;
            return leafs[leafIndex].cluster;
        }
        nodeIndex = nextChild;
        depth++;
    }
    printf("Failed to find cluster\n");
    return -1;
}

bool CQuake3BSP::IsClusterVisible(int sourceCluster, int testCluster)
{
    if (sourceCluster < 0) return true;
    if (testCluster < 0) return true;
    if (visData.n_vecs == 0) return true;

    int byteIndex = (sourceCluster * visData.sz_vecs) + (testCluster / 8);
    int bitIndex = testCluster % 8;

    unsigned char byteValue = visData.vecs[byteIndex];
    return (byteValue & (1 << bitIndex)) != 0;
}

// You can move this somewhere global / static
struct QLightStyle
{
    std::string pattern;
};

static QLightStyle g_LightStyles[64] =
{
    { "m" },                                            // 0 normal
    { "mmnmmommommnonmmonqnmmo" },                      // 1 flicker
    { "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba" }, // 2 pulse
    { "mmmmmaaaaammmmmaaaaaabcdefgabcdefg" },            // 3 candle
    { "mamamamamama" },                                 // 4 fast strobe
    { "jklmnopqrstuvwxyzyxwvutsrqponmlkj" },             // 5 gentle pulse
    { "nmonqnmomnmomomno" },                            // 6 flicker
    { "mmmaaaabcdefgmmmmaaaammmaamm" },                  // 7 candle
    { "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa" },    // 8 candle
    { "aaaaaaaazzzzzzzz" },                             // 9 slow strobe
    { "mmamammmmammamamaaamammma" },                    // 10 fluorescent
    { "abcdefghijklmnopqrrqponmlkjihgfedcba" }           // 11 pulse no black
    // rest default empty
};

glm::vec3 CQuake3BSP::GetStyleColor(uint8_t style) const
{
    if (style == LS_NORMAL) return glm::vec3(1.0f);
    if (style == LS_NONE)   return glm::vec3(0.0f);



    const std::string& pattern = g_LightStyles[style].pattern;

    if (pattern.empty())
        return glm::vec3(1.0f); // fallback

    // --- TIME ---
    // Quake uses ~10 Hz animation
    float time = Time::GameTime;   // seconds
    float speed = 10.0f;

    int frame = (int)(time * speed) % pattern.size();

    char c = pattern[frame];

    // --- CHAR → BRIGHTNESS ---
    // 'a' = 0, 'm' = 1.0, 'z' = 2.0
    float value = (c - 'a') / float('m' - 'a');

    // Optional clamp (Quake sometimes exceeds 1.0 slightly)
    // value = glm::clamp(value, 0.0f, 2.0f);

    return glm::vec3(value);
}   

// ─────────────────────────────────────────────────────────────────────────────
// Rendering
// ─────────────────────────────────────────────────────────────────────────────

void CQuake3BSP::DrawForward(mat4x4 view, mat4x4 projection)
{
    bool first = true;
    for (auto& model : models)
    {
        vec3 min = vec3(model.mins[0], model.mins[1], model.mins[2]) / MAP_SCALE;
        vec3 max = vec3(model.maxs[0], model.maxs[1], model.maxs[2]) / MAP_SCALE;

        if (Camera::frustum.IsBoxVisible(min, max))
            RenderBSP(Camera::finalizedPosition, model, mat4(1.0f / MAP_SCALE), first, first);

        first = false;
    }
}

void CQuake3BSP::RenderBSP(const glm::vec3& cameraPos, tBSPModel& model,
    mat4 modelMatrix, bool useClusterVis, bool lightmap)
{
    int cameraCluster = FindClusterAtPosition(cameraPos);
    int drawnFaces = 0;

    LightVolPointData lightData = { vec3(0), vec3(0), vec3(0) };

    // bgfx: depth writes are controlled per-draw via BGFX_STATE_WRITE_Z in the shader submit call.
    // Opaque pass uses depth write (set in RenderMergedFace / RenderSingleFace state flags).
    if (!lightmap)
    {
        // model.mins/maxs are already in engine Y-up space after the LoadBSP
        // axis swap, but still in raw BSP units (not divided by MAP_SCALE).
        const vec3 localCenter(
            (model.mins[0] + model.maxs[0]) * 0.5f,
            (model.mins[1] + model.maxs[1]) * 0.5f,
            (model.mins[2] + model.maxs[2]) * 0.5f
        );

        // modelMatrix maps raw-BSP local space → engine world space (MAP_SCALE-divided).
        // Apply it directly — no axis re-conversion needed, the swap already happened at load.
        const vec3 worldCenter_engine = vec3(modelMatrix * vec4(localCenter, 1.0f));


        // GetLightvolColorPoint expects raw BSP units (engine Y-up), so scale back up.
        lightData = GetLightvolColorPoint(worldCenter_engine * MAP_SCALE);
        lightData *= 2.0f;
    }

    std::vector<bool> renderedFaces(m_numOfFaces);

    if (useClusterVis)
    {
        for (const tBSPLeaf& leaf : leafs)
        {
            if (leaf.cluster < 0) continue;
            if (!IsClusterVisible(cameraCluster, leaf.cluster)) continue;

            for (int i = 0; i < leaf.n_leaffaces; i++)
            {
                int faceIndex = leafFaces[leaf.leafface + i];

                if (model.face <= faceIndex && faceIndex < model.face + model.n_faces)
                {
                    int mergedIndex = mergedFacesMapping[faceIndex];

                    if (!renderedFaces[mergedIndex])
                    {
                        renderedFaces[mergedIndex] = true;

                        FaceRenderData renderData;
                        renderData.faceIndex = mergedIndex;
                        renderData.useLightmap = lightmap;
                        renderData.lightPointData = lightData;
                        renderData.modelMatrix = modelMatrix;

                        if (IsFaceTransparent(mergedFacesData[mergedIndex].referenceFace))
                            facesToDrawTransparent.push_back(renderData);
                        else if (RenderMergedFace(mergedIndex, lightmap, lightData, modelMatrix))
                            drawnFaces++;
                    }
                }
            }
        }
    }
    else
    {
        for (int i = model.face; i < model.face + model.n_faces; i++)
        {
            const int& mergedIndex = mergedFacesMapping[i];

            if (!renderedFaces[mergedIndex])
            {
                renderedFaces[mergedIndex] = true;

                FaceRenderData renderData;
                renderData.faceIndex = mergedIndex;
                renderData.useLightmap = lightmap;
                renderData.lightPointData = lightData;
                renderData.modelMatrix = modelMatrix;

                if (IsFaceTransparent(mergedFacesData[renderData.faceIndex].referenceFace))
                    facesToDrawTransparent.push_back(renderData);
                else if (RenderMergedFace(renderData.faceIndex, lightmap, lightData, modelMatrix))
                    drawnFaces++;
            }
        }
    }
}

void CQuake3BSP::RenderTransparentFaces()
{
    // bgfx: transparent faces disable depth writes – pass BGFX_STATE_WRITE_Z=0 in submit flags.
    // The state is applied inside RenderMergedFace when called from this path.

    for (auto& face : facesToDrawTransparent)
        RenderMergedFace(face.faceIndex, face.useLightmap, face.lightPointData, face.modelMatrix);

    facesToDrawTransparent.clear();
    facesToDrawTransparent.reserve(100);
}

bool CQuake3BSP::IsFaceTransparent(int index)
{
    return cachedFaces[index].transparent;
}

// ─── RenderSingleFace ────────────────────────────────────────────────────────
// Issues one indexed draw call for a single BSP face using its prebuilt VAO.
// All texture binding and draw submission go through the existing abstraction
// layer (ShaderProgram::SetTexture / VAO::DrawIndexed).
// ─────────────────────────────────────────────────────────────────────────────

bool CQuake3BSP::RenderSingleFace(int index, bool lightmap,
    LightVolPointData lightData, mat4 model)
{
    auto bounds = faceBounds[index];
    bounds = bounds.Transform(model);

    if (!Camera::frustum.IsBoxVisible(bounds.Min, bounds.Max))
        return false;

    auto& buffers = FB_array.FB_Idx[index];
    if (!bgfx::isValid(buffers.VBO) || !bgfx::isValid(buffers.EBO))
        return false;

    const CachedFaceTextureData& data = cachedFaces[index];

    int faceTexture = GetFaceTextureNativeId(data.textureId);
    if (faceTexture == 0) return false;

    Shader* shader = ShaderManager::GetShaderProgram(
        "bsp", data.isCube ? "bsp/bsp_cube" : "bsp/bsp");
    shader->UseProgram();

    shader->SetUniform("light_color", lightData.ambientColor);
    shader->SetUniform("direct_light_color", lightData.directColor);
    shader->SetUniform("direct_light_dir", lightData.direction);

    bgfx::TextureHandle albedoHandle = { static_cast<uint16_t>(faceTexture) };
    if (data.isCube)
        shader->SetCubemapTexture("s_bspTexture", albedoHandle);
    else
        shader->SetTexture("s_bspTexture", albedoHandle);

    // Bind all active lightmap slots. Slot 0 is always s_bspLightmap (primary).
    // Slots 1-3 are s_bspLightmap1 .. s_bspLightmap3 for multi-style blending.
    static const char* lmSamplers[BSP_MAX_LIGHTMAP_STYLES] = {
        "s_bspLightmap", "s_bspLightmap1", "s_bspLightmap2", "s_bspLightmap3"
    };
    int missingId = m_missingLightmap ? (int)m_missingLightmap->getID() : 0;
    for (int s = 0; s < BSP_MAX_LIGHTMAP_STYLES; ++s) {
        int lmId = (s < data.numActiveSlots) ? data.lightmapIds[s] : missingId;
        bgfx::TextureHandle lmHandle = { static_cast<uint16_t>(lmId) };
        shader->SetTexture(lmSamplers[s], lmHandle);
    }
    shader->SetUniform("numLightmapSlots", (float)data.numActiveSlots);

    shader->SetUniform("view", Camera::finalizedView);
    shader->SetUniform("projection", Camera::finalizedProjection);
    shader->SetUniform("model", model);

    EngineMain::MainInstance->MainRenderer->SetSurfaceShaderUniforms(shader);

    bgfx::setVertexBuffer(0, buffers.VBO);
    bgfx::setIndexBuffer(buffers.EBO);

    shader->Submit(ViewIdManager::GetCurrentId());
    return true;
}

// ─── RenderMergedFace ────────────────────────────────────────────────────────

bool CQuake3BSP::RenderMergedFace(int mergedIndex, bool lightmap,
    LightVolPointData lightData, mat4 model)
{

    auto startState = BgfxStateManager::GetState();

    const auto& mergedFace = mergedFacesData[mergedIndex];

    auto bounds = mergedFace.bounds;
    bounds = bounds.Transform(model * scale(vec3(1.0f) * MAP_SCALE));

    if (!Camera::frustum.IsBoxVisible(bounds.Min, bounds.Max))
        return false;

    if (!bgfx::isValid(mergedFace.vbo) || !bgfx::isValid(mergedFace.ibo))
        return false;

    const CachedFaceTextureData& data = cachedFaces[mergedFace.referenceFace];


    int faceTexture = GetFaceTextureNativeId(data.textureId);


    if (data.animatedTextureFrames.size() > 0)
    {
        // Simple frame animation: cycle through frames based on time.
        uint64_t timeMs = Time::GameTime * 1000.0f;
        size_t frameIndex = (timeMs / 200) % data.animatedTextureFrames.size(); // Change frame every 200ms
        faceTexture = GetFaceTextureNativeId(data.animatedTextureFrames[frameIndex]);
    }

    if (faceTexture == 0) return false;

    Shader* shader = ShaderManager::GetShaderProgram(
        "bsp/vs_bsp", data.isCube ? "bsp/fs_bsp_cube" : "bsp/fs_bsp");
    shader->UseProgram();

    shader->SetUniform("useVertexLight",
        data.lightmapId == (m_missingLightmap ? (int)m_missingLightmap->getID() : 0));
    shader->SetUniform("light_color", lightData.ambientColor);
    shader->SetUniform("direct_light_color", lightData.directColor);
    shader->SetUniform("direct_light_dir", lightData.direction);
	shader->SetUniform("isRBSP", m_isRBSP);

    bgfx::TextureHandle albedoHandle = { static_cast<uint16_t>(faceTexture) };
    if (data.isCube)
        shader->SetCubemapTexture("s_bspTexture", albedoHandle);
    else
        shader->SetTexture("s_bspTexture", albedoHandle);

    // Bind all active lightmap slots. When lightmaps are disabled, fall back to
    // the black/white dummy for slot 0 and missing for the rest.
    static const char* lmSamplers[BSP_MAX_LIGHTMAP_STYLES] = {
        "s_bspLightmap", "s_bspLightmap1", "s_bspLightmap2", "s_bspLightmap3"
    };
    int missingId = m_missingLightmap ? (int)m_missingLightmap->getID() : 0;
    for (int s = 0; s < BSP_MAX_LIGHTMAP_STYLES; ++s) {
        int lmId;
        if (!lightmap) {
            lmId = (s == 0) ? GetWhiteLightmapNativeId() : missingId;
        }
        else {
            lmId = (s < data.numActiveSlots) ? data.lightmapIds[s] : missingId;
        }
        bgfx::TextureHandle lmHandle = { static_cast<uint16_t>(lmId) };
        shader->SetTexture(lmSamplers[s], lmHandle);
    }
    shader->SetUniform("numLightmapSlots", (float)(lightmap ? data.numActiveSlots : 0));

    static const char* styleUniforms[BSP_MAX_LIGHTMAP_STYLES] = {
    "lmStyleColor0", "lmStyleColor1", "lmStyleColor2", "lmStyleColor3"
    };
    for (int s = 0; s < BSP_MAX_LIGHTMAP_STYLES; ++s) {
        glm::vec3 sc = GetStyleColor(data.lightmapStyles[s]);
        shader->SetUniform(styleUniforms[s], glm::vec4(sc, 1.0f));
    }

    shader->SetUniform("view", Camera::finalizedView);
    shader->SetUniform("projection", Camera::finalizedProjection);
    shader->SetUniform("model", model);

    EngineMain::MainInstance->MainRenderer->SetSurfaceShaderUniforms(shader);
    LightManager::ApplyPointLightToShader(shader, bounds.Min, bounds.Max);

    bgfx::setVertexBuffer(0, mergedFace.vbo);
    bgfx::setIndexBuffer(mergedFace.ibo);

    BgfxStateManager::Apply();
    shader->Submit(ViewIdManager::GetCurrentId());
    BgfxStateManager::SetState(startState);
    return true;
}

void CQuake3BSP::renderFaces()
{
    for (auto& f : Rbuffers.v_faceVBOs)
        RenderSingleFace(f.first, true, LightVolPointData(), scale(vec3(1.0f) / MAP_SCALE));
}

// ─────────────────────────────────────────────────────────────────────────────
// Misc
// ─────────────────────────────────────────────────────────────────────────────

vector<BSPModelRef> CQuake3BSP::GetAllModelRefs()
{
    vector<BSPModelRef> refs;
    bool first = true;
    for (size_t i = 0; i < models.size(); ++i)
    {
        BSPModelRef ref(this, static_cast<int>(i), models[i]);
        ref.useBspVisibility = first;
        refs.push_back(ref);
        first = false;
    }
    return refs;
}

void CQuake3BSP::BuildStaticOpaqueObstacles()
{
    opaqueVBOs.resize(models.size());

    for (int i = 0; i < (int)models.size(); i++)
    {
        BSPModelRef ref(this, i, models[i]);

        auto vertices = ref.GetVertices(true, true);
        auto indices = ref.GetIndices(true, true);

        OpaqueModelVBO modelVBO;

        if (vertices.size() > 0 && indices.size() > 0)
        {
            const bgfx::Memory* vMem = bgfx::copy(
                vertices.data(),
                static_cast<uint32_t>(vertices.size() * sizeof(VertexData)));
            modelVBO.vbo = bgfx::createVertexBuffer(vMem, VertexData::Declaration());

            const bgfx::Memory* iMem = bgfx::copy(
                indices.data(),
                static_cast<uint32_t>(indices.size() * sizeof(uint32_t)));
            modelVBO.ibo = bgfx::createIndexBuffer(iMem, BGFX_BUFFER_INDEX32);
            modelVBO.IndexCount = static_cast<uint32_t>(indices.size());
        }

        opaqueVBOs[i] = modelVBO;
    }
}

std::string CQuake3BSP::GetLightMapFilePathFromId(int id, const std::string& filePath)
{
    const size_t len = filePath.size();
    assert(len > 4 && filePath.compare(len - 4, 4, ".bsp") == 0);

    const char* str = filePath.data();
    const char* end = str + len;
    const char* p = end;
    while (p > str && *(p - 1) != '/' && *(p - 1) != '\\') --p;

    size_t prefixLen = p - str;
    size_t mapNameLen = len - prefixLen - 4;

    std::string result;
    result.reserve(prefixLen + mapNameLen + 1 + 3 + 4 + 4);
    result.append(str, prefixLen);
    result.append(p, mapNameLen);
    result.push_back('/');
    result += "lm_";

    char digits[4];
    int tmp = id;
    for (int i = 3; i >= 0; --i) {
        digits[i] = char('0' + (tmp % 10));
        tmp /= 10;
    }

    if (digits[3] == '-')
        return ""; // invalid ID

    result.append(digits, 4);
    result += ".tga";
    return result;
}

void CQuake3BSP::BSPDebug(int index)
{
    printf("\nFace:----> %d\n", index);
    printf("EndFace.\n");
}

static void AddPhysicsBodyForEntityAndModel(Entity* entity, BSPModelRef& model) {

    vec3 bodyPos = vec3(0);

    vector<RefConst<Shape>> shapes;
    vector<RefConst<Shape>> shapesSky;

    // Box / AABB special-case (keep as original intent)
    if (model.model.face > 0 && model.model.n_faces == 0)
    {
        vec3 min = vec3(
            model.model.mins[0],
            model.model.mins[1],
            model.model.mins[2]) / MAP_SCALE;

        vec3 max = vec3(
            model.model.maxs[0],
            model.model.maxs[1],
            model.model.maxs[2]) / MAP_SCALE;

        bodyPos = (min + max) / 2.0f;

        auto boxShape = Physics::CreateBoxShape(abs(max - min));
        shapes.push_back(boxShape);
    }
    else
    {
        // Combined mesh structures for non-convex geometry
        struct CombinedMesh {
            vector<vec3> vertices;
            vector<uint32_t> indices;
            vector<string> materials;
        };

        CombinedMesh standardMesh;
        CombinedMesh skyMesh;

        // For convex collisions: points grouped per texture
        unordered_map<string, vector<vec3>> convexPointsByTexture;
        unordered_map<string, vector<vec3>> convexPointsSkyByTexture;

        // Iterate faces and distribute into buckets
        for (int i = model.model.face; i < model.model.face + model.model.n_faces; ++i)
        {
            tBSPFace face = model.bsp->m_pFaces[i];
            string textureName = string(model.bsp->pTextures[face.textureID].strName);

            bool sky = false;
            if (StringHelper::Contains(textureName, "_cube"))
            {
                sky = true;
                if (model.id != 0)
                    continue; // same behavior as before
            }

            // Get the vertex array & indices for face i
            auto& vertices = model.bsp->Rbuffers.v_faceVBOs[i];
            auto& indices = model.bsp->Rbuffers.v_faceIDXs[i];

            // Convert vertex positions
            vector<vec3> facePositions;
            facePositions.reserve(vertices.size());
            for (auto& v : vertices)
                facePositions.push_back(v.Position / MAP_SCALE);


            // Build a temporary mesh for this face to clean up degenerates FIRST
            MeshUtils::PositionVerticesIndices faceMesh;
            faceMesh.vertices = facePositions;
            for (auto idx : indices)
                faceMesh.indices.push_back(static_cast<int>(idx));

            faceMesh = MeshUtils::RemoveDegenerates(faceMesh, 0.01f, 0.00f);

            if (faceMesh.vertices.empty() || faceMesh.indices.empty())
                continue;

            // Append to the proper combined mesh buffer
            CombinedMesh& targetMesh = sky ? skyMesh : standardMesh;
            uint32_t offset = static_cast<uint32_t>(targetMesh.vertices.size());

            targetMesh.vertices.insert(targetMesh.vertices.end(), faceMesh.vertices.begin(), faceMesh.vertices.end());

            for (int idx : faceMesh.indices)
                targetMesh.indices.push_back(static_cast<uint32_t>(idx) + offset);

            // Populate the material list per triangle
            size_t numTriangles = faceMesh.indices.size() / 3;
            for (size_t t = 0; t < numTriangles; ++t)
                targetMesh.materials.push_back(textureName);
        } // end faces loop



        {
            // Create single mesh shape for all standard faces
            if (!standardMesh.vertices.empty() && !standardMesh.indices.empty())
            {
                auto shape = Physics::CreateMeshShape(standardMesh.vertices, standardMesh.indices, standardMesh.materials, entity->ConvexCollision);
                shapes.push_back(shape);
            }

            // Create single mesh shape for all sky faces
            if (!skyMesh.vertices.empty() && !skyMesh.indices.empty())
            {
                auto shape = Physics::CreateMeshShape(skyMesh.vertices, skyMesh.indices, skyMesh.materials);
                shapesSky.push_back(shape);
            }
        }
    }

    if (shapes.empty() && shapesSky.empty())
        return; // Safety bailout if geometry is completely empty

    // Use single shape directly if only 1 exists, otherwise wrap in a static compound shape
    RefConst<Shape> finalShape = shapes.size() == 1 ? shapes[0] : Physics::CreateStaticCompoundShapeFromConvexShapes(shapes);

    Body* body = Physics::CreateBodyFromShape(entity, vec3(0), finalShape, 300, entity->Static ? JPH::EMotionType::Static : JPH::EMotionType::Kinematic, entity->DefaultBrushGroup, entity->DefaultBrushCollisionMask);

    Physics::SetBodyPosition(body, bodyPos);

    entity->LeadBody = body;

    model.StaticNavigation = entity->Static;

    if (!shapesSky.empty())
    {
        RefConst<Shape> skyShape = shapesSky.size() == 1 ? shapesSky[0] : Physics::CreateStaticCompoundShapeFromConvexShapes(shapesSky);
        Body* bodySky = Physics::CreateBodyFromShape(entity, vec3(0), skyShape, 10, JPH::EMotionType::Static, BodyType::WorldSkybox, BodyType::CharacterCapsule);
        entity->Bodies.push_back(bodySky);
    }
}

void CQuake3BSP::LoadToLevel()
{
    auto parsedEntities = MapParser::ParseBSPEntities(entities);
    parsedEntities[0].Properties["classname"] = "worldspawn";

    auto models = GetAllModelRefs();

    for (auto& entityData : parsedEntities)
    {
        Entity* ent = LevelObjectFactory::instance().create(entityData.Classname);
        if (!ent) ent = new Entity();

        ent->FromData(entityData);

        string modelStr = entityData.GetPropertyString("model");

        if (modelStr.size() > 1)
        {
            if (modelStr[0] == '*')
            {
                int modelId = stoi(modelStr.substr(1));

                BSPModelRef modelRef = models[modelId];
                AddPhysicsBodyForEntityAndModel(ent, modelRef);
                modelRef.CalculateAveragePosition();
                ent->Drawables.push_back(new BSPModelRef(modelRef));
            }
        }
        else if (ent->ClassName == "worldspawn")
        {
            BSPModelRef modelRef = models[0];
            AddPhysicsBodyForEntityAndModel(ent, modelRef);
            modelRef.CalculateAveragePosition();
            ent->Drawables.push_back(new BSPModelRef(modelRef));
        }

        Level::Current->AddEntity(ent);
    }
}



// ─────────────────────────────────────────────────────────────────────────────
// BSPModelRef
// ─────────────────────────────────────────────────────────────────────────────

BoundingBox BSPModelRef::GetTransformedBounds()
{
    return bounds.Transform(GetWorldMatrix() * scale(vec3(1.0f) * MAP_SCALE));
}

mat4 BSPModelRef::GetWorldMatrix()
{
    return translate(Position) * MathHelper::GetRotationMatrix(Rotation) * scale(Scale / MAP_SCALE);
}

BSPModelRef::BSPModelRef(CQuake3BSP* bsp_ptr, int model_id, tBSPModel& model_ref)
    : bsp(bsp_ptr), id(model_id), model(model_ref)
{
    auto vertices = GetVertices();

    vector<vec3> points;
    points.reserve(vertices.size());
    for (const auto& v : vertices)
        points.push_back(v.Position / MAP_SCALE);

    bounds = BoundingBox::FromPoints(points);

    ReceiveDetailShadows = true;

}

BSPModelRef::~BSPModelRef() {}

void BSPModelRef::BuildVisBlocker() {}

float BSPModelRef::GetDistanceToCamera()
{
    return distance(avgPosition, Camera::finalizedPosition);
}

void BSPModelRef::CalculateAveragePosition()
{
    avgPosition = GetTransformedBounds().Center();
}

bool BSPModelRef::IsCameraVisible()
{
    if (IsInFrustrum(Camera::frustum))
        return IsBspVisible();
    return false;
}

bool BSPModelRef::IsInFrustrum(Frustum frustrum)
{
    auto b = GetTransformedBounds();
    return frustrum.IsBoxVisible(b.Min, b.Max);
}

bool BSPModelRef::IsBspVisible()
{
    int sourceC = bsp->FindClusterAtPosition(Camera::finalizedPosition);
    auto b = GetTransformedBounds();

    if (CheckPointBspVisible(sourceC, b.Center())) return true;
    if (CheckPointBspVisible(sourceC, b.Max))      return true;
    if (CheckPointBspVisible(sourceC, b.Min))      return true;
    if (CheckPointBspVisible(sourceC, mix(b.Center(), b.Min, 0.5f))) return true;

    return false;
}

bool BSPModelRef::CheckPointBspVisible(int cameraCluster, vec3 position)
{
    int targetC = bsp->FindClusterAtPosition(position);
    return bsp->IsClusterVisible(cameraCluster, targetC);
}

vector<tBSPFace> BSPModelRef::GetFaces()
{
    vector<tBSPFace> faces;
    for (int i = model.face; i < model.face + model.n_faces; i++)
        faces.push_back(bsp->m_pFaces[i]);
    return faces;
}

vector<MeshUtils::PositionVerticesIndices> BSPModelRef::GetNavObstacleMeshes()
{
    vector<MeshUtils::PositionVerticesIndices> result;

    MeshUtils::PositionVerticesIndices meshData;
    meshData.indices = GetIndices(true);

    auto vertices = GetVertices(true);
    for (auto& vertex : vertices)
        meshData.vertices.push_back(vertex.Position / MAP_SCALE);

    meshData = MeshUtils::RemoveDegenerates(meshData, 0.01f, 0.00f);
    result.push_back(meshData);
    return result;
}

std::vector<VertexData> BSPModelRef::GetVertices(bool collisionOnly, bool opaqueOnly)
{
    std::vector<VertexData> result;

    for (int i = model.face; i < model.face + model.n_faces; i++)
    {
        if (collisionOnly)
        {
            string textureName = string(bsp->pTextures[bsp->m_pFaces[i].textureID].strName);
            if (StringHelper::Contains(textureName, "_cube")) continue;
        }

        if (opaqueOnly)
        {
            auto& faceData = bsp->cachedFaces[i];
            if (faceData.textureId == 0 || faceData.transparent) continue;
        }

        auto& faceVertices = bsp->Rbuffers.v_faceVBOs[i];
        result.insert(result.end(), faceVertices.begin(), faceVertices.end());
    }
    return result;
}

std::vector<uint32_t> BSPModelRef::GetIndices(bool collisionOnly, bool opaqueOnly)
{
    std::vector<uint32_t> result;
    uint32_t vertexOffset = 0;

    for (int i = model.face; i < model.face + model.n_faces; i++)
    {
        if (collisionOnly)
        {
            string textureName = string(bsp->pTextures[bsp->m_pFaces[i].textureID].strName);
            if (StringHelper::Contains(textureName, "_cube")) continue;
        }

        if (opaqueOnly)
        {
            auto& faceData = bsp->cachedFaces[i];
            if (faceData.textureId == 0 || faceData.transparent) continue;
        }

        auto& faceIndices = bsp->Rbuffers.v_faceIDXs[i];
        for (auto idx : faceIndices)
            result.push_back(static_cast<uint32_t>(idx) + vertexOffset);

        vertexOffset += static_cast<uint32_t>(bsp->Rbuffers.v_faceVBOs[i].size());
    }
    return result;
}

void BSPModelRef::FinalizeFrameData()
{
    finalWorldMatrix = GetWorldMatrix();
}

void BSPModelRef::DrawForward(mat4x4 view, mat4x4 projection)
{

    auto state = BgfxStateManager::GetState();

    if (Transparent)
    {
        BgfxStateManager::SetWriteDepth(false);
    }

    bsp->RenderBSP(Camera::finalizedPosition, model, finalWorldMatrix,
        useBspVisibility, Static);

    if (Transparent)
    {
        bsp->RenderTransparentFaces();
    }



    BgfxStateManager::SetState(state);

}

void BSPModelRef::DrawDepth(mat4x4 view, mat4x4 projection)
{
    const auto& vbo = bsp->opaqueVBOs[id];
    if (vbo.IndexCount == 0)
        return;

    if (Transparent) return;

    if (!bgfx::isValid(vbo.vbo) || !bgfx::isValid(vbo.ibo)) return;

    Shader* shader = ShaderManager::GetShaderProgram("bsp/vs_bsp", "bsp/fs_bsp_empty");
    shader->UseProgram();
    shader->SetUniform("view", view);
    shader->SetUniform("projection", projection);
    shader->SetUniform("model", finalWorldMatrix);

    bgfx::setVertexBuffer(0, vbo.vbo);
    bgfx::setIndexBuffer(vbo.ibo);

    BgfxStateManager::Apply();

    shader->Submit(ViewIdManager::GetCurrentId());
}