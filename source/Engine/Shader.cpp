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
bgfx::TextureHandle Shader::s_blackTexture   = BGFX_INVALID_HANDLE;

// ---------------------------------------------------------------------------
// Path resolution helpers
// ---------------------------------------------------------------------------

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

std::string Shader::ResolveCompiledPath(const std::string& shaderName)
{
    return "GameData/shaders/compiled/" + GetPlatformRendererFolder() + shaderName + ".bin";
}

std::string Shader::ResolveSourcePath(const std::string& shaderName)
{
    return "GameData/shaders/source/" + shaderName + ".sc";
}

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
// Populates m_uniformIndex, m_uniformList, m_samplerIndices, and sizes the
// two flat buffers to match.
// ---------------------------------------------------------------------------
void Shader::ReflectUniforms(bgfx::ShaderHandle vsh, bgfx::ShaderHandle fsh)
{
    m_uniformIndex.clear();
    m_uniformList.clear();
    m_samplerIndices.clear();

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

            if (IsBgfxBuiltin(uname))        continue;
            if (m_uniformIndex.count(uname)) continue; // already added from VS stage

            UniformMeta u;
            u.handle = bgfx::createUniform(info.name, info.type, info.num);
            u.num    = info.num;
            u.index  = static_cast<uint16_t>(m_uniformList.size());

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

                if (u.samplerSlot > 15)
                    Logger::Log("Shader error: sampler \"" + uname + "\" uses invalid slot > 15");

                m_samplerIndices.push_back(u.index);
                Logger::Log("Sampler: " + uname + " → slot " + std::to_string(u.samplerSlot));
                break;
            }
            case bgfx::UniformType::Vec4: u.kind = UniformMeta::Kind::Vec4; break;
            case bgfx::UniformType::Mat3: u.kind = UniformMeta::Kind::Mat3; break;
            case bgfx::UniformType::Mat4: u.kind = UniformMeta::Kind::Mat4; break;
            default:
                bgfx::destroy(u.handle);
                continue;
            }

            m_uniformIndex.emplace(uname, u.index);
            m_uniformList.push_back(u);
        }
    }

    // Size flat buffers to match the reflected uniform list.
    // Both are indexed 1:1 with m_uniformList — no string key needed at flush time.
    m_uniformBuffer.assign(m_uniformList.size(), UniformEntry{});
    m_textureBuffer.assign(m_uniformList.size(), TextureEntry{});
}

