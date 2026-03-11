#ifndef _QUAKE3BSP_H
#define _QUAKE3BSP_H

#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "../VertexData.h"

#include "../IDrawMesh.h"

#include "../glm.h"
#include "../ShaderManager.h"
#include "../BoundingBox.hpp"

#include <cstddef>


#define FACE_POLYGON 1
#define MAX_TEXTURES 1000

#define MAP_SCALE 32.0f

// This is our BSP header structure
struct tBSPHeader {
    char strID[4]; // This should always be 'IBSP'
    int  version;  // This should be 0x2e for Quake 3 files
};

// This is our BSP lump structure
struct tBSPLump {
    int offset; // The offset into the file for the start of this lump
    int length; // The length in bytes for this lump
};

// This is our BSP vertex structure
struct tBSPVertex {
    glm::vec3 vPosition;      // (x, y, z) position.
    glm::vec2 vTextureCoord;  // (u, v) texture coordinate
    glm::vec2 vLightmapCoord; // (u, v) lightmap coordinate
    glm::vec3 vNormal;        // (x, y, z) normal vector
    uint8_t color[4];       // RGBA color for the vertex
};

// This is our BSP face structure
struct tBSPFace {
    int       textureID;      // The index into the texture array
    int       effect;         // The index for the effects (or -1 = n/a)
    int       type;           // 1=polygon, 2=patch, 3=mesh, 4=billboard
    int       startVertIndex; // The starting index into this face's first vertex
    int       numOfVerts;     // The number of vertices for this face
    int       startIndex;     // The starting index into the indices array for this face
    int       numOfIndices;   // The number of indices for this face
    int       lightmapID;     // The texture index for the lightmap
    int       lMapCorner[2];  // The face's lightmap corner in the image
    int       lMapSize[2];    // The size of the lightmap section
    glm::vec3 lMapPos;        // The 3D origin of lightmap.
    glm::vec3 lMapVecs[2];    // The 3D space for s and t unit vectors.
    glm::vec3 vNormal;        // The face normal.
    int       size[2];        // The bezier patch dimensions.
};

// This is our BSP texture structure
struct tBSPTexture {
    char strName[64]; // The name of the texture w/o the extension
    int  flags;       // The surface flags (unknown)
    int  contents;    // The content flags (unknown)
};

struct tBSPLightmap {
    uint8_t imageBits[128][128][3]; // The RGB data in a 128x128 image
};

// Plane structure
struct tBSPPlane {
    glm::vec3 normal;
    float dist;
};

// Node structure
struct tBSPNode {
    int plane;
    int children[2];
    int mins[3];
    int maxs[3];
};

// Leaf structure
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

// Model structure
struct tBSPModel {
    float mins[3];
    float maxs[3];
    int face;
    int n_faces;
    int brush;
    int n_brushes;
};

// Brush structure
struct tBSPBrush {
    int brushside;
    int n_brushsides;
    int texture;
};

// Brush side structure
struct tBSPBrushSide {
    int plane;
    int texture;
};

// Meshvert structure
struct tBSPMeshVert {
    int offset;
};

// Effect structure
struct tBSPEffect {
    char name[64];
    int brush;
    int unknown;
};

// Light volume structure
struct tBSPLightvol {
    uint8_t ambient[3];
    uint8_t directional[3];
    uint8_t dir[2];
};

// Visdata structure
struct tBSPVisData {
    int n_vecs;
    int sz_vecs;
    std::vector<uint8_t> vecs;
};

struct FaceBuffers {
    std::unique_ptr<VertexArrayObject> VAO;
    std::unique_ptr<VertexBuffer> VBO;
    std::unique_ptr<IndexBuffer> EBO;
};

struct FaceBuffArray {
    std::map<int, FaceBuffers> FB_Idx;
};

struct RenderBuffers // m_renderBuffers.m_faceVBOs[idx].m_vertexBuffer
{
    std::map<int, std::vector<VertexData>> v_faceVBOs; // Changed to VertexData
    std::map<int, std::vector<GLuint>> v_faceIDXs;
    std::map<int, std::string>          texvec;
    std::vector<tBSPLightmap>           G_lightMaps;
};

struct CachedFaceTextureData
{
    int textureId = 0;
    string textureName = "";
    bool isCube = false;
    int lightmapId = 0;
    bool transparent = false;
    int numOfIndices = 0;
};

struct LightVolPointData {
    glm::vec3 directColor;
    glm::vec3 ambientColor;
    glm::vec3 direction;

    // Addition
    friend LightVolPointData operator+(const LightVolPointData& a, const LightVolPointData& b) {
        return {
            a.directColor + b.directColor,
            a.ambientColor + b.ambientColor,
            a.direction + b.direction
        };
    }

    // Subtraction
    friend LightVolPointData operator-(const LightVolPointData& a, const LightVolPointData& b) {
        return {
            a.directColor - b.directColor,
            a.ambientColor - b.ambientColor,
            a.direction - b.direction
        };
    }

