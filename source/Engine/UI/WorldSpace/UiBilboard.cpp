#include "UiBilboard.h"

#include "../UiRenderer.h"


static std::map<hashed_string, std::vector<RenderTexture*>> bilboardRtCache;

UiBilboard::~UiBilboard()
{
	if (renderTexture)
	{
		hashed_string hs = to_string(renderTexture->width()) + ";" + to_string(renderTexture->height());
		bilboardRtCache[hs].push_back(renderTexture); // store it in the vector
		renderTexture = nullptr;
	}
}

void UiBilboard::DrawForward(mat4x4 view, mat4x4 projection)
{

	ColorTextureId = renderTexture->id();


	if (model == nullptr) return;

	if (Transparent == false)
	{
		if (DepthWrite)
		{
			glDepthMask(GL_TRUE);
		}
		else
		{
			glDepthMask(GL_FALSE);
		}
	}


	if (TwoSided)
	{
		glDisable(GL_CULL_FACE);
	}
	else
	{
		glEnable(GL_CULL_FACE);
	}


	if (forward_shader_program == nullptr)
		forward_shader_program = ShaderManager::GetShaderProgram("default_vertex", PixelShader);

	forward_shader_program->UseProgram();


	forward_shader_program->SetUniform("masked", Masked);

	forward_shader_program->SetUniform("brightness", 1.0f * Brightness);

	mat4x4 world = finalizedWorld;


	forward_shader_program->SetUniform("view", view);
	forward_shader_program->SetUniform("projection", projection);
	forward_shader_program->SetUniform("viewmodelScaleFactor", ViewmodelScaleFactor);

	forward_shader_program->SetUniform("world", world);

	forward_shader_program->SetUniform("isViewmodel", IsViewmodel);

	forward_shader_program->SetUniform("view", view);

	forward_shader_program->SetUniform("customId", CustomId);




	if (ColorTexture)
	{
		forward_shader_program->SetTexture("u_texture", ColorTexture);
	}
	else
	{
		forward_shader_program->SetTexture("u_texture", ColorTextureId);
	}

	if (EmissiveTexture)
	{
		forward_shader_program->SetTexture("u_textureEmissive", EmissiveTexture);
	}
	else
	{
		forward_shader_program->SetTexture("u_textureEmissive", EmissiveTextureId);
	}


	model->meshes[0].VAO->Bind();

	glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(model->meshes[0].VAO->IndexCount), GL_UNSIGNED_INT, 0);

	VertexArrayObject::Unbind();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_CULL_FACE);


}

void UiBilboard::FinalizeFrameData()
{

	Rotation = Camera::rotation;

	Canvas.size = vec2((float)ViewportSize.x, (float)ViewportSize.y);

	Scale.x = (float)ViewportSize.x / PixelPerMeter;
	Scale.y = (float)ViewportSize.y / PixelPerMeter;

	Canvas.FinalizeChildren();

	StaticMesh::FinalizeFrameData();
}

void UiBilboard::Update()
{
	Canvas.Update();
	Canvas.UpdateChildrenOffsetRecursive();
}

void UiBilboard::DrawUi()
{

	EnsureRenderTarget();

	UiRenderer::customViewport = true;

	UiRenderer::customViewportSize = ViewportSize;

	renderTexture->bindFramebuffer();
	glViewport(0, 0, ViewportSize.x, ViewportSize.y);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // alpha = 0
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	Canvas.Draw();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	UiRenderer::customViewport = false;

}

void UiBilboard::PreDraw()
{
	StaticMesh::PreDraw();

	DrawUi();

}


void UiBilboard::EnsureRenderTarget()
{
	hashed_string hs = to_string(ViewportSize.x) + ";" + to_string(ViewportSize.y);

	// Try to reuse from cache first
	auto& vec = bilboardRtCache[hs];
	if (!vec.empty())
	{
		if (renderTexture)
		{
			// Push the old one back to cache before swapping
			hashed_string oldHS = to_string(renderTexture->width()) + ";" + to_string(renderTexture->height());
			bilboardRtCache[oldHS].push_back(renderTexture);
		}

		renderTexture = vec.back();
		vec.pop_back();
		return;
	}

	// No cached one found: either create new or resize existing
	if (renderTexture == nullptr)
	{
		renderTexture = new RenderTexture(
			ViewportSize.x,
			ViewportSize.y,
			TextureFormat::RGBA8,
			TextureType::Texture2D,
			false,
			GL_LINEAR,
			GL_LINEAR,
			GL_CLAMP_TO_EDGE
		);

		renderTexture->SetName("UiBilboard RT " + to_string(ViewportSize.x) + "x" + to_string(ViewportSize.y));

	}
	else if (renderTexture->width() != ViewportSize.x || renderTexture->height() != ViewportSize.y)
	{
		renderTexture->resize(ViewportSize.x, ViewportSize.y);
	}
}
