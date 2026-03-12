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
    m_pVerts(nullptr), m_pFaces(nullptr), m_pIndices(nullptr),
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
    delete[] m_pFaces;
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

    // LightVols
    {
        size_t cnt = 0;
        if (checkLump(kLightVolumes, sizeof(tBSPLightvol), cnt)) {
            lightVols.resize(cnt);
            memcpy(lightVols.data(), base + lumps[kLightVolumes].offset, cnt * sizeof(tBSPLightvol));

            std::unordered_map<uint64_t, uint32_t> lookup;
            lookup.reserve(cnt);
            lightVolIndices.reserve(cnt);

            for (const auto& lv : lightVols) {
                uint64_t key = 0;
                memcpy(&key, &lv, sizeof(tBSPLightvol));
                auto [it, inserted] = lookup.emplace(key, (uint32_t)lightVolPalette.size());
                if (inserted)
                    lightVolPalette.push_back(lv);
                lightVolIndices.push_back(it->second);
            }

            lightVolIndices.shrink_to_fit();
            lightVolPalette.shrink_to_fit();

            Logger::Log("Compressed light volumes from " + std::to_string(lightVols.size()) +
                " to " + std::to_string(lightVolPalette.size()));

            lightVols.clear();
            lightVols.shrink_to_fit();
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

    // Indices, Faces, Textures, Lightmaps
    {
        size_t cntIdx = 0;
        if (checkLump(kIndices, sizeof(int), cntIdx)) {
            m_numOfIndices = static_cast<int>(cntIdx);
            m_pIndices = new int[cntIdx];
            memcpy(m_pIndices, base + lumps[kIndices].offset, cntIdx * sizeof(int));
        }

        size_t cntFaces = 0;
        if (checkLump(kFaces, sizeof(tBSPFace), cntFaces)) {
            m_numOfFaces = static_cast<int>(cntFaces);
            m_pFaces = new tBSPFace[cntFaces];
            memcpy(m_pFaces, base + lumps[kFaces].offset, cntFaces * sizeof(tBSPFace));
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

        size_t cntLM = 0;
        if (checkLump(kLightmaps, sizeof(tBSPLightmap), cntLM)) {
            m_numOfLightmaps = static_cast<int>(cntLM);
            pLightmaps = new tBSPLightmap[cntLM];
            memcpy(pLightmaps, base + lumps[kLightmaps].offset, cntLM * sizeof(tBSPLightmap));
            for (int i = 0; i < m_numOfLightmaps; ++i)
                Rbuffers.G_lightMaps.push_back(pLightmaps[i]);
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
}

void CQuake3BSP::CreateVBO(int index)
{
    tBSPFace* pFace = &m_pFaces[index];
    auto& vertices = Rbuffers.v_faceVBOs[index];

    for (int v = 0; v < pFace->numOfVerts; v++) {
        const tBSPVertex& bspVert = m_pVerts[pFace->startVertIndex + v];
        VertexData vd;

        vd.Position = bspVert.vPosition;
        vd.Normal = bspVert.vNormal;
        vd.TextureCoordinate = bspVert.vTextureCoord;
        vd.ShadowMapCoords = bspVert.vLightmapCoord;

        vd.Color = glm::vec4(
            (float)bspVert.color[0] / 255.0f,
            (float)bspVert.color[1] / 255.0f,
            (float)bspVert.color[2] / 255.0f,
            (float)bspVert.color[3] / 255.0f);

        vertices.push_back(vd);
    }

    auto bounds = BoundingBox::FromVertices(vertices);
    bounds.Min -= vec3(0.1f);
    bounds.Max += vec3(0.1f);
    faceBounds.push_back(bounds);
}

void CQuake3BSP::CreateIndices(int index)
{
    tBSPFace* pFace = &m_pFaces[index];
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

void CQuake3BSP::CreateRenderBuffers(int index)
{
    const auto& vertices = Rbuffers.v_faceVBOs[index];
    const auto& indices  = Rbuffers.v_faceIDXs[index];

    auto& fb = FB_array.FB_Idx[index];

    // ── Vertex buffer ────────────────────────────────────────────────────────
    const bgfx::Memory* vMem = bgfx::copy(
        vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(VertexData)));
    fb.VBO = bgfx::createVertexBuffer(vMem, VertexData::Declaration());

    // ── Index buffer (uint32) ────────────────────────────────────────────────
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
        const uint8_t v = 200;
        uint8_t pixels[16] = {
            v,v,v,v,  v,v,v,v,
            v,v,v,v,  v,v,v,v
        };
        // Texture(data, width, height, format, generateMipmaps)
        m_missingLightmap = std::make_shared<Texture>(pixels, 2, 2, bgfx::TextureFormat::RGBA8, false);
		m_missingLightmap->setName("Missing Lightmap");
    }

    // ── White lightmap (full-bright 2×2 RGBA) ────────────────────────────────
    {
        uint8_t pixels[16] = {
            255,255,255,255,  255,255,255,255,
            255,255,255,255,  255,255,255,255
        };
        m_whiteLightmap = std::make_shared<Texture>(pixels, 2, 2, bgfx::TextureFormat::RGBA8, false);
		m_whiteLightmap->setName("White Lightmap");
    }

    // ── Per-BSP-lightmap textures (128×128 RGB) ───────────────────────────────
    // The Texture(data, width, height, format) constructor uploads the pixel
    // data and sets appropriate filtering/wrap parameters internally, so no
    // manual glTexParameteri calls are needed here.
    m_lightmapTextures.reserve(Rbuffers.G_lightMaps.size());
    for (size_t i = 0; i < Rbuffers.G_lightMaps.size(); ++i)
    {
        const uint8_t* pixels =
            reinterpret_cast<const uint8_t*>(Rbuffers.G_lightMaps[i].imageBits);

        // generateMipmaps = true to match original glGenerateMipmap call
        auto tex = std::make_shared<Texture>(pixels, 128, 128, bgfx::TextureFormat::RGB8, true);
		tex->setName("BSP Lightmap " + std::to_string(i));
        m_lightmapTextures.push_back(std::move(tex));
    }
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
    tBSPFace* pFace = &m_pFaces[index];

    string textureName = string(pTextures[pFace->textureID].strName);
    int    nameL = (int)textureName.length();

    bool isCube = false;
    if (nameL > 5) {
        isCube =
            (textureName[nameL - 1] == 'e') &&
            (textureName[nameL - 2] == 'b') &&
            (textureName[nameL - 3] == 'u') &&
            (textureName[nameL - 4] == 'c');
    }

    // ShaderProgram is only referenced during rendering, not preload.
    string texturePath = "GameData/" + textureName + ".png";

    if (isCube) {
        auto splitPath = StringHelper::Split(texturePath, '/');
        string fileName = splitPath[splitPath.size() - 1];
        texturePath = "GameData/env/" + fileName;
    }

    int faceTexture;
    if (isCube)
        faceTexture = AssetRegistry::GetTextureCubeFromFile(texturePath)->getID();
    else
        faceTexture = AssetRegistry::GetTextureFromFile(texturePath)->getID();

    // Determine lightmap native ID using the abstracted helper.
    // When lightmaps were loaded from BSP data, GetLightmapNativeId handles
    // the slot lookup and falls back to the missing-lightmap texture.
    // When lightmaps come from external .tga files, AssetRegistry is used.
    int lightmapId = 0;

    if (m_numOfLightmaps > 0)
    {
        lightmapId = GetLightmapNativeId(pFace->lightmapID);
    }
    else if (!isCube)
    {
        string lightMapPath = GetLightMapFilePathFromId(pFace->lightmapID, filePath);

        if (lightMapPath.empty())
        {
            // No lightmap file found for this face; use missing-lightmap fallback.
			lightmapId = GetLightmapNativeId(-1);
        }
        else
        {
            auto   lmTex = AssetRegistry::GetTextureFromFile(lightMapPath);
            lightmapId = (lmTex && lmTex->getID() != 0)
                ? (int)lmTex->getID()
                : GetLightmapNativeId(-1); // missing fallback
        }


    }

    CachedFaceTextureData data;
    data.isCube = isCube;
    data.lightmapId = lightmapId;
    data.textureId = faceTexture;
    data.textureName = textureName;
    data.transparent = textureName.ends_with("_t");
    data.numOfIndices = pFace->numOfIndices;
    cachedFaces[index] = data;
}

void CQuake3BSP::PreloadFaces()
{
    cachedFaces = new CachedFaceTextureData[m_numOfFaces];
    for (int i = 0; i < m_numOfFaces; i++)
        PreloadFace(i);
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
            string finalStr = modelId + "|" + texId + "|" + lightmapId;

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
    return Physics::SphereTrace(position, volPosition, maxDimension / MAP_SCALE * 0.3f,
        BodyType::WorldOpaque).hasHit == false;
}

LightVolPointData CQuake3BSP::GetLightvolColorPoint(const glm::vec3& position, bool wallCheck)
{
    if (lightVolIndices.empty())
        return { vec3(0), vec3(0.3f), vec3(0, -1, 0) };

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
            const tBSPLightvol& vol = GetLightVol(idx);
            glm::vec3 ambient(
                static_cast<float>(vol.ambient[0]) / 255.0f,
                static_cast<float>(vol.ambient[1]) / 255.0f,
                static_cast<float>(vol.ambient[2]) / 255.0f);
            glm::vec3 directional(
                static_cast<float>(vol.directional[0]) / 255.0f,
                static_cast<float>(vol.directional[1]) / 255.0f,
                static_cast<float>(vol.directional[2]) / 255.0f);
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
                    const tBSPLightvol& vol = GetLightVol(idx);
                    LightData ld{
                        glm::vec3(vol.ambient[0] / 255.0f,
                                  vol.ambient[1] / 255.0f,
                                  vol.ambient[2] / 255.0f),
                        glm::vec3(vol.directional[0] / 255.0f,
                                  vol.directional[1] / 255.0f,
                                  vol.directional[2] / 255.0f),
                        computeLightDirection(vol.dir)
                    };

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

    LightVolPointData lightData = { vec3(0), vec3(1), vec3(0) };

    // bgfx: depth writes are controlled per-draw via BGFX_STATE_WRITE_Z in the shader submit call.
    // Opaque pass uses depth write (set in RenderMergedFace / RenderSingleFace state flags).

    if (!lightmap)
    {
        vec3 min = vec3(model.mins[0], model.mins[1], model.mins[2]);
        vec3 max = vec3(model.maxs[0], model.maxs[1], model.maxs[2]);
        lightData = GetLightvolColorPoint((min + max) / 2.0f);
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
    int lmId = data.lightmapId; // already a native ID stored at preload time

    if (faceTexture == 0) return false;

    Shader* shader = ShaderManager::GetShaderProgram(
        "bsp", data.isCube ? "bsp_cube" : "bsp");
    shader->UseProgram();

    shader->SetUniform("light_color", lightData.ambientColor);
    shader->SetUniform("direct_light_color", lightData.directColor);
    shader->SetUniform("direct_light_dir", lightData.direction);

    bgfx::TextureHandle albedoHandle = { static_cast<uint16_t>(faceTexture) };
    bgfx::TextureHandle lmHandle     = { static_cast<uint16_t>(lmId) };

    if (data.isCube)
        shader->SetCubemapTexture("s_bspTexture", albedoHandle);
    else
        shader->SetTexture("s_bspTexture", albedoHandle);

    shader->SetTexture("s_bspLightmap", lmHandle);
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
    const auto& mergedFace = mergedFacesData[mergedIndex];

    auto bounds = mergedFace.bounds;
    bounds = bounds.Transform(model * scale(vec3(1.0f) * MAP_SCALE));

    if (!Camera::frustum.IsBoxVisible(bounds.Min, bounds.Max))
        return false;

    if (!bgfx::isValid(mergedFace.vbo) || !bgfx::isValid(mergedFace.ibo))
        return false;

    const CachedFaceTextureData& data = cachedFaces[mergedFace.referenceFace];

    int faceTexture = GetFaceTextureNativeId(data.textureId);

    // Select lightmap: use white texture when lightmaps are disabled
    int lmId = lightmap ? data.lightmapId : GetWhiteLightmapNativeId();

    if (faceTexture == 0) return false;

    Shader* shader = ShaderManager::GetShaderProgram(
        "vs_bsp", data.isCube ? "fs_bsp_cube" : "fs_bsp");
    shader->UseProgram();

    shader->SetUniform("useVertexLight",
        data.lightmapId == (m_missingLightmap ? (int)m_missingLightmap->getID() : 0));

    shader->SetUniform("light_color", lightData.ambientColor);
    shader->SetUniform("direct_light_color", lightData.directColor);
    shader->SetUniform("direct_light_dir", lightData.direction);

    bgfx::TextureHandle albedoHandle = { static_cast<uint16_t>(faceTexture) };
    bgfx::TextureHandle lmHandle     = { static_cast<uint16_t>(lmId) };

    if (data.isCube)
        shader->SetCubemapTexture("s_bspTexture", albedoHandle);
    else
        shader->SetTexture("s_bspTexture", albedoHandle);

    shader->SetTexture("s_bspLightmap", lmHandle);
    shader->SetUniform("view", Camera::finalizedView);
    shader->SetUniform("projection", Camera::finalizedProjection);
    shader->SetUniform("model", model);

    EngineMain::MainInstance->MainRenderer->SetSurfaceShaderUniforms(shader);
    LightManager::ApplyPointLightToShader(shader, bounds.Min, bounds.Max);

    bgfx::setVertexBuffer(0, mergedFace.vbo);
    bgfx::setIndexBuffer(mergedFace.ibo);

    BgfxStateManager::Apply();

    shader->Submit(ViewIdManager::GetCurrentId());
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

        auto vertices = ref.GetVertices(false, true);
        auto indices = ref.GetIndices(false, true);

        OpaqueModelVBO modelVBO;

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

    if(digits[3] == '-')
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

static void AddPhysicsBodyForEntityAndModel(Entity* entity, BSPModelRef& model)
{

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
        // Grouped meshes per texture (normal and sky)
        unordered_map<string, MeshUtils::PositionVerticesIndices> meshesByTexture;
        unordered_map<string, MeshUtils::PositionVerticesIndices> meshesSkyByTexture;

        // For convex collisions: points per texture
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

            if (entity->ConvexCollision)
            {
                // Collect points per texture (sky/normal)
                if (sky)
                    convexPointsSkyByTexture[textureName].insert(convexPointsSkyByTexture[textureName].end(),
                        facePositions.begin(), facePositions.end());
                else
                    convexPointsByTexture[textureName].insert(convexPointsByTexture[textureName].end(),
                        facePositions.begin(), facePositions.end());
                continue; // no indices needed for convex hull
            }

            // Append to the proper mesh bucket (with index offset)
            if (sky)
            {
                auto& bucket = meshesSkyByTexture[textureName];
                size_t offset = bucket.vertices.size();
                bucket.vertices.insert(bucket.vertices.end(), facePositions.begin(), facePositions.end());
                for (int idx : indices)
                    bucket.indices.push_back(static_cast<int>(idx) + static_cast<int>(offset));
            }
            else
            {
                auto& bucket = meshesByTexture[textureName];
                size_t offset = bucket.vertices.size();
                bucket.vertices.insert(bucket.vertices.end(), facePositions.begin(), facePositions.end());
                for (int idx : indices)
                    bucket.indices.push_back(static_cast<int>(idx) + static_cast<int>(offset));
            }
        } // end faces loop

        // Create convex hull shapes per texture (if convex mode)
        if (entity->ConvexCollision)
        {
            for (auto& p : convexPointsByTexture)
            {
                auto& points = p.second;
                if (!points.empty())
                {
                    auto shape = Physics::CreateConvexHullFromPoints(points);
                    shapes.push_back(shape);
                }
            }
            for (auto& p : convexPointsSkyByTexture)
            {
                auto& points = p.second;
                if (!points.empty())
                {
                    auto shape = Physics::CreateConvexHullFromPoints(points);
                    shapesSky.push_back(shape);
                }
            }
        }
        else
        {
            // Create mesh shapes per texture (normal)
            for (auto& kv : meshesByTexture)
            {
                auto textureName = kv.first;
                auto mesh = kv.second;

                if (mesh.vertices.empty() || mesh.indices.empty())
                    continue;

                mesh = MeshUtils::RemoveDegenerates(mesh, 0.01f, 0.00f);
                if (mesh.vertices.empty() || mesh.indices.empty())
                    continue;

                auto shape = Physics::CreateMeshShape(mesh.vertices, mesh.indices, textureName);
                shapes.push_back(shape);
            }

            // Create mesh shapes per texture (sky)
            for (auto& kv : meshesSkyByTexture)
            {
                auto textureName = kv.first;
                auto mesh = kv.second;

                if (mesh.vertices.empty() || mesh.indices.empty())
                    continue;

                mesh = MeshUtils::RemoveDegenerates(mesh, 0.01f, 0.00f);
                if (mesh.vertices.empty() || mesh.indices.empty())
                    continue;

                auto shape = Physics::CreateMeshShape(mesh.vertices, mesh.indices, textureName);
                shapesSky.push_back(shape);
            }
        }
    }


    RefConst<Shape> finalShape = Physics::CreateStaticCompoundShapeFromConvexShapes(shapes);

    Body* body = Physics::CreateBodyFromShape(entity, vec3(0), finalShape, 10, entity->Static ? JPH::EMotionType::Static : JPH::EMotionType::Kinematic, entity->DefaultBrushGroup, entity->DefaultBrushCollisionMask);

    Physics::SetBodyPosition(body, bodyPos);

    entity->LeadBody = body;

    model.StaticNavigation = entity->Static;

    if (shapesSky.size())
    {
        RefConst<Shape> skyShape = Physics::CreateStaticCompoundShapeFromConvexShapes(shapesSky);
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
    bsp->RenderBSP(Camera::finalizedPosition, model, finalWorldMatrix,
        useBspVisibility, Static);

    if (Transparent)
        bsp->RenderTransparentFaces();
}

void BSPModelRef::DrawDepth(mat4x4 view, mat4x4 projection)
{
    const auto& vbo = bsp->opaqueVBOs[id];
    if (!bgfx::isValid(vbo.vbo) || !bgfx::isValid(vbo.ibo)) return;

    Shader* shader = ShaderManager::GetShaderProgram("vs_bsp", "fs_empty");
    shader->UseProgram();
    shader->SetUniform("view", view);
    shader->SetUniform("projection", projection);
    shader->SetUniform("model", finalWorldMatrix);

    bgfx::setVertexBuffer(0, vbo.vbo);
    bgfx::setIndexBuffer(vbo.ibo);

    BgfxStateManager::Apply();

    shader->Submit(ViewIdManager::GetCurrentId());
}