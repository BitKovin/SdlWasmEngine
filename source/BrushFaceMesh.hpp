#pragma once

#include "StaticMesh.hpp"

#include <vector>
#include <string>

#include "model.hpp"

using namespace std;

class BrushFaceMesh : public StaticMesh
{
private:


public:

	vector<vec3> vertexLocations; //for physics shape generation

	string material;
	
	BrushFaceMesh()
	{

	}

	~BrushFaceMesh()
	{

		for (auto mesh : model->meshes)
		{
			delete(mesh.VAO);
		}

		delete(model);
	}

	static vector<BrushFaceMesh*> GetMeshesFromName(string filePath, string name)
	{

		roj::LoaderGlobalParams::MeshNameLimit = name;
		roj::LoaderGlobalParams::Size = 1 / 32.0f;

		vector<BrushFaceMesh*> faces;

		roj::ModelLoader<roj::SkinnedMesh> modelLoader;

		modelLoader.load(filePath);

		Logger::Log(modelLoader.getInfoLog());

		auto model = modelLoader.get();

		for (auto mesh : model.meshes)
		{
			
			BrushFaceMesh* face = new BrushFaceMesh();

			roj::SkinnedModel* newModel = new roj::SkinnedModel();

			newModel->meshes.push_back(mesh);

			newModel->boundingSphere = model.boundingSphere;

			for (auto& vertex : mesh.vertexLocations)
			{
				face->vertexLocations.push_back(vertex.Position);
			}

			face->model = newModel;

			face->material = mesh.materialName;

			face->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/cat.png");

			faces.push_back(face);


		}

		roj::LoaderGlobalParams::Size = 1;
		roj::LoaderGlobalParams::MeshNameLimit = "";

		return faces;
	}

};