#include "model.hpp"
#include <filesystem>

#include "gl.h"

namespace roj
{

template<>
std::vector<VertexData> ModelLoader<Mesh>::getMeshVertices(aiMesh* mesh)
{
    std::vector<VertexData> vertices;
    for (uint32_t i = 0; i < mesh->mNumVertices; i++)
    {
        VertexData vertex;
        vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
        if (mesh->HasNormals())
        {
            vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
        }
        if (mesh->mTextureCoords[0])
        {
            vertex.TextureCoordinate = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
            vertex.Tangent = { mesh->mTangents[i].x,   mesh->mTangents[i].y,   mesh->mTangents[i].z };
            vertex.BiTangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
        }
        else
            vertex.TextureCoordinate = glm::vec2(0.0f, 0.0f);

        vertices.push_back(vertex);
    }

    return vertices;
}

template<>
Mesh ModelLoader<Mesh>::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<MeshTexture> textures = getMeshTextures(scene->mMaterials[mesh->mMaterialIndex]);
    std::vector<VertexData>  vertices = getMeshVertices(mesh);
    std::vector<uint32_t> indices;
    for (uint32_t i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace& face = mesh->mFaces[i];
        indices.insert(indices.end(), face.mIndices, face.mIndices + face.mNumIndices);
    }

    Mesh m;

    m.vertexBuffer = new VertexBuffer(vertices, VertexData::Declaration());

    return m;
}

template<>
void ModelLoader<Mesh>::processNode(aiNode* node, const aiScene* scene)
{
    for (uint32_t i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_model.push_back(processMesh(mesh, scene));
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

template<>
bool ModelLoader<Mesh>::load(const std::string& path)
{
    resetLoader();
    const aiScene* scene = m_import.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    m_relativeDir = "GameData/";// static_cast<std::filesystem::path>(path).parent_path().string();

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        m_infoLog += m_import.GetErrorString();
        return false;
    }

    processNode(scene->mRootNode, scene);
    for (Mesh& mesh : m_model)
    {
        mesh.VAO = new VertexArrayObject(*mesh.vertexBuffer, *mesh.indexBuffer);
    }
    return true;
}




}