// ---------------------------------------------------------------------------
// BuildResolvedData
// Must be called after ReflectUniforms + ParseAllTextureBindings.
// Converts string-keyed textureBindings and m_defaultUniforms into pre-indexed
// structures so UseProgram / ApplyTextureBindings have zero per-frame string work.
// ---------------------------------------------------------------------------
void Shader::BuildResolvedData()
{
    // --- @texture bindings ---
    m_resolvedBindings.clear();
    m_resolvedBindings.reserve(textureBindings.size());

    for (const auto& [hs, path] : textureBindings)
    {
        auto it = m_uniformIndex.find(hs.str());
        if (it == m_uniformIndex.end()) continue;

        ResolvedBinding rb;
        rb.samplerIdx = it->second;
        rb.path       = path;

        // Pre-resolve the optional <sampler>_size companion uniform.
        const std::string sizeName = hs.str() + "_size";
        auto sIt = m_uniformIndex.find(sizeName);
        rb.sizeIdx = (sIt != m_uniformIndex.end()) ? sIt->second : kInvalidIndex;

        m_resolvedBindings.push_back(std::move(rb));
    }

    // --- default uniforms ---
    m_resolvedDefaults.clear();
    m_resolvedDefaults.reserve(m_defaultUniforms.size());

    for (const auto& [name, def] : m_defaultUniforms)
    {
        auto it = m_uniformIndex.find(name);
        if (it == m_uniformIndex.end()) continue;

        ResolvedDefault d;
        d.idx = it->second;
        const size_t count = std::min(def.data.size(), size_t(4));
        std::memcpy(d.data, def.data.data(), count * sizeof(float));
        std::memset(d.data + count, 0, (4 - count) * sizeof(float));
        m_resolvedDefaults.push_back(d);
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
    s->m_vsName       = vsName;
    s->m_fsName       = fsName;
    s->m_vsSourcePath = ResolveSourcePath(vsName);
    s->m_fsSourcePath = ResolveSourcePath(fsName);

    // Must reflect before createProgram consumes (destroys) the shader handles.
    s->ReflectUniforms(vsh, fsh);

    // createProgram takes ownership of vsh / fsh (destroyShaders = true).
    s->m_program = bgfx::createProgram(vsh, fsh, /*destroyShaders=*/true);
    if (!bgfx::isValid(s->m_program))
    {
        Logger::Log(std::string("Shader::FromFiles — bgfx::createProgram failed for: ") + vsName + " / " + fsName);
        delete s;
        return nullptr;
    }

    // Parse @texture annotations and build pre-resolved lookup tables.
    s->textureBindings = s->ParseAllTextureBindings();
    s->BuildResolvedData();

    EnsureMissingTexture();
    return s;
}

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
Shader::~Shader()
{
    for (auto& u : m_uniformList)
        if (bgfx::isValid(u.handle)) bgfx::destroy(u.handle);

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

    // Destroy old uniform handles.
    for (auto& u : m_uniformList)
        if (bgfx::isValid(u.handle)) bgfx::destroy(u.handle);

    if (bgfx::isValid(m_program))
        bgfx::destroy(m_program);

    // Re-reflect: load fresh copies just for reflection, then discard them.
    {
        bgfx::ShaderHandle vsRef = LoadShaderBinary(m_vsName);
        bgfx::ShaderHandle fsRef = LoadShaderBinary(m_fsName);
        ReflectUniforms(vsRef, fsRef);
        if (bgfx::isValid(vsRef)) bgfx::destroy(vsRef);
        if (bgfx::isValid(fsRef)) bgfx::destroy(fsRef);
    }

    m_program = newProgram;

    // Re-parse annotations and rebuild pre-resolved tables.
    textureBindings = ParseAllTextureBindings();
    BuildResolvedData();

    // Note: m_uniformBuffer and m_textureBuffer were resized by ReflectUniforms.
    // Any previously-set values that still exist in the new reflection are
    // re-applied correctly because the index layout is rebuilt from scratch.

    Logger::Log("Shader::Reload succeeded for: " + m_vsName + " / " + m_fsName);
    return true;
}

// ---------------------------------------------------------------------------
// Uniform lookup
// Returns the index into m_uniformList, or kInvalidIndex if not found / invalid.
// ---------------------------------------------------------------------------
uint16_t Shader::FindUniformIndex(const std::string& uname) const
{
    auto it = m_uniformIndex.find(uname);
    if (it == m_uniformIndex.end())
    {
        if (!AllowMissingUniforms)
            Logger::Log("Shader warning: uniform \"" + uname + "\" not found in " + m_vsName);
        return kInvalidIndex;
    }

    if (!bgfx::isValid(m_uniformList[it->second].handle))
    {
        Logger::Log("Shader error: uniform \"" + uname + "\" has invalid handle in " + m_vsName);
        return kInvalidIndex;
    }

    return it->second;
}

// ---------------------------------------------------------------------------
// Internal helper: safe num clamped against declared capacity
// ---------------------------------------------------------------------------
static uint16_t SafeUniformNum(const UniformMeta& u, size_t requested, const std::string& uname)
{
    if (requested == 0) return 0;

    constexpr size_t kMax = std::numeric_limits<uint16_t>::max();
    if (requested > kMax)
    {
        Logger::Log("Shader error: uniform \"" + uname + "\" requested count " +
            std::to_string(requested) + " exceeds uint16_t max, clamping");
        requested = kMax;
    }

    if (u.num > 0 && requested > static_cast<size_t>(u.num))
    {
        Logger::Log("Shader error: uniform \"" + uname + "\" requested " +
            std::to_string(requested) + " slots but was created with " +
            std::to_string(u.num) + ", clamping");
        requested = u.num;
    }

    return static_cast<uint16_t>(requested);
}

// ---------------------------------------------------------------------------
// Scalar / small-type setters
// Each resolves the name to an index once, then writes directly into the flat
// buffer — no second map lookup, no heap allocation for the common case.
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
    const uint16_t idx = FindUniformIndex(uname);
    if (idx == kInvalidIndex) return;

    auto& e = m_uniformBuffer[idx];
    e.inlineData[0] = value;
    e.inlineData[1] = e.inlineData[2] = e.inlineData[3] = 0.0f;
    e.heapData.clear();
    e.num = 1;
    e.set = true;
}

