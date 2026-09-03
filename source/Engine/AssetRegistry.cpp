#include "AssetRegistry.h"
#include "FileSystem/FileSystem.h"
#include <Logger.hpp>
#include <chrono>

std::unordered_map<std::string, Shader*> AssetRegistry::shaderCache;
std::unordered_map<std::string, Texture*> AssetRegistry::textureCache;
std::unordered_map<std::string, Video*> AssetRegistry::videoCache;
std::unordered_map<std::string, CubemapTexture*> AssetRegistry::textureCubeCache;
std::unordered_map<std::string, roj::SkinnedModel*> AssetRegistry::skinnedModelCache;
std::unordered_map<std::string, roj::SkinnedModel*> AssetRegistry::skinnedModelAnimationCache;
std::set<std::string> AssetRegistry::loadedAssetsDuringLoading;
std::set<std::string> AssetRegistry::constantlyLoaded;

std::recursive_mutex AssetRegistry::cacheMutex;
ThreadPool* AssetRegistry::loaderThreadPool = nullptr;
std::mutex AssetRegistry::uploadQueueMutex;
std::deque<AssetRegistry::PendingUpload> AssetRegistry::uploadQueue;
std::atomic<uint64_t> AssetRegistry::frameCounter{ 0 };

void AssetRegistry::ClearMemory()
{
	std::lock_guard<std::recursive_mutex> lock(cacheMutex);

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
	std::lock_guard<std::recursive_mutex> lock(cacheMutex);

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
	std::lock_guard<std::recursive_mutex> lock(cacheMutex);
	return loadedAssetsDuringLoading.count(filename) || constantlyLoaded.count(filename);
}

void AssetRegistry::ReloadShaders()
{

	for (auto shader : shaderCache)
	{

		shader.second->Reload();

	}

}

Texture* AssetRegistry::GetTextureFromFile(string filename, AssetLoadTier requestedTier)
{
	std::lock_guard<std::recursive_mutex> lock(cacheMutex);

	MarkAsUsed(filename);

	string key = filename;

	// requestedTier is only ever consulted outside a level load - see the
	// declaration comment in AssetRegistry.h. Inside one, always synchronous.
	// VisualImmediately behaves exactly like Visual here - see the comment
	// on AssetLoadTier in AssetRegistry.h for why it's a separate value.
	const bool forceSync = loadingLevel || LoadingConstantAssets
		|| requestedTier == AssetLoadTier::Visual
		|| requestedTier == AssetLoadTier::VisualImmediately;

	auto it = textureCache.find(key);
	if (it != textureCache.end())
	{
		TouchUsage(it->second->loadState);

		if (forceSync)
			SynchronouslyFinishTextureLoad(it->second, key);
		else
			EnsureTextureTierLazy(it->second, key, requestedTier);

		return it->second;
	}

	if (forceSync)
	{
		Texture* texture = new Texture(filename, true); // unchanged - fully synchronous, as before
		textureCache[key] = texture;
		TouchUsage(texture->loadState);
		return texture;
	}

	Texture* texture = new Texture();
	textureCache[key] = texture;
	TouchUsage(texture->loadState);

	EnsureTextureTierLazy(texture, key, requestedTier);

	return texture;
}

CubemapTexture* AssetRegistry::GetTextureCubeFromFile(string filename)
{
	std::lock_guard<std::recursive_mutex> lock(cacheMutex);

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
	std::lock_guard<std::recursive_mutex> lock(cacheMutex);

	MarkAsUsed(path);

	auto it = textureCache.find(path);
	if (it != textureCache.end())
	{
		Logger::Log("Warning: Texture already registered for path: " + path);
		delete it->second; // Clean up existing texture to prevent memory leak
	}

	textureCache[path] = texture;
	TouchUsage(texture->loadState);

}

Video* AssetRegistry::GetVideoFromFile(string filename)
{
	std::lock_guard<std::recursive_mutex> lock(cacheMutex);

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
	std::lock_guard<std::recursive_mutex> lock(cacheMutex);
	loadedAssetsDuringLoading.clear();
	loadingLevel = true;
}

void AssetRegistry::EndLevelLoad()
{
	std::lock_guard<std::recursive_mutex> lock(cacheMutex);

	if (loadingLevel == false) return;

	loadingLevel = false;

	ClearUnusedMemory();

}

void AssetRegistry::MarkAsUsed(std::string filename)
{
	std::lock_guard<std::recursive_mutex> lock(cacheMutex);

	if (loadingLevel == false && !LoadingConstantAssets) return;

	loadedAssetsDuringLoading.insert(filename);

	if (LoadingConstantAssets)
		constantlyLoaded.insert(filename);

}