    // Multiply by scalar
    friend LightVolPointData operator*(const LightVolPointData& v, float s) {
        return {
            v.directColor * s,
            v.ambientColor * s,
            v.direction * s
        };
    }
    friend LightVolPointData operator*(float s, const LightVolPointData& v) {
        return v * s;
    }

    // Divide by scalar
    friend LightVolPointData operator/(const LightVolPointData& v, float s) {
        return {
            v.directColor / s,
            v.ambientColor / s,
            v.direction / s
        };
    }

    // Compound-assignment versions
    LightVolPointData& operator+=(const LightVolPointData& o) {
        directColor += o.directColor;
        ambientColor += o.ambientColor;
        direction += o.direction;
        return *this;
    }
    LightVolPointData& operator-=(const LightVolPointData& o) {
        directColor -= o.directColor;
        ambientColor -= o.ambientColor;
        direction -= o.direction;
        return *this;
    }
    LightVolPointData& operator*=(float s) {
        directColor *= s;
        ambientColor *= s;
        direction *= s;
        return *this;
    }
    LightVolPointData& operator/=(float s) {
        directColor /= s;
        ambientColor /= s;
        direction /= s;
        return *this;
    }

    // Lerp between two LightVolPointData:
// - directColor & ambientColor: linear interpolation
// - direction: spherical linear interpolation (then normalized)
    static LightVolPointData Lerp(const LightVolPointData& a,
        const LightVolPointData& b,
        float t)
    {
        LightVolPointData result;
        // linear interp for colors
        result.directColor = glm::mix(a.directColor, b.directColor, t);
        result.ambientColor = glm::mix(a.ambientColor, b.ambientColor, t);
        // spherical interp for direction
        result.direction = glm::normalize(glm::slerp(a.direction, b.direction, t));
        return result;
    }

};

// This is our lumps enumeration
enum eLumps {
    kEntities = 0, // Stores player/object positions, etc...
    kTextures,     // Stores texture information
    kPlanes,       // Stores the splitting planes
    kNodes,        // Stores the BSP nodes
    kLeafs,        // Stores the leafs of the nodes
    kLeafFaces,    // Stores the leaf's indices into the faces
    kLeafBrushes,  // Stores the leaf's indices into the brushes
    kModels,       // Stores the info of world models
    kBrushes,      // Stores the brushes info (for collision)
    kBrushSides,   // Stores the brush surfaces info
    kVertices,     // Stores the level vertices
    kIndices,      // Stores the level indices
    kShaders,      // Stores the shader files (blending, anims..)
    kFaces,        // Stores the faces for the level
    kLightmaps,    // Stores the lightmaps for the level
    kLightVolumes, // Stores extra world lighting information
    kVisData,      // Stores PVS and cluster info (visibility)
    kMaxLumps      // A constant to store the number of lumps
};

struct FaceRenderData
{
    int faceIndex;
    bool useLightmap;
    LightVolPointData lightPointData;
    mat4 modelMatrix;
};


struct OpaqueModelVBO
{
    VertexBuffer* vbo = nullptr;
    IndexBuffer* ibo = nullptr;
    VertexArrayObject* vao = nullptr;
};

struct MergedModelFacesData
{
    VertexBuffer* vbo = nullptr;
    IndexBuffer* ibo = nullptr;
    VertexArrayObject* vao = nullptr;

    uint32 referenceFace = 0; //stores face data that will be referenced (for example for texturing)

    uint32 uId = 0; //unique id to avoid drawing same faces multiple times

    BoundingBox bounds;

};

class BSPModelRef;
// This is our Quake3 BSP class
class CQuake3BSP : public IDrawMesh
{

  public:
    // Our constructor
    CQuake3BSP();
    ~CQuake3BSP();

    // This loads a .bsp file by it's file name (Returns true if successful)
    bool LoadBSP(const char* filename);
    int m_numOfVerts; // The number of verts in the model

    int count;
    int indcount;
    int tcoordcount;

    char   tname[MAX_TEXTURES][64];
    int    textureID; // The index into the texture array
    GLuint texture[MAX_TEXTURES];

    string filePath = "";

    // private:

    // r3c:: new functions
    void GenerateTexture();
    void GenerateLightmap();
    bool RenderSingleFace(int index, bool lightmap, LightVolPointData lightData, mat4 model);
    bool RenderMergedFace(int index, bool lightmap, LightVolPointData lightData, mat4 model);
    void renderFaces();

    void BuildVBO();
    void CreateVBO(int m_numOfFaces);
    void BSPDebug(int index);
    void CreateRenderBuffers(int index);
    void CreateIndices(int index);

    std::string GetLightMapFilePathFromId(int id, const std::string& filePath);

    void PreloadFace(int index);
    void PreloadFaces();

    void BuildMergedModels();

