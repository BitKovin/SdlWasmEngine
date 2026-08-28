#include "DetailShadowUtils.h"

#include "../ShaderManager.h"
#include "../Shader.hpp"
#include "../Renderer/Renderer.h"

#include <BgfxStateManager.h>

namespace DetailShadowUtils
{

void ApplyShadowDarkening(const vec3& shadowColor)
{
	BgfxStateManager::SetCull(BgfxStateManager::Cull::CW);
	BgfxStateManager::SetWriteRGB(true);
	BgfxStateManager::SetWriteAlpha(true);
	BgfxStateManager::SetBlend(BgfxStateManager::Blend::Multiply);
	BgfxStateManager::SetWriteDepth(false);
	BgfxStateManager::SetMSAA(false);
	BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::None);
	BgfxStateManager::Apply();

	Shader* colorShader = ShaderManager::GetShaderProgram("vs_fullscreen", "fs_fullscreen_color");
	colorShader->UseProgram();
	colorShader->SetUniform("u_color", vec4(shadowColor, 1.0f));

	bgfx::setStencil(
		BGFX_STENCIL_TEST_NOTEQUAL | BGFX_STENCIL_FUNC_REF(0) |
		BGFX_STENCIL_FUNC_RMASK(0xFF) | BGFX_STENCIL_OP_FAIL_S_KEEP |
		BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_KEEP);
	BgfxStateManager::Apply();
	Renderer::Instance->RenderFullscreenQuad(colorShader);
}

void ClearShadowStencil()
{
	BgfxStateManager::SetWriteRGB(false);
	BgfxStateManager::SetWriteAlpha(false);
	BgfxStateManager::SetWriteDepth(false);
	BgfxStateManager::Apply();

	Shader* colorShader = ShaderManager::GetShaderProgram("vs_fullscreen", "fs_fullscreen_color");

	bgfx::setStencil(
		BGFX_STENCIL_TEST_NOTEQUAL | BGFX_STENCIL_FUNC_REF(0) |
		BGFX_STENCIL_FUNC_RMASK(0xFF) | BGFX_STENCIL_OP_FAIL_S_KEEP |
		BGFX_STENCIL_OP_FAIL_Z_KEEP | BGFX_STENCIL_OP_PASS_Z_REPLACE);
	BgfxStateManager::Apply();
	Renderer::Instance->RenderFullscreenQuad(colorShader);
}

}
