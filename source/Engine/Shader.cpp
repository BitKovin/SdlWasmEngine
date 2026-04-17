#include "Shader.hpp"

#include "AssetRegistry.h"

#include <regex>
#include <cstring>
#include <unordered_set>

#include <ShaderManager.h>
#include <BgfxStateManager.h>

// ---------------------------------------------------------------------------
// Static member definitions
// ---------------------------------------------------------------------------
bgfx::TextureHandle Shader::s_missingTexture = BGFX_INVALID_HANDLE;
bgfx::TextureHandle Shader::s_blackTexture = BGFX_INVALID_HANDLE;

// ---------------------------------------------------------------------------
// Path resolution helpers
// ---------------------------------------------------------------------------

// Returns the platform + renderer subfolder for compiled shader binaries.
// e.g. "windows/dx11/"
static std::string GetPlatformRendererFolder()
{
    std::string platform;
#if defined(_WIN32)
    platform = "windows/";
#elif defined(__APPLE__)
    platform = "osx/";
#elif defined(__EMSCRIPTEN__)
    platform = "web/";
#elif defined(__ANDROID__)
    platform = "android/";
#elif defined(__linux__)
    platform = "linux/";
#else
    platform = "unknown/";
#endif

    std::string renderer;
    switch (bgfx::getRendererType())
    {
    case bgfx::RendererType::Direct3D11: renderer = "dx11/";   break;
    case bgfx::RendererType::Direct3D12: renderer = "dx12/";   break;
    case bgfx::RendererType::OpenGL:     renderer = "gl/";     break;
    case bgfx::RendererType::OpenGLES:   renderer = "gles/";   break;
    case bgfx::RendererType::Vulkan:     renderer = "spirv/";  break;
    case bgfx::RendererType::Metal:      renderer = "metal/";  break;
    case bgfx::RendererType::WebGPU:     renderer = "wgpu/";   break;
    default:                             renderer = "unknown/"; break;
    }

    return platform + renderer;
}

// Compiled binary: GameData/shaders/compiled/<platform>/<renderer>/<n>.bin
std::string Shader::ResolveCompiledPath(const std::string& shaderName)
{
    return "GameData/shaders/compiled/" + GetPlatformRendererFolder() + shaderName + ".bin";
}

// Source .sc file: GameData/shaders/source/<n>.sc
std::string Shader::ResolveSourcePath(const std::string& shaderName)
{
    return "GameData/shaders/source/" + shaderName + ".sc";
}

// Load a compiled binary and create a bgfx ShaderHandle.
bgfx::ShaderHandle Shader::LoadShaderBinary(const std::string& shaderName)
{
    const std::string path = ResolveCompiledPath(shaderName);
    Logger::Log("Loading shader binary: " + path);

    std::vector<uint8_t> data = FileSystemEngine::ReadFileBinary(path);
    if (data.empty())
    {
        Logger::Log("Shader::LoadShaderBinary — file empty or not found: " + path);
        return BGFX_INVALID_HANDLE;
    }

    // bgfx requires a null-terminated block
    const bgfx::Memory* mem = bgfx::alloc(uint32_t(data.size() + 1));
    memcpy(mem->data, data.data(), data.size());
    mem->data[data.size()] = '\0';

    bgfx::ShaderHandle handle = bgfx::createShader(mem);
    bgfx::setName(handle, path.c_str());
    return handle;
}

// ---------------------------------------------------------------------------
// Built-in uniform filter
// ---------------------------------------------------------------------------
bool Shader::IsBgfxBuiltin(const std::string& uname)
{
    static const std::unordered_set<std::string> builtins =
    {
        "u_view", "u_invView",
        "u_proj", "u_invProj",
        "u_viewProj", "u_invViewProj",
        "u_modelView", "u_invModelView",
        "u_modelViewProj",
        "u_viewRect", "u_viewTexel",
        "u_alphaRef4", "u_model"
    };
    return builtins.count(uname) != 0;
}

