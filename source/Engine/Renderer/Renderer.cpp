#include "Renderer.h"

#include "../EngineMain.h"

#include "../LightSystem/LightManager.h"
#include "../DebugDraw.hpp"

#include "../FogManager.h"

Renderer::Renderer()
{
	ivec2 screenResolution = GetScreenResolution();

    InitFrameBuffers();
    InitResolveFrameBuffers();

	fullscreenShader = ShaderManager::GetShaderProgram("fullscreen_vertex","postprocessing");
    blurShader = ShaderManager::GetShaderProgram("fullscreen_vertex", "motionBlur");
    blurApplyShader = ShaderManager::GetShaderProgram("fullscreen_vertex", "motionBlur_apply");

    BlurResultBuffer = new RenderTexture(screenResolution.x, screenResolution.y, TextureFormat::RGBA16F);
    BlurAccumulatedBuffer = new RenderTexture(screenResolution.x, screenResolution.y, TextureFormat::RGBA16F);

    if (LightManager::DirectionalShadowsEnabled)
    {
        DirectionalShadowMap = new RenderTexture(LightManager::ShadowMapResolution, LightManager::ShadowMapResolution, TextureFormat::Depth32F, TextureType::Texture2D);
        DirectionalShadowMapFBO = new Framebuffer();
        DirectionalShadowMapFBO->attachDepth(DirectionalShadowMap);

        DetailDirectionalShadowMap = new RenderTexture(LightManager::ShadowMapResolution, LightManager::ShadowMapResolution, TextureFormat::Depth32F, TextureType::Texture2D);
        DetailDirectionalShadowMapFBO = new Framebuffer();
        DetailDirectionalShadowMapFBO->attachDepth(DetailDirectionalShadowMap);
    }


	InitFullscreenVAO();

}

Renderer::~Renderer()
{
	delete(colorBuffer);
	delete(depthBuffer);

	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);
}

void Renderer::RenderLevel(Level* level)
{
    if (LightManager::DirectionalShadowsEnabled)
    {
        RenderDirectionalLightShadows(level->ShadowRenderList, *DirectionalShadowMapFBO, 4);
        RenderDirectionalLightShadows(level->DetailShadowRenderList, *DirectionalShadowMapFBO, 3);
    }
	RenderCameraForward(level->VissibleRenderList);

    ivec2 screenResolution = GetScreenResolution();
    glViewport(0, 0, screenResolution.x, screenResolution.y);

    BlurAccumulatedBuffer->resize(screenResolution.x, screenResolution.y);
    BlurResultBuffer->resize(screenResolution.x, screenResolution.y);

    const bool bulurEnabled = false;

    if (bulurEnabled)
    {
        BlurResultBuffer->bindFramebuffer();

        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_FALSE);
        glClear(GL_COLOR_BUFFER_BIT);

        glDisable(GL_BLEND);

        blurShader->UseProgram();
        blurShader->SetTexture("uAccumulated", BlurAccumulatedBuffer->id());
        blurShader->SetTexture("uCustomIdTex", customIdResolveBuffer->id());
        blurShader->SetUniform("uDeltaTime", EngineMain::MainInstance->Paused ? 0.0f : Time::DeltaTimeFNoTimeScale);
        blurShader->SetUniform("GameTime", (float)Time::GameTime);
        blurShader->SetUniform("uPersistence", 0.20f);
        blurShader->SetUniform("uMotionScale", 3.0f);
        blurShader->SetTexture("screenTexture", colorResolveBuffer->id());
        RenderFullscreenQuad();
        glEnable(GL_BLEND);

        BlurAccumulatedBuffer->copyFrom(BlurResultBuffer);

        BlurResultBuffer->bindFramebuffer();
        blurApplyShader->UseProgram();
        blurApplyShader->SetTexture("screenTexture", colorResolveBuffer->id());
        blurApplyShader->SetTexture("blurTexture", BlurAccumulatedBuffer->id());
        RenderFullscreenQuad();
        Framebuffer::unbind();
    }

    int resultTexId = bulurEnabled ? BlurResultBuffer->id() : colorResolveBuffer->id();

    ivec2 nativeScreenResolution = GetNativeScreenResolution();
    glViewport(0, 0, nativeScreenResolution.x, nativeScreenResolution.y);

	//rendering to screen
    fullscreenShader->UseProgram();
    fullscreenShader->SetTexture("screenTexture", resultTexId);
    fullscreenShader->SetUniform("screenResolution", nativeScreenResolution);
	RenderFullscreenQuad();

}

