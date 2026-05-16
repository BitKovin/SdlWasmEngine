#include <Entity.h>

#include <Renderer/Abstractions/RenderTexture.h>
#include <EngineMain.h>

class LevelEndLogic : public Entity
{
public:
	
	std::string nextLevel = "";

	bool hasToCaptureFrame = false;

	void FromData(EntityData data) override
	{
		Entity::FromData(data);
		
		nextLevel = data.GetPropertyString("nextLevel", "");

	}

	void Finalize() override
	{
		Entity::Finalize();

		if (hasToCaptureFrame == false) return;

		if (LevelEndFrame == nullptr 
			|| LevelEndFrame->width() != EngineMain::MainInstance->FinalFrameRenderTexture->width() 
			|| LevelEndFrame->height() != EngineMain::MainInstance->FinalFrameRenderTexture->height())
		{

			if(LevelEndFrame)
				delete(LevelEndFrame);

			LevelEndFrame = new RenderTexture(
				EngineMain::MainInstance->FinalFrameRenderTexture->width(), 
				EngineMain::MainInstance->FinalFrameRenderTexture->height(), TextureFormat::RGBA8);
		}

		LevelEndFrame->copyFrom(EngineMain::MainInstance->FinalFrameRenderTexture);
		hasToCaptureFrame = false;
	}

	void UpdateDebugUI() override
	{
		if(LevelEndFrame)
			ImGui::Image((ImTextureID)(uintptr_t)LevelEndFrame->textureHandle().idx, ImVec2(512, 512));
	}

	void OnAction(string action) override
	{
		Spawn("level_end_camera");
		hasToCaptureFrame = true;
	}

private:

	static inline RenderTexture* LevelEndFrame = nullptr;

};

REGISTER_ENTITY(LevelEndLogic, "logic_level_end")