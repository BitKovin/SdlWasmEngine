// glb_embedded_textures.h
// Drop this header into plugins/model/ and #include it in model.cpp
// Also usable from libs/picomodel/pm_assimp.cpp
//
// Usage:
//   #include "glb_embedded_textures.h"
//
//   // After aiImportFile / Importer::ReadFile:
//   GlbEmbeddedTextures emb(scene, modelFilePath);
//
//   // When resolving a per-mesh texture path:
//   std::string shaderName = emb.resolve(rawTexturePath);
//
// The resolved path is either:
//   - the original string unchanged (external texture or empty)
//   - an absolute path to a temp file on disk (embedded texture)

#pragma once

#include <cstdio>
#include <cstring>
#include <string>
#include <map>

#include <assimp/scene.h>       // aiScene, aiTexture, aiTexel

// ---------------------------------------------------------------------------
// Platform helpers
// ---------------------------------------------------------------------------

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  define GLB_PATH_SEP "\\"
static inline std::string glb_getTempDir()
{
    char buf[MAX_PATH] = {};
    DWORD len = GetTempPathA(MAX_PATH, buf);
    if (len > 0 && (buf[len-1] == '\\' || buf[len-1] == '/'))
        buf[len-1] = '\0';
    return buf;
}
static inline void glb_mkDir(const std::string& p)
{
    CreateDirectoryA(p.c_str(), nullptr);
}
#else
#  include <sys/stat.h>
#  define GLB_PATH_SEP "/"
static inline std::string glb_getTempDir()
{
    const char* t = getenv("TMPDIR");
    if (!t) t = getenv("TMP");
    if (!t) t = "/tmp";
    // strip trailing slash
    std::string s(t);
    while (!s.empty() && (s.back() == '/' || s.back() == '\\'))
        s.pop_back();
    return s;
}
static inline void glb_mkDir(const std::string& p)
{
    mkdir(p.c_str(), 0700);
}
#endif

// ---------------------------------------------------------------------------
// Write raw aiTexel array (BGRA, mWidth x mHeight) as 32-bit TGA
// ---------------------------------------------------------------------------
static inline bool glb_writeTGA(const std::string& path,
                                const aiTexel* pixels,
                                unsigned int w, unsigned int h)
{
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) return false;

    unsigned char hdr[18] = {};
    hdr[2]  = 2;                            // uncompressed truecolour
    hdr[12] = (unsigned char)(w        & 0xFF);
    hdr[13] = (unsigned char)((w >> 8) & 0xFF);
    hdr[14] = (unsigned char)(h        & 0xFF);
    hdr[15] = (unsigned char)((h >> 8) & 0xFF);
    hdr[16] = 32;                           // 32 bpp
    hdr[17] = 8;                            // 8-bit alpha, lower-left origin
    fwrite(hdr, 1, 18, f);
    // aiTexel = { b, g, r, a } — exactly what TGA expects
    fwrite(pixels, 4, (size_t)w * h, f);
    fclose(f);
    return true;
}

// ---------------------------------------------------------------------------
// Simple djb2 hash of a C-string → 8 hex chars (no collision concern needed)
// ---------------------------------------------------------------------------
static inline std::string glb_hashPath(const char* s)
{
    unsigned long h = 5381;
    for (; *s; ++s)
        h = ((h << 5) + h) ^ (unsigned char)*s;
    char buf[16];
    snprintf(buf, sizeof(buf), "%08lx", h);
    return buf;
}

// ---------------------------------------------------------------------------
// Check if a path is absolute (to avoid running it through VFS)
// ---------------------------------------------------------------------------
static inline bool glb_isAbsolute(const std::string& p)
{
    if (p.empty()) return false;
#ifdef _WIN32
    // "C:\..." or "C:/..." or UNC "\\server\..."
    return (p.size() >= 2 && p[1] == ':') ||
           (p[0] == '\\' && p.size() >= 2 && p[1] == '\\');
#else
    return p[0] == '/';
#endif
}

// ===========================================================================
// GlbEmbeddedTextures
// ===========================================================================

class GlbEmbeddedTextures
{
public:
    // Construct and immediately extract all embedded textures to disk.
    // modelPath  — absolute path to the .glb / .gltf file being loaded.
    //              Used to create a stable per-model temp sub-directory.
    GlbEmbeddedTextures(const aiScene* scene, const char* modelPath)
    {
        if (!scene || scene->mNumTextures == 0)
            return;

        // Root temp dir:  <OS_TEMP>/nrc_glb_tex/<modelhash>/
        std::string root = glb_getTempDir()
                         + GLB_PATH_SEP "nrc_glb_tex";
        glb_mkDir(root);

        std::string subDir = root
                           + GLB_PATH_SEP
                           + glb_hashPath(modelPath ? modelPath : "unknown");
        glb_mkDir(subDir);

        for (unsigned int i = 0; i < scene->mNumTextures; i++) {
            const aiTexture* tex = scene->mTextures[i];
            std::string filePath;

            if (tex->mHeight == 0) {
                // Compressed blob (PNG / JPEG / WebP / …)
                // achFormatHint is e.g. "png", "jpg", "webp", or ""
                const char* ext = (tex->achFormatHint[0] != '\0')
                                  ? tex->achFormatHint : "bin";

                char name[64];
                snprintf(name, sizeof(name), "tex%u.%s", i, ext);
                filePath = subDir + GLB_PATH_SEP + name;

                // Only write if not already on disk from a previous load
                if (!fileExists(filePath)) {
                    FILE* f = fopen(filePath.c_str(), "wb");
                    if (f) {
                        fwrite(tex->pcData, 1, tex->mWidth, f);
                        fclose(f);
                    }
                }
            } else {
                // Raw ARGB pixels
                char name[64];
                snprintf(name, sizeof(name), "tex%u.tga", i);
                filePath = subDir + GLB_PATH_SEP + name;

                if (!fileExists(filePath)) {
                    glb_writeTGA(filePath,
                        reinterpret_cast<const aiTexel*>(tex->pcData),
                        tex->mWidth, tex->mHeight);
                }
            }

            if (!filePath.empty())
                m_paths[i] = filePath;
        }
    }

    // Resolve a material texture path.
    // "*N"  → absolute path to the extracted temp file for embedded index N
    // else  → original string unchanged (external file reference)
    std::string resolve(const char* rawPath) const
    {
        if (!rawPath || rawPath[0] == '\0')
            return "";

        if (rawPath[0] == '*') {
            char* end = nullptr;
            long idx = strtol(rawPath + 1, &end, 10);
            if (end && end != rawPath + 1) {
                auto it = m_paths.find((int)idx);
                if (it != m_paths.end())
                    return it->second;
            }
        }
        return rawPath;
    }

    bool empty() const { return m_paths.empty(); }

private:
    std::map<int, std::string> m_paths;

    static bool fileExists(const std::string& p) {
        FILE* f = fopen(p.c_str(), "rb");
        if (f) { fclose(f); return true; }
        return false;
    }
};