void Renderer::RenderCameraForward(vector<IDrawMesh*>& VissibleRenderList)
{
    

    ivec2 res = GetScreenResolution();

    #ifndef GL_ES_PROFILE

    if (MultiSampleCount)
    {
        glEnable(GL_MULTISAMPLE);
    }
    else
    {
        glDisable(GL_MULTISAMPLE);
    }

    #else

    MultiSampleCount = 0;

    #endif // !GL_ES_PROFILE

#ifndef GL_ES_PROFILE

    if (MultiSampleCount) 
    {

        if (colorBuffer->type() == TextureType::Texture2D)
        {
            InitFrameBuffers();
        }

        colorBuffer->setSamples(MultiSampleCount);
        depthBuffer->setSamples(MultiSampleCount);
    }
    else
    {

        if (colorBuffer->type() == TextureType::Texture2DMultisample)
        {
            InitFrameBuffers();
        }

        colorBuffer->setSamples(1);
        depthBuffer->setSamples(1);
    }

#endif

    // resize all our buffers
    colorBuffer->resize(res.x, res.y);
    depthBuffer->resize(res.x, res.y);

    colorResolveBuffer->resize(res.x, res.y);
    customIdResolveBuffer->resize(res.x, res.y);
    depthResolveBuffer->resize(res.x, res.y);



    // 1) bind the one multisample FBO with both attachments
    forwardFBO->bind();

    glViewport(0, 0, res.x, res.y);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(3.0f, 3.0f); // slopeScale, units

    //
    // A) Depth‑only pass
    //
    // disable color writes, enable depth writes, clear depth
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_TRUE);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);


    for (auto* mesh : VissibleRenderList) 
    {
        if (mesh->Transparent)
        {
            //continue;
        }
        const mat4& P = mesh->IsViewmodel
            ? Camera::finalizedProjectionViewmodel
            : Camera::finalizedProjection;
        mesh->DrawDepth(Camera::finalizedView, P);

    }
    glUseProgram(0);

    forwardFBO->unbind();
    glDisable(GL_POLYGON_OFFSET_FILL);

    forwardFBO->resolve(*forwardResolveFBO,
        GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
        GL_LINEAR);

    forwardFBO->bind();

    //
    // B) Opaque + transparent color passes
    //
    // re‑enable color writes, disable depth writes, clear color    
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_FALSE);
    //glClear(GL_COLOR_BUFFER_BIT);

    // draw opaque where depth ==
    glDepthFunc(GL_LEQUAL);
    for (auto* mesh : VissibleRenderList) {
        if (mesh->Transparent) continue;
        const mat4& P = mesh->IsViewmodel
            ? Camera::finalizedProjectionViewmodel
            : Camera::finalizedProjection;
        mesh->DrawForward(Camera::finalizedView, P);
    }


    // draw transparent with normal depth test
    glDepthFunc(GL_LEQUAL);

    Level::Current->BspData.RenderTransparentFaces();

    for (auto* mesh : VissibleRenderList) {
        if (!mesh->Transparent) continue;
        const mat4& P = mesh->IsViewmodel
            ? Camera::finalizedProjectionViewmodel
            : Camera::finalizedProjection;
        mesh->DrawForward(Camera::finalizedView, P);
    }

    DebugDraw::Draw();
    glUseProgram(0);

    //this part makes it work correctly if premultiplied alpha is enabled by webgl. can be disabled by injecting js code in shell, but better to keep just in case
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
    glClearColor(0, 0, 0, 1);    // only alpha channel to 1
    glClear(GL_COLOR_BUFFER_BIT);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    forwardFBO->unbind();

    // 2) resolve to single‐sample FBO
    forwardFBO->resolve(*forwardResolveFBO,
        GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,  
        GL_NEAREST);

    customIdFBO->bind();

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_FALSE);
    glClear(GL_COLOR_BUFFER_BIT);

    for (auto* mesh : VissibleRenderList)
    {

        const mat4& P = mesh->IsViewmodel
            ? Camera::finalizedProjectionViewmodel
            : Camera::finalizedProjection;
        mesh->DrawCustomId(Camera::finalizedView, P);

    }

    customIdFBO->unbind();