void Shader::SetUniform(const std::string& uname, const glm::vec2& value)
{
    const uint16_t idx = FindUniformIndex(uname);
    if (idx == kInvalidIndex) return;

    auto& e = m_uniformBuffer[idx];
    e.inlineData[0] = value.x;
    e.inlineData[1] = value.y;
    e.inlineData[2] = e.inlineData[3] = 0.0f;
    e.heapData.clear();
    e.num = 1;
    e.set = true;
}

void Shader::SetUniform(const std::string& uname, const glm::vec3& value)
{
    const uint16_t idx = FindUniformIndex(uname);
    if (idx == kInvalidIndex) return;

    auto& e = m_uniformBuffer[idx];
    e.inlineData[0] = value.x;
    e.inlineData[1] = value.y;
    e.inlineData[2] = value.z;
    e.inlineData[3] = 0.0f;
    e.heapData.clear();
    e.num = 1;
    e.set = true;
}

void Shader::SetUniform(const std::string& uname, const glm::vec4& value)
{
    const uint16_t idx = FindUniformIndex(uname);
    if (idx == kInvalidIndex) return;

    auto& e = m_uniformBuffer[idx];
    std::memcpy(e.inlineData, glm::value_ptr(value), 4 * sizeof(float));
    e.heapData.clear();
    e.num = 1;
    e.set = true;
}

// mat2 → pack column-major into one Vec4: [m00, m10, m01, m11]
void Shader::SetUniform(const std::string& uname, const glm::mat2& value)
{
    const uint16_t idx = FindUniformIndex(uname);
    if (idx == kInvalidIndex) return;

    auto& e = m_uniformBuffer[idx];
    e.inlineData[0] = value[0][0]; e.inlineData[1] = value[0][1];
    e.inlineData[2] = value[1][0]; e.inlineData[3] = value[1][1];
    e.heapData.clear();
    e.num = 1;
    e.set = true;
}

// mat3 → bgfx Mat3 layout: 3 row-major Vec4 rows, last component padded to 0.
void Shader::SetUniform(const std::string& uname, const glm::mat3& value)
{
    const uint16_t idx = FindUniformIndex(uname);
    if (idx == kInvalidIndex) return;

    auto& e = m_uniformBuffer[idx];
    const float* src = glm::value_ptr(value); // column-major: col0[3], col1[3], col2[3]

    // row 0
    e.inlineData[0]  = src[0]; e.inlineData[1]  = src[3]; e.inlineData[2]  = src[6]; e.inlineData[3]  = 0.0f;
    // row 1
    e.inlineData[4]  = src[1]; e.inlineData[5]  = src[4]; e.inlineData[6]  = src[7]; e.inlineData[7]  = 0.0f;
    // row 2
    e.inlineData[8]  = src[2]; e.inlineData[9]  = src[5]; e.inlineData[10] = src[8]; e.inlineData[11] = 0.0f;

    e.heapData.clear();
    e.num = 1;
    e.set = true;
}

