#include "UiRenderer.h"

#include "../gl.h"
#include "../ShaderManager.h"

#include "../Camera.h"

static GLuint quadVAO = 0;
static GLuint quadVBO = 0;
static ShaderProgram* texturedShader = nullptr;
static ShaderProgram* flatColorShader = nullptr;

void UiRenderer::Init() {
	float quadVertices[] = {
		// pos      // uv
		 0.0f, 1.0f,  0.0f, 1.0f,
		 1.0f, 0.0f,  1.0f, 0.0f,
		 0.0f, 0.0f,  0.0f, 0.0f,

		 0.0f, 1.0f,  0.0f, 1.0f,
		 1.0f, 1.0f,  1.0f, 1.0f,
		 1.0f, 0.0f,  1.0f, 0.0f,
	};

	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);

	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0); // position
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1); // texcoord
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glBindVertexArray(0);

	texturedShader = ShaderManager::GetShaderProgram("ui", "ui_textured");
	flatColorShader = ShaderManager::GetShaderProgram("ui", "ui_flatcolor");

}

void UiRenderer::Shutdown() {
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);
	delete texturedShader;
	delete flatColorShader;
}

void SetShaderProjection(ShaderProgram* shader)
{

	int screenWidth = 1080 * Camera::AspectRatio;
	int screenHeight = 1080;

	glm::mat4 uiProjection = glm::ortho(
		0.0f,
		static_cast<float>(screenWidth),
		static_cast<float>(screenHeight),
		0.0f,
		-1.0f,
		1.0f
	);
	shader->SetUniform("u_Projection", uiProjection);
}

void UiRenderer::DrawTexturedRect(const glm::vec2& pos, const glm::vec2& size, GLuint texture, const glm::vec4& color) {
	texturedShader->UseProgram();

	SetShaderProjection(texturedShader);

	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
	model = glm::scale(model, glm::vec3(size, 1.0f));
	texturedShader->SetUniform("u_Model", model);
	texturedShader->SetUniform("u_Color", color);

	texturedShader->SetTexture("u_Texture", texture);

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void UiRenderer::DrawBorderRect(const glm::vec2& pos, const glm::vec2& size, const glm::vec4& color) 
{
#ifndef __EMSCRIPTEN__



	flatColorShader->UseProgram();
	SetShaderProjection(flatColorShader);
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
	model = glm::scale(model, glm::vec3(size, 1.0f));
	flatColorShader->SetUniform("u_Model", model);
	flatColorShader->SetUniform("u_Color", color);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

#endif // !__EMSCRIPTEN
}

namespace UiRenderer {

	void DrawText(const std::string& text, TTF_Font* font, const glm::vec2& pos, const glm::vec4& color, const glm::vec2& scale)
	{
		if (!font) {
			std::cerr << "No font provided for DrawText." << std::endl;
			return;
		}

		// Convert our glm color (0.0 - 1.0) to SDL_Color (0 - 255)
		SDL_Color sdlColor = {
			static_cast<Uint8>(color.r * 255),
			static_cast<Uint8>(color.g * 255),
			static_cast<Uint8>(color.b * 255),
			static_cast<Uint8>(color.a * 255)
		};

		// Render the text to an SDL_Surface using blended rendering
		SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), sdlColor);
		if (!surface) {
			std::cerr << "TTF_RenderUTF8_Blended Error: " << TTF_GetError() << std::endl;
			return;
		}

		GLuint textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// Ensure proper pixel alignment and row length.
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, surface->pitch / surface->format->BytesPerPixel);

		// Set texture parameters for scaling and wrapping.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// Determine the format: use GL_RGB or GL_RGBA based on the surface.
		GLenum format = (surface->format->BytesPerPixel == 3) ? GL_RGB : GL_RGBA;
		glTexImage2D(GL_TEXTURE_2D, 0, format,
			surface->w, surface->h, 0, format,
			GL_UNSIGNED_BYTE, surface->pixels);

		// Reset UNPACK_ROW_LENGTH to default.
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

		// Enable blending so that the alpha channel is used.
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Compute the drawing size applying the provided scale.
		glm::vec2 drawSize(scale.x * surface->w, scale.y * surface->h);

		// Draw the text as a textured quad.
		DrawTexturedRect(pos, drawSize, textureID);

		// Cleanup texture and surface.
		glDeleteTextures(1, &textureID);
		SDL_FreeSurface(surface);
	}

} // namespace UiRenderer




