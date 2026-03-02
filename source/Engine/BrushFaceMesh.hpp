#pragma once

#include "StaticMesh.h"

#include <vector>
#include <string>

#include "model.hpp"


#include "MeshUtils.hpp"

#include <unordered_map> // Required for unordered_map

#include "DebugDraw.hpp"

using namespace std;

class BrushFaceMesh : public StaticMesh
{
private:


public:

	vector<vec3> vertexLocations; //for physics shape generation

	string material;
	
    int closestLightSource = -1;

    BrushFaceMesh(Entity* owner): StaticMesh(owner)
    {

    }

	~BrushFaceMesh()
	{

		for (auto mesh : model->meshes)
		{
			delete(mesh.VAO);
            delete(mesh.vertices);
            delete(mesh.indices);
		}

		delete(model);
	}

    bool IsInFrustrum(Frustum frustrum) override
    {

        auto box = model->boundingBox.Transform(Position, Rotation, Scale);

        //DebugDraw::Bounds(box.Min, box.Max, 0.0);

        return frustrum.IsBoxVisible(box.Min, box.Max);
    };

	static vector<BrushFaceMesh*> GetMeshesFromName(Entity* owner, string filePath, string name, roj::ModelLoader<roj::SkinnedMesh>* modelLoader)
	{
        /*
		roj::LoaderGlobalParams::MeshNameLimit = name;
		roj::LoaderGlobalParams::Size = 1 / 32.0f;

		vector<BrushFaceMesh*> faces;

		modelLoader->load(filePath);

        string log = modelLoader->getInfoLog();

        if (log.size() > 1)
            Logger::Log(log);

		auto model = modelLoader->get();

		for (auto mesh : model.meshes)
		{
			
			BrushFaceMesh* face = new BrushFaceMesh(owner);

			roj::SkinnedModel* newModel = new roj::SkinnedModel();

            mesh.textures.clear();

            roj::MeshTexture texture;
            texture.type = aiTextureType_BASE_COLOR;
            texture.src = mesh.materialName + ".png";

            mesh.textures.push_back(texture);

			newModel->meshes.push_back(mesh);

			newModel->boundingSphere = model.boundingSphere;

			for (auto& vertex : mesh.vertexLocations)
			{
				face->vertexLocations.push_back(vertex.Position);
			}

			face->model = newModel;

			face->material = mesh.materialName;

			//face->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/cat.png");

			faces.push_back(face);


		}

		roj::LoaderGlobalParams::Size = 1;
		roj::LoaderGlobalParams::MeshNameLimit = "";

		return faces;
        */

		return vector<BrushFaceMesh*>();
	}


    static vector<BrushFaceMesh*> MergeMeshesByMaterial(vector<BrushFaceMesh*> faces)
    {
        // Step 1: Group meshes by material using an unordered_map
        std::unordered_map<std::string, std::vector<BrushFaceMesh*>> materialToMeshes;
        for (auto face : faces)
        {
            std::string material = face->material + "|" + to_string(face->closestLightSource);
            materialToMeshes[material].push_back(face);
        }

        // Step 2: Process each material group and merge meshes
        std::vector<BrushFaceMesh*> mergedFaces;
        for (const auto& pair : materialToMeshes)
        {
            std::string material = pair.first;

            for (int i = 0; i < material.size(); i++)
            {
                if (material[i] == '|')
                {
                    material = material.substr(0, i);
                    break;
                }
            }

            std::vector<BrushFaceMesh*> meshesToMerge = pair.second;

            // Collect vertices and indices from all meshes in this material group
            std::vector<MeshUtils::VerticesIndices> VIs;
            for (auto face : meshesToMerge)
            {
                // Each BrushFaceMesh has a model with typically one mesh
                auto mesh = face->model->meshes[0];
                MeshUtils::VerticesIndices VI;
                VI.vertices = mesh.vertexLocations; // Vector of roj::Vertex
                VI.indices = mesh.vertexIndices;    // Vector of indices
                VIs.push_back(VI);
            }

            // Merge all meshes for this material
            MeshUtils::VerticesIndices merged = MeshUtils::MergeMeshes(VIs);

            // Step 3: Create a new SkinnedMesh with the merged data
            roj::SkinnedMesh mergedMesh;
            mergedMesh.vertexLocations = merged.vertices; // Assign merged vertices
            mergedMesh.vertexIndices = merged.indices;   // Assign merged indices

            mergedMesh.textures.clear();

            roj::MeshTexture texture;
            texture.type = aiTextureType_BASE_COLOR;
            texture.src = material + ".png";

            mergedMesh.textures.push_back(texture);

            mergedMesh.vertices = new VertexBuffer(mergedMesh.vertexLocations, VertexData::Declaration(), GL_STATIC_DRAW);
            mergedMesh.indices = new IndexBuffer(mergedMesh.vertexIndices, GL_STATIC_DRAW);

            mergedMesh.VAO = new VertexArrayObject(*mergedMesh.vertices, *mergedMesh.indices);

            mergedMesh.materialName = material;          // Set the material name

            // Step 4: Create a new SkinnedModel and add the merged mesh
            roj::SkinnedModel* newModel = new roj::SkinnedModel();
            newModel->meshes.push_back(mergedMesh);

            newModel->boundingSphere = BoundingSphere::FromVertices(merged.vertices);
            newModel->boundingBox = BoundingBox::FromVertices(merged.vertices);

            // Create a new BrushFaceMesh and set its properties
            BrushFaceMesh* mergedFace = new BrushFaceMesh(faces[0]->OwnerEntity);
            mergedFace->model = newModel;
            mergedFace->material = material;

            // Update vertexLocations for physics shape generation
            std::vector<vec3> positions;
            for (const auto& vertex : merged.vertices)
            {
                positions.push_back(vertex.Position); // Extract position from each vertex
            }
            mergedFace->vertexLocations = positions;

            // Optional: Set ColorTexture if needed, but typically handled elsewhere
            // mergedFace->ColorTexture = ...;

            // Add the merged BrushFaceMesh to the result vector

            //mergedFace->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/cat.png");

            mergedFaces.push_back(mergedFace);
        }

        for (auto face : faces) // delete old brushes, models and it's meshes
        {

            delete(face);

        }

        // Step 5: Return the vector of merged BrushFaceMesh pointers
        return mergedFaces;
    }


};