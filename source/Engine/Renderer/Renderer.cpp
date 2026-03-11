// Renderer.cpp — bgfx port
#include "Renderer.h"

#include "../EngineMain.h"
#include "../LightSystem/LightManager.h"
#include "../DebugDraw.hpp"
#include "../FogManager.h"

#include "Abstractions/ViewIdManager.h"

#include <bgfx/bgfx.h>

// -----------------------------------------------------------------------
// bgfx clear-color helper  (RGBA packed as uint32_t 0xRRGGBBAA)
// -----------------------------------------------------------------------
static constexpr uint32_t kClearBlack = 0x000000ff;
static constexpr uint32_t kClearAlpha1 = 0x000000ff; // alpha = 1, RGB = 0

// -----------------------------------------------------------------------
// Constructor / destructor
// -----------------------------------------------------------------------
Renderer::Renderer()
{
    ivec2 screenResolution = GetScreenResolution();

    InitFrameBuffers();
    InitResolveFrameBuffers();

    fullscreenShader = ShaderManager::GetShaderProgram("fullscreen_vertex", "postprocessing");
    blurShader = ShaderManager::GetShaderProgram("fullscreen_vertex", "motionBlur");
    blurApplyShader = ShaderManager::GetShaderProgram("fullscreen_vertex", "motionBlur_apply");

    BlurResultBuffer = new RenderTexture(screenResolution.x, screenResolution.y,
        TextureFormat::RGBA16F);
    BlurResultBuffer->SetName("BlurResultBuffer");

    BlurAccumulatedBuffer = new RenderTexture(screenResolution.x, screenResolution.y,
        TextureFormat::RGBA16F);
    BlurAccumulatedBuffer->SetName("BlurAccumulatedBuffer");

    if (LightManager::DirectionalShadowsEnabled)
    {
        DirectionalShadowMap = new RenderTexture(
            LightManager::ShadowMapResolution, LightManager::ShadowMapResolution,
            TextureFormat::Depth32F, TextureType::Texture2D);
        DirectionalShadowMap->SetName("DirectionalShadowMap");
        DirectionalShadowMapFBO = new Framebuffer();
        DirectionalShadowMapFBO->attachDepth(DirectionalShadowMap);

        DetailDirectionalShadowMap = new RenderTexture(
            LightManager::ShadowMapResolution, LightManager::ShadowMapResolution,
            TextureFormat::Depth32F, TextureType::Texture2D);
        DetailDirectionalShadowMap->SetName("DetailDirectionalShadowMap");
        DetailDirectionalShadowMapFBO = new Framebuffer();
        DetailDirectionalShadowMapFBO->attachDepth(DetailDirectionalShadowMap);
    }

    InitFullscreenBuffers();
}

Renderer::~Renderer()
{
    delete colorBuffer;
    delete depthBuffer;

    if (bgfx::isValid(m_fullscreenVB))
        bgfx::destroy(m_fullscreenVB);
}

