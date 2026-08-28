#include "BSPModelDrawCommand.h"

#include "Quake3Bsp.h"

#include "../ShaderManager.h"
#include "../Shader.hpp"
#include "../Camera.h"

#include <BgfxStateManager.h>
#include <Renderer/Abstractions/ViewIdManager.h>

void BSPModelDrawCommand::DrawForward(const mat4& view, const mat4& projection)
{
	if (!bsp || !model) return;

	auto state = BgfxStateManager::GetState();

	// No Transparent branch here - see the header comment.
	bsp->RenderBSP(Camera::finalizedPosition, *model, WorldMatrix, useBspVisibility, IsStatic);

	BgfxStateManager::SetState(state);
}

void BSPModelDrawCommand::DrawDepth(const mat4& view, const mat4& projection)
{
	if (!bsp) return;

	const auto& vbo = bsp->opaqueVBOs[id];
	if (vbo.IndexCount == 0) return;
	if (!bgfx::isValid(vbo.vbo) || !bgfx::isValid(vbo.ibo)) return;

	Shader* shader = ShaderManager::GetShaderProgram("bsp/vs_bsp", "bsp/fs_bsp_empty");
	shader->UseProgram();
	shader->SetUniform("view", view);
	shader->SetUniform("projection", projection);
	shader->SetUniform("model", WorldMatrix);

	bgfx::setVertexBuffer(0, vbo.vbo);
	bgfx::setIndexBuffer(vbo.ibo);

	BgfxStateManager::Apply();
	shader->Submit(ViewIdManager::GetCurrentId());
}