void Shader::SetUniform(const std::string& uname, const glm::mat4& value)
{
    const uint16_t idx = FindUniformIndex(uname);
    if (idx == kInvalidIndex) return;

    auto& e = m_uniformBuffer[idx];
    std::memcpy(e.inlineData, glm::value_ptr(value), 16 * sizeof(float));
    e.heapData.clear();
    e.num = 1;
    e.set = true;
}

// ---------------------------------------------------------------------------
// Array setters — each element padded to a Vec4 slot (or Mat4 slot for mat4).
// Fits in inline storage for up to 4 elements (vec4) / 1 element (mat4, mat3).
// Falls back to heap for larger arrays.
// ---------------------------------------------------------------------------
void Shader::SetUniform(const std::string& uname, const std::vector<float>& values)
{
    const uint16_t idx = FindUniformIndex(uname);
    if (idx == kInvalidIndex || values.empty()) return;

    const uint16_t num = SafeUniformNum(m_uniformList[idx], values.size(), uname);
    if (num == 0) return;

    const size_t total = static_cast<size_t>(num) * 4;
    auto& e = m_uniformBuffer[idx];

    if (total <= UniformEntry::kInline)
    {
        e.heapData.clear();
        for (uint16_t i = 0; i < num; ++i)
        {
            e.inlineData[i * 4 + 0] = values[i];
            e.inlineData[i * 4 + 1] = e.inlineData[i * 4 + 2] = e.inlineData[i * 4 + 3] = 0.0f;
        }
    }
    else
    {
        e.heapData.assign(total, 0.0f);
        for (uint16_t i = 0; i < num; ++i)
            e.heapData[i * 4] = values[i];
    }
    e.num = num;
    e.set = true;
}

void Shader::SetUniform(const std::string& uname, const std::vector<glm::vec2>& values)
{
    const uint16_t idx = FindUniformIndex(uname);
    if (idx == kInvalidIndex || values.empty()) return;

    const uint16_t num = SafeUniformNum(m_uniformList[idx], values.size(), uname);
    if (num == 0) return;

    const size_t total = static_cast<size_t>(num) * 4;
    auto& e = m_uniformBuffer[idx];

    if (total <= UniformEntry::kInline)
    {
        e.heapData.clear();
        for (uint16_t i = 0; i < num; ++i)
        {
            e.inlineData[i * 4 + 0] = values[i].x;
            e.inlineData[i * 4 + 1] = values[i].y;
            e.inlineData[i * 4 + 2] = e.inlineData[i * 4 + 3] = 0.0f;
        }
    }
    else
    {
        e.heapData.assign(total, 0.0f);
        for (uint16_t i = 0; i < num; ++i)
        {
            e.heapData[i * 4 + 0] = values[i].x;
            e.heapData[i * 4 + 1] = values[i].y;
        }
    }
    e.num = num;
    e.set = true;
}

void Shader::SetUniform(const std::string& uname, const std::vector<glm::vec3>& values)
{
    const uint16_t idx = FindUniformIndex(uname);
    if (idx == kInvalidIndex || values.empty()) return;

    const uint16_t num = SafeUniformNum(m_uniformList[idx], values.size(), uname);
    if (num == 0) return;

    const size_t total = static_cast<size_t>(num) * 4;
    auto& e = m_uniformBuffer[idx];

    if (total <= UniformEntry::kInline)
    {
        e.heapData.clear();
        for (uint16_t i = 0; i < num; ++i)
        {
            e.inlineData[i * 4 + 0] = values[i].x;
            e.inlineData[i * 4 + 1] = values[i].y;
            e.inlineData[i * 4 + 2] = values[i].z;
            e.inlineData[i * 4 + 3] = 0.0f;
        }
    }
    else
    {
        e.heapData.assign(total, 0.0f);
        for (uint16_t i = 0; i < num; ++i)
        {
            e.heapData[i * 4 + 0] = values[i].x;
            e.heapData[i * 4 + 1] = values[i].y;
            e.heapData[i * 4 + 2] = values[i].z;
        }
    }
    e.num = num;
    e.set = true;
}

