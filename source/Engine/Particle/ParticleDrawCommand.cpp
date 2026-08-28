#include "ParticleDrawCommand.h"

#include "ParticleEmitter.h"

#include "../ShaderManager.h"
#include "../Shader.hpp"
#include "../Renderer/Renderer.h"
#include "../AssetRegistry.h"

#include <Renderer/Abstractions/ViewIdManager.h>

// ParticleDrawCommand
void ParticleDrawCommand::DrawForward(const mat4& view, const mat4& projection)
{
	if (Instances.empty()) return;

	ParticleEmitter::InitBilboardVaoIfNeeded();

	bgfx::VertexBufferHandle vbh = ParticleEmitter::GetBillboardVbh();
	bgfx::IndexBufferHandle ibh = ParticleEmitter::GetBillboardIbh();
	if (!bgfx::isValid(vbh) || !bgfx::isValid(ibh)) return;

	auto startState = BgfxStateManager::GetState();

	BgfxStateManager::SetWriteDepth(false);
	BgfxStateManager::SetCull(BgfxStateManager::Cull::None);
	BgfxStateManager::SetBlend(BlendMode);

	Shader* shader = ShaderManager::GetShaderProgram("vs_instanced_billboard", PixelShader);
	if (shader == nullptr) return;

	const uint32_t totalInstances = static_cast<uint32_t>(Instances.size());
	const uint16_t instanceStride = static_cast<uint16_t>(sizeof(InstanceData));

	uint32_t drawInstances = bgfx::getAvailInstanceDataBuffer(totalInstances, instanceStride);
	if (drawInstances == 0) return; // nothing to draw this frame

	bgfx::InstanceDataBuffer idb;
	bgfx::allocInstanceDataBuffer(&idb, drawInstances, instanceStride);
	memcpy(idb.data, Instances.data(), drawInstances * instanceStride);

	shader->UseProgram();

	shader->SetUniform("view", view);
	shader->SetUniform("projection", projection);
	shader->SetUniform("is_decal", IsDecal);

	shader->SetUniform("viewmodelScaleFactor", 1);
	shader->SetUniform("isViewmodel", IsViewmodel);

	Renderer::SetSurfaceShaderUniforms(shader);

	shader->SetTexture("u_texture", ResolvedTexture);
	shader->SetUniform("viewProjectionInv", glm::inverse(projection * view));

	bgfx::setVertexBuffer(0, vbh);
	bgfx::setIndexBuffer(ibh);
	bgfx::setInstanceDataBuffer(&idb);

	BgfxStateManager::Apply();
	shader->Submit(ViewIdManager::GetCurrentId());

	BgfxStateManager::SetState(startState);
}

// RibbonDrawCommand
void RibbonDrawCommand::DrawForward(const mat4& view, const mat4& projection)
{
	if (Vertices.size() < 2 || Indices.empty()) return;

	auto startState = BgfxStateManager::GetState();

	BgfxStateManager::SetWriteDepth(false);
	BgfxStateManager::SetCull(BgfxStateManager::Cull::None);
	BgfxStateManager::SetBlend(BlendMode);

	Shader* shader = ShaderManager::GetShaderProgram("vs_default", PixelShader);
	if (shader == nullptr) return;

	shader->UseProgram();

	shader->SetUniform("view", view);
	shader->SetUniform("projection", projection);
	shader->SetUniform("world", glm::identity<mat4>());
	shader->SetUniform("isViewmodel", IsViewmodel);
	shader->SetUniform("is_particle", true);
	shader->SetUniform("is_decal", false);
	shader->SetUniform("viewmodelScaleFactor", 1);

	Renderer::SetSurfaceShaderUniforms(shader);
	shader->SetTexture("u_texture", ResolvedTexture);

	const bgfx::VertexLayout layout = VertexData::Declaration();
	const uint32_t vCount = static_cast<uint32_t>(Vertices.size());
	const uint32_t idxCount = static_cast<uint32_t>(Indices.size());

	if (bgfx::getAvailTransientVertexBuffer(vCount, layout) < vCount ||
		bgfx::getAvailTransientIndexBuffer(idxCount, true) < idxCount)
	{
		BgfxStateManager::SetState(startState);
		return;
	}

	bgfx::TransientVertexBuffer tvb;
	bgfx::TransientIndexBuffer tib;
	bgfx::allocTransientVertexBuffer(&tvb, vCount, layout);
	bgfx::allocTransientIndexBuffer(&tib, idxCount, true);
	memcpy(tvb.data, Vertices.data(), vCount * sizeof(VertexData));
	memcpy(tib.data, Indices.data(), idxCount * sizeof(uint32_t));

	bgfx::setVertexBuffer(0, &tvb);
	bgfx::setIndexBuffer(&tib);

	BgfxStateManager::Apply();
	shader->Submit(ViewIdManager::GetCurrentId());

	BgfxStateManager::SetState(startState);
}