    int m_numOfFaces;    // The number of faces in the model
    int m_numOfIndices;  // The number of indices for the model
    int m_numOfTextures; // The number of texture maps
    int m_numOfLightmaps;
    int m_textures[MAX_TEXTURES];  // The texture and lightmap array for the level
    int m_lightmaps[MAX_TEXTURES]; // The lightmap texture array
    int numVisibleFaces;
    int skipindices;

    static inline vec3 lightVolGridSize = vec3(64,64,128);

    GLuint* m_lightmap_gen_IDs;
    GLuint* m_Textures;
    GLuint  missing_LM_id;
    GLuint  white_LM_id;
    GLuint  missing_id;

    int*        m_pIndices; // The object's indices for rendering
    tBSPVertex* m_pVerts;   // The object's vertices
    tBSPFace*   m_pFaces;   // The faces information of the object

    FaceBuffArray FB_array;
    RenderBuffers Rbuffers;
    tBSPTexture*  pTextures;
    tBSPLightmap* pLightmaps;

    std::string entities;
    std::vector<tBSPPlane> planes;
    std::vector<tBSPNode> nodes;
    std::vector<tBSPLeaf> leafs;
    std::vector<int> leafFaces;
    std::vector<int> leafBrushes;
    std::vector<tBSPModel> models;
    std::vector<tBSPBrush> brushes;
    std::vector<tBSPBrushSide> brushSides;
    std::vector<tBSPMeshVert> meshVerts;
    std::vector<tBSPEffect> effects;

    std::vector<tBSPLightvol> lightVols;
    std::vector<tBSPLightvol> lightVolPalette;
    std::vector<uint32_t>     lightVolIndices;

    std::vector<OpaqueModelVBO> opaqueVBOs;

    std::vector<MergedModelFacesData> mergedFacesData;

    vector<int> mergedFacesMapping;

    CachedFaceTextureData* cachedFaces;

    std::vector<BoundingBox> faceBounds;

    tBSPVisData visData;

    vector<FaceRenderData> facesToDrawTransparent;

    glm::vec3 originalMins; // Original model bounds in Z-up (before transformation)
    glm::vec3 originalMaxs;

    std::vector<VertexData> GetFaceVertices(int faceId);
    std::vector<uint32_t> GetFaceIndices(int faceId);

    inline tBSPLightvol GetLightVol(int i) const {
        return lightVolPalette[lightVolIndices[i]];
    }

    bool CheckLightProbeAcess(const glm::vec3& position, const glm::vec3& volPosition);

    // Get lighting for a dynamic object at position (x, y, z)
    LightVolPointData GetLightvolColorPoint(const glm::vec3& position, bool wallCheck = false);

	//[DEPRECATED]
    //samples light in bigger radius, but after all improvements to GetLightvolColorPoint it has no use case
    LightVolPointData GetLightvolColor(const glm::vec3& position, bool wallCheck = false);
    int FindClusterAtPosition(glm::vec3 cameraPos);

    bool IsClusterVisible(int sourceCluster, int testCluster);

    void DrawForward(mat4x4 view, mat4x4 projection);

    // Optimized rendering loop
    void RenderBSP(const glm::vec3& cameraPos, tBSPModel& model, mat4 modelMatrix, bool useClusterVis, bool lightmap);

    void RenderTransparentFaces();

    bool IsFaceTransparent(int index);

    vector <BSPModelRef> GetAllModelRefs();

    void BuildStaticOpaqueObstacles();

    void LoadToLevel();

};


class BSPModelRef : public IDrawMesh
{
private:


public:

    CQuake3BSP* bsp = nullptr;
    int id = -1;

    tBSPModel& model;

    bool Static = true;
    bool useBspVisibility = false;

    vec3 Position = vec3(0);
    vec3 Rotation = vec3(0);
    vec3 Scale = vec3(1);

    vec3 avgPosition = vec3(0);

    mat4 finalWorldMatrix;

    BoundingBox bounds;

    BoundingBox GetTransformedBounds();

    mat4 GetWorldMatrix();

    BSPModelRef(CQuake3BSP* bsp_ptr, int model_id, tBSPModel& model_ref);
    
    ~BSPModelRef();

    void BuildVisBlocker();

    float GetDistanceToCamera();

    void CalculateAveragePosition();

    bool IsCameraVisible();

    bool IsInFrustrum(Frustum frustrum);

    bool IsBspVisible();

    bool CheckPointBspVisible(int cameraCluster, vec3 position);

    vector<tBSPFace> GetFaces();

    vector<MeshUtils::PositionVerticesIndices> GetNavObstacleMeshes();
    vector<VertexData> GetVertices(bool collisionOnly = false, bool opaqueOnly = false);
    vector<uint32_t> GetIndices(bool collisionOnly = false, bool opaqueOnly = false);
    
    void FinalizeFrameData();

    void DrawForward(mat4x4 view, mat4x4 projection);
    void DrawDepth(mat4x4 view, mat4x4 projection);

};

#endif
