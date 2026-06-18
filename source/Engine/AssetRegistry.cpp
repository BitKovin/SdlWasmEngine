#include "AssetRegistry.h"
#include "FileSystem/FileSystem.h"
#include <Logger.hpp>

std::unordered_map<std::string, Shader*> AssetRegistry::shaderCache;
std::unordered_map<std::string, Texture*> AssetRegistry::textureCache;
std::unordered_map<std::string, Video*> AssetRegistry::videoCache;
std::unordered_map<std::string, CubemapTexture*> AssetRegistry::textureCubeCache;
std::unordered_map<std::string, roj::SkinnedModel*> AssetRegistry::skinnedModelCache;
std::unordered_map<std::string, roj::SkinnedModel*> AssetRegistry::skinnedModelAnimationCache;
std::set<std::string> AssetRegistry::loadedAssetsDuringLoading;
std::set<std::string> AssetRegistry::constantlyLoaded;

void AssetRegistry::ClearMemory()
{

	for (auto texCube : textureCubeCache)
	{
		if (texCube.second == nullptr) continue;
		delete(texCube.second);
	}

	textureCubeCache.clear();

	for (auto tex : textureCache)
	{
		if (tex.second == nullptr) continue;
		delete(tex.second);
	}
	textureCache.clear();

	for (auto model : skinnedModelCache)
	{
		if (model.second == nullptr) continue;

		model.second->clear();
		delete(model.second);
	}
	skinnedModelCache.clear();

	for (auto model : skinnedModelAnimationCache)
	{
		if (model.second == nullptr) continue;

		model.second->clear();
		delete(model.second);
	}
	skinnedModelAnimationCache.clear();

	for (auto model : videoCache)
	{
		if (model.second == nullptr) continue;

		delete(model.second);
	}
	videoCache.clear();


}

void AssetRegistry::ClearUnusedMemory()
{
	// Texture Cubes
	for (auto it = textureCubeCache.begin(); it != textureCubeCache.end(); )
	{
		if (!IsAssetUsed(it->first))
		{
			delete it->second;
			it = textureCubeCache.erase(it);
		}
		else
		{
			++it;
		}
	}

	// Textures
	for (auto it = textureCache.begin(); it != textureCache.end(); )
	{
		if (!IsAssetUsed(it->first))
		{
			delete it->second;
			it = textureCache.erase(it);
		}
		else
		{
			++it;
		}
	}

	// Skinned Models
	for (auto it = skinnedModelCache.begin(); it != skinnedModelCache.end(); )
	{
		if (!IsAssetUsed(it->first))
		{
			if (it->second)
			{
				it->second->clear();
				delete it->second;
			}
			it = skinnedModelCache.erase(it);
		}
		else
		{
			++it;
		}
	}

	// Skinned Model Animations
	for (auto it = skinnedModelAnimationCache.begin(); it != skinnedModelAnimationCache.end(); )
	{
		if (!IsAssetUsed(it->first))
		{
			if (it->second)
			{
				it->second->clear();
				delete it->second;
			}
			it = skinnedModelAnimationCache.erase(it);
		}
		else
		{
			++it;
		}
	}

	// Videos
	for (auto it = videoCache.begin(); it != videoCache.end(); )
	{
		if (!IsAssetUsed(it->first))
		{
			delete it->second;
			it = videoCache.erase(it);
		}
		else
		{
			++it;
		}
	}
}

bool AssetRegistry::IsAssetUsed(std::string filename)
{
	return loadedAssetsDuringLoading.count(filename) || constantlyLoaded.count(filename);
}

void AssetRegistry::ReloadShaders()
{

	for (auto shader : shaderCache)
	{

		shader.second->Reload();

	}

}

Texture* AssetRegistry::GetTextureFromFile(string filename)
{

	MarkAsUsed(filename);

	string key = filename;

	auto it = textureCache.find(key);
	if (it != textureCache.end())
	{
		return it->second;
	}

	textureCache[key] = new Texture(filename, true);

	return textureCache[key];

}

CubemapTexture* AssetRegistry::GetTextureCubeFromFile(string filename)
{

	MarkAsUsed(filename);

	string key = filename;

	auto it = textureCubeCache.find(key);
	if (it != textureCubeCache.end())
	{
		return it->second; 
	}

	textureCubeCache[key] = new CubemapTexture(filename, false);

	return textureCubeCache[key];
}

void AssetRegistry::RegisterTexture(Texture* texture, string path)
{
	MarkAsUsed(path);

	if (textureCache.count(path))
	{
		Logger::Log("Warning: Texture already registered for path: " + path);
		delete textureCache[path]; // Clean up existing texture to prevent memory leak
	}

	textureCache[path] = texture;

}

Video* AssetRegistry::GetVideoFromFile(string filename)
{
	MarkAsUsed(filename);
	auto it = videoCache.find(filename);
	if (it != videoCache.end())
	{
		return it->second;
	}

	auto data = FileSystemEngine::ReadFileBinary(filename);

	Video* video = Video::FromVector(data);

	videoCache[filename] = video;

	return video;

}

std::string AssetRegistry::ReadFileToString(string filename)
{
	return FileSystemEngine::ReadFile(filename);
}

void AssetRegistry::BeginLevelLoad()
{
	loadedAssetsDuringLoading.clear();
	loadingLevel = true;
}

void AssetRegistry::EndLevelLoad()
{

	if (loadingLevel == false) return;

	loadingLevel = false;

	ClearUnusedMemory();

}

void AssetRegistry::MarkAsUsed(std::string filename)
{
	if (loadingLevel == false && !LoadingConstantAssets) return;

	loadedAssetsDuringLoading.insert(filename);

	if(LoadingConstantAssets)
		constantlyLoaded.insert(filename);

}

roj::SkinnedModel* AssetRegistry::GetSkinnedModelFromFile(const string& path)
{
	MarkAsUsed(path);
    auto it = skinnedModelCache.find(path);
    if (it != skinnedModelCache.end())
    {
		
		MarkModelTexturesAsUsed(it->second, path);
		

        return it->second;
    }

    roj::ModelLoader<roj::SkinnedMesh> modelLoader;

    modelLoader.load(path);

    Logger::Log(modelLoader.getInfoLog());

	skinnedModelCache[path] = new roj::SkinnedModel(std::move(modelLoader.get()));

	if (IsAssetUsed(path) == false)
	{
		MarkModelTexturesAsUsed(skinnedModelCache[path], path);
	}

    return skinnedModelCache[path];
}

roj::SkinnedModel* AssetRegistry::GetSkinnedAnimationFromFile(const string& path)
{
	MarkAsUsed(path);
	auto it = skinnedModelAnimationCache.find(path);
	if (it != skinnedModelAnimationCache.end())
	{
		return it->second;
	}

	roj::ModelLoader<roj::SkinnedMesh> modelLoader;

	modelLoader.SkipVisual = true;

	modelLoader.load(path);

	if(modelLoader.getInfoLog().empty() == false)
		std::cerr << modelLoader.getInfoLog() << endl;


	skinnedModelAnimationCache[path] = new roj::SkinnedModel(std::move(modelLoader.get()));


	return skinnedModelAnimationCache[path];
}

void AssetRegistry::MarkModelTexturesAsUsed(roj::SkinnedModel* model, std::string path)
{

	for (auto& s : textureCache)
	{
		if (StringHelper::Contains(s.first, path))
		{
			MarkAsUsed(s.first);
		}
	}

	for (auto& mesh : model->meshes)
	{
		mesh.cachedEmissiveColor = nullptr;
		mesh.cachedBaseColor = nullptr;
	}

}