#ifndef GL_ES_PROFILE
    glDisable(GL_MULTISAMPLE);   
#endif

}

void Renderer::RenderDirectionalLightShadows(vector<IDrawMesh*>& ShadowRenderList, Framebuffer& fbo, int numCascades)
{
    glUseProgram(0);
    fbo.bind();

    int halfRes = LightManager::ShadowMapResolution / 2;

    

    glEnable(GL_DEPTH_TEST);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_TRUE);
    glClear(GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);


    glViewport(0, 0, halfRes, halfRes);
    for (auto* mesh : ShadowRenderList) {
        mesh->DrawShadow(LightManager::lightView1, LightManager::lightProjection1);
    }

    glViewport(halfRes, 0, halfRes, halfRes);
    for (auto* mesh : ShadowRenderList) {
        mesh->DrawShadow(LightManager::lightView2, LightManager::lightProjection2);
    }

    glViewport(0, halfRes, halfRes, halfRes);
    for (auto* mesh : ShadowRenderList) {
        mesh->DrawShadow(LightManager::lightView3, LightManager::lightProjection3);
    }

    glViewport(halfRes, halfRes, halfRes, halfRes);
    for (auto* mesh : ShadowRenderList) {
        mesh->DrawShadow(LightManager::lightView4, LightManager::lightProjection4);
    }
    glUseProgram(0);
}

void Renderer::RenderFullscreenQuad()
{

    glDisable(GL_DITHER);
	glDisable(GL_DEPTH_TEST);

	// Draw quad
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);


	glEnable(GL_DEPTH_TEST);


}

void Renderer::SetSurfaceShaderUniforms(ShaderProgram* shader, float brightnessScale)
{
    if (shader == nullptr) return;

    shader->SetUniform("lightDirection", LightManager::LightDirection);

    shader->SetUniform("brightness", 1.0f * brightnessScale);


    if (LightManager::DirectionalShadowsEnabled)
    {
        shader->SetUniform("lightMatrix1", LightManager::lightProjection1 * LightManager::lightView1);
        shader->SetUniform("lightMatrix2", LightManager::lightProjection2 * LightManager::lightView2);
        shader->SetUniform("lightMatrix3", LightManager::lightProjection3 * LightManager::lightView3);
        shader->SetUniform("lightMatrix4", LightManager::lightProjection4 * LightManager::lightView4);


        shader->SetTexture("shadowMap", EngineMain::MainInstance->MainRenderer->DirectionalShadowMap->id());
        shader->SetTexture("shadowMapDetail", EngineMain::MainInstance->MainRenderer->DetailDirectionalShadowMap->id());
        shader->SetTexture("shadowMapRaw", EngineMain::MainInstance->MainRenderer->DirectionalShadowMap->id());
        shader->SetTexture("shadowMapDetailRaw", EngineMain::MainInstance->MainRenderer->DetailDirectionalShadowMap->id());
        shader->SetUniform("shadowDistance1", LightManager::LightDistance1);
        shader->SetUniform("shadowDistance2", LightManager::LightDistance2);
        shader->SetUniform("shadowDistance3", LightManager::LightDistance3);
        shader->SetUniform("shadowDistance4", LightManager::LightDistance4);

        shader->SetUniform("shadowRadius1", LightManager::LightRadius1);
        shader->SetUniform("shadowRadius2", LightManager::LightRadius2);
        shader->SetUniform("shadowRadius3", LightManager::LightRadius3);
        shader->SetUniform("shadowRadius4", LightManager::LightRadius4);
    }
    else
    {
        //shader->SetTexture("shadowMap", nullptr);
       // shader->SetTexture("shadowMapDetail", nullptr);
        //shader->SetTexture("shadowMapRaw", nullptr);
        //shader->SetTexture("shadowMapDetailRaw", nullptr);
    }

    shader->SetTexture("depthTexture", EngineMain::MainInstance->MainRenderer->depthResolveBuffer->id());

    shader->SetUniform("fog_start", FogManager::StartDistance);
    shader->SetUniform("fog_end", FogManager::EndDistance);
    shader->SetUniform("fog_opacity", FogManager::Opacity);
    shader->SetUniform("fog_color", FogManager::Color);


    shader->SetUniform("cameraPosition", Camera::finalizedPosition);
    shader->SetUniform("shadowMapSize", LightManager::ShadowMapResolution);

    shader->SetUniform("shaddowOffsetScale", LightManager::ShaddowOffsetScale);

}