// ---------------------------------------------------------------------------
// Uniform reflection
// ---------------------------------------------------------------------------
void Shader::ReflectUniforms(bgfx::ShaderHandle vsh, bgfx::ShaderHandle fsh)
{
    m_uniforms.clear();

    //  NEW: get real sampler slots from shader source
    auto samplerSlots = ParseAllSamplerSlots();

    bgfx::ShaderHandle stages[2] = { vsh, fsh };

    for (bgfx::ShaderHandle stage : stages)
    {
        if (!bgfx::isValid(stage)) continue;

        uint16_t count = bgfx::getShaderUniforms(stage, nullptr, 0);
        if (count == 0) continue;

        std::vector<bgfx::UniformHandle> handles(count);
        bgfx::getShaderUniforms(stage, handles.data(), count);

        for (uint16_t i = 0; i < count; ++i)
        {
            bgfx::UniformInfo info;
            bgfx::getUniformInfo(handles[i], info);

            const std::string uname(info.name);

            if (IsBgfxBuiltin(uname))   continue;
            if (m_uniforms.count(uname)) continue;

            UniformMeta u;
            u.handle = bgfx::createUniform(info.name, info.type, info.num);
            u.num = info.num;

            switch (info.type)
            {
            case bgfx::UniformType::Sampler:
            {
                u.kind = UniformMeta::Kind::Sampler;

                auto itSlot = samplerSlots.find(uname);
                if (itSlot != samplerSlots.end())
                {
                    u.samplerSlot = itSlot->second;
                }
                else
                {
                    Logger::Log("Shader warning: sampler \"" + uname +
                        "\" has no explicit slot. Defaulting to 0.");
                    u.samplerSlot = 0;
                }

                // Optional safety check
                if (u.samplerSlot > 15)
                {
                    Logger::Log("Shader error: sampler \"" + uname +
                        "\" uses invalid slot > 15");
                }

                break;
            }

            case bgfx::UniformType::Vec4:
                u.kind = UniformMeta::Kind::Vec4;
                break;

            case bgfx::UniformType::Mat3:
                u.kind = UniformMeta::Kind::Mat3;
                break;

            case bgfx::UniformType::Mat4:
                u.kind = UniformMeta::Kind::Mat4;
                break;

            default:
                bgfx::destroy(u.handle);
                continue;
            }

            m_uniforms.emplace(uname, u);

            // Debug (optional, very useful)
            if (u.kind == UniformMeta::Kind::Sampler)
            {
                Logger::Log("Sampler: " + uname +
                    " → slot " + std::to_string(u.samplerSlot));
            }
        }

    }


}

// ---------------------------------------------------------------------------
// Factory
// ---------------------------------------------------------------------------
Shader* Shader::FromFiles(const char* vsName, const char* fsName)
{
    bgfx::ShaderHandle vsh = LoadShaderBinary(vsName);
    bgfx::ShaderHandle fsh = LoadShaderBinary(fsName);

    if (!bgfx::isValid(vsh) || !bgfx::isValid(fsh))
    {
        Logger::Log(std::string("Shader::FromFiles — failed to load: ") + vsName + " / " + fsName);
        if (bgfx::isValid(vsh)) bgfx::destroy(vsh);
        if (bgfx::isValid(fsh)) bgfx::destroy(fsh);
        return nullptr;
    }

    Shader* s = new Shader();
    s->m_vsName = vsName;
    s->m_fsName = fsName;

    // Source paths for @texture annotation parsing
    s->m_vsSourcePath = ResolveSourcePath(vsName);
    s->m_fsSourcePath = ResolveSourcePath(fsName);

    // createProgram(vsh, fsh, destroyShaders=true)
    // bgfx takes ownership of shader handles when destroyShaders is true.
    // We must reflect BEFORE handing them to createProgram.
    s->ReflectUniforms(vsh, fsh);

    s->m_program = bgfx::createProgram(vsh, fsh, /*destroyShaders=*/true);

    if (!bgfx::isValid(s->m_program))
    {
        Logger::Log(std::string("Shader::FromFiles — bgfx::createProgram failed for: ") + vsName + " / " + fsName);
        delete s;
        return nullptr;
    }

    // Parse @texture annotations from source files
    s->textureBindings = s->ParseAllTextureBindings();

    EnsureMissingTexture();

    return s;
}

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
Shader::~Shader()
{
    // Destroy all created uniform handles
    for (auto& [n, u] : m_uniforms)
    {
        if (bgfx::isValid(u.handle))
            bgfx::destroy(u.handle);
    }

    if (bgfx::isValid(m_program))
        bgfx::destroy(m_program);
}