// -----------------------------------------------------------------------
// RenderLevel
// -----------------------------------------------------------------------
void Renderer::RenderLevel(Level* level)
{
    if (LightManager::DirectionalShadowsEnabled)
    {
        RenderDirectionalLightShadows(level->ShadowRenderList, *DirectionalShadowMapFBO, 4);
        RenderDirectionalLightShadows(level->DetailShadowRenderList, *DetailDirectionalShadowMapFBO, 3);
    }

    for (auto drawable : level->VissibleRenderList)
        drawable->PreDraw();

    RenderCameraForward(level->VissibleRenderList);

    ivec2 screenResolution = GetScreenResolution();
    BlurAccumulatedBuffer->resize(screenResolution.x, screenResolution.y);
    BlurResultBuffer->resize(screenResolution.x, screenResolution.y);

    const bool blurEnabled = false;

    if (blurEnabled)
    {
        // ---- Motion blur accumulate pass ----
        BlurResultBuffer->setAsRenderTarget();

        bgfx::setViewClear(BlurResultBuffer->viewId(),
            BGFX_CLEAR_COLOR, kClearBlack, 1.0f, 0);

        blurShader->UseProgram();
        blurShader->SetTexture("uAccumulated", BlurAccumulatedBuffer->textureHandle());
        blurShader->SetTexture("uCustomIdTex", customIdResolveBuffer->textureHandle());
        blurShader->SetUniform("uDeltaTime", EngineMain::MainInstance->Paused
            ? 0.0f
            : Time::DeltaTimeFNoTimeScale);
        blurShader->SetUniform("GameTime", (float)Time::GameTime);
        blurShader->SetUniform("uPersistence", 0.20f);
        blurShader->SetUniform("uMotionScale", 3.0f);
        blurShader->SetTexture("screenTexture", colorResolveBuffer->textureHandle());
        RenderFullscreenQuad(blurShader);

        // Copy accumulate result back
        BlurAccumulatedBuffer->copyFrom(BlurResultBuffer);

        // ---- Motion blur apply pass ----
        BlurResultBuffer->setAsRenderTarget();
        blurApplyShader->UseProgram();
        blurApplyShader->SetTexture("screenTexture", colorResolveBuffer->textureHandle());
        blurApplyShader->SetTexture("blurTexture", BlurAccumulatedBuffer->textureHandle());
        RenderFullscreenQuad(blurApplyShader);
    }

    // ---- Final blit to backbuffer ----
    bgfx::TextureHandle resultTex = blurEnabled
        ? BlurResultBuffer->textureHandle()
        : colorResolveBuffer->textureHandle();

    ivec2 nativeRes = GetNativeScreenResolution();

    // View 0 is reserved as the "present to screen" view in bgfx.
    // Configure it to cover the full native resolution.
    bgfx::setViewRect(0, 0, 0,
        static_cast<uint16_t>(nativeRes.x),
        static_cast<uint16_t>(nativeRes.y));
    bgfx::setViewFrameBuffer(0, BGFX_INVALID_HANDLE); // default backbuffer

    fullscreenShader->UseProgram();
    fullscreenShader->SetTexture("screenTexture", resultTex);
    fullscreenShader->SetUniform("screenResolution", nativeRes);
    RenderFullscreenQuad(0);
}