roj::SkinnedModel* AssetRegistry::GetSkinnedModelFromFile(const string& path, AssetLoadTier requestedTier)
{
	std::lock_guard<std::recursive_mutex> lock(cacheMutex);

	MarkAsUsed(path);

	const bool forceSync = loadingLevel || LoadingConstantAssets
		|| requestedTier == AssetLoadTier::Visual
		|| requestedTier == AssetLoadTier::VisualImmediately;

	auto it = skinnedModelCache.find(path);
	if (it != skinnedModelCache.end())
	{
		roj::SkinnedModel* model = it->second;
		TouchUsage(model->loadState);

		if (forceSync)
		{
			// Unchanged cache-hit bookkeeping/reset for the synchronous path.
			MarkModelTexturesAsUsed(model, path);
			SynchronouslyFinishLoad(model, path);
		}
		else
		{
			EnsureTierLazy(model, path, requestedTier);
		}

		return model;
	}

	if (forceSync)
	{
		// Unchanged - fully synchronous, exactly as before.
		roj::ModelLoader<roj::SkinnedMesh> modelLoader;

		modelLoader.load(path);

		Logger::Log(modelLoader.getInfoLog());

		roj::SkinnedModel* model = new roj::SkinnedModel(std::move(modelLoader.get()));

		model->loadState.generation.store(1, std::memory_order_release);
		model->loadState.currentTier.store(AssetLoadTier::Visual, std::memory_order_release);
		model->loadState.queuedUpTo.store(AssetLoadTier::Visual, std::memory_order_release);

		skinnedModelCache[path] = model;
		TouchUsage(model->loadState);

		if (IsAssetUsed(path) == false)
			MarkModelTexturesAsUsed(model, path);

		return model;
	}

	// Lazy miss (the default): register an empty placeholder and insert it
	// into the cache right now, while still holding cacheMutex for this
	// entire function call. That's what makes two threads racing on the
	// same brand-new path impossible to duplicate: whichever thread gets
	// here first inserts under the lock, and any other thread requesting
	// the same path can only start running this function after that lock
	// is released - by which point `it != skinnedModelCache.end()` above
	// will already find this same object, never a second one.
	roj::SkinnedModel* model = new roj::SkinnedModel();
	skinnedModelCache[path] = model;
	TouchUsage(model->loadState);

	EnsureTierLazy(model, path, requestedTier);

	return model;
}

roj::SkinnedModel* AssetRegistry::GetSkinnedAnimationFromFile(const string& path, AssetLoadTier requestedTier)
{
	std::lock_guard<std::recursive_mutex> lock(cacheMutex);

	MarkAsUsed(path);

	// This cache never holds anything above Logic (it's animations-only -
	// no meshes), so forceSync and VisualImmediately don't mean anything
	// extra here beyond "make sure Logic is loaded" - same as the default.
	const bool forceSync = loadingLevel || LoadingConstantAssets
		|| requestedTier == AssetLoadTier::Visual
		|| requestedTier == AssetLoadTier::VisualImmediately;

	auto it = skinnedModelAnimationCache.find(path);
	if (it != skinnedModelAnimationCache.end())
	{
		roj::SkinnedModel* model = it->second;
		TouchUsage(model->loadState);

		// queuedUpTo is the claim ticket here: since this whole call holds
		// cacheMutex, no other thread can be inside this function
		// concurrently, but a PREVIOUS call for this same path may already
		// have loaded (or be responsible for loading) Logic - don't redo it.
		if ((forceSync || requestedTier != AssetLoadTier::None) &&
			model->loadState.queuedUpTo.load(std::memory_order_acquire) < AssetLoadTier::Logic)
		{
			model->loadState.queuedUpTo.store(AssetLoadTier::Logic, std::memory_order_release);
			LoadLogicTierNow(model, path);
		}

		return model;
	}

	if (forceSync)
	{
		// Unchanged - fully synchronous, exactly as before.
		roj::ModelLoader<roj::SkinnedMesh> modelLoader;

		modelLoader.SkipVisual = true;

		modelLoader.load(path);

		if (modelLoader.getInfoLog().empty() == false)
			std::cerr << modelLoader.getInfoLog() << endl;

		roj::SkinnedModel* model = new roj::SkinnedModel(std::move(modelLoader.get()));

		model->loadState.generation.store(1, std::memory_order_release);
		model->loadState.currentTier.store(AssetLoadTier::Logic, std::memory_order_release);
		model->loadState.queuedUpTo.store(AssetLoadTier::Logic, std::memory_order_release);

		skinnedModelAnimationCache[path] = model;
		TouchUsage(model->loadState);

		return model;
	}

	// Lazy miss (the default): insert the placeholder under cacheMutex first
	// (same duplicate-prevention reasoning as GetSkinnedModelFromFile), then
	// load its Logic tier synchronously, right here - it's CPU-only
	// (SkipVisual=true), so this never touches bgfx and is safe on any thread.
	roj::SkinnedModel* model = new roj::SkinnedModel();
	skinnedModelAnimationCache[path] = model;
	TouchUsage(model->loadState);

	if (requestedTier != AssetLoadTier::None)
	{
		model->loadState.queuedUpTo.store(AssetLoadTier::Logic, std::memory_order_release);
		LoadLogicTierNow(model, path);
	}

	return model;
}