// ---------------------------------------------------------------------------
// Hot reload
// ---------------------------------------------------------------------------
bool Shader::Reload()
{
    bgfx::ShaderHandle vsh = LoadShaderBinary(m_vsName);
    bgfx::ShaderHandle fsh = LoadShaderBinary(m_fsName);

    if (!bgfx::isValid(vsh) || !bgfx::isValid(fsh))
    {
        Logger::Log("Shader::Reload — binary load failed for: " + m_vsName + " / " + m_fsName);
        if (bgfx::isValid(vsh)) bgfx::destroy(vsh);
        if (bgfx::isValid(fsh)) bgfx::destroy(fsh);
        return false;
    }

    bgfx::ProgramHandle newProgram = bgfx::createProgram(vsh, fsh, /*destroyShaders=*/true);
    if (!bgfx::isValid(newProgram))
    {
        Logger::Log("Shader::Reload — bgfx::createProgram failed, keeping old program.");
        return false;
    }

    // Destroy old resources
    for (auto& [n, u] : m_uniforms)
        if (bgfx::isValid(u.handle)) bgfx::destroy(u.handle);

    if (bgfx::isValid(m_program))
        bgfx::destroy(m_program);

    // Re-reflect (handles may have changed, array sizes may differ)
    // We need fresh handles for reflection before destroyShaders consumed them,
    // so load again just for reflection then destroy those copies.
    {
        bgfx::ShaderHandle vsRef = LoadShaderBinary(m_vsName);
        bgfx::ShaderHandle fsRef = LoadShaderBinary(m_fsName);
        ReflectUniforms(vsRef, fsRef);
        if (bgfx::isValid(vsRef)) bgfx::destroy(vsRef);
        if (bgfx::isValid(fsRef)) bgfx::destroy(fsRef);
    }

    m_program = newProgram;
    textureBindings = ParseAllTextureBindings();

    // Note: m_uniformBuffer and m_textureBuffer are intentionally preserved.
    // The new uniform handles have the same names, so buffered values will be
    // re-applied correctly on the next Submit() call.

    Logger::Log("Shader::Reload succeeded for: " + m_vsName + " / " + m_fsName);
    return true;
}

// ---------------------------------------------------------------------------
// Uniform lookup
// ---------------------------------------------------------------------------
const UniformMeta* Shader::FindUniform(const std::string& uname) const
{
    auto it = m_uniforms.find(uname);
    if (it != m_uniforms.end())
    {
        // Guard against a handle that was destroyed externally or never created
        if (!bgfx::isValid(it->second.handle))
        {
            Logger::Log("Shader error: uniform \"" + uname + "\" has invalid handle in " + m_vsName);
            return nullptr;
        }
        return &it->second;
    }

    if (!AllowMissingUniforms)
        Logger::Log("Shader warning: uniform \"" + uname + "\" not found in " + m_vsName);

    return nullptr;
}

// ---------------------------------------------------------------------------
// Internal helper: safe num clamped against declared capacity
// ---------------------------------------------------------------------------
static uint16_t SafeUniformNum(const UniformMeta* u, size_t requested, const std::string& uname)
{
    if (requested == 0) return 0;

    // uint16_t overflow guard
    constexpr size_t kMax = std::numeric_limits<uint16_t>::max();
    if (requested > kMax)
    {
        Logger::Log("Shader error: uniform \"" + uname + "\" requested count " +
            std::to_string(requested) + " exceeds uint16_t max, clamping");
        requested = kMax;
    }

    // Clamp to declared capacity — exceeding this corrupts the uniform cache
    if (u->num > 0 && requested > static_cast<size_t>(u->num))
    {
        Logger::Log("Shader error: uniform \"" + uname + "\" requested " +
            std::to_string(requested) + " slots but was created with " +
            std::to_string(u->num) + ", clamping");
        requested = u->num;
    }

    return static_cast<uint16_t>(requested);
}

