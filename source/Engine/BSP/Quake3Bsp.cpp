
#include "Quake3Bsp.h"

#include <algorithm> // std::sort
#include <cstring>   // GCC7 fix
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include "../DebugDraw.hpp"

#include "../AssetRegistry.h"

#include "../Camera.h"

#include "../Level.hpp"

#include "../MapParser.h"

#include "../LevelObjectFactory.h"
#include "../Level.hpp"

#include "../Renderer/Renderer.h"
#include "../EngineMain.h"
#include "../FileSystem/FileSystem.h"

#include "../LightSystem/LightManager.h"

#include <Physics.h>

#ifndef _MSC_VER 

#define strcpy_s strcpy
#define strcat_s strcat

#endif

CQuake3BSP::CQuake3BSP() {
    m_numOfVerts = 0;
    m_numOfFaces = 0;
    m_numOfIndices = 0;
    m_numOfTextures = 0;
    m_numOfLightmaps = 0;
    numVisibleFaces = 0;
    skipindices = 0;

    m_pVerts = NULL;
    m_pFaces = NULL;
    m_pIndices = NULL;
}

CQuake3BSP::~CQuake3BSP() 
{

    if (filePath == "") return;

    for (auto modelVBO : opaqueVBOs)
    {
        delete(modelVBO.vao);
        delete(modelVBO.ibo);
        delete(modelVBO.vbo);
    }

    for (auto mergedModel : mergedFacesData)
    {
        delete(mergedModel.vao);
        delete(mergedModel.ibo);
        delete(mergedModel.vbo);
    }
    if (m_pVerts != nullptr)
    {
        delete[] m_pVerts;
    }
    if (m_pFaces != nullptr)
    {
        delete[] m_pFaces;
    }
    if (m_pIndices != nullptr)
    {
        delete[] m_pIndices;
    }
    if (pTextures != nullptr)
    {
        delete[] pTextures;
    }
    if (pLightmaps != nullptr)
    {
        delete[] pLightmaps;
    }


    if (m_lightmap_gen_IDs) {
        glDeleteTextures(m_numOfLightmaps, m_lightmap_gen_IDs);
    }


    if (cachedFaces != nullptr)
    {
        delete[] cachedFaces;
    }
    

    glDeleteTextures(1, &missing_LM_id);
    glDeleteTextures(1, &white_LM_id);
}