inline ivec2 Renderer::GetScreenResolution() const
{
	return ivec2(EngineMain::MainInstance->ScreenSize.x * ResolutionScale, EngineMain::MainInstance->ScreenSize.y * ResolutionScale);
}

inline ivec2 Renderer::GetNativeScreenResolution() const
{
    return ivec2(EngineMain::MainInstance->ScreenSize.x, EngineMain::MainInstance->ScreenSize.y);
}

void Renderer::InitFullscreenVAO()
{
	float quadVertices[] = {
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
	};

	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glBindVertexArray(0);
}

void Renderer::InitFrameBuffers()
{

    TextureFormat colorTextureFormat = TextureFormat::RGBA16F;

    ivec2 screenResolution = GetScreenResolution();

    if (colorBuffer != nullptr)
    {
        delete(colorBuffer);
    }

    if (depthBuffer != nullptr)
    {
        delete(depthBuffer);
    }

    if (forwardFBO != nullptr)
    {
        delete(forwardFBO);
    }

    TextureType textureType = (MultiSampleCount > 0) ? TextureType::Texture2DMultisample : TextureType::Texture2D;

    colorBuffer = new RenderTexture(screenResolution.x, screenResolution.y, colorTextureFormat, textureType, false, GL_LINEAR, GL_LINEAR,
        GL_CLAMP_TO_EDGE, 1);


    depthBuffer = new RenderTexture(screenResolution.x, screenResolution.y, TextureFormat::Depth24, textureType, false, GL_LINEAR, GL_LINEAR,
        GL_CLAMP_TO_EDGE, 1);

    forwardFBO = new Framebuffer();
    forwardFBO->attachDepth(depthBuffer);
    forwardFBO->attachColor(colorBuffer, 0u);

    // resize all our buffers
    colorBuffer->resize(screenResolution.x, screenResolution.y);
    depthBuffer->resize(screenResolution.x, screenResolution.y);
    colorBuffer->setSamples(MultiSampleCount);
    depthBuffer->setSamples(MultiSampleCount);

}

void Renderer::InitResolveFrameBuffers()
{

    ivec2 screenResolution = GetScreenResolution();
    TextureFormat colorTextureFormat = TextureFormat::RGBA16F;

    //colorTextureFormat = TextureFormat::RGB8;

    colorResolveBuffer = new RenderTexture(screenResolution.x, screenResolution.y, colorTextureFormat, TextureType::Texture2D);
    customIdResolveBuffer = new RenderTexture(screenResolution.x, screenResolution.y, TextureFormat::RGB8, TextureType::Texture2D);
    depthResolveBuffer = new RenderTexture(screenResolution.x, screenResolution.y, TextureFormat::Depth24, TextureType::Texture2D);

    forwardResolveFBO = new Framebuffer();

    forwardResolveFBO->attachDepth(depthResolveBuffer);
    forwardResolveFBO->attachColor(colorResolveBuffer,0u);

    colorResolveBuffer->resize(screenResolution.x, screenResolution.y);
    customIdResolveBuffer->resize(screenResolution.x, screenResolution.y);
    depthResolveBuffer->resize(screenResolution.x, screenResolution.y);

    customIdFBO = new Framebuffer();
    customIdFBO->attachDepth(depthResolveBuffer);
    customIdFBO->attachColor(customIdResolveBuffer);

}
