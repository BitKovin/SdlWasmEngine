#include "AndroidFileSystem.h"
#include <SDL2/SDL.h>
#include <utility>
#include <exception>

#ifdef __ANDROID__
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <json.hpp>

namespace {

    using json = nlohmann::json;

    // Lazily resolved and cached for the process lifetime -- getAssets() is a
    // JNI round trip, no need to repeat it per call.
    AAssetManager* GetAssetManager() {
        static AAssetManager* s_assetManager = nullptr;
        if (s_assetManager) return s_assetManager;

        JNIEnv* env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
        jobject activity = static_cast<jobject>(SDL_AndroidGetActivity());
        if (!env || !activity) {
            SDL_Log("AndroidFileSystem: couldn't get JNIEnv/Activity for getAssets()");
            return nullptr;
        }

        jclass activityClass = env->GetObjectClass(activity);
        jmethodID getAssetsId = env->GetMethodID(activityClass, "getAssets", "()Landroid/content/res/AssetManager;");
        jobject assetManagerLocal = env->CallObjectMethod(activity, getAssetsId);

        // AAssetManager_fromJava needs the jobject to stay valid for as long as
        // we keep using the AAssetManager* -- promote to a global ref before the
        // local ref below gets cleaned up.
        jobject assetManagerGlobal = env->NewGlobalRef(assetManagerLocal);
        s_assetManager = AAssetManager_fromJava(env, assetManagerGlobal);

        env->DeleteLocalRef(assetManagerLocal);
        env->DeleteLocalRef(activityClass);
        env->DeleteLocalRef(activity);

        if (!s_assetManager) {
            SDL_Log("AndroidFileSystem: AAssetManager_fromJava returned null");
        }
        return s_assetManager;
    }

} // namespace

// Assets have no per-entry timestamp API at all -- they're baked into the
// APK at build time and can't change without a new APK/OBB, so the APK
// file's own mtime is the closest honest stand-in when the manifest itself
// doesn't have an entry for a given path. A member function (not a free
// function) because it needs *this* to call
// NativeFileSystem::GetFileModificationTime, itself a non-static member.
uint32_t AndroidFileSystem::GetApkModTime() {
    static uint32_t s_apkModTime = 0;
    static bool s_queried = false;
    if (s_queried) return s_apkModTime;
    s_queried = true;

    JNIEnv* env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    jobject activity = static_cast<jobject>(SDL_AndroidGetActivity());
    if (!env || !activity) return 0;

    jclass activityClass = env->GetObjectClass(activity);
    jmethodID getPackageCodePathId = env->GetMethodID(activityClass, "getPackageCodePath", "()Ljava/lang/String;");
    jstring apkPathJ = static_cast<jstring>(env->CallObjectMethod(activity, getPackageCodePathId));

    if (apkPathJ) {
        const char* apkPathC = env->GetStringUTFChars(apkPathJ, nullptr);
        std::string apkPath = apkPathC;
        env->ReleaseStringUTFChars(apkPathJ, apkPathC);
        env->DeleteLocalRef(apkPathJ);

        // The APK path itself is a real absolute filesystem path, so the
        // base class's std::filesystem-based implementation handles it fine.
        s_apkModTime = NativeFileSystem::GetFileModificationTime(apkPath);
    }

    env->DeleteLocalRef(activityClass);
    env->DeleteLocalRef(activity);
    return s_apkModTime;
}

bool AndroidFileSystem::LoadManifest() {
    auto text = NativeFileSystem::ReadFile("GameData/manifest.json");
    if (!text) {
        SDL_Log("AndroidFileSystem: no GameData/manifest.json found -- falling back to AAssetManager enumeration");
        return false;
    }

    // Everything below -- parsing plus walking the 'entries' array -- is
    // wrapped in one try/catch. nlohmann::json throws on malformed JSON
    // (parse_error) and on unexpected field types (type_error), so a
    // corrupted or unexpectedly-shaped manifest degrades to the
    // AAssetManager fallback instead of taking the app down at startup,
    // same contract the old hand-rolled parser had.
    try {
        json root = json::parse(*text);
        if (!root.is_object()) {
            SDL_Log("AndroidFileSystem: failed to parse GameData/manifest.json");
            return false;
        }

        auto rootIt = root.find("root");
        std::string rootName = (rootIt != root.end() && rootIt->is_string()) ? rootIt->get<std::string>() : "GameData";
        m_manifestRootPrefix = rootName + "/";

        auto entriesIt = root.find("entries");
        if (entriesIt == root.end() || !entriesIt->is_array()) {
            SDL_Log("AndroidFileSystem: GameData/manifest.json has no 'entries' array");
            return false;
        }

        m_manifestDirs.clear();
        m_manifestFiles.clear();
        m_manifestDirChildren.clear();

        auto addChild = [this](const std::string& fullRelPath) {
            auto slash = fullRelPath.find_last_of('/');
            std::string parent = (slash == std::string::npos) ? std::string() : fullRelPath.substr(0, slash);
            std::string name = (slash == std::string::npos) ? fullRelPath : fullRelPath.substr(slash + 1);
            m_manifestDirChildren[parent].push_back(std::move(name));
            };

        for (const auto& entry : *entriesIt) {
            auto pathIt = entry.find("path");
            auto typeIt = entry.find("type");
            if (pathIt == entry.end() || typeIt == entry.end() || !pathIt->is_string()) continue;

            const std::string entryPath = pathIt->get<std::string>();
            const std::string entryType = typeIt->is_string() ? typeIt->get<std::string>() : std::string();

            if (entryType == "dir") {
                m_manifestDirs.insert(entryPath);
                addChild(entryPath);
            }
            else if (entryType == "file") {
                ManifestFileEntry fe;
                fe.size = entry.value("size", uint64_t(0));
                fe.mtime = entry.value("mtime", uint32_t(0));
                fe.archive = entry.value("archive", std::string());
                m_manifestFiles[entryPath] = std::move(fe);
                addChild(entryPath);
            }
        }

        SDL_Log("AndroidFileSystem: loaded manifest.json (%zu dirs, %zu files)",
            m_manifestDirs.size(), m_manifestFiles.size());
        return true;
    }
    catch (const std::exception& e) {
        SDL_Log("AndroidFileSystem: exception parsing GameData/manifest.json: %s", e.what());
        return false;
    }
}
#endif // __ANDROID__

