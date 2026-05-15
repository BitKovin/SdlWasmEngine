#pragma once

#include "../Level.hpp"
#include "../glm.h"

#include "Abstractions/FrameBuffer.h"
#include "Abstractions/RenderTexture.h"

#include "../ShaderManager.h"
#include "../Shader.hpp"

#include <bgfx/bgfx.h>

class Renderer
{
public:
	Renderer();
	~Renderer();

	static inline Renderer* Instance = nullptr;

	void RenderLevel(Level* level, bgfx::FrameBufferHandle targetFrameBuffer);

	void RenderCameraForward(vector<IDrawMesh*>& VissibleRenderList);

	void RenderDirectionalLightShadows(vector<IDrawMesh*>& ShadowRenderList, Framebuffer& fbo, int numCascades);

	void RenderFullscreenQuad(Shader* shader);

	static void SetSurfaceShaderUniforms(Shader* shader, float brightnessScale = 1);

	int MultiSampleCount = 8;

	bool FXAAEnabled = false;

	float ResolutionScale = 1.0f;

	Shader* fullscreenShader = nullptr;
	Shader* fullscreenTextureShader = nullptr;

	Shader* copyShader = nullptr;
	Shader* depthCopyShader = nullptr;
	Shader* depthMsaaResolveShader = nullptr;

private:

	inline ivec2 GetScreenResolution() const;
	inline ivec2 GetNativeScreenResolution() const;

	void InitFullscreenBuffers();

	void InitFrameBuffers();
	void InitResolveFrameBuffers();

	bgfx::VertexLayout m_fullscreenLayout;
	bgfx::VertexBufferHandle m_fullscreenVB;

	RenderTexture* colorBuffer = nullptr;
	RenderTexture* depthBuffer = nullptr;

	RenderTexture* BlurResultBuffer = nullptr;
	RenderTexture* BlurAccumulatedBuffer = nullptr;

	RenderTexture* colorResolveBuffer = nullptr;
	RenderTexture* depthResolveBuffer = nullptr;

	Framebuffer* forwardFBO = nullptr;
	Framebuffer* forwardResolveFBO = nullptr;

	RenderTexture* customIdResolveBuffer = nullptr;
	Framebuffer* customIdFBO = nullptr;

	RenderTexture* DirectionalShadowMap = nullptr;
	Framebuffer* DirectionalShadowMapFBO = nullptr;

	RenderTexture* DetailDirectionalShadowMap = nullptr;
	Framebuffer* DetailDirectionalShadowMapFBO = nullptr;


	Shader* blurShader = nullptr;
	Shader* blurApplyShader = nullptr;

};