bool CQuake3BSP::LoadBSP(const char* filename) {
    if (!filename) {
        printf("ERROR:: You must specify BSP file as parameter\n");
        return false;
    }

    // 1) Slurp entire BSP into RAM
    std::vector<uint8_t> fileData;
    try {
        fileData = FileSystemEngine::ReadFileBinary(filename);
    }
    catch (const std::exception& e) {
        printf("ERROR:: cannot read BSP file '%s': %s\n",
            filename, e.what());
        return false;
    }

    const uint8_t* base = fileData.data();
    size_t        totalSize = fileData.size();

    // 2) Read header
    if (totalSize < sizeof(tBSPHeader) + sizeof(tBSPLump) * kMaxLumps) {
        printf("ERROR:: BSP too small\n");
        return false;
    }

    tBSPHeader header;
    memcpy(&header, base, sizeof(header));

    tBSPLump lumps[kMaxLumps];
    memcpy(lumps,
        base + sizeof(header),
        sizeof(lumps));

    filePath = filename;

    // 3) Helper to validate a lump and compute element count
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

    // 4) Entities (text)
    {
        size_t cnt = 0;
        if (checkLump(kEntities, 1, cnt) && cnt > 0) {
            const char* ptr = reinterpret_cast<const char*>(base + lumps[kEntities].offset);
            entities.assign(ptr, cnt);
        }
    }

    // 5) Planes
    {
        size_t cnt = 0;
        if (checkLump(kPlanes, sizeof(tBSPPlane), cnt)) {
            planes.resize(cnt);
            memcpy(planes.data(),
                base + lumps[kPlanes].offset,
                cnt * sizeof(tBSPPlane));
            for (auto& p : planes) {
                float oldY = p.normal.y;
                p.normal.y = p.normal.z;
                p.normal.z = -oldY;
            }
        }
    }

    // 6) Nodes
    {
        size_t cnt = 0;
        if (checkLump(kNodes, sizeof(tBSPNode), cnt)) {
            nodes.resize(cnt);
            memcpy(nodes.data(),
                base + lumps[kNodes].offset,
                cnt * sizeof(tBSPNode));
        }
    }

    // 7) Leafs
    {
        size_t cnt = 0;
        if (checkLump(kLeafs, sizeof(tBSPLeaf), cnt)) {
            leafs.resize(cnt);
            memcpy(leafs.data(),
                base + lumps[kLeafs].offset,
                cnt * sizeof(tBSPLeaf));
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

    // 8) LeafFaces
    {
        size_t cnt = 0;
        if (checkLump(kLeafFaces, sizeof(int), cnt)) {
            leafFaces.resize(cnt);
            memcpy(leafFaces.data(),
                base + lumps[kLeafFaces].offset,
                cnt * sizeof(int));
        }
    }

    // 9) LeafBrushes
    {
        size_t cnt = 0;
        if (checkLump(kLeafBrushes, sizeof(int), cnt)) {
            leafBrushes.resize(cnt);
            memcpy(leafBrushes.data(),
                base + lumps[kLeafBrushes].offset,
                cnt * sizeof(int));
        }
    }

    // 10) Models + original bounds
    {
        size_t cnt = 0;
        if (checkLump(kModels, sizeof(tBSPModel), cnt)) {
            models.resize(cnt);
            memcpy(models.data(),
                base + lumps[kModels].offset,
                cnt * sizeof(tBSPModel));

            if (!models.empty()) {
                originalMins = glm::vec3(
                    models[0].mins[0],
                    models[0].mins[1],
                    models[0].mins[2]);
                originalMaxs = glm::vec3(
                    models[0].maxs[0],
                    models[0].maxs[1],
                    models[0].maxs[2]);
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

    // 11) Brushes & BrushSides
    {
        size_t cntB = 0, cntBS = 0;
        if (checkLump(kBrushes, sizeof(tBSPBrush), cntB)) {
            brushes.resize(cntB);
            memcpy(brushes.data(),
                base + lumps[kBrushes].offset,
                cntB * sizeof(tBSPBrush));
        }
        if (checkLump(kBrushSides, sizeof(tBSPBrushSide), cntBS)) {
            brushSides.resize(cntBS);
            memcpy(brushSides.data(),
                base + lumps[kBrushSides].offset,
                cntBS * sizeof(tBSPBrushSide));
        }
    }

    // 12) MeshVerts
    {
        size_t cnt = 0;
        if (checkLump(kIndices, sizeof(tBSPMeshVert), cnt)) {
            meshVerts.resize(cnt);
            memcpy(meshVerts.data(),
                base + lumps[kIndices].offset,
                cnt * sizeof(tBSPMeshVert));
        }
    }

    // 13) Effects
    {
        size_t cnt = 0;
        if (checkLump(kShaders, sizeof(tBSPEffect), cnt)) {
            effects.resize(cnt);
            memcpy(effects.data(),
                base + lumps[kShaders].offset,
                cnt * sizeof(tBSPEffect));
        }
    }

    // 14) Lightvols
    {
        size_t cnt = 0;
        if (checkLump(kLightVolumes, sizeof(tBSPLightvol), cnt)) {
            lightVols.resize(cnt);
            memcpy(lightVols.data(),
                base + lumps[kLightVolumes].offset,
                cnt * sizeof(tBSPLightvol));
        }
    }

    // 15) VisData
    if (lumps[kVisData].length >= 2 * sizeof(int)) {
        size_t off = lumps[kVisData].offset;
        visData.n_vecs = *reinterpret_cast<const int*>(base + off);
        visData.sz_vecs = *reinterpret_cast<const int*>(base + off + sizeof(int));
        size_t totalVis = size_t(visData.n_vecs) * visData.sz_vecs;
        visData.vecs.resize(totalVis);
        memcpy(visData.vecs.data(),
            base + off + 2 * sizeof(int),
            totalVis);
    }

    // 16) Vertices (Y‑up)
    {
        size_t cnt = 0;
        if (checkLump(kVertices, sizeof(tBSPVertex), cnt)) {
            m_numOfVerts = static_cast<int>(cnt);
            m_pVerts = new tBSPVertex[cnt];
            const uint8_t* ptr = base + lumps[kVertices].offset;
            for (size_t i = 0; i < cnt; ++i) {
                memcpy(&m_pVerts[i],
                    ptr + i * sizeof(tBSPVertex),
                    sizeof(tBSPVertex));
                float t = m_pVerts[i].vPosition.y;
                m_pVerts[i].vPosition.y = m_pVerts[i].vPosition.z;
                m_pVerts[i].vPosition.z = -t;
                t = m_pVerts[i].vNormal.y;
                m_pVerts[i].vNormal.y = m_pVerts[i].vNormal.z;
                m_pVerts[i].vNormal.z = -t;
            }
        }
    }

    // 17) Indices, Faces, Textures, Lightmaps
    {
        size_t cntIdx = 0;
        if (checkLump(kIndices, sizeof(int), cntIdx)) {
            m_numOfIndices = static_cast<int>(cntIdx);
            m_pIndices = new int[cntIdx];
            memcpy(m_pIndices,
                base + lumps[kIndices].offset,
                cntIdx * sizeof(int));
        }

        size_t cntFaces = 0;
        if (checkLump(kFaces, sizeof(tBSPFace), cntFaces)) {
            m_numOfFaces = static_cast<int>(cntFaces);
            m_pFaces = new tBSPFace[cntFaces];
            memcpy(m_pFaces,
                base + lumps[kFaces].offset,
                cntFaces * sizeof(tBSPFace));
        }

        size_t cntTex = 0;
        if (checkLump(kTextures, sizeof(tBSPTexture), cntTex)) {
            m_numOfTextures = static_cast<int>(cntTex);
            pTextures = new tBSPTexture[cntTex];
            memcpy(pTextures,
                base + lumps[kTextures].offset,
                cntTex * sizeof(tBSPTexture));
            for (int i = 0; i < m_numOfTextures; ++i) {
                strcpy_s(tname[i], pTextures[i].strName);
                strcat_s(tname[i], ".jpg");
            }
        }

        size_t cntLM = 0;
        if (checkLump(kLightmaps, sizeof(tBSPLightmap), cntLM)) {
            m_numOfLightmaps = static_cast<int>(cntLM);
            pLightmaps = new tBSPLightmap[cntLM];
            memcpy(pLightmaps,
                base + lumps[kLightmaps].offset,
                cntLM * sizeof(tBSPLightmap));
            for (int i = 0; i < m_numOfLightmaps; ++i) {
                Rbuffers.G_lightMaps.push_back(pLightmaps[i]);
            }
        }
    }

    printf("BSP loaded successfully: %s\n", filename);
    return true;
}



// int skipindices = 0;
void CQuake3BSP::BuildVBO() {
    for (int index = 0; index < m_numOfFaces; index++) {
        tBSPFace* pFace = &m_pFaces[index];

        if (pFace->type == FACE_POLYGON)
            skipindices += pFace->numOfIndices;

        CreateVBO(index);
        CreateIndices(index);
        CreateRenderBuffers(index);
        // BSPDebug(index);
        numVisibleFaces++;
    }
}

void CQuake3BSP::CreateVBO(int index) {
    tBSPFace* pFace = &m_pFaces[index];
    auto& vertices = Rbuffers.v_faceVBOs[index]; // Now a vector<VertexData>

    for (int v = 0; v < pFace->numOfVerts; v++) {
        const tBSPVertex& bspVert = m_pVerts[pFace->startVertIndex + v];
        VertexData vd;

        vd.Position = bspVert.vPosition;
        vd.Normal = bspVert.vNormal;
        vd.TextureCoordinate = bspVert.vTextureCoord;
        vd.ShadowMapCoords = bspVert.vLightmapCoord; // Using ShadowMapCoords for lightmap

        // Convert color from byte[4] to vec4
        vd.Color = glm::vec4(
            (float)bspVert.color[0] / 255.0f,
            (float)bspVert.color[1] / 255.0f,
            (float)bspVert.color[2] / 255.0f,
            (float)bspVert.color[3] / 255.0f
        );

        vertices.push_back(vd);
        
    }

    auto bounds = BoundingBox::FromVertices(vertices);
    bounds.Min -= vec3(0.1f);
    bounds.Max += vec3(0.1f);

    faceBounds.push_back(bounds);

}

void CQuake3BSP::CreateIndices(int index) {
    tBSPFace* pFace = &m_pFaces[index];
    int start = pFace->startIndex;
    int count = pFace->numOfIndices;
    auto& out = Rbuffers.v_faceIDXs[index];

    // reserve once to avoid repeated reallocations
    out.reserve(count);

    // process each triangle (3 indices) and swap the last two
    for (int j = 0; j < count; j += 3) {
        unsigned short i0 = m_pIndices[start + j + 0];
        unsigned short i1 = m_pIndices[start + j + 1];
        unsigned short i2 = m_pIndices[start + j + 2];
        // Original (CCW): i0, i1, i2
        // Flipped (CW):    i0, i2, i1
        out.push_back(i0);
        out.push_back(i2);
        out.push_back(i1);
    }
}

void CQuake3BSP::PreloadFace(int index)
{
    // bind your textures as before
    tBSPFace* pFace = &m_pFaces[index];

    string textureName = string(pTextures[pFace->textureID].strName);

    int nameL = textureName.length();

    bool isCube = false;

    if (nameL > 5)
    {
        isCube =
            (textureName[nameL - 1] == 'e') &&
            (textureName[nameL - 2] == 'b') &&
            (textureName[nameL - 3] == 'u') &&
            (textureName[nameL - 4] == 'c');
    }

    ShaderProgram* shader = ShaderManager::GetShaderProgram("bsp", isCube ? "bsp_cube" : "bsp");
    shader->UseProgram();

    string texturePath = "GameData/" + textureName + ".png";

    if (isCube)
    {

        auto splitPath = StringHelper::Split(texturePath, '/');

        string fileName = splitPath[splitPath.size() - 1];

        texturePath = "GameData/env/" + fileName;
        texturePath = texturePath;

    }

    int faceTexture;

    if (isCube)
    {
        faceTexture = AssetRegistry::GetTextureCubeFromFile(texturePath)->getID();
    }
    else
    {
        faceTexture = AssetRegistry::GetTextureFromFile(texturePath)->getID();
    }

    GLuint lightmapId = (pFace->lightmapID >= 0)
        ? m_lightmap_gen_IDs[pFace->lightmapID]
        : missing_LM_id;

    if (m_numOfLightmaps == 0 && isCube == false)
    {

        string lightMapPath = GetLightMapFilePathFromId(pFace->lightmapID, filePath);

        lightmapId = AssetRegistry::GetTextureFromFile(lightMapPath)->getID();

        if (lightmapId == 0)
        {
            lightmapId = missing_LM_id;
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

    for (size_t i = 0; i < m_numOfFaces; i++)
    {
        PreloadFace(i);
    }


}

void CQuake3BSP::BuildMergedModels()
{

    uint32 mId = -1;

    unordered_map<string, vector<int>> facesMap;

    for (const auto& model : models)
    {
        mId++;

        string modelId = to_string(mId);

        for (int i = model.face; i < model.face + model.n_faces; i++)
        {

            
            string texId = to_string(m_pFaces[i].textureID);
            string lightmapId = to_string(m_pFaces[i].lightmapID);

            string finalString = modelId + "|" + texId + "|" + lightmapId;

            auto searchRes = facesMap.find(finalString);

            if (searchRes == facesMap.end())
            {
                facesMap[finalString] = vector<int>();
            }

            facesMap[finalString].push_back(i);


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
        data.ibo = new IndexBuffer(mergedMesh.indices);
        data.vbo = new VertexBuffer(mergedMesh.vertices, VertexData::Declaration());
        data.vao = new VertexArrayObject(*data.vbo, *data.ibo);
        data.referenceFace = keyPair.second[0];
        data.uId = mergedFacesData.size();

        data.bounds = BoundingBox::FromVertices(mergedMesh.vertices).Transform(glm::scale(vec3(1.0f / MAP_SCALE)));

        mergedFacesData.push_back(data);

        for (int i : keyPair.second)
        {
            mergedFacesMapping[i] = data.uId;
        }


    }

}

glm::vec3 computeLightDirection(const unsigned char vol_dir[2]) {
    // Quake 3 encodes pitch (elevation) from 0 (up) to 255 (down)
    float pitch = glm::radians((static_cast<float>(vol_dir[0]) / 255.0f) * 180.0f);
    float yaw = glm::radians((static_cast<float>(vol_dir[1]) / 255.0f) * 360.0f);

    // Convert from spherical coordinates to Cartesian
    float x = sinf(pitch) * cosf(yaw);
    float y = sinf(pitch) * sinf(yaw);
    float z = cosf(pitch);

    // Quake uses Z-up, your engine uses Y-up: map (x, y, z) → (x, z, -y)
    glm::vec3 quake_vec(x, y, z);
    glm::vec3 engine_vec = glm::vec3(quake_vec.x, quake_vec.z, -quake_vec.y);

    return glm::normalize(engine_vec);
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

	DebugDraw::Line(position, volPosition, 0.01f, 0.01f);

    return Physics::LineTrace(position, volPosition, BodyType::WorldOpaque | BodyType::WorldSkybox).hasHit == false;

}

LightVolPointData CQuake3BSP::GetLightvolColorPoint(const glm::vec3& position, bool wallCheck) 
{
    if (lightVols.size() == 0)
    {
        return { vec3(0), vec3(0.3f), vec3(0,-1,0) };
    }

    // Transform position from Y-up (engine) to Z-up (Quake 3)
    glm::vec3 pos_quake(position.x, -position.z, position.y);

    // Use original bounds in Z-up
    glm::vec3 modelMins = originalMins;
    glm::vec3 modelMaxs = originalMaxs;

    // Calculate grid dimensions according to Quake 3 specs (Z-up)
    glm::ivec3 lightVolGridDims(
        static_cast<int>(std::floor(modelMaxs.x / lightVolGridSize.x) - std::ceil(modelMins.x / lightVolGridSize.x) + 1),
        static_cast<int>(std::floor(modelMaxs.y / lightVolGridSize.y) - std::ceil(modelMins.y / lightVolGridSize.y) + 1),
        static_cast<int>(std::floor(modelMaxs.z / lightVolGridSize.z) - std::ceil(modelMins.z / lightVolGridSize.z) + 1)
    );

    // Calculate grid indices and fractional offsets
    float fx = (pos_quake.x - modelMins.x) / lightVolGridSize.x;
    float fy = (pos_quake.y - modelMins.y) / lightVolGridSize.y;
    float fz = (pos_quake.z - modelMins.z) / lightVolGridSize.z;

    // Get integer indices for the lower corner
    int nx = static_cast<int>(std::floor(fx));
    int ny = static_cast<int>(std::floor(fy));
    int nz = static_cast<int>(std::floor(fz));

    // Get fractional components for interpolation
    fx = fx - nx; // Range [0,1]
    fy = fy - ny;
    fz = fz - nz;

    // Clamp indices to prevent out-of-bounds access
    int nx0 = glm::clamp(nx, 0, lightVolGridDims.x - 1);
    int ny0 = glm::clamp(ny, 0, lightVolGridDims.y - 1);
    int nz0 = glm::clamp(nz, 0, lightVolGridDims.z - 1);
    int nx1 = glm::clamp(nx + 1, 0, lightVolGridDims.x - 1);
    int ny1 = glm::clamp(ny + 1, 0, lightVolGridDims.y - 1);
    int nz1 = glm::clamp(nz + 1, 0, lightVolGridDims.z - 1);

    // Function to get grid position in engine space
    auto getGridEnginePos = [&](int x, int y, int z) -> glm::vec3 {
        glm::vec3 quake = modelMins + glm::vec3(x, y, z) * lightVolGridSize;
        return glm::vec3(quake.x, quake.z, -quake.y);
        };

    // Fetch light volumes at the eight corners
    auto getLightVolData = [&](int x, int y, int z) -> std::tuple<glm::vec3, glm::vec3, glm::vec3> {
        int index = z * (lightVolGridDims.x * lightVolGridDims.y) + y * lightVolGridDims.x + x;
        const tBSPLightvol& vol = lightVols[index];
        glm::vec3 ambient(
            static_cast<float>(vol.ambient[0]) / 255.0f,
            static_cast<float>(vol.ambient[1]) / 255.0f,
            static_cast<float>(vol.ambient[2]) / 255.0f
        );
        glm::vec3 directional(
            static_cast<float>(vol.directional[0]) / 255.0f,
            static_cast<float>(vol.directional[1]) / 255.0f,
            static_cast<float>(vol.directional[2]) / 255.0f
        );
        glm::vec3 dir_engine = computeLightDirection(vol.dir);
        return { ambient, directional, dir_engine };
        };

    auto [amb000, dir000, vec000] = getLightVolData(nx0, ny0, nz0);
    auto [amb100, dir100, vec100] = getLightVolData(nx1, ny0, nz0);
    auto [amb010, dir010, vec010] = getLightVolData(nx0, ny1, nz0);
    auto [amb110, dir110, vec110] = getLightVolData(nx1, ny1, nz0);
    auto [amb001, dir001, vec001] = getLightVolData(nx0, ny0, nz1);
    auto [amb101, dir101, vec101] = getLightVolData(nx1, ny0, nz1);
    auto [amb011, dir011, vec011] = getLightVolData(nx0, ny1, nz1);
    auto [amb111, dir111, vec111] = getLightVolData(nx1, ny1, nz1);

    // Validity checks (scale to world units) for the 8 corners
    bool valid000 = true;
    bool valid100 = true;
    bool valid010 = true;
    bool valid110 = true;
    bool valid001 = true;
    bool valid101 = true;
    bool valid011 = true;
    bool valid111 = true;

    if (wallCheck)
    {
        valid000 = CheckLightProbeAcess(position / MAP_SCALE, getGridEnginePos(nx0, ny0, nz0) / MAP_SCALE);
        valid010 = CheckLightProbeAcess(position / MAP_SCALE, getGridEnginePos(nx0, ny1, nz0) / MAP_SCALE);
        valid100 = CheckLightProbeAcess(position / MAP_SCALE, getGridEnginePos(nx1, ny0, nz0) / MAP_SCALE);
        valid110 = CheckLightProbeAcess(position / MAP_SCALE, getGridEnginePos(nx1, ny1, nz0) / MAP_SCALE);
        valid001 = CheckLightProbeAcess(position / MAP_SCALE, getGridEnginePos(nx0, ny0, nz1) / MAP_SCALE);
        valid101 = CheckLightProbeAcess(position / MAP_SCALE, getGridEnginePos(nx1, ny0, nz1) / MAP_SCALE);
        valid011 = CheckLightProbeAcess(position / MAP_SCALE, getGridEnginePos(nx0, ny1, nz1) / MAP_SCALE);
        valid111 = CheckLightProbeAcess(position / MAP_SCALE, getGridEnginePos(nx1, ny1, nz1) / MAP_SCALE);
    }

    // Define data and weights
    struct LightData { glm::vec3 amb, dir_color, dir_vec; };
    LightData datas[8] = {
        {amb000, dir000, vec000},
        {amb100, dir100, vec100},
        {amb010, dir010, vec010},
        {amb110, dir110, vec110},
        {amb001, dir001, vec001},
        {amb101, dir101, vec101},
        {amb011, dir011, vec011},
        {amb111, dir111, vec111}
    };
    bool valids[8] = { valid000, valid100, valid010, valid110, valid001, valid101, valid011, valid111 };
    float weights[8] = {
        (1 - fx) * (1 - fy) * (1 - fz),
        fx * (1 - fy) * (1 - fz),
        (1 - fx) * fy * (1 - fz),
        fx * fy * (1 - fz),
        (1 - fx) * (1 - fy) * fz,
        fx * (1 - fy) * fz,
        (1 - fx) * fy * fz,
        fx * fy * fz
    };

    // Weighted sum for valid samples
    glm::vec3 sum_amb(0), sum_dir_color(0), sum_dir_vec(0);
    float total_weight = 0.0f;
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
    else
    {
        // --- Fallback search: find nearest sample within a radius (engine units) ---
        const float SEARCH_RADIUS = 128; // engine units; change to fit your scale
        const float SEARCH_RADIUS_SQR = SEARCH_RADIUS * SEARCH_RADIUS;

        // Convert radius to number of grid steps in each axis (safe integer radius)
        int rx = static_cast<int>(std::ceil(SEARCH_RADIUS / lightVolGridSize.x));
        int ry = static_cast<int>(std::ceil(SEARCH_RADIUS / lightVolGridSize.y));
        int rz = static_cast<int>(std::ceil(SEARCH_RADIUS / lightVolGridSize.z));

        // Use clamped center index as search center
        int cx = glm::clamp(nx, 0, lightVolGridDims.x - 1);
        int cy = glm::clamp(ny, 0, lightVolGridDims.y - 1);
        int cz = glm::clamp(nz, 0, lightVolGridDims.z - 1);

        int bestIdx = -1;
        float bestDist2 = std::numeric_limits<float>::max();
        LightData bestData{ glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f) };

        // iterate within bounding box around center, clamped to grid extents
        int x0 = glm::max(0, cx - rx), x1 = glm::min(lightVolGridDims.x - 1, cx + rx);
        int y0 = glm::max(0, cy - ry), y1 = glm::min(lightVolGridDims.y - 1, cy + ry);
        int z0 = glm::max(0, cz - rz), z1 = glm::min(lightVolGridDims.z - 1, cz + rz);

        for (int z = z0; z <= z1; ++z) {
            for (int y = y0; y <= y1; ++y) {
                for (int x = x0; x <= x1; ++x) {
                    glm::vec3 engPos = getGridEnginePos(x, y, z);
                    float d2 = glm::distance2(position, engPos);
                    if (d2 > SEARCH_RADIUS_SQR) continue; // outside radius

                    // Respect wallCheck: if requested, require cluster check to pass
                    if (wallCheck) {
                        if (FindClusterAtPosition(engPos / MAP_SCALE) < 0) continue;
                    }

                    // Get sample data
                    int index = z * (lightVolGridDims.x * lightVolGridDims.y) + y * lightVolGridDims.x + x;
                    const tBSPLightvol& vol = lightVols[index];
                    LightData ld{
                        glm::vec3(static_cast<float>(vol.ambient[0]) / 255.0f,
                                  static_cast<float>(vol.ambient[1]) / 255.0f,
                                  static_cast<float>(vol.ambient[2]) / 255.0f),
                        glm::vec3(static_cast<float>(vol.directional[0]) / 255.0f,
                                  static_cast<float>(vol.directional[1]) / 255.0f,
                                  static_cast<float>(vol.directional[2]) / 255.0f),
                        computeLightDirection(vol.dir)
                    };

                    if (d2 < bestDist2) {
                        bestDist2 = d2;
                        bestData = ld;
                        bestIdx = index;
                    }
                }
            }
        }

        if (bestIdx >= 0) {
            ambient = bestData.amb;
            directional = bestData.dir_color;
            if (glm::length2(bestData.dir_vec) > 1e-6f)
                dir_engine = glm::normalize(bestData.dir_vec);
            else
                dir_engine = glm::vec3(0, -1, 0);
        }
        else {
            // Final fallback when nothing found in radius
            ambient = vec3(0.3f);
            directional = vec3(0);
            dir_engine = vec3(0, -1, 0);
        }
    }

    return LightVolPointData{ directional, ambient, dir_engine };
}



LightVolPointData CQuake3BSP::GetLightvolColor(const glm::vec3& position, bool wallCheck)
{

    if (lightVols.size() == 0)
    {
        return { vec3(0), vec3(0.3f), vec3(0,-1,0) };
    }

    auto data = GetLightvolColorPoint(position);
    auto centerData = data;
    float radius = lightVolGridSize.x;
    data += GetLightvolColorPoint(position + vec3(radius,0,0), wallCheck);
    data += GetLightvolColorPoint(position + vec3(-radius, 0, 0), wallCheck);
    data += GetLightvolColorPoint(position + vec3(0, 0, radius), wallCheck);
    data += GetLightvolColorPoint(position + vec3(0, 0, -radius), wallCheck);

    data /= 5.0f;

    centerData.direction = data.direction;

    centerData.direction = normalize(centerData.direction);

    return centerData;
}


int CQuake3BSP::FindClusterAtPosition(glm::vec3 cameraPos) 
{
    cameraPos *= MAP_SCALE;

    int nodeIndex = 0;
    int depth = 0;
    while (nodeIndex >= 0 && depth < 200) {
		const tBSPNode& node = nodes[nodeIndex];

		const vec3 min = vec3(node.mins[0], node.mins[1], node.mins[2]) / MAP_SCALE;
		const vec3 max = vec3(node.maxs[0], node.maxs[1], node.maxs[2]) / MAP_SCALE;

        const tBSPPlane& plane = planes[node.plane];
        float distance = glm::dot(cameraPos, plane.normal) - plane.dist;

        int childIndex = distance >= 0 ? 0 : 1;
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

    if (sourceCluster < 0)
    {
        return true;
    }

    if (sourceCluster < 0 || testCluster < 0) return true;

    int byteIndex = (sourceCluster * visData.sz_vecs) + (testCluster / 8);
    int bitIndex = testCluster % 8;

    if (visData.n_vecs == 0)
        return true;

    // Convert std::byte to unsigned integer for bitwise operations
    unsigned char byteValue = (visData.vecs[byteIndex]);
    return (byteValue & (1 << bitIndex)) != 0;
}

void CQuake3BSP::DrawForward(mat4x4 view, mat4x4 projection)
{
    
    bool first = true;
    for (auto& model : models)
    {

        vec3 min = vec3(model.mins[0], model.mins[1], model.mins[2]) / MAP_SCALE;
        vec3 max = vec3(model.maxs[0], model.maxs[1], model.maxs[2]) / MAP_SCALE;

        if (Camera::frustum.IsBoxVisible(min, max))
        {
			RenderBSP(Camera::finalizedPosition, model, mat4(1.0f / MAP_SCALE), first, first);
        }


        first = false;
    }

}

void CQuake3BSP::RenderBSP(const glm::vec3& cameraPos, tBSPModel& model, mat4 modelMatrix, bool useClusterVis, bool lightmap)
{

    
    //auto light = GetLightvolColor(Camera::finalizedPosition * MAP_SCALE);
    //printf("light : %f, %f, %f \n", light.ambientColor.x, light.ambientColor.y, light.ambientColor.z);

    //DebugDraw::Line(Camera::finalizedPosition + Camera::Forward(), Camera::finalizedPosition + Camera::Forward() + light.direction, 0.01f);


    // 1. Find camera's cluster via BSP tree traversal
    int cameraCluster = FindClusterAtPosition(cameraPos);

    int drawnFaces = 0;


	LightVolPointData lightData = { vec3(0),vec3(1) ,vec3(0) };

    vector<int> facesToDraw;
    
    glDepthMask(GL_TRUE);

    if (lightmap == false)
    {

        vec3 min = vec3(model.mins[0], model.mins[1], model.mins[2]);
        vec3 max = vec3(model.maxs[0], model.maxs[1], model.maxs[2]);

		lightData = GetLightvolColorPoint((min + max) / 2.0f);

        //lightData.ambientColor = vec3(1);

        //DebugDraw::Line((min + max) / 2.0f / MAP_SCALE + vec3(1, 0, 0), (min + max) / 2.0f / MAP_SCALE + vec3(1, 0, 0) + lightData.direction, 0.01f);

    }

    std::vector<bool> renderedFaces(m_numOfFaces);

    if (useClusterVis)
    {
        // Initialize a boolean array to track rendered faces for this model


        // 2. Iterate through all leaves
        for (const tBSPLeaf& leaf : leafs) {
            if (leaf.cluster < 0)
                continue; // Skip invalid clusters

            // 3. Check visibility using visdata
            if (!IsClusterVisible(cameraCluster, leaf.cluster))
                continue;

            // 4. Render visible leaf's faces
            for (int i = 0; i < leaf.n_leaffaces; i++)
            {
                int faceIndex = leafFaces[leaf.leafface + i];

                if (model.face <= faceIndex && faceIndex < model.face + model.n_faces)
                {

                    int mergedIndex = mergedFacesMapping[faceIndex];

                    // Compute the local index within the model's face range
                    int localIndex = mergedIndex;

                    // Only render if the face hasn’t been rendered yet
                    if (!renderedFaces[localIndex])
                    {
                        renderedFaces[localIndex] = true; // Mark as rendered

                        FaceRenderData renderData;
                        renderData.faceIndex = mergedIndex;
                        renderData.useLightmap = lightmap;
                        renderData.lightPointData = lightData;
                        renderData.modelMatrix = modelMatrix;

                        if (IsFaceTransparent(mergedFacesData[mergedIndex].referenceFace))
                        {
                            facesToDrawTransparent.push_back(renderData);
                        }
                        else
                        {

                            bool drawn = RenderMergedFace(mergedIndex, lightmap, lightData, modelMatrix);
                            if (drawn)
                            {
                                drawnFaces++;
                            }
                        }

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
                renderedFaces[mergedIndex] = true; // Mark as rendered

                FaceRenderData renderData;
                renderData.faceIndex = mergedIndex;
                renderData.useLightmap = lightmap;
                renderData.lightPointData = lightData;
                renderData.modelMatrix = modelMatrix;

                if (IsFaceTransparent(mergedFacesData[renderData.faceIndex].referenceFace))
                {
                    facesToDrawTransparent.push_back(renderData);
                }
                else
                {
                    bool drawn = RenderMergedFace(renderData.faceIndex, lightmap, lightData, modelMatrix);
                    if (drawn)
                    {
                        drawnFaces++;
                    }
                }
            }

        }
    }

    //printf("drawn %i faces\n", drawnFaces);

}

void CQuake3BSP::RenderTransparentFaces()
{

    glDepthMask(GL_FALSE);

    for (auto& face : facesToDrawTransparent)
    {
        bool drawn = RenderMergedFace(face.faceIndex, face.useLightmap, face.lightPointData, face.modelMatrix);
    }

    facesToDrawTransparent.clear();
    facesToDrawTransparent.reserve(100);
}

bool CQuake3BSP::IsFaceTransparent(int index)
{

    return cachedFaces[index].transparent;

    tBSPFace* pFace = &m_pFaces[index];


}

vector<BSPModelRef> CQuake3BSP::GetAllModelRefs()
{
    vector<BSPModelRef> refs;
    bool first = true;
    for (size_t i = 0; i < models.size(); ++i)
    {
        BSPModelRef ref(this, static_cast<int>(i), models[i]);
        ref.useBspVisibility = first;
        refs.push_back(ref);             // Add the reference to the vector
        first = false;
    }
    return refs;
}

void CQuake3BSP::BuildStaticOpaqueObstacles()
{

    opaqueVBOs.resize(models.size());

    for (int i = 0; i < models.size(); i++)
    {

        BSPModelRef ref(this, static_cast<int>(i), models[i]);

        auto vertices = ref.GetVertices(false, true);
        auto indices = ref.GetIndices(false, true);

        OpaqueModelVBO modelVBO;
        modelVBO.vbo = new VertexBuffer(vertices, VertexData::Declaration());
        modelVBO.ibo = new IndexBuffer(indices);
        modelVBO.vao = new VertexArrayObject(*modelVBO.vbo, *modelVBO.ibo);

        opaqueVBOs[i] = modelVBO;

    }
}

void AddPhysicsBodyForEntityAndModel(Entity* entity, BSPModelRef& model)
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

    Body* body = Physics::CreateBodyFromShape(entity, vec3(0), finalShape,10, entity->Static ? JPH::EMotionType::Static : JPH::EMotionType::Kinematic,entity->DefaultBrushGroup, entity->DefaultBrushCollisionMask);

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

        if (ent == nullptr)
            ent = new Entity();

        ent->FromData(entityData);

        string modelStr = entityData.GetPropertyString("model");

        if (modelStr.size() > 1)
        {
            if (modelStr[0] == '*')
            {

                int modelId = stoi(modelStr.substr(1, modelStr.size() - 1));

                BSPModelRef modelRef = models[modelId];

                AddPhysicsBodyForEntityAndModel(ent, modelRef);

                modelRef.CalculateAveragePosition();

                ent->Drawables.push_back(new BSPModelRef(modelRef));

            }
		}
		else if (ent->ClassName == "worldspawn") //worldspawn always has 0 model
		{
			BSPModelRef modelRef = models[0];

            AddPhysicsBodyForEntityAndModel(ent, modelRef);

            modelRef.CalculateAveragePosition();

            ent->Drawables.push_back(new BSPModelRef(modelRef));

		}

        Level::Current->AddEntity(ent);

	}

}

void CQuake3BSP::BSPDebug(int index) {
    printf("\n");
    printf("Face:----> %d\n", index);

    //for (unsigned int x = 0; x < Rbuffers.v_faceVBOs[index].size(); x++)
        //printf(">- %f\n", Rbuffers.v_faceVBOs[index][x]);

    //for (unsigned int x = 0; x < Rbuffers.v_faceIDXs[index].size(); x++)
        //printf("%d-\n", Rbuffers.v_faceIDXs[index][x]);

    //printf("VBuffer size=%lu(bytes)\n", sizeof(GLfloat) * Rbuffers.v_faceVBOs[index].size());
    //printf("IBuffer size=%lu(bytes)\n", sizeof(GLuint) * Rbuffers.v_faceIDXs[index].size());
    printf("EndFace.\n");
}

void CQuake3BSP::CreateRenderBuffers(int index) {
    auto& vertices = Rbuffers.v_faceVBOs[index];
    auto& indices = Rbuffers.v_faceIDXs[index];

    // Create buffers
    FB_array.FB_Idx[index].VBO = std::make_unique<VertexBuffer>(
        vertices, VertexData::Declaration()
    );
    FB_array.FB_Idx[index].EBO = std::make_unique<IndexBuffer>(indices);

    // Create VAO linking them
    FB_array.FB_Idx[index].VAO = std::make_unique<VertexArrayObject>(
        *FB_array.FB_Idx[index].VBO,
        *FB_array.FB_Idx[index].EBO
    );
}

std::string CQuake3BSP::GetLightMapFilePathFromId(int id, const std::string& filePath) {
    const size_t len = filePath.size();
    // filePath must end with ".bsp"
    assert(len > 4 && filePath.compare(len - 4, 4, ".bsp") == 0);

    // Find last '/' or '\' in one pass
    const char* str = filePath.data();
    const char* end = str + len;
    const char* p = end;
    while (p > str && *(p - 1) != '/' && *(p - 1) != '\\') {
        --p;
    }
    size_t prefixLen = p - str;             // includes the slash
    size_t mapNameLen = len - prefixLen - 4; // minus ".bsp"

    // Reserve exact capacity to avoid reallocations
    // prefixLen + mapNameLen + 1 (slash) + 3 ("lm_") + 4 (digits) + 4 (".tga")
    std::string result;
    result.reserve(prefixLen + mapNameLen + 1 + 3 + 4 + 4);

    // Append folder path and map name
    result.append(str, prefixLen);
    result.append(p, mapNameLen);
    result.push_back('/');

    // Append "lm_" + zero-padded 4-digit id + ".tga"
    result += "lm_";

    char digits[4];
    int tmp = id;
    for (int i = 3; i >= 0; --i) {
        digits[i] = char('0' + (tmp % 10));
        tmp /= 10;
    }
    result.append(digits, 4);
    result += ".tga";

    return result;
}
bool CQuake3BSP::RenderSingleFace(int index , bool lightmap, LightVolPointData lightData, mat4 model)
{
    auto bounds = faceBounds[index];
    bounds = bounds.Transform(model);

    if (Camera::frustum.IsBoxVisible(bounds.Min, bounds.Max) == false) 
        return false;

    // bind the face's VAO (which has its VBO/EBO & attribs)
    auto& buffers = FB_array.FB_Idx[index];

    buffers.VAO->Bind();


    const CachedFaceTextureData& data = cachedFaces[index];

    bool isCube = data.isCube;
    int faceTexture = data.textureId;
    GLuint lightmapId = data.lightmapId;

    ShaderProgram* shader = ShaderManager::GetShaderProgram("bsp", isCube? "bsp_cube" : "bsp");
    shader->UseProgram();



    
    
    if (faceTexture == 0) return false;



    shader->SetUniform("light_color", lightData.ambientColor);
    shader->SetUniform("direct_light_color", lightData.directColor);
    shader->SetUniform("direct_light_dir", lightData.direction);

    if (isCube)
    {
        shader->SetCubemapTexture("s_bspTexture", faceTexture);
    }
    else
    {
        shader->SetTexture("s_bspTexture", faceTexture);
    }
    

    shader->SetTexture("s_bspLightmap", lightmapId);
    shader->SetUniform("view", Camera::finalizedView);
    shader->SetUniform("projection", Camera::finalizedProjection);
    shader->SetUniform("model", model);

    EngineMain::MainInstance->MainRenderer->SetSurfaceShaderUniforms(shader);

    


    // draw using the EBO already bound in the VAO; offset = 0
    glDrawElements(GL_TRIANGLES,
        data.numOfIndices,
        GL_UNSIGNED_INT,
        0);


    VertexArrayObject::Unbind();

    return true;
}

bool CQuake3BSP::RenderMergedFace(int mergedIndex, bool lightmap, LightVolPointData lightData, mat4 model)
{

    const auto& mergedFace = mergedFacesData[mergedIndex];


    auto bounds = mergedFace.bounds;

    bounds = bounds.Transform(model * scale(vec3(1.0f)*MAP_SCALE));

	if (Camera::frustum.IsBoxVisible(bounds.Min, bounds.Max) == false)
	    return false;


    // bind the face's VAO (which has its VBO/EBO & attribs)
    mergedFace.vao->Bind();


    const CachedFaceTextureData& data = cachedFaces[mergedFace.referenceFace];

    bool isCube = data.isCube;
    int faceTexture = data.textureId;
    GLuint lightmapId = data.lightmapId;
    if (lightmap == false)
    {
        lightmapId = white_LM_id;
    }

    ShaderProgram* shader = ShaderManager::GetShaderProgram("bsp", isCube ? "bsp_cube" : "bsp");
    shader->UseProgram();


    if (faceTexture == 0) return false;



    shader->SetUniform("useVertexLight", data.lightmapId == missing_LM_id);

    shader->SetUniform("light_color", lightData.ambientColor);
    shader->SetUniform("direct_light_color", lightData.directColor);
    shader->SetUniform("direct_light_dir", lightData.direction);

    if (isCube)
    {
        shader->SetCubemapTexture("s_bspTexture", faceTexture);
    }
    else
    {
        shader->SetTexture("s_bspTexture", faceTexture);
    }


    shader->SetTexture("s_bspLightmap", lightmapId);
    shader->SetUniform("view", Camera::finalizedView);
    shader->SetUniform("projection", Camera::finalizedProjection);
    shader->SetUniform("model", model);

    EngineMain::MainInstance->MainRenderer->SetSurfaceShaderUniforms(shader);

    LightManager::ApplyPointLightToShader(shader, bounds.Min, bounds.Max);


    // draw using the EBO already bound in the VAO; offset = 0
    glDrawElements(GL_TRIANGLES,
        mergedFace.vao->IndexCount,
        GL_UNSIGNED_INT,
        0);


    VertexArrayObject::Unbind();

    return true;
}

void CQuake3BSP::GenerateTexture() 
{
   

} // end

void CQuake3BSP::GenerateLightmap() {
    // GLfloat aniso = 8.0f;


    float b = 1.0f;

    const int missingLmColor = 200;

    // generate missing lightmap
    GLubyte missing_lightmap[] = {
        missingLmColor,missingLmColor,missingLmColor,missingLmColor,   // pixel #1 (RGBA)
        missingLmColor,missingLmColor,missingLmColor,missingLmColor,   // pixel #2
        missingLmColor,missingLmColor,missingLmColor,missingLmColor,   // pixel #3
        missingLmColor,missingLmColor,missingLmColor,missingLmColor    // pixel #4
    };

    glGenTextures(1, &missing_LM_id);
    glBindTexture(GL_TEXTURE_2D, missing_LM_id);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,             // internalFormat
        2, 2,
        0,
        GL_RGBA,             // format
        GL_UNSIGNED_BYTE,    // type
        missing_lightmap       // data pointer
    );
    //glGenerateMipmap(GL_TEXTURE_2D);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // generate missing lightmap
    GLubyte white_lightmap[] = {
        255,255,255,255,   // pixel #1 (RGBA)
        255,255,255,255,   // pixel #2
        255,255,255,255,   // pixel #3
        255,255,255,255    // pixel #4
    };

    glGenTextures(1, &white_LM_id);
    glBindTexture(GL_TEXTURE_2D, white_LM_id);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,             // internalFormat
        2, 2,
        0,
        GL_RGBA,             // format
        GL_UNSIGNED_BYTE,    // type
        white_lightmap       // data pointer
    );
    //glGenerateMipmap(GL_TEXTURE_2D);

    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);




    // generate lightmaps
    m_lightmap_gen_IDs = new GLuint[m_numOfLightmaps];
    glGenTextures(Rbuffers.G_lightMaps.size(), m_lightmap_gen_IDs);

    for (GLuint i = 0; i < Rbuffers.G_lightMaps.size(); i++) {
        glBindTexture(GL_TEXTURE_2D, m_lightmap_gen_IDs[i]);
        // glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &aniso);
        // glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, aniso);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128, 128, 0, GL_RGB, GL_UNSIGNED_BYTE, &Rbuffers.G_lightMaps[i].imageBits);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

}

void CQuake3BSP::renderFaces() {
    for (auto& f : Rbuffers.v_faceVBOs)
        RenderSingleFace(f.first, true, LightVolPointData(), scale(vec3(1.0f) / MAP_SCALE));
}

BoundingBox BSPModelRef::GetTransformedBounds()
{
    return bounds.Transform(GetWorldMatrix() * scale(vec3(1.0f, 1.0f, 1.0f) * MAP_SCALE));
}

mat4 BSPModelRef::GetWorldMatrix()
{
	return translate(Position) * MathHelper::GetRotationMatrix(Rotation) * scale(Scale/MAP_SCALE);
}

BSPModelRef::BSPModelRef(CQuake3BSP* bsp_ptr, int model_id, tBSPModel& model_ref) 
    : bsp(bsp_ptr), id(model_id), model(model_ref)
{


    auto vertices = GetVertices();

    vector<vec3> points;
    points.reserve(vertices.size());
    for (const auto& v : vertices)
    {
        points.push_back(v.Position/MAP_SCALE);
    }

    bounds = BoundingBox::FromPoints(points);

}

BSPModelRef::~BSPModelRef()
{



}

void BSPModelRef::BuildVisBlocker()
{
}


float BSPModelRef::GetDistanceToCamera()
{
    return distance(avgPosition, Camera::finalizedPosition);
}

void BSPModelRef::CalculateAveragePosition()
{

    avgPosition = GetTransformedBounds().Center();
    /*
    auto vertices = GetVertices();

    avgPosition = vec3(0);

    for (auto& vertex : vertices)
    {
        avgPosition += vertex.Position / MAP_SCALE;
    }

    avgPosition /= (float)vertices.size();
    */
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

    if (CheckPointBspVisible(sourceC, b.Center()))
        return true;

    if (CheckPointBspVisible(sourceC, b.Max))
        return true;

    if (CheckPointBspVisible(sourceC, b.Min))
        return true;

    if (CheckPointBspVisible(sourceC, mix(b.Center(), b.Min,0.5)))
        return true;

    if (CheckPointBspVisible(sourceC, mix(b.Center(), b.Min, 0.5)))
        return true;

    //DebugDraw::Line(Camera::position - vec3(0,1,0), b.Center(),0.01f);

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
    {
        faces.push_back(bsp->m_pFaces[i]);
    }

    return faces;
}

vector<MeshUtils::PositionVerticesIndices> BSPModelRef::GetNavObstacleMeshes()
{
    vector<MeshUtils::PositionVerticesIndices> result;

    mat3 world = GetWorldMatrix();



    MeshUtils::PositionVerticesIndices meshData;

    meshData.indices = GetIndices(true);

    auto vertices = GetVertices(true);

    for (auto& vertex : vertices)
    {
		meshData.vertices.push_back(vertex.Position / MAP_SCALE);
    }

    meshData = MeshUtils::RemoveDegenerates(meshData, 0.01f, 0.00f);

    //DebugDraw::IndexedMesh(meshData.vertices, meshData.indices, 100);

    result.push_back(meshData);

    return result;
}

std::vector<VertexData> BSPModelRef::GetVertices(bool collisionOnly, bool opaqueOnly)
{
    std::vector<VertexData> result;
    // Iterate over all faces of the model
    for (int i = model.face; i < model.face + model.n_faces; i++) 
    {
		if (collisionOnly)
		{
			tBSPFace face = bsp->m_pFaces[i];

			string textureName = string(bsp->pTextures[face.textureID].strName);

			if (StringHelper::Contains(textureName, "_cube"))
				continue;
		}

        if (opaqueOnly)
        {
            auto& faceData = bsp->cachedFaces[i];

            if (faceData.textureId == 0 || faceData.transparent)
                continue;
            
        }

        // Get the vertex array for face i
        auto& faceVertices = bsp->Rbuffers.v_faceVBOs[i];
        // Append all vertices from this face to the result
        result.insert(result.end(), faceVertices.begin(), faceVertices.end());
    }
    return result;
}

std::vector<uint32_t> BSPModelRef::GetIndices(bool collisionOnly, bool opaqueOnly) {
    std::vector<uint32_t> result;
    uint32_t vertexOffset = 0; // Tracks the number of vertices before the current face
    // Iterate over all faces of the model
    for (int i = model.face; i < model.face + model.n_faces; i++) 
    {

        if (collisionOnly)
        {
            tBSPFace face = bsp->m_pFaces[i];

            string textureName = string(bsp->pTextures[face.textureID].strName);

            if (StringHelper::Contains(textureName, "_cube"))
                continue;
        }

        if (opaqueOnly)
        {
            auto& faceData = bsp->cachedFaces[i];

            if (faceData.textureId == 0 || faceData.transparent)
                continue;

        }

        // Get the index array for face i
        auto& faceIndices = bsp->Rbuffers.v_faceIDXs[i];
        // Add each index, adjusted by the current offset
        for (auto idx : faceIndices) {
            result.push_back(static_cast<uint32_t>(idx) + vertexOffset);
        }
        // Update the offset by the number of vertices in this face
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
    bsp->RenderBSP(Camera::finalizedPosition, model,finalWorldMatrix, useBspVisibility, Static);

    if(Transparent)
        bsp->RenderTransparentFaces();

}

void BSPModelRef::DrawDepth(mat4x4 view, mat4x4 projection)
{

    const auto& vbo = bsp->opaqueVBOs[id];

    if (vbo.vao == nullptr)
        return;

    vbo.vao->Bind();

    ShaderProgram* shader = ShaderManager::GetShaderProgram("bsp", "empty_pixel");
    shader->UseProgram();




    shader->SetUniform("view", view);
    shader->SetUniform("projection", projection);
    shader->SetUniform("model", finalWorldMatrix);





    // draw using the EBO already bound in the VAO; offset = 0
    glDrawElements(GL_TRIANGLES,
        vbo.vao->IndexCount,
        GL_UNSIGNED_INT,
        0);


    VertexArrayObject::Unbind();

}