void AssetRegistry::MarkModelTexturesAsUsed(roj::SkinnedModel* model, std::string path)
{
	std::lock_guard<std::recursive_mutex> lock(cacheMutex);

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

// ─────────────────────────────────────────────────────────────────────────────
// Async / tiered loading
// ─────────────────────────────────────────────────────────────────────────────

void AssetRegistry::TouchUsage(AssetLoadState& state)
{
	state.lastUsedFrame.store(frameCounter.load(std::memory_order_relaxed), std::memory_order_relaxed);
}

void AssetRegistry::AdvanceFrame()
{
	frameCounter.fetch_add(1, std::memory_order_relaxed);
}

void AssetRegistry::StartLoaderThread()
{
	if (loaderThreadPool != nullptr) return;

	loaderThreadPool = new ThreadPool("Asset Loader Thread");
	loaderThreadPool->Start(1); // single worker - see AssetRegistry.h
}

void AssetRegistry::StopLoaderThread()
{
	if (loaderThreadPool == nullptr) return;

	loaderThreadPool->Stop();
	delete loaderThreadPool;
	loaderThreadPool = nullptr;
}

void AssetRegistry::EnqueuePendingUpload(std::function<void()> apply, size_t approxBytes)
{
	std::lock_guard<std::mutex> lock(uploadQueueMutex);
	uploadQueue.push_back(PendingUpload{ std::move(apply), approxBytes });
}

void AssetRegistry::ProcessPendingUploads(float msBudget)
{
	auto start = std::chrono::steady_clock::now();

	for (;;)
	{
		PendingUpload item;
		{
			std::lock_guard<std::mutex> lock(uploadQueueMutex);
			if (uploadQueue.empty()) break;
			item = std::move(uploadQueue.front());
			uploadQueue.pop_front();
		}

		{
			// Every closure queued here (AdoptVisualTierGeometry, UploadDecoded,
			// UnloadAll/UnloadVisualTier, UnloadGPU) mutates the same model/
			// texture that a Game-thread forceSync load (SynchronouslyFinishLoad/
			// SynchronouslyFinishTextureLoad) can be touching concurrently.
			// cacheMutex is what makes those mutually exclusive - previously
			// this ran unlocked and could corrupt the containers it writes to.
			std::lock_guard<std::recursive_mutex> lock(cacheMutex);
			item.Apply(); // the actual bgfx call, then currentTier.store(Visual, release)
		}

		if (std::chrono::duration<float, std::milli>(std::chrono::steady_clock::now() - start).count() >= msBudget)
			break; // rest picked up next frame - already-applied items stay applied
	}
}

// Parses just the Logic tier (skeleton/bones/animation clips - no meshes,
// no textures, no bgfx) and commits it directly, on whichever thread calls
// this. SkipVisual+DeferGPUUpload keep this entirely CPU-side (see
// ModelLoader<SkinnedMesh>::load() and LoadTextureFromScene()), which is
// exactly what makes it safe to run synchronously on the requesting thread
// instead of bouncing through the Loader thread the way Visual-tier work
// still has to.
//
// Callers gate this behind a queuedUpTo < Logic check before calling (see
// EnsureTierLazy / GetSkinnedAnimationFromFile), and every Get*FromFile call
// holds cacheMutex for its entire duration, so two threads can never be
// parsing the same model's Logic tier at once, and a later call for the
// same path just finds the already-loaded result instead of redoing this.
//
// Caller holds cacheMutex.
void AssetRegistry::LoadLogicTierNow(roj::SkinnedModel* model, const std::string& path)
{
	roj::ModelLoader<roj::SkinnedMesh> loader;
	loader.SkipVisual = true;
	loader.DeferGPUUpload = true; // no meshes to build either way, but also keeps any embedded texture off this thread

	loader.load(path);

	if (!loader.getInfoLog().empty())
		Logger::Log(loader.getInfoLog());

	// Defensive, not load-bearing under normal call patterns (see above):
	// a forceSync call for this same path (Visual/VisualImmediately, or a
	// level load) could only have run strictly before or after us, never
	// during - but if one already finished the whole model, don't stomp its
	// full result with our now-stale partial one.
	if (model->loadState.currentTier.load(std::memory_order_acquire) == AssetLoadTier::Visual)
		return;

	model->AdoptLogicTierData(std::move(loader.get()));

	model->loadState.generation.fetch_add(1, std::memory_order_release);
	model->loadState.currentTier.store(AssetLoadTier::Logic, std::memory_order_release);
}

void AssetRegistry::QueueVisualTierJob(roj::SkinnedModel* model, std::string path)
{
	loaderThreadPool->QueueJob([model, path]()
		{
			roj::ModelLoader<roj::SkinnedMesh> loader;
			loader.DeferGPUUpload = true;

			loader.load(path);

			if (!loader.getInfoLog().empty())
				Logger::Log(loader.getInfoLog());

			// Re-parses bones/animations too (SkipVisual=false) - wasted work,
			// but reusing load() as-is keeps this a small, easy-to-trust diff.
			// AdoptVisualTierGeometry() only takes what it needs from it.
			//
			// SkinnedModel is move-only (loadState can't be copied), but
			// std::function needs its target to be copy-constructible - a
			// shared_ptr keeps the closure itself copyable even though this
			// parse is only ever actually moved-from, exactly once, below.
			auto parsed = std::make_shared<roj::SkinnedModel>(std::move(loader.get()));

			size_t approxBytes = 0;
			for (auto& mesh : parsed->meshes)
				approxBytes += sizeof(VertexData) * mesh.vertices.size() + sizeof(uint32_t) * mesh.indices.size();

			EnqueuePendingUpload(
				[model, parsed]()
				{
					// Same race guard as LoadLogicTierNow - a synchronous
					// force-finish may have already replaced this model outright.
					if (model->loadState.currentTier.load(std::memory_order_acquire) == AssetLoadTier::Visual)
						return;

					model->AdoptVisualTierGeometry(std::move(*parsed));

					for (auto& mesh : model->meshes)
						roj::UploadSkinnedMeshGPUBuffers(mesh);

					model->loadState.generation.fetch_add(1, std::memory_order_release);
					model->loadState.currentTier.store(AssetLoadTier::Visual, std::memory_order_release);
				},
				approxBytes);
		});
}

void AssetRegistry::QueueTextureUploadJob(Texture* texture, std::string path)
{
	loaderThreadPool->QueueJob([texture, path]()
		{
			Texture::Decoded decoded = Texture::DecodeFromFile(path, true);
			if (!decoded.valid) return; // stays at None - matches today's error-logging-only behavior on a bad file

			size_t approxBytes = decoded.pixels.size();

			EnqueuePendingUpload(
				[texture, decoded = std::move(decoded), path]() mutable
				{
					texture->UploadDecoded(std::move(decoded), path);
				},
				approxBytes);
		});
}

void AssetRegistry::EnsureTierLazy(roj::SkinnedModel* model, const std::string& path, AssetLoadTier requestedTier)
{
	if (requestedTier == AssetLoadTier::None) return; // fully caller-driven from here - see RequestVisualLoad

	// Reaching this function at all means we're not forcing sync, so
	// requestedTier here is only ever None (already returned above) or
	// Logic - Visual/VisualImmediately always take GetSkinnedModelFromFile's
	// forceSync branch instead. Logic is the "just load it" default, and
	// that means loading Logic tier right now (see LoadLogicTierNow - it's
	// cheap and bgfx-free, so there's no reason to make the caller wait a
	// frame for it) while still queueing Visual lazily, or the model would
	// sit at bones-only forever - "late appearing" means it does still
	// arrive, just not blocking anything on the way there.
	//
	// Caller holds cacheMutex, so this check-then-set is race-free even
	// though queuedUpTo is atomic (atomic here for cheap lock-free reads
	// elsewhere, not because this particular update needs to be).
	if (model->loadState.queuedUpTo.load(std::memory_order_acquire) < AssetLoadTier::Logic)
	{
		model->loadState.queuedUpTo.store(AssetLoadTier::Logic, std::memory_order_release);
		LoadLogicTierNow(model, path);
	}

	if (model->loadState.queuedUpTo.load(std::memory_order_acquire) < AssetLoadTier::Visual)
	{
		model->loadState.queuedUpTo.store(AssetLoadTier::Visual, std::memory_order_release);
		QueueVisualTierJob(model, path);
	}
}

void AssetRegistry::EnsureTextureTierLazy(Texture* texture, const std::string& path, AssetLoadTier requestedTier)
{
	if (requestedTier == AssetLoadTier::None) return;
	if (texture->loadState.queuedUpTo.load(std::memory_order_acquire) == AssetLoadTier::Visual) return; // already done or in flight

	texture->loadState.queuedUpTo.store(AssetLoadTier::Visual, std::memory_order_release);
	QueueTextureUploadJob(texture, path);
}

void AssetRegistry::SynchronouslyFinishLoad(roj::SkinnedModel* model, const std::string& path)
{
	if (model->loadState.currentTier.load(std::memory_order_acquire) == AssetLoadTier::Visual) return;

	// Drop whatever a lazy load already landed first, so the full reparse
	// below doesn't leak the meshes/bones it's about to replace wholesale.
	model->UnloadAll();

	roj::ModelLoader<roj::SkinnedMesh> modelLoader;
	modelLoader.load(path);
	Logger::Log(modelLoader.getInfoLog());

	*model = std::move(modelLoader.get()); // custom move-assign - everything except loadState, see skinned_model.hpp

	model->loadState.generation.fetch_add(1, std::memory_order_release);
	model->loadState.currentTier.store(AssetLoadTier::Visual, std::memory_order_release);
	model->loadState.queuedUpTo.store(AssetLoadTier::Visual, std::memory_order_release);
}

void AssetRegistry::SynchronouslyFinishTextureLoad(Texture* texture, const std::string& path)
{
	if (texture->loadState.currentTier.load(std::memory_order_acquire) == AssetLoadTier::Visual) return;

	Texture::Decoded decoded = Texture::DecodeFromFile(path, true);
	texture->UploadDecoded(std::move(decoded), path); // stamps loadState itself - see Texture.hpp

	texture->loadState.queuedUpTo.store(AssetLoadTier::Visual, std::memory_order_release);
}

void AssetRegistry::RequestVisualLoad(roj::SkinnedModel* model, string path)
{
	if (model == nullptr) return;
	if (model->loadState.currentTier.load(std::memory_order_acquire) == AssetLoadTier::Visual) return;
	if (model->loadState.queuedUpTo.load(std::memory_order_acquire) == AssetLoadTier::Visual) return; // already in flight

	model->loadState.queuedUpTo.store(AssetLoadTier::Visual, std::memory_order_release);
	QueueVisualTierJob(model, path);
}

void AssetRegistry::RequestVisualLoad(Texture* texture, string path)
{
	if (texture == nullptr) return;
	if (texture->loadState.currentTier.load(std::memory_order_acquire) == AssetLoadTier::Visual) return;
	if (texture->loadState.queuedUpTo.load(std::memory_order_acquire) == AssetLoadTier::Visual) return;

	texture->loadState.queuedUpTo.store(AssetLoadTier::Visual, std::memory_order_release);
	QueueTextureUploadJob(texture, path);
}

void AssetRegistry::UnloadToTier(roj::SkinnedModel* model, AssetLoadTier tier)
{
	if (model == nullptr) return;

	EnqueuePendingUpload([model, tier]()
		{
			if (model->loadState.currentTier.load(std::memory_order_acquire) <= tier)
				return; // already unloaded past this point, or a reload beat us to it

			if (tier == AssetLoadTier::None)
				model->UnloadAll();
			else
				model->UnloadVisualTier();

			model->loadState.generation.fetch_add(1, std::memory_order_release);
			model->loadState.currentTier.store(tier, std::memory_order_release);
			model->loadState.queuedUpTo.store(tier, std::memory_order_release); // so a later lazy load re-queues what we just dropped
		});
}

void AssetRegistry::UnloadToTier(Texture* texture, AssetLoadTier tier)
{
	if (texture == nullptr) return;

	EnqueuePendingUpload([texture]()
		{
			if (texture->loadState.currentTier.load(std::memory_order_acquire) == AssetLoadTier::None)
				return;

			texture->UnloadGPU();
			texture->loadState.generation.fetch_add(1, std::memory_order_release);
			texture->loadState.currentTier.store(AssetLoadTier::None, std::memory_order_release);
			texture->loadState.queuedUpTo.store(AssetLoadTier::None, std::memory_order_release);
		});
}