// ---------------------------------------------------------------------------
// Scalar / small-type setters  (all map to Vec4 with padding)
// ---------------------------------------------------------------------------
void Shader::SetUniform(const std::string& uname, int value)
{
    SetUniform(uname, static_cast<float>(value));
}

void Shader::SetUniform(const std::string& uname, bool value)
{
    SetUniform(uname, value ? 1.0f : 0.0f);
}

void Shader::SetUniform(const std::string& uname, float value)
{
    if (!FindUniform(uname)) return;

    auto& entry = m_uniformBuffer[uname];
    entry.data = { value, 0.0f, 0.0f, 0.0f };
    entry.num = 1;
}

void Shader::SetUniform(const std::string& uname, const glm::vec2& value)
{
    if (!FindUniform(uname)) return;

    auto& entry = m_uniformBuffer[uname];
    entry.data = { value.x, value.y, 0.0f, 0.0f };
    entry.num = 1;
}

void Shader::SetUniform(const std::string& uname, const glm::vec3& value)
{
    if (!FindUniform(uname)) return;

    auto& entry = m_uniformBuffer[uname];
    entry.data = { value.x, value.y, value.z, 0.0f };
    entry.num = 1;
}

void Shader::SetUniform(const std::string& uname, const glm::vec4& value)
{
    if (!FindUniform(uname)) return;

    auto& entry = m_uniformBuffer[uname];
    entry.data.assign(glm::value_ptr(value), glm::value_ptr(value) + 4);
    entry.num = 1;
}

// mat2 → pack column-major into one Vec4: [m00, m10, m01, m11]
void Shader::SetUniform(const std::string& uname, const glm::mat2& value)
{
    if (!FindUniform(uname)) return;

    auto& entry = m_uniformBuffer[uname];
    entry.data = {
        value[0][0], value[0][1],
        value[1][0], value[1][1]
    };
    entry.num = 1;
}

void Shader::SetUniform(const std::string& uname, const glm::mat3& value)
{
    if (!FindUniform(uname)) return;

    // bgfx Mat3 layout: 3 Vec4 rows, column-major source, last component of each row is padding.
    const float* src = glm::value_ptr(value); // column-major: col0[3], col1[3], col2[3]

    auto& entry = m_uniformBuffer[uname];
    entry.data.resize(12);
    float* d = entry.data.data();
    d[0] = src[0]; d[1] = src[3]; d[2] = src[6]; d[3] = 0.0f; // row 0
    d[4] = src[1]; d[5] = src[4]; d[6] = src[7]; d[7] = 0.0f; // row 1
    d[8] = src[2]; d[9] = src[5]; d[10] = src[8]; d[11] = 0.0f; // row 2
    entry.num = 1;
}

void Shader::SetUniform(const std::string& uname, const glm::mat4& value)
{
    if (!FindUniform(uname)) return;

    auto& entry = m_uniformBuffer[uname];
    entry.data.assign(glm::value_ptr(value), glm::value_ptr(value) + 16);
    entry.num = 1;
}

// ---------------------------------------------------------------------------
// Array setters — each element is padded to a Vec4 slot
// ---------------------------------------------------------------------------
void Shader::SetUniform(const std::string& uname, const std::vector<float>& values)
{
    const UniformMeta* u = FindUniform(uname);
    if (!u || values.empty()) return;

    const uint16_t num = SafeUniformNum(u, values.size(), uname);
    if (num == 0) return;

    auto& entry = m_uniformBuffer[uname];
    entry.data.assign(num * 4, 0.0f);
    for (uint16_t i = 0; i < num; ++i)
        entry.data[i * 4] = values[i];
    entry.num = num;
}