// -----------------------------------------------------------------------
// RenderCameraForward
// -----------------------------------------------------------------------
void Renderer::RenderCameraForward(vector<IDrawMesh*>& VissibleRenderList)
{
    ivec2 res = GetScreenResolution();

    // ---- MSAA bookkeeping ----
    // bgfx handles MSAA via texture flags; we keep the sample count on the
    // RenderTexture and let it rebuild its resources when it changes.
#if defined(BGFX_PLATFORM_EMSCRIPTEN)
    MultiSampleCount = 0; // WebGL: no MSAA
#endif

    if (MultiSampleCount)
    {
        if (colorBuffer->type() == TextureType::Texture2D)
            InitFrameBuffers();

        colorBuffer->setSamples(MultiSampleCount);
        depthBuffer->setSamples(MultiSampleCount);
    }
    else
    {
        if (colorBuffer->type() == TextureType::Texture2DMultisample)
            InitFrameBuffers();

        colorBuffer->setSamples(0);
        depthBuffer->setSamples(0);
    }

    // ---- Resize all buffers ----
    colorBuffer->resize(res.x, res.y);
    depthBuffer->resize(res.x, res.y);
    colorResolveBuffer->resize(res.x, res.y);
    customIdResolveBuffer->resize(res.x, res.y);
    depthResolveBuffer->resize(res.x, res.y);

    // ====================================================================
    // Pass A — Depth pre-pass
    // ====================================================================
    {
        bgfx::ViewId vid = forwardFBO->viewId();
        forwardFBO->bind(0, 0,
            static_cast<uint16_t>(res.x),
            static_cast<uint16_t>(res.y));

        bgfx::setViewClear(vid,
            BGFX_CLEAR_DEPTH,
            kClearBlack, 1.0f, 0);

        // Depth pre-pass state:
        //   - No color writes
        //   - Depth write + less
        //   - Back-face culling
        //   - Polygon offset (slope=3, units=3) → encode as depth bias in
        //     material state; bgfx has no global polygon offset, so meshes
        //     that need it must include BGFX_STATE_DEPTH_TEST_LESS with a
        //     per-draw bias uniform. Here we set the shared view state.
        uint64_t depthPassState =
            BGFX_STATE_WRITE_Z
            | BGFX_STATE_DEPTH_TEST_LESS
            | BGFX_STATE_CULL_CW;   // adjust CW/CCW to match your winding

        for (auto* mesh : VissibleRenderList)
        {
            const mat4& P = mesh->IsViewmodel
                ? Camera::finalizedProjectionViewmodel
                : Camera::finalizedProjection;
            mesh->DrawDepth(Camera::finalizedView, P);
        }
    }

    // Resolve depth pre-pass → single-sample (needed for sampling later)
    forwardFBO->resolve(*forwardResolveFBO);

    // ====================================================================
    // Pass B — Opaque + transparent color pass
    // ====================================================================
    {
        bgfx::ViewId vid = forwardFBO->viewId();
        forwardFBO->bind(0, 0,
            static_cast<uint16_t>(res.x),
            static_cast<uint16_t>(res.y));

        // Clear only color (depth already populated by pre-pass)
        bgfx::setViewClear(vid,
            BGFX_CLEAR_NONE,   // depth kept, color NOT cleared here
            kClearBlack, 1.0f, 0);

        // Opaque: color write, no depth write, depth == (lequal)
        uint64_t opaqueState =
            BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A
            | BGFX_STATE_DEPTH_TEST_LEQUAL
            | BGFX_STATE_CULL_CW;

        for (auto* mesh : VissibleRenderList)
        {
            if (mesh->Transparent) continue;
            const mat4& P = mesh->IsViewmodel
                ? Camera::finalizedProjectionViewmodel
                : Camera::finalizedProjection;
            mesh->DrawForward(Camera::finalizedView, P);
        }

        // Transparent: color write, no depth write, depth lequal, blend
        uint64_t transparentState =
            BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A
            | BGFX_STATE_DEPTH_TEST_LEQUAL
            | BGFX_STATE_BLEND_ALPHA;

        Level::Current->BspData.RenderTransparentFaces();

        for (auto* mesh : VissibleRenderList)
        {
            if (!mesh->Transparent) continue;
            const mat4& P = mesh->IsViewmodel
                ? Camera::finalizedProjectionViewmodel
                : Camera::finalizedProjection;
            mesh->DrawForward(Camera::finalizedView, P);
        }

        DebugDraw::Draw();
    }

    // Resolve color + depth → single-sample resolve FBO
    forwardFBO->resolve(*forwardResolveFBO);

    // ====================================================================
    // Pass C — Custom ID pass  (single-sample, no depth write)
    // ====================================================================
    {
        bgfx::ViewId vid = customIdFBO->viewId();
        customIdFBO->bind(0, 0,
            static_cast<uint16_t>(res.x),
            static_cast<uint16_t>(res.y));

        bgfx::setViewClear(vid,
            BGFX_CLEAR_COLOR, kClearBlack, 1.0f, 0);

        uint64_t idState =
            BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A
            | BGFX_STATE_DEPTH_TEST_ALWAYS;   // depth already resolved; just write color

        for (auto* mesh : VissibleRenderList)
        {
            const mat4& P = mesh->IsViewmodel
                ? Camera::finalizedProjectionViewmodel
                : Camera::finalizedProjection;
            mesh->DrawCustomId(Camera::finalizedView, P);
        }
    }
}

// -----------------------------------------------------------------------
// RenderDirectionalLightShadows
// Cascaded shadow maps: 4 sub-viewports tiled 2×2 inside the shadow atlas.
// In bgfx each viewport is a separate bgfx::setViewRect call on the same
// view, submitted as consecutive draw-call batches (sequenced by order of
// submission within the view).
// -----------------------------------------------------------------------
void Renderer::RenderDirectionalLightShadows(vector<IDrawMesh*>& ShadowRenderList,
    Framebuffer& fbo,
    int                 /*numCascades*/)
{

}

// -----------------------------------------------------------------------
// RenderFullscreenQuad
// Submits a fullscreen triangle-strip to the given bgfx view.
// -----------------------------------------------------------------------
void Renderer::RenderFullscreenQuad(Shader* shader)
{
    bgfx::setVertexBuffer(0, m_fullscreenVB);

    // No depth test, no depth write, no culling
    bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);

    // The caller is responsible for setting the program/uniforms via
    // the active Shader wrapper before calling this function.
    // The Shader::UseProgram() should call bgfx::submit() internally,
    // or we call it here with the cached program handle:
    bgfx::submit(ViewIdManager::getCurrentViewId(), shader->GetProgram());
}