// TrailDrawCommand
void TrailDrawCommand::DrawForward(const mat4& view, const mat4& projection)
{
	if (Vertices.size() < 2 || Indices.empty()) return;

	auto startState = BgfxStateManager::GetState();

	BgfxStateManager::SetWriteDepth(true);
	BgfxStateManager::SetCull(BgfxStateManager::Cull::None);
	BgfxStateManager::SetBlend(BlendMode);

	// Enable stencil: PASS only if stencil == 0, then increment to 1
	uint32_t stencilState =
		BGFX_STENCIL_TEST_EQUAL
		| BGFX_STENCIL_FUNC_REF(0)
		| BGFX_STENCIL_FUNC_RMASK(0xFF)
		| BGFX_STENCIL_OP_FAIL_S_KEEP
		| BGFX_STENCIL_OP_FAIL_Z_KEEP
		| BGFX_STENCIL_OP_PASS_Z_INCR;

	bgfx::setStencil(stencilState, stencilState);

	Shader* shader = ShaderManager::GetShaderProgram("vs_default", PixelShader);
	if (shader == nullptr) return;

	shader->UseProgram();
	shader->SetUniform("view", view);
	shader->SetUniform("projection", projection);
	shader->SetUniform("world", glm::identity<mat4>());
	shader->SetUniform("isViewmodel", IsViewmodel);
	shader->SetUniform("is_particle", true);
	shader->SetUniform("is_decal", false);
	shader->SetUniform("viewmodelScaleFactor", 1);
	Renderer::SetSurfaceShaderUniforms(shader);
	shader->SetTexture("u_texture", ResolvedTexture);

	const bgfx::VertexLayout layout = VertexData::Declaration();
	const uint32_t vCount = static_cast<uint32_t>(Vertices.size());
	const uint32_t idxCount = static_cast<uint32_t>(Indices.size());

	if (bgfx::getAvailTransientVertexBuffer(vCount, layout) < vCount ||
		bgfx::getAvailTransientIndexBuffer(idxCount, true) < idxCount)
	{
		bgfx::setStencil(BGFX_STENCIL_NONE, BGFX_STENCIL_NONE);
		BgfxStateManager::SetState(startState);
		return;
	}

	bgfx::TransientVertexBuffer tvb;
	bgfx::TransientIndexBuffer tib;
	bgfx::allocTransientVertexBuffer(&tvb, vCount, layout);
	bgfx::allocTransientIndexBuffer(&tib, idxCount, true);
	memcpy(tvb.data, Vertices.data(), vCount * sizeof(VertexData));
	memcpy(tib.data, Indices.data(), idxCount * sizeof(uint32_t));

	bgfx::setVertexBuffer(0, &tvb);
	bgfx::setIndexBuffer(&tib);

	BgfxStateManager::Apply();
	shader->Submit(ViewIdManager::GetCurrentId());

	// --- Second pass: reset stencil back to 0 where we just drew ---
	uint32_t stencilStateClear =
		BGFX_STENCIL_TEST_ALWAYS
		| BGFX_STENCIL_FUNC_REF(0)
		| BGFX_STENCIL_FUNC_RMASK(0xFF)
		| BGFX_STENCIL_OP_FAIL_S_KEEP
		| BGFX_STENCIL_OP_FAIL_Z_KEEP
		| BGFX_STENCIL_OP_PASS_Z_REPLACE;

	// Rebind the identical frame-allocated buffers.
	bgfx::setVertexBuffer(0, &tvb);
	bgfx::setIndexBuffer(&tib);

	bgfx::setStencil(stencilStateClear, stencilStateClear);
	BgfxStateManager::SetWriteAlpha(false);
	BgfxStateManager::SetWriteRGB(false);
	BgfxStateManager::Apply();
	shader->Submit(ViewIdManager::GetCurrentId());

	bgfx::setStencil(BGFX_STENCIL_NONE, BGFX_STENCIL_NONE);
	BgfxStateManager::SetState(startState);
}