void Shader::SetUniform(const std::string& uname, const std::vector<glm::vec2>& values)
{
    const UniformMeta* u = FindUniform(uname);
    if (!u || values.empty()) return;

    const uint16_t num = SafeUniformNum(u, values.size(), uname);
    if (num == 0) return;

    auto& entry = m_uniformBuffer[uname];
    entry.data.assign(num * 4, 0.0f);
    for (uint16_t i = 0; i < num; ++i)
    {
        entry.data[i * 4 + 0] = values[i].x;
        entry.data[i * 4 + 1] = values[i].y;
    }
    entry.num = num;
}

void Shader::SetUniform(const std::string& uname, const std::vector<glm::vec3>& values)
{
    const UniformMeta* u = FindUniform(uname);
    if (!u || values.empty()) return;

    const uint16_t num = SafeUniformNum(u, values.size(), uname);
    if (num == 0) return;

    auto& entry = m_uniformBuffer[uname];
    entry.data.assign(num * 4, 0.0f);
    for (uint16_t i = 0; i < num; ++i)
    {
        entry.data[i * 4 + 0] = values[i].x;
        entry.data[i * 4 + 1] = values[i].y;
        entry.data[i * 4 + 2] = values[i].z;
    }
    entry.num = num;
}

void Shader::SetUniform(const std::string& uname, const std::vector<glm::vec4>& values)
{
    const UniformMeta* u = FindUniform(uname);
    if (!u || values.empty()) return;

    const uint16_t num = SafeUniformNum(u, values.size(), uname);
    if (num == 0) return;

    static_assert(sizeof(glm::vec4) == 4 * sizeof(float), "glm::vec4 layout assumption broken");

    auto& entry = m_uniformBuffer[uname];
    entry.data.assign(glm::value_ptr(values[0]), glm::value_ptr(values[0]) + num * 4);
    entry.num = num;
}

// mat2 array: each mat2 → one Vec4 slot
void Shader::SetUniform(const std::string& uname, const std::vector<glm::mat2>& values)
{
    const UniformMeta* u = FindUniform(uname);
    if (!u || values.empty()) return;

    const uint16_t num = SafeUniformNum(u, values.size(), uname);
    if (num == 0) return;

    auto& entry = m_uniformBuffer[uname];
    entry.data.resize(num * 4);
    for (uint16_t i = 0; i < num; ++i)
    {
        entry.data[i * 4 + 0] = values[i][0][0];
        entry.data[i * 4 + 1] = values[i][0][1];
        entry.data[i * 4 + 2] = values[i][1][0];
        entry.data[i * 4 + 3] = values[i][1][1];
    }
    entry.num = num;
}

// mat3 array: each mat3 → 3 Vec4 rows (12 floats)
void Shader::SetUniform(const std::string& uname, const std::vector<glm::mat3>& values)
{
    const UniformMeta* u = FindUniform(uname);
    if (!u || values.empty()) return;

    const uint16_t num = SafeUniformNum(u, values.size(), uname);
    if (num == 0) return;

    auto& entry = m_uniformBuffer[uname];
    entry.data.assign(num * 12, 0.0f);
    for (uint16_t i = 0; i < num; ++i)
    {
        const float* src = glm::value_ptr(values[i]);
        float* dst = entry.data.data() + i * 12;
        // row 0
        dst[0] = src[0]; dst[1] = src[3]; dst[2] = src[6]; dst[3] = 0.0f;
        // row 1
        dst[4] = src[1]; dst[5] = src[4]; dst[6] = src[7]; dst[7] = 0.0f;
        // row 2
        dst[8] = src[2]; dst[9] = src[5]; dst[10] = src[8]; dst[11] = 0.0f;
    }
    entry.num = num;
}

void Shader::SetUniform(const std::string& uname, const std::vector<glm::mat4>& values)
{
    const UniformMeta* u = FindUniform(uname);
    if (!u || values.empty()) return;

    const uint16_t num = SafeUniformNum(u, values.size(), uname);
    if (num == 0) return;

    static_assert(sizeof(glm::mat4) == 16 * sizeof(float), "glm::mat4 layout assumption broken");

    auto& entry = m_uniformBuffer[uname];
    entry.data.assign(glm::value_ptr(values[0]), glm::value_ptr(values[0]) + num * 16);
    entry.num = num;
}

