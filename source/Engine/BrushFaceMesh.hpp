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
		return {};
    }


};