void Shader::SetUniform(const std::string& uname, const std::vector<glm::vec4>& values)
{
    const uint16_t idx = FindUniformIndex(uname);
    if (idx == kInvalidIndex || values.empty()) return;

    const uint16_t num = SafeUniformNum(m_uniformList[idx], values.size(), uname);
    if (num == 0) return;

    static_assert(sizeof(glm::vec4) == 4 * sizeof(float), "glm::vec4 layout assumption broken");

    m_uniformBuffer[idx].assign(glm::value_ptr(values[0]), static_cast<size_t>(num) * 4, num);
}

// mat2 array: each mat2 → one Vec4 slot
void Shader::SetUniform(const std::string& uname, const std::vector<glm::mat2>& values)
{
    const uint16_t idx = FindUniformIndex(uname);
    if (idx == kInvalidIndex || values.empty()) return;

    const uint16_t num = SafeUniformNum(m_uniformList[idx], values.size(), uname);
    if (num == 0) return;

    const size_t total = static_cast<size_t>(num) * 4;
    auto& e = m_uniformBuffer[idx];

    if (total <= UniformEntry::kInline)
    {
        e.heapData.clear();
        for (uint16_t i = 0; i < num; ++i)
        {
            e.inlineData[i * 4 + 0] = values[i][0][0];
            e.inlineData[i * 4 + 1] = values[i][0][1];
            e.inlineData[i * 4 + 2] = values[i][1][0];
            e.inlineData[i * 4 + 3] = values[i][1][1];
        }
    }
    else
    {
        e.heapData.resize(total);
        for (uint16_t i = 0; i < num; ++i)
        {
            e.heapData[i * 4 + 0] = values[i][0][0];
            e.heapData[i * 4 + 1] = values[i][0][1];
            e.heapData[i * 4 + 2] = values[i][1][0];
            e.heapData[i * 4 + 3] = values[i][1][1];
        }
    }
    e.num = num;
    e.set = true;
}

// mat3 array: each mat3 → 3 Vec4 rows (12 floats)
void Shader::SetUniform(const std::string& uname, const std::vector<glm::mat3>& values)
{
    const uint16_t idx = FindUniformIndex(uname);
    if (idx == kInvalidIndex || values.empty()) return;

    const uint16_t num = SafeUniformNum(m_uniformList[idx], values.size(), uname);
    if (num == 0) return;

    const size_t total = static_cast<size_t>(num) * 12;
    auto& e = m_uniformBuffer[idx];

    // mat3 arrays rarely fit inline (>1 matrix = >12 floats → exactly 12 for 1),
    // so branch once and write to whichever buffer we end up in.
    float* dst;
    if (total <= UniformEntry::kInline)
    {
        e.heapData.clear();
        dst = e.inlineData;
    }
    else
    {
        e.heapData.assign(total, 0.0f);
        dst = e.heapData.data();
    }

    for (uint16_t i = 0; i < num; ++i)
    {
        const float* src = glm::value_ptr(values[i]);
        float* d = dst + i * 12;
        d[0]  = src[0]; d[1]  = src[3]; d[2]  = src[6]; d[3]  = 0.0f;
        d[4]  = src[1]; d[5]  = src[4]; d[6]  = src[7]; d[7]  = 0.0f;
        d[8]  = src[2]; d[9]  = src[5]; d[10] = src[8]; d[11] = 0.0f;
    }
    e.num = num;
    e.set = true;
}

void Shader::SetUniform(const std::string& uname, const std::vector<glm::mat4>& values)
{
    const uint16_t idx = FindUniformIndex(uname);
    if (idx == kInvalidIndex || values.empty()) return;

    const uint16_t num = SafeUniformNum(m_uniformList[idx], values.size(), uname);
    if (num == 0) return;

    static_assert(sizeof(glm::mat4) == 16 * sizeof(float), "glm::mat4 layout assumption broken");

    m_uniformBuffer[idx].assign(glm::value_ptr(values[0]), static_cast<size_t>(num) * 16, num);
}

