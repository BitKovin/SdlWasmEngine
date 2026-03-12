// Renderer.cpp — bgfx port
#include "Renderer.h"

#include "../EngineMain.h"
#include "../LightSystem/LightManager.h"
#include "../DebugDraw.hpp"
#include "../FogManager.h"

#include "Abstractions/ViewIdManager.h"

#include <bgfx/bgfx.h>

#include <BgfxStateManager.h>

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

    fullscreenShader = ShaderManager::GetShaderProgram("vs_fullscreen", "fs_postprocessing");
    blurShader = ShaderManager::GetShaderProgram("vs_fullscreen", "fs_motionBlur");
    blurApplyShader = ShaderManager::GetShaderProgram("vs_fullscreen", "fs_motionBlur_apply");

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

    bgfx::setViewRect(0, 0, 0,
        static_cast<uint16_t>(nativeRes.x),
        static_cast<uint16_t>(nativeRes.y));
    bgfx::setViewFrameBuffer(0, BGFX_INVALID_HANDLE);
    ViewIdManager::setCurrentViewId(0);

    fullscreenShader->UseProgram();
    fullscreenShader->SetTexture("screenTexture", resultTex);
    fullscreenShader->SetUniform("screenResolution", nativeRes);
    RenderFullscreenQuad(fullscreenShader);
}

// -----------------------------------------------------------------------
// RenderCameraForward
// -----------------------------------------------------------------------
void Renderer::RenderCameraForward(vector<IDrawMesh*>& VissibleRenderList)
{
    ivec2 res = GetScreenResolution();

    // ---- MSAA bookkeeping ----
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
            BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
            kClearBlack, 1.0f, 0);

        // Depth write only — no color writes, less-than test, back-face cull.
        BgfxStateManager::Reset();
        BgfxStateManager::SetWriteRGB(false);
        BgfxStateManager::SetWriteAlpha(false);
        BgfxStateManager::SetWriteDepth(true);
        BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Less);
        BgfxStateManager::SetCull(BgfxStateManager::Cull::CW);

        for (auto* mesh : VissibleRenderList)
        {
            BgfxStateManager::Apply();
            const mat4& P = mesh->IsViewmodel
                ? Camera::finalizedProjectionViewmodel
                : Camera::finalizedProjection;
            mesh->DrawDepth(Camera::finalizedView, P);
        }
    }

    // Only resolve depth — color hasn't been written yet.
    if (forwardFBO->depthAttachment() && forwardResolveFBO->depthAttachment())
        forwardResolveFBO->depthAttachment()->copyFrom(forwardFBO->depthAttachment());

    // ====================================================================
    // Pass B — Opaque + transparent color pass
    // ====================================================================
    {
        bgfx::ViewId vid = forwardFBO->viewId();
        forwardFBO->bind(0, 0,
            static_cast<uint16_t>(res.x),
            static_cast<uint16_t>(res.y));

        bgfx::setViewClear(vid,
            BGFX_CLEAR_COLOR,
            kClearBlack, 1.0f, 0);

        // Opaque: RGB+A write, no depth write, lequal test, back-face cull.
        BgfxStateManager::Reset();
        BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::LEqual);
        BgfxStateManager::SetCull(BgfxStateManager::Cull::CW);

        for (auto* mesh : VissibleRenderList)
        {
            if (mesh->Transparent) continue;
            BgfxStateManager::Apply();
            const mat4& P = mesh->IsViewmodel
                ? Camera::finalizedProjectionViewmodel
                : Camera::finalizedProjection;
            mesh->DrawForward(Camera::finalizedView, P);
        }

        // Transparent: RGB+A write, no depth write, lequal test, alpha blend.
        BgfxStateManager::Reset();
        BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::LEqual);
        BgfxStateManager::SetBlend(BgfxStateManager::Blend::Alpha);

        Level::Current->BspData.RenderTransparentFaces();

        for (auto* mesh : VissibleRenderList)
        {
            if (!mesh->Transparent) continue;
            BgfxStateManager::Apply();
            const mat4& P = mesh->IsViewmodel
                ? Camera::finalizedProjectionViewmodel
                : Camera::finalizedProjection;
            mesh->DrawForward(Camera::finalizedView, P);
        }

        DebugDraw::Draw();
    }

    // Resolve color + depth → single-sample resolve FBO.
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

        // Depth already resolved; write color IDs only, no depth test.
        BgfxStateManager::Reset();
        BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Always);

        for (auto* mesh : VissibleRenderList)
        {
            BgfxStateManager::Apply();
            const mat4& P = mesh->IsViewmodel
                ? Camera::finalizedProjectionViewmodel
                : Camera::finalizedProjection;
            mesh->DrawCustomId(Camera::finalizedView, P);
        }
    }
}

// -----------------------------------------------------------------------
// RenderDirectionalLightShadows
// -----------------------------------------------------------------------
void Renderer::RenderDirectionalLightShadows(vector<IDrawMesh*>& ShadowRenderList,
    Framebuffer& fbo,
    int                 /*numCascades*/)
{

}

// -----------------------------------------------------------------------
// RenderFullscreenQuad
// -----------------------------------------------------------------------
void Renderer::RenderFullscreenQuad(Shader* shader)
{
    bgfx::setVertexBuffer(0, m_fullscreenVB);

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
// -----------------------------------------------------------------------
void Renderer::InitFullscreenBuffers()
{
    m_fullscreenLayout.begin()
        .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
        .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
        .end();

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
    forwardFBO->attachColor(colorBuffer, 0u);
    forwardFBO->attachDepth(depthBuffer);

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
    forwardResolveFBO->attachColor(colorResolveBuffer, 0u);
    forwardResolveFBO->attachDepth(depthResolveBuffer);

    colorResolveBuffer->resize(screenResolution.x, screenResolution.y);
    customIdResolveBuffer->resize(screenResolution.x, screenResolution.y);
    depthResolveBuffer->resize(screenResolution.x, screenResolution.y);

    customIdFBO = new Framebuffer();
    customIdFBO->attachColor(customIdResolveBuffer, 0u);
    //customIdFBO->attachDepth(customIdDepthBuffer);
}