// -----------------------------------------------------------------------
// SetSurfaceShaderUniforms
// -----------------------------------------------------------------------
void Renderer::SetSurfaceShaderUniforms(Shader* shader, float brightnessScale)
{
    if (!shader) return;

    shader->SetUniform("lightDirection", LightManager::LightDirection);
    shader->SetUniform("brightness", 1.0f * brightnessScale);

    if (LightManager::DirectionalShadowsEnabled)
    {
        shader->SetUniform("lightMatrix1",
            LightManager::lightProjection1 * LightManager::lightView1);
        shader->SetUniform("lightMatrix2",
            LightManager::lightProjection2 * LightManager::lightView2);
        shader->SetUniform("lightMatrix3",
            LightManager::lightProjection3 * LightManager::lightView3);
        shader->SetUniform("lightMatrix4",
            LightManager::lightProjection4 * LightManager::lightView4);

        shader->SetTexture("shadowMap",
            EngineMain::MainInstance->MainRenderer->DirectionalShadowMap->textureHandle());
        shader->SetTexture("shadowMapDetail",
            EngineMain::MainInstance->MainRenderer->DetailDirectionalShadowMap->textureHandle());
        shader->SetTexture("shadowMapRaw",
            EngineMain::MainInstance->MainRenderer->DirectionalShadowMap->textureHandle());
        shader->SetTexture("shadowMapDetailRaw",
            EngineMain::MainInstance->MainRenderer->DetailDirectionalShadowMap->textureHandle());

        shader->SetUniform("shadowDistance1", LightManager::LightDistance1);
        shader->SetUniform("shadowDistance2", LightManager::LightDistance2);
        shader->SetUniform("shadowDistance3", LightManager::LightDistance3);
        shader->SetUniform("shadowDistance4", LightManager::LightDistance4);

        shader->SetUniform("shadowRadius1", LightManager::LightRadius1);
        shader->SetUniform("shadowRadius2", LightManager::LightRadius2);
        shader->SetUniform("shadowRadius3", LightManager::LightRadius3);
        shader->SetUniform("shadowRadius4", LightManager::LightRadius4);
    }

    shader->SetTexture("depthTexture",
        EngineMain::MainInstance->MainRenderer->depthResolveBuffer->textureHandle());

    shader->SetUniform("fog_start", FogManager::StartDistance);
    shader->SetUniform("fog_end", FogManager::EndDistance);
    shader->SetUniform("fog_opacity", FogManager::Opacity);
    shader->SetUniform("fog_color", FogManager::Color);

    shader->SetUniform("cameraPosition", Camera::finalizedPosition);
    shader->SetUniform("shadowMapSize", LightManager::ShadowMapResolution);
    shader->SetUniform("shaddowOffsetScale", LightManager::ShaddowOffsetScale);
}

// -----------------------------------------------------------------------
// Resolution helpers
// -----------------------------------------------------------------------
inline ivec2 Renderer::GetScreenResolution() const
{
    return ivec2(EngineMain::MainInstance->ScreenSize.x * ResolutionScale,
        EngineMain::MainInstance->ScreenSize.y * ResolutionScale);
}

inline ivec2 Renderer::GetNativeScreenResolution() const
{
    return ivec2(EngineMain::MainInstance->ScreenSize.x,
        EngineMain::MainInstance->ScreenSize.y);
}

// -----------------------------------------------------------------------
// InitFullscreenBuffers
// Replaces the old VAO/VBO setup with a bgfx transient/static vertex buffer.
// Layout: float2 position, float2 texcoord.
// -----------------------------------------------------------------------
void Renderer::InitFullscreenBuffers()
{
    // Vertex layout: pos(x,y) + uv(u,v)
    m_fullscreenLayout.begin()
        .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();

    // Triangle-strip covering the screen (NDC)
    struct QuadVertex { float x, y, u, v; };
    static const QuadVertex kVerts[4] = {
        { -1.0f,  1.0f,  0.0f, 1.0f },
        { -1.0f, -1.0f,  0.0f, 0.0f },
        {  1.0f,  1.0f,  1.0f, 1.0f },
        {  1.0f, -1.0f,  1.0f, 0.0f },
    };

    m_fullscreenVB = bgfx::createVertexBuffer(
        bgfx::makeRef(kVerts, sizeof(kVerts)),
        m_fullscreenLayout
    );
}