// ---------------------------------------------------------------------------
// Texture setters — write directly into the indexed flat buffer.
// ---------------------------------------------------------------------------
void Shader::SetTexture(const std::string& uname, bgfx::TextureHandle texture)
{
    const uint16_t idx = FindUniformIndex(uname);
    if (idx == kInvalidIndex) return;

    if (!bgfx::isValid(texture))
    {
        Logger::Log("Shader warning: invalid texture handle for \"" + uname +
            "\" in " + m_vsName + ", using missing texture fallback");
        texture = s_missingTexture;
    }

    const UniformMeta& u = m_uniformList[idx];
    auto& entry          = m_textureBuffer[idx];
    entry.slot           = u.samplerSlot;
    entry.samplerHandle  = u.handle;
    entry.texture        = texture;
    entry.set            = true;
}

void Shader::SetTexture(const std::string& uname, int texture)
{
    const uint16_t idx = FindUniformIndex(uname);
    if (idx == kInvalidIndex) return;

    const bgfx::TextureHandle handle = (texture == 0)
        ? s_blackTexture
        : bgfx::TextureHandle{ static_cast<uint16_t>(texture) };

    const UniformMeta& u = m_uniformList[idx];
    auto& entry          = m_textureBuffer[idx];
    entry.slot           = u.samplerSlot;
    entry.samplerHandle  = u.handle;
    entry.texture        = handle;
    entry.set            = true;
}

void Shader::SetTexture(const hashed_string& uname, Texture* texture)
{
    const uint16_t idx = FindUniformIndex(uname.str());
    if (idx == kInvalidIndex) return;

    const UniformMeta& u = m_uniformList[idx];
    auto& entry          = m_textureBuffer[idx];
    entry.slot           = u.samplerSlot;
    entry.samplerHandle  = u.handle;
    entry.set            = true;

    if (texture && texture->valid)
    {
        bgfx::TextureHandle handle = texture->getHandle();
        if (!bgfx::isValid(handle))
        {
            Logger::Log("Shader warning: Texture object reports valid but handle is invalid for \"" +
                uname.str() + "\" in " + m_vsName + ", using missing texture");
            EnsureMissingTexture();
            handle = s_missingTexture;
        }
        entry.texture = handle;
    }
    else
    {
        entry.texture = s_blackTexture; // null or invalid → black fallback
    }
}

void Shader::SetCubemapTexture(const std::string& uname, bgfx::TextureHandle texture)
{
    SetTexture(uname, texture);
}

// ---------------------------------------------------------------------------
// FlushBuffers — push all buffered state to bgfx (called by Submit)
//
// Hot-path design:
//   - Iterates two flat vectors with cache-friendly sequential access.
//   - No string keys, no map lookups, no indirection through UniformEntry::data
//     for the common case (inlineData is in-struct).
//   - Only sampler indices are visited for texture binding (m_samplerIndices).
// ---------------------------------------------------------------------------
void Shader::FlushBuffers() const
{
    // Flush uniform values
    for (size_t i = 0; i < m_uniformBuffer.size(); ++i)
    {
        const auto& entry = m_uniformBuffer[i];
        if (!entry.set) continue;

        const UniformMeta& u = m_uniformList[i];
        if (!bgfx::isValid(u.handle)) continue;

        bgfx::setUniform(u.handle, entry.data(), entry.num);
    }

    // Flush texture bindings — iterate only sampler-kind indices.
    for (const uint16_t idx : m_samplerIndices)
    {
        const auto& entry = m_textureBuffer[idx];
        if (!entry.set) continue;

        bgfx::setTexture(entry.slot, entry.samplerHandle, entry.texture);
    }
}