// ---------------------------------------------------------------------------
// Texture setters — buffer the binding; applied in FlushBuffers()
// ---------------------------------------------------------------------------
void Shader::SetTexture(const std::string& uname, bgfx::TextureHandle texture)
{
    const UniformMeta* u = FindUniform(uname);
    if (!u) return;

	if (bgfx::isValid(texture))
    {
        // Texture handle is valid — use it directly.
    }
    else
    {

        Logger::Log("Shader warning: invalid texture handle for \"" + uname + "\" in " + m_vsName +
			", using missing texture fallback");

        // Invalid handle (including 0) — bind black texture as a safe default.
        texture = s_missingTexture;
    }

    auto& entry = m_textureBuffer[uname];
    entry.slot = u->samplerSlot;
    entry.samplerHandle = u->handle;
    entry.texture = texture;
}

void Shader::SetTexture(const std::string& uname, int texture)
{
    const UniformMeta* u = FindUniform(uname);
    if (!u) return;

    bgfx::TextureHandle handle = (texture == 0)
        ? s_blackTexture
        : bgfx::TextureHandle{ static_cast<uint16_t>(texture) };

    auto& entry = m_textureBuffer[uname];
    entry.slot = u->samplerSlot;
    entry.samplerHandle = u->handle;
    entry.texture = handle;
}

void Shader::SetTexture(const hashed_string& uname, Texture* texture)
{
    const UniformMeta* u = FindUniform(uname.str());
    if (!u) return;

    bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;

    if (texture && texture->valid)
    {
        handle = texture->getHandle();
        if (!bgfx::isValid(handle))
        {
            Logger::Log("Shader warning: Texture object reports valid but handle is invalid for \"" +
                uname.str() + "\" in " + m_vsName + ", using missing texture");
            handle = BGFX_INVALID_HANDLE;
        }
    }
    else
    {
        SetTexture(uname.str(), 0); // bind black texture for null/invalid Texture objects
        return;
    }

    if (!bgfx::isValid(handle))
    {
        EnsureMissingTexture();
        handle = s_missingTexture;
        if (!bgfx::isValid(handle)) return; // EnsureMissingTexture failed somehow
    }

    auto& entry = m_textureBuffer[uname.str()];
    entry.slot = u->samplerSlot;
    entry.samplerHandle = u->handle;
    entry.texture = handle;
}

void Shader::SetCubemapTexture(const std::string& uname, bgfx::TextureHandle texture)
{
    SetTexture(uname, texture);
}

// ---------------------------------------------------------------------------
// FlushBuffers — push all buffered state to bgfx (called by Submit)
// ---------------------------------------------------------------------------
void Shader::FlushBuffers() const
{
    // Flush uniform values
    for (const auto& [uname, entry] : m_uniformBuffer)
    {
        auto it = m_uniforms.find(uname);
        if (it == m_uniforms.end()) continue;

        const UniformMeta& u = it->second;
        if (!bgfx::isValid(u.handle)) continue;
        if (entry.data.empty())       continue;

        bgfx::setUniform(u.handle, entry.data.data(), entry.num);
    }

    // Flush texture bindings.
    // For every reflected sampler uniform, either use the buffered binding or
    // fall back to the default black texture so the slot is never left unbound.
    for (const auto& [uname, u] : m_uniforms)
    {
        if (u.kind != UniformMeta::Kind::Sampler) continue;
        if (!bgfx::isValid(u.handle))             continue;

        auto it = m_textureBuffer.find(uname);
        if (it != m_textureBuffer.end())
        {
            bgfx::setTexture(it->second.slot, it->second.samplerHandle, it->second.texture);
        }
        else
        {
            // No texture was set for this sampler — bind black as a safe default.
            //bgfx::setTexture(u.samplerSlot, u.handle, s_blackTexture);
        }
    }
}