// -----------------------------------------------------------------------
// InitFrameBuffers
// -----------------------------------------------------------------------
void Renderer::InitFrameBuffers()
{
    ivec2 screenResolution = GetScreenResolution();
    TextureFormat colorFmt = TextureFormat::RGBA16F;

    delete colorBuffer;  colorBuffer = nullptr;
    delete depthBuffer;  depthBuffer = nullptr;
    delete forwardFBO;   forwardFBO = nullptr;

    TextureType texType = (MultiSampleCount > 0)
        ? TextureType::Texture2DMultisample
        : TextureType::Texture2D;

    // Sampler flags: clamp + linear filtering
    constexpr uint64_t kSamplerFlags =
        BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP
        | BGFX_SAMPLER_MIN_ANISOTROPIC | BGFX_SAMPLER_MAG_ANISOTROPIC;

    colorBuffer = new RenderTexture(
        screenResolution.x, screenResolution.y,
        colorFmt, texType,
        /*sampleDepth=*/false,
        kSamplerFlags,
        MultiSampleCount > 0 ? MultiSampleCount : 0);
    colorBuffer->SetName("MainColorBuffer");

    depthBuffer = new RenderTexture(
        screenResolution.x, screenResolution.y,
        TextureFormat::Depth24, texType,
        /*sampleDepth=*/false,
        kSamplerFlags,
        MultiSampleCount > 0 ? MultiSampleCount : 0);
    depthBuffer->SetName("MainDepthBuffer");

    forwardFBO = new Framebuffer();
    forwardFBO->attachDepth(depthBuffer);
    forwardFBO->attachColor(colorBuffer, 0u);

    // resize() is a no-op when dims haven't changed, but call for consistency
    colorBuffer->resize(screenResolution.x, screenResolution.y);
    depthBuffer->resize(screenResolution.x, screenResolution.y);
}

// -----------------------------------------------------------------------
// InitResolveFrameBuffers
// -----------------------------------------------------------------------
void Renderer::InitResolveFrameBuffers()
{
    ivec2 screenResolution = GetScreenResolution();
    TextureFormat colorFmt = TextureFormat::RGBA16F;

    constexpr uint64_t kSamplerFlags =
        BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP
        | BGFX_SAMPLER_MIN_ANISOTROPIC | BGFX_SAMPLER_MAG_ANISOTROPIC;

    colorResolveBuffer = new RenderTexture(
        screenResolution.x, screenResolution.y,
        colorFmt, TextureType::Texture2D,
        false, kSamplerFlags);
    colorResolveBuffer->SetName("ColorResolveBuffer");

    customIdResolveBuffer = new RenderTexture(
        screenResolution.x, screenResolution.y,
        TextureFormat::RGBA8, TextureType::Texture2D,
        false, kSamplerFlags);
    customIdResolveBuffer->SetName("CustomIdResolveBuffer");

    depthResolveBuffer = new RenderTexture(
        screenResolution.x, screenResolution.y,
        TextureFormat::Depth24, TextureType::Texture2D,
        false, kSamplerFlags);
    depthResolveBuffer->SetName("DepthResolveBuffer");

    forwardResolveFBO = new Framebuffer();
    forwardResolveFBO->attachDepth(depthResolveBuffer);
    forwardResolveFBO->attachColor(colorResolveBuffer, 0u);

    colorResolveBuffer->resize(screenResolution.x, screenResolution.y);
    customIdResolveBuffer->resize(screenResolution.x, screenResolution.y);
    depthResolveBuffer->resize(screenResolution.x, screenResolution.y);

    customIdFBO = new Framebuffer();
    customIdFBO->attachDepth(depthResolveBuffer);
    customIdFBO->attachColor(customIdResolveBuffer, 0u);
}