// ---------------------------------------------------------------------------
// UseProgram / Submit
// ---------------------------------------------------------------------------
void Shader::UseProgram()
{
    EnsureMissingTexture();

    // Reset buffered state — flip flags only, no allocation or deallocation.
    // This is the key change from the original clear() approach: the vectors
    // retain their capacity and all UniformEntry inline storage is reused as-is.
    for (auto& e : m_uniformBuffer) e.set = false;
    for (auto& e : m_textureBuffer) e.set = false;

    // Apply pre-resolved defaults (no map lookups, no string hashing).
    for (const auto& def : m_resolvedDefaults)
    {
        auto& e = m_uniformBuffer[def.idx];
        std::memcpy(e.inlineData, def.data, 4 * sizeof(float));
        e.heapData.clear();
        e.num = 1;
        e.set = true;
    }

    ApplyTextureBindings();
}

void Shader::Submit(uint16_t viewId) const
{

#ifndef DISTRIBUTION

    ShaderManager::RegisterPSO(BgfxStateManager::GetState(), m_vsSourcePath, m_fsSourcePath);

#endif

    FlushBuffers();
    bgfx::submit(viewId, m_program);
}

// ---------------------------------------------------------------------------
// @texture annotation parsing
// ---------------------------------------------------------------------------
std::unordered_map<hashed_string, std::string>
Shader::ParseTextureBindings(const std::string& sourceCode)
{
    std::unordered_map<hashed_string, std::string> result;

    std::regex reUniform(R"(uniform\s+(?:sampler2D|samplerCube)\s+(\w+)\s*;[^\n]*@texture\s+([^\s]+))");
    std::regex reMacro  (R"(SAMPLER(?:2D|CUBE)\s*\(\s*(\w+)\s*,[^)]*\)\s*;[^\n]*@texture\s+([^\s]+))");

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

    std::regex re(R"(SAMPLER(?:2D|CUBE)\s*\(\s*(\w+)\s*,\s*(\d+)\s*\))");
    std::smatch match;
    auto it = source.cbegin();

    while (std::regex_search(it, source.cend(), match, re))
    {
        result[match[1].str()] = static_cast<uint8_t>(std::stoi(match[2].str()));
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

        for (auto& [k, v] : ParseSamplerSlots(source))
            result[k] = v;
    }
    return result;
}

void Shader::ParseDefaultUniforms(const std::string& source)
{
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

        for (auto& [k, v] : ParseTextureBindings(source))
            result[k] = v;
    }
    return result;
}

// Apply auto-bindings from pre-resolved m_resolvedBindings.
// No string operations occur here — all uniform indices were pre-computed in
// BuildResolvedData() at load / reload time.
void Shader::ApplyTextureBindings()
{
    for (const auto& rb : m_resolvedBindings)
    {
        Texture* tex = AssetRegistry::GetTextureFromFile(rb.path);

        const UniformMeta& u = m_uniformList[rb.samplerIdx];
        auto& entry          = m_textureBuffer[rb.samplerIdx];
        entry.slot           = u.samplerSlot;
        entry.samplerHandle  = u.handle;
        entry.set            = true;

        if (tex && tex->valid)
        {
            const bgfx::TextureHandle handle = tex->getHandle();
            entry.texture = bgfx::isValid(handle) ? handle : s_missingTexture;

            // Auto-set <sampler>_size if the shader declared it.
            if (rb.sizeIdx != kInvalidIndex)
            {
                auto& sizeE = m_uniformBuffer[rb.sizeIdx];
                sizeE.inlineData[0] = static_cast<float>(tex->width);
                sizeE.inlineData[1] = static_cast<float>(tex->height);
                sizeE.inlineData[2] = 0.0f;
                sizeE.inlineData[3] = 0.0f;
                sizeE.heapData.clear();
                sizeE.num = 1;
                sizeE.set = true;
            }
        }
        else
        {
            entry.texture = s_blackTexture;
        }
    }
}

// ---------------------------------------------------------------------------
// Missing / default texture fallbacks
// ---------------------------------------------------------------------------
void Shader::EnsureMissingTexture()
{
    if (!bgfx::isValid(s_missingTexture))
    {
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
