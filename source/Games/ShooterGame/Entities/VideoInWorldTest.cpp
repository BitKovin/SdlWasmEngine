#include <Entity.h>
#include <UI/WorldSpace/UiBilboard.h>
#include <UI/UiVideo.hpp>

class VideoInWorldTest : public Entity
{
public:
	VideoInWorldTest(){}
	
	UiBilboard* bilboard = nullptr;
	std::shared_ptr<UiVideo> uiVideo;

	void LoadAssets() override
	{

		uiVideo = make_shared<UiVideo>();
		uiVideo->VideoPath = "GameData/videos/writeshit.mpg";
		uiVideo->size = ivec2(512);

		bilboard = new UiBilboard(this);
		bilboard->TwoSided = true;
		bilboard->Position = Position;
		bilboard->ViewportSize = ivec2(512);
		bilboard->Canvas.AddChild(uiVideo);
		bilboard->blendMode = BgfxStateManager::Blend::Additive;
		bilboard->PixelPerMeter = 256;
		Drawables.push_back(bilboard);

	}

	void Update() override
	{
		bilboard->Update();
	}

private:

};

REGISTER_ENTITY(VideoInWorldTest, "videoInWorld")