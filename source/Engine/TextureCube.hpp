#pragma once
#include <string>
#include <iostream>
#include <vector>
#include "FileSystem/FileSystem.h"
#include <includedLibraries/stb_image.h>
#include <includedLibraries/stb_image_write.h>
#include <bgfx/bgfx.h>
#include "malloc_override.h"
#include "Helpers/StringHelper.h"
#include <Profiling/ResourceStatistics.hpp>
#include <cstring> // for memcpy
#include <cmath> // for atan2, asin, sqrt
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
class CubemapTexture
{
public:
	// faces should be provided in this order:
	// right(+X), left(-X), top(+Y), bottom(-Y), front(+Z), back(-Z)
	CubemapTexture(const std::vector<std::string>& faces, bool generateMipmaps = false) {
		if (faces.size() != 6) {
			std::cerr << "Cubemap texture requires exactly 6 faces." << std::endl;
			return;
		}
		loadFromFiles(faces, generateMipmaps);
	}
	// -----------------------------------------------------------------------
	// Smart single-path constructor.
	//
	// Pass any one of the face paths or the panorama path — the constructor
	// checks whether all 6 directional variants exist on disk:
	// <base>_lf.<ext> _rt _up _dn _ft _bk
	// If every file is present it uses them directly (original behaviour).
	// If any are missing it falls back to loading 'base' as an equirectangular
	// panorama and converts it on the fly.
	// -----------------------------------------------------------------------
	CubemapTexture(const std::string& base, bool generateMipmaps = false) {
		std::vector<std::string> faces = {
			StringHelper::Replace(base, ".", "_lf."),
			StringHelper::Replace(base, ".", "_rt."),
			StringHelper::Replace(base, ".", "_up."),
			StringHelper::Replace(base, ".", "_dn."),
			StringHelper::Replace(base, ".", "_ft."),
			StringHelper::Replace(base, ".", "_bk."),
		};
		bool allExist = true;
		for (const auto& f : faces) {
			if (FileSystemEngine::ReadFileBinary(f).empty()) {
				allExist = false;
				break;
			}
		}
		if (allExist)
			loadFromFiles(faces, generateMipmaps);
		else
			loadFromPanorama(base, generateMipmaps);
	}
	~CubemapTexture() {
		ResourceStatistics::Instance().unregisterResource(ResourceType::TextureCube, m_handle.idx);
		if (bgfx::isValid(m_handle))
			bgfx::destroy(m_handle);
	}
	void bind(uint8_t stage, bgfx::UniformHandle sampler) const {
		bgfx::setTexture(stage, sampler, m_handle);
	}
	bool valid = false;
	bgfx::TextureHandle getHandle() const {
		return m_handle;
	}
	// Backward-compat: return numeric ID for ResourceStatistics etc.
	uint16_t getID() const {
		return m_handle.idx;
	}
	bgfx::TextureHandle getTextureHandle() const {
		return m_handle;
	}
	void setName(const std::string& name) {
		ResourceStatistics::Instance().setResourceName(ResourceType::TextureCube, m_handle.idx, name);
		if (bgfx::isValid(m_handle))
			bgfx::setName(m_handle, name.c_str(), (int32_t)name.size());
	}
private:
	bgfx::TextureHandle m_handle = BGFX_INVALID_HANDLE;
	// -----------------------------------------------------------------------
	// Helper: rotate an RGBA buffer 90° CW or CCW.
	// Allocates and returns a new buffer. Caller must free with free().
	// (also used by the file-based path for the up/dn faces)
	// -----------------------------------------------------------------------
	static stbi_uc* rotate90_rgba(const stbi_uc* src, int w, int h, bool cw)
	{
		stbi_uc* dst = static_cast<stbi_uc*>(malloc(w * h * 4));
		if (!dst) return nullptr;
		for (int y = 0; y < h; ++y)
		{
			for (int x = 0; x < w; ++x)
			{
				const stbi_uc* s = src + (y * w + x) * 4;
				stbi_uc* d;
				if (cw)
					d = dst + (x * h + (h - 1 - y)) * 4;
				else
					d = dst + ((w - 1 - x) * h + y) * 4;
				d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3];
			}
		}
		return dst;
	}
	// -----------------------------------------------------------------------
	// Flags
	// -----------------------------------------------------------------------
	static uint64_t buildFlags(bool generateMipmaps) {
		uint64_t flags = BGFX_TEXTURE_NONE;
		// Anisotropic filtering requires a mip chain. Requesting it without
		// mipmaps makes bgfx internally force hasMips=true, which causes a
		// fatal storage-size mismatch when only base-level data is supplied.
		if (generateMipmaps)
			flags |= BGFX_SAMPLER_MIN_ANISOTROPIC | BGFX_SAMPLER_MAG_ANISOTROPIC;
		return flags;
	}
	// -----------------------------------------------------------------------
	// Panorama helpers
	// -----------------------------------------------------------------------
	// Bilinear sample from an equirectangular RGBA8 image.
	// lon in [-π, π], lat in [-π/2, π/2].
	static void sampleEquirect(const stbi_uc* img, int srcW, int srcH,
		double lon, double lat, stbi_uc* out)
	{
		double u = (lon / (2.0 * M_PI) + 0.5) * srcW - 0.5;
		double v = (0.5 - lat / M_PI) * srcH - 0.5;
		int x0 = (int)floor(u), y0 = (int)floor(v);
		int x1 = x0 + 1, y1 = y0 + 1;
		double fx = u - x0, fy = v - y0;
		auto wrapX = [&](int x) { return ((x % srcW) + srcW) % srcW; };
		auto clampY = [&](int y) { return y < 0 ? 0 : (y >= srcH ? srcH - 1 : y); };
		int ix0 = wrapX(x0), ix1 = wrapX(x1);
		int iy0 = clampY(y0), iy1 = clampY(y1);
		for (int c = 0; c < 4; ++c)
		{
			double p00 = img[(iy0 * srcW + ix0) * 4 + c];
			double p10 = img[(iy0 * srcW + ix1) * 4 + c];
			double p01 = img[(iy1 * srcW + ix0) * 4 + c];
			double p11 = img[(iy1 * srcW + ix1) * 4 + c];
			double val = p00 * (1.0 - fx) * (1.0 - fy)
				+ p10 * fx * (1.0 - fy)
				+ p01 * (1.0 - fx) * fy
				+ p11 * fx * fy;
			out[c] = (stbi_uc)(val + 0.5);
		}
	}
	// Map a pixel (u, v in [-1,1]) on a given bgfx face slot to a 3-D direction
	// that corresponds to the Quake 3 skybox layout used by this class:
	//
	// slot 0 (+X / lf): camera looks in -X → left of the panorama
	// slot 1 (-X / rt): camera looks in +X → right
	// slot 2 (+Y / up): camera looks in +Y → up (image rotated -90° after)
	// slot 3 (-Y / dn): camera looks in -Y → down (image rotated -90° after)
	// slot 4 (+Z / ft): camera looks in +Z → front (lon = 0)
	// slot 5 (-Z / bk): camera looks in -Z → back
	//
	// u increases to the right, v increases downward (image space).
	static void bgfxFaceDir(int slot, double u, double v,
		double& dx, double& dy, double& dz)
	{
		switch (slot)
		{
		case 0: dx = -1.0; dy = -v; dz = u; break; // lf: look -X
		case 1: dx = 1.0; dy = -v; dz = -u; break; // rt: look +X
		case 2: dx = u; dy = 1.0; dz = v; break; // up: look +Y
		case 3: dx = u; dy = -1.0; dz = -v; break; // dn: look -Y
		case 4: dx = u; dy = -v; dz = 1.0; break; // ft: look +Z
		default: dx = -u; dy = -v; dz = -1.0; break; // bk: look -Z
		}
	}
	// -----------------------------------------------------------------------
	// Panorama → cubemap loader
	// -----------------------------------------------------------------------
	void loadFromPanorama(const std::string& panoramaPath, bool generateMipmaps)
	{
		// --- Load equirectangular source image ---
		std::vector<uint8_t> fileData = FileSystemEngine::ReadFileBinary(panoramaPath);
		if (fileData.empty()) {
			std::cerr << "[Cubemap] Panorama file empty or not found: " << panoramaPath << "\n";
			return;
		}
		int panoW = 0, panoH = 0, channels = 0;
		stbi_uc* panorama = stbi_load_from_memory(
			fileData.data(), (int)fileData.size(),
			&panoW, &panoH, &channels, STBI_rgb_alpha
		);
		if (!panorama) {
			std::cerr << "[Cubemap] stbi_load_from_memory failed for "
				<< panoramaPath << ": " << stbi_failure_reason() << "\n";
			return;
		}
		// Face size: half the panorama height (keeps good texel density).
		const int faceSize = panoH / 2;
		const size_t faceBytes = (size_t)faceSize * faceSize * 4;
		// Build the 6 output paths (same suffixes as the file-based loader).
		static const char* kSuffix[6] = { "_lf.", "_rt.", "_up.", "_dn.", "_bk.", "_ft." };
		std::vector<std::string> facePaths(6);
		for (int i = 0; i < 6; ++i)
			facePaths[i] = StringHelper::Replace(panoramaPath, ".", kSuffix[i]);
		// PNG write callback — appends encoded bytes into a std::vector<uint8_t>.
		auto pngWriteCallback = [](void* ctx, void* data, int size) {
			auto* buf = static_cast<std::vector<uint8_t>*>(ctx);
			const uint8_t* bytes = static_cast<const uint8_t*>(data);
			buf->insert(buf->end(), bytes, bytes + size);
		};
		// --- Generate 6 faces ---
		std::vector<unsigned char> allData(faceBytes * 6);
		for (int slot = 0; slot < 6; ++slot)
		{
			std::vector<stbi_uc> rawFace(faceBytes);
			for (int py = 0; py < faceSize; ++py)
			{
				for (int px = 0; px < faceSize; ++px)
				{
					// Map pixel centre to [-1, 1]
					double u = (px + 0.5) / faceSize * 2.0 - 1.0;
					double v = (py + 0.5) / faceSize * 2.0 - 1.0;
					double dx, dy, dz;
					bgfxFaceDir(slot, u, v, dx, dy, dz);
					// Normalise
					double len = sqrt(dx * dx + dy * dy + dz * dz);
					dx /= len; dy /= len; dz /= len;
					// To spherical
					double lon = atan2(dx, dz); // [-π, π]
					double lat = asin(dy); // [-π/2, π/2]
					stbi_uc* pixel = rawFace.data() + (py * faceSize + px) * 4;
					sampleEquirect(panorama, panoW, panoH, lon, lat, pixel);
				}
			}
			// up (slot 2) and dn (slot 3) need a -90° (CCW) rotation to match
			// the orientation produced by the file-based loader.
			stbi_uc* faceData = rawFace.data();
			std::vector<stbi_uc> rotatedStorage;
			if (slot == 2 || slot == 3)
			{
				stbi_uc* rotated = rotate90_rgba(faceData, faceSize, faceSize, /*cw=*/slot == 2);
				if (rotated) {
					rotatedStorage.assign(rotated, rotated + faceBytes);
					free(rotated);
					faceData = rotatedStorage.data();
				}
			}
			// --- Save face as PNG next to the panorama ---
			{
				std::vector<uint8_t> pngData;
				stbi_write_png_to_func(pngWriteCallback, &pngData,
					faceSize, faceSize, 4,
					faceData, faceSize * 4);
				if (!pngData.empty()) {
					if (!FileSystemEngine::WriteFileBinary(facePaths[slot], pngData))
						std::cerr << "[Cubemap] Failed to save face: " << facePaths[slot] << "\n";
				}
				else {
					std::cerr << "[Cubemap] PNG encode failed for slot " << slot << "\n";
				}
			}
			memcpy(allData.data() + slot * faceBytes, faceData, faceBytes);
		}
		stbi_image_free(panorama);
		std::vector<std::string> faces = {
			StringHelper::Replace(panoramaPath, ".", "_lf."),
			StringHelper::Replace(panoramaPath, ".", "_rt."),
			StringHelper::Replace(panoramaPath, ".", "_up."),
			StringHelper::Replace(panoramaPath, ".", "_dn."),
			StringHelper::Replace(panoramaPath, ".", "_ft."),
			StringHelper::Replace(panoramaPath, ".", "_bk."),
		};
		loadFromFiles(faces, generateMipmaps);
	}
	// -----------------------------------------------------------------------
	// Load (all faces → single contiguous memory block for bgfx::createTextureCube)
	// -----------------------------------------------------------------------
	void loadFromFiles(const std::vector<std::string>& faces, bool generateMipmaps)
	{
		if (faces.size() != 6) {
			std::cerr << "[Cubemap] Need 6 faces, got " << faces.size() << std::endl;
			return;
		}
		uint16_t faceSize = 0;
		int commonWidth = 0;
		int commonHeight = 0;
		size_t totalBaseSize = 0;
		std::vector<stbi_uc*> loadedPixels(6, nullptr);
		bool loadSuccess = true;
		for (unsigned i = 0; i < 6; ++i)
		{
			std::vector<uint8_t> fileData = FileSystemEngine::ReadFileBinary(faces[i]);
			if (fileData.empty()) {
				std::cerr << "[Cubemap] File empty or not found: " << faces[i] << "\n";
				loadSuccess = false;
				continue;
			}
			int width = 0, height = 0, channels = 0;
			stbi_uc* pixels = stbi_load_from_memory(
				fileData.data(),
				static_cast<int>(fileData.size()),
				&width, &height, &channels,
				STBI_rgb_alpha
			);
			if (!pixels) {
				std::cerr << "[Cubemap] stbi_load_from_memory failed for "
					<< faces[i] << ": " << stbi_failure_reason() << std::endl;
				loadSuccess = false;
				continue;
			}
			// rotate +Y (index 2) 90° CW, –Y (index 3) 90° CCW (exact same as original)
			if (i == 2 || i == 3) {
				bool cw = (i == 2);
				stbi_uc* rotated = rotate90_rgba(pixels, width, height, cw);
				stbi_image_free(pixels);
				if (!rotated) {
					std::cerr << "[Cubemap] Rotation failed for "
						<< faces[i] << std::endl;
					loadSuccess = false;
					continue;
				}
				pixels = rotated;
				std::swap(width, height);
			}
			loadedPixels[i] = pixels;
			// dimensions from first face + consistency check
			if (i == 0) {
				commonWidth = width;
				commonHeight = height;
				faceSize = (uint16_t)width;
			}
			else if (width != commonWidth || height != commonHeight) {
				std::cerr << "[Cubemap] Face " << i << " size mismatch after processing: "
					<< width << "x" << height << " vs " << commonWidth << "x" << commonHeight << std::endl;
				loadSuccess = false;
			}
			const size_t faceBytes = static_cast<size_t>(width) * height * 4;
			totalBaseSize += faceBytes;
		}
		if (!loadSuccess || faceSize == 0) {
			for (auto p : loadedPixels) if (p) stbi_image_free(p);
			return;
		}
		// Build single contiguous RGBA8 buffer (6 faces in bgfx order)
		const size_t faceBytes = static_cast<size_t>(commonWidth) * commonHeight * 4;
		totalBaseSize = faceBytes * 6;
		std::vector<unsigned char> allData(totalBaseSize);
		size_t offset = 0;
		for (unsigned i = 0; i < 6; ++i) {
			if (loadedPixels[i]) {
				memcpy(&allData[offset], loadedPixels[i], faceBytes);
				stbi_image_free(loadedPixels[i]);
				offset += faceBytes;
			}
			else {
				memset(&allData[offset], 0, faceBytes);
				offset += faceBytes;
			}
		}
		const bgfx::Memory* mem = bgfx::copy(allData.data(), (uint32_t)totalBaseSize);
		uint64_t flags = buildFlags(generateMipmaps);
		m_handle = bgfx::createTextureCube(
			faceSize, // side length (square faces)
			generateMipmaps, // hasMips – bgfx auto-generates mipmaps if true
			1, // numLayers (regular cubemap)
			bgfx::TextureFormat::RGBA8,
			flags,
			mem
		);
		if (!bgfx::isValid(m_handle)) {
			std::cerr << "bgfx::createTextureCube failed (" << faceSize << ")\n";
			return;
		}
		// Resource statistics (identical logic to Texture.hpp)
		size_t textureSize = totalBaseSize;
		if (generateMipmaps)
			textureSize += totalBaseSize / 3;
		// Common prefix for resource name (exact same as original)
		std::string resourceName = "Cubemap";
		if (!faces.empty()) {
			resourceName = faces[0];
			for (size_t i = 1; i < faces.size(); ++i) {
				size_t minLen = std::min(resourceName.length(), faces[i].length());
				size_t j = 0;
				while (j < minLen && resourceName[j] == faces[i][j])
					++j;
				resourceName = resourceName.substr(0, j);
			}
			if (resourceName.empty() || resourceName.length() < 3)
				resourceName = "Cubemap";
		}
		ResourceStatistics::Instance().registerResource(
			ResourceType::TextureCube,
			m_handle.idx,
			textureSize,
			resourceName
		);
		bgfx::setName(m_handle, resourceName.c_str(), (int32_t)resourceName.size());
		valid = true;
	}
};