AndroidFileSystem::AndroidFileSystem() {
    Init();
}

bool AndroidFileSystem::Init() {
#ifdef __ANDROID__
    // Touch it once at startup so a missing asset manager shows up in the
    // log immediately instead of silently on the first asset read.
    GetAssetManager();
    m_manifestLoaded = LoadManifest();
#else
    // Every override below this point is gated on __ANDROID__, so on a
    // non-Android build (e.g. a desktop/editor debug run) this class is
    // silently identical to NativeFileSystem: GameData/manifest.json is
    // never read, and GetFilesInPath/IsDirectory/GetFileModificationTime
    // fall straight through to std::filesystem against whatever is really
    // sitting on disk relative to the process's working directory. If a
    // directory listing looks incomplete (e.g. missing the .zip archives
    // finalize_gamedata.py folds files into), that's very likely why --
    // this isn't a manifest bug, there's just no manifest involved here.
    SDL_Log("AndroidFileSystem::Init: built without __ANDROID__ -- GameData/manifest.json "
        "is not used; falling through to NativeFileSystem's real-filesystem lookups.");
#endif
    return NativeFileSystem::Init();
}

bool AndroidFileSystem::IsAssetPath(const std::string& path) {
    return path.empty() || path.front() != '/';
}

std::string AndroidFileSystem::GetPhysicalPath(const std::string& path) {
#ifdef __ANDROID__
    // Asset-relative paths (packaged inside the APK) have no real location
    // on disk -- report "" so callers like ZipVFS::openZip() don't try to
    // zip_open() them directly and instead fall back to ReadFileBinary(),
    // which correctly reaches into the APK via SDL_RWFromFile/AAssetManager.
    if (IsAssetPath(path)) return "";
#endif
    return NativeFileSystem::GetPhysicalPath(path);
}

std::optional<std::string> AndroidFileSystem::ToManifestRelative(const std::string& path) const {
    if (!m_manifestLoaded) return std::nullopt;

    if (path.compare(0, m_manifestRootPrefix.size(), m_manifestRootPrefix) == 0) {
        return path.substr(m_manifestRootPrefix.size());
    }

    // The bare root itself, e.g. "GameData" with no trailing slash/content --
    // represents the manifest's own top level (relative path "").
    if (path + "/" == m_manifestRootPrefix) return std::string();

    return std::nullopt;
}

std::vector<std::string> AndroidFileSystem::GetFilesInPath(const std::string& path) {
#ifdef __ANDROID__
    if (auto rel = ToManifestRelative(path)) {
        auto it = m_manifestDirChildren.find(*rel);
        if (it != m_manifestDirChildren.end()) return it->second;
        return {}; // covered by the manifest, it's just empty (or doesn't exist)
    }

    if (IsAssetPath(path)) {
        // Outside the manifest's root (or manifest failed to load) --
        // best-effort fallback. Known Android platform limitation, not
        // something this code works around: AAssetManager_openDir only
        // ever lists files, never subdirectories, on some NDK/API-level
        // combinations.
        std::vector<std::string> result;
        AAssetManager* mgr = GetAssetManager();
        if (!mgr) return result;

        AAssetDir* dir = AAssetManager_openDir(mgr, path.c_str());
        if (!dir) return result;

        const char* name = nullptr;
        while ((name = AAssetDir_getNextFileName(dir)) != nullptr) {
            result.push_back(name);
        }
        AAssetDir_close(dir);
        return result;
    }
#endif
    return NativeFileSystem::GetFilesInPath(path);
}

bool AndroidFileSystem::IsDirectory(const std::string& path) {
#ifdef __ANDROID__
    if (auto rel = ToManifestRelative(path)) {
        return rel->empty() || m_manifestDirs.count(*rel) != 0;
    }

    if (IsAssetPath(path)) {
        AAssetManager* mgr = GetAssetManager();
        if (!mgr) return false;

        // AAssetManager has no direct "is this a directory" query. Try it as
        // a regular asset first; if that succeeds, it's a file, not a
        // directory. If it fails, fall back to opening it as a directory --
        // openDir succeeds even for a directory with zero entries.
        AAsset* asFile = AAssetManager_open(mgr, path.c_str(), AASSET_MODE_UNKNOWN);
        if (asFile) {
            AAsset_close(asFile);
            return false;
        }

        AAssetDir* asDir = AAssetManager_openDir(mgr, path.c_str());
        bool isDir = asDir != nullptr;
        if (asDir) AAssetDir_close(asDir);
        return isDir;
    }
#endif
    return NativeFileSystem::IsDirectory(path);
}

uint32_t AndroidFileSystem::GetFileModificationTime(const std::string& path) {
#ifdef __ANDROID__
    if (auto rel = ToManifestRelative(path)) {
        auto it = m_manifestFiles.find(*rel);
        if (it != m_manifestFiles.end()) return it->second.mtime;
        // A manifest-covered directory, or a path the manifest doesn't
        // know about -- no per-file time to give, fall through.
    }

    if (IsAssetPath(path)) {
        return GetApkModTime();
    }
#endif
    return NativeFileSystem::GetFileModificationTime(path);
}