// ---------------------------------------------------------------------------
// Missing texture fallback
// ---------------------------------------------------------------------------
void Shader::EnsureMissingTexture()
{
    if (!bgfx::isValid(s_missingTexture))
    {
        // 2×2 magenta checkerboard — used when a texture is set but invalid
        const uint32_t M = 0xFFFF00FF;
        const uint32_t B = 0xFF000000;
        uint32_t pixels[4] = { M, B, B, M };

        const bgfx::Memory* mem = bgfx::copy(pixels, sizeof(pixels));
        s_missingTexture = bgfx::createTexture2D(
            2, 2, false, 1,
            bgfx::TextureFormat::RGBA8,
            BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT,
            mem
        );

        if (!bgfx::isValid(s_missingTexture))
            Logger::Log("Shader error: failed to create missing texture fallback");
        else
            bgfx::setName(s_missingTexture, "missing_texture");
    }

    if (!bgfx::isValid(s_blackTexture))
    {
        // 1×1 opaque black — bound to any sampler that has no explicit texture set
        uint32_t pixel = 0xFF000000;

        const bgfx::Memory* mem = bgfx::copy(&pixel, sizeof(pixel));
        s_blackTexture = bgfx::createTexture2D(
            1, 1, false, 1,
            bgfx::TextureFormat::RGBA8,
            BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT,
            mem
        );

        if (!bgfx::isValid(s_blackTexture))
            Logger::Log("Shader error: failed to create black texture fallback");
        else
            bgfx::setName(s_blackTexture, "default_black_texture");
    }
}

// ---------------------------------------------------------------------------
// UseProgram / Submit
// ---------------------------------------------------------------------------
void Shader::UseProgram()
{
    EnsureMissingTexture();

    // Clear all buffered state so stale values from the previous draw don't
    // bleed into this one. ApplyTextureBindings() will repopulate the texture
    // buffer from @texture annotations immediately after.
    m_uniformBuffer.clear();
    m_textureBuffer.clear();

    for (auto& [name, def] : m_defaultUniforms)
    {
        // Only apply if the uniform actually exists in the reflected metadata
        if (m_uniforms.count(name))
        {
            auto& entry = m_uniformBuffer[name];
            entry.data = def.data;
            entry.num = 1;
        }
    }

    ApplyTextureBindings();  // writes into m_textureBuffer via SetTexture()
}

void Shader::Submit(uint16_t viewId) const
{


    ShaderManager::RegisterPSO(BgfxStateManager::GetState(), m_vsSourcePath, m_fsSourcePath);
    
    FlushBuffers();                    // setUniform + setTexture for everything buffered
    bgfx::submit(viewId, m_program);
}

// ---------------------------------------------------------------------------
// @texture annotation parsing
// ---------------------------------------------------------------------------

// Parse a single source string for @texture annotations in both forms:
//   uniform sampler2D  u_name;        // @texture path/to/file.png
//   uniform samplerCube u_env;        // @texture path/to/cubemap
//   SAMPLER2D(u_name, slot);          // @texture path/to/file.png
//   SAMPLERCUBE(u_env, slot);         // @texture path/to/cubemap
std::unordered_map<hashed_string, std::string>
Shader::ParseTextureBindings(const std::string& sourceCode)
{
    std::unordered_map<hashed_string, std::string> result;

    // Branch 1: uniform sampler2D/samplerCube u_name;  // @texture <path>
    std::regex reUniform(R"(uniform\s+(?:sampler2D|samplerCube)\s+(\w+)\s*;[^\n]*@texture\s+([^\s]+))");
    // Branch 2: SAMPLER2D(u_name, slot);               // @texture <path>
    std::regex reMacro(R"(SAMPLER(?:2D|CUBE)\s*\(\s*(\w+)\s*,[^)]*\)\s*;[^\n]*@texture\s+([^\s]+))");

    auto scan = [&](const std::regex& re)
        {
            std::smatch match;
            auto it = sourceCode.cbegin();
            while (std::regex_search(it, sourceCode.cend(), match, re))
            {
                result[match[1].str()] = match[2].str();
                it = match.suffix().first;
            }
        };

    scan(reUniform);
    scan(reMacro);

    return result;
}

std::unordered_map<std::string, uint8_t>
Shader::ParseSamplerSlots(const std::string& source)
{
    std::unordered_map<std::string, uint8_t> result;

    // Matches: SAMPLER2D(name, slot) or SAMPLERCUBE(name, slot)
    std::regex re(R"(SAMPLER(?:2D|CUBE)\s*\(\s*(\w+)\s*,\s*(\d+)\s*\))");

    std::smatch match;
    auto it = source.cbegin();

    while (std::regex_search(it, source.cend(), match, re))
    {
        const std::string name = match[1].str();
        const uint8_t slot = static_cast<uint8_t>(std::stoi(match[2].str()));

        result[name] = slot;

        it = match.suffix().first;
    }

    return result;
}

std::unordered_map<std::string, uint8_t>
Shader::ParseAllSamplerSlots() 
{
    std::unordered_map<std::string, uint8_t> result;

    for (const std::string& path : { m_vsSourcePath, m_fsSourcePath })
    {
        if (path.empty()) continue;

        const std::string source = FileSystemEngine::ReadFile(path);
        if (source.empty()) continue;

        auto parsed = ParseSamplerSlots(source);

        for (auto& [k, v] : parsed)
        {
            result[k] = v; // Fragment shader overrides vertex if duplicate
        }
    }

    return result;
}

void Shader::ParseDefaultUniforms(const std::string& source)
{
    // Regex matches: uniform <type> <name>; // @ (<f>, <f>, <f>, <f>)
    // Group 1: name, Group 2-5: float values
    std::regex reDefault(R"(uniform\s+\w+\s+(\w+)\s*;[^\n]*\/\/\s*@\s*\(\s*([^,]+)\s*,\s*([^,]+)\s*,\s*([^,]+)\s*,\s*([^)]+)\s*\))");

    std::smatch match;
    auto it = source.cbegin();
    while (std::regex_search(it, source.cend(), match, reDefault))
    {
        std::string name = match[1].str();

        DefaultUniformValue val;
        try {
            val.data = {
                std::stof(match[2].str()),
                std::stof(match[3].str()),
                std::stof(match[4].str()),
                std::stof(match[5].str())
            };
            m_defaultUniforms[name] = val;
        }
        catch (const std::exception& e) {
            Logger::Log("Shader error: Failed to parse default value for " + name + " - " + e.what());
        }

        it = match.suffix().first;
    }
}

// Read both .sh source files and merge their @texture annotations.
std::unordered_map<hashed_string, std::string>
Shader::ParseAllTextureBindings()
{
    std::unordered_map<hashed_string, std::string> result;

    for (const std::string& path : { m_vsSourcePath, m_fsSourcePath })
    {
        if (path.empty()) continue;

        const std::string source = FileSystemEngine::ReadFile(path);
        if (source.empty()) continue;

		ParseDefaultUniforms(source);

        auto parsed = ParseTextureBindings(source);
        for (auto& [k, v] : parsed)
            result[k] = v;   // later stage wins on collision
    }

    return result;
}

// Apply all auto-bindings: resolve each path via AssetRegistry, call SetTexture.
// For each binding, if a uniform named <samplerName>_size exists in the shader,
// automatically set it to vec4(width, height, 0, 0) so shaders never need the
// caller to manually push texture dimensions (e.g. noiseTexture_size, LutTexture_size).
void Shader::ApplyTextureBindings()
{
    for (const auto& [uniformName, path] : textureBindings)
    {
        Texture* tex = AssetRegistry::GetTextureFromFile(path);
        SetTexture(uniformName, tex);

        // Auto-set <name>_size if the shader declared it and the texture is valid
        const std::string sizeUniform = uniformName.str() + "_size";
        if (tex && tex->valid && m_uniforms.count(sizeUniform))
        {
            const glm::vec4 size(
                static_cast<float>(tex->width),
                static_cast<float>(tex->height),
                0.0f, 0.0f
            );
            SetUniform(sizeUniform, size);
        }
    }
}