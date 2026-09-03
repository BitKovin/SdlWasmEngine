#include <Entity.h>
#include <SkeletalMesh.hpp>
#include <Physics.h>

#include <imgui/imgui.h>
#include <imgui/imgui_stdlib.h>

class SkeletalEditorEntity : public Entity
{
public:
	SkeletalMesh* mesh;

	string filePath = "GameData/models/enemies/dog/dog.glb";

	int selectedHitbox = 0;
	int selectedAnimation = 0;
	int selectedAnimationEvent = 0;

	float animationTime = 0;

	bool animationPlaying = false;

	bool restPose = false;

	SkeletalEditorEntity()
	{

		mesh = new SkeletalMesh(this);

		mesh->LoadFromFile("GameData/models/enemies/dog/dog.glb");
		mesh->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/models/enemies/dog/cat.png");
		Drawables.push_back(mesh);
		mesh->PlayAnimation("run", true);

		Physics::DebugDraw = true;
	}

	void Update()
	{
		if (mesh->InRagdoll == false)
		{
			mesh->CreateHitboxes(this);
		}
		
		if (restPose)
		{
			mesh->PasteAnimationPose(AnimationPose());
		}
		else
		{
			mesh->Update((int)animationPlaying);
		}
		
		mesh->UpdateHitboxes();

		auto events = mesh->PullAnimationEvents();

		for (auto event : events)
		{
			Logger::Log(event.eventName);
		}


	}

	// Callback to resize the std::string buffer when editing
	static int InputTextCallback(ImGuiInputTextCallbackData* data)
	{
		if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
		{
			std::string* str = (std::string*)data->UserData;
			str->resize(data->BufTextLen);
			data->Buf = (char*)str->c_str();
		}
		return 0;
	}

	void LoadModel()
	{
		mesh->ClearHitboxes();
		mesh->LoadFromFile(filePath);
	}

	vector<const char*> GetHitboxNames()
	{
		vector<const char*> hitboxes;
		hitboxes.reserve(mesh->metaData.hitboxes.size());
		for (const auto& hitbox : mesh->metaData.hitboxes)  // ← note const auto&
			hitboxes.push_back(hitbox.boneName.c_str());
		return hitboxes;
	}


	void HitboxWindow(HitboxData* hitbox)
	{
		ImGui::Begin("hitbox editor");

		static char buffer[256];  // Fixed-size buffer (adjust size as needed)
		strncpy_s(buffer, hitbox->boneName.c_str(), sizeof(buffer));
		buffer[sizeof(buffer) - 1] = '\0';  // Ensure null termination

		if (ImGui::InputText("bone name", buffer, sizeof(buffer)))
		{
			hitbox->boneName = std::string(buffer);  // Update std::string only if user edited the input
		}

		ImGui::DragFloat3("position", &hitbox->position.x, 0.001f);
		ImGui::DragFloat3("rotation", &hitbox->rotation.x, 0.1f);
		ImGui::DragFloat3("size", &hitbox->size.x, 0.001f, 0.001);

		static char buffer2[256];  // Fixed-size buffer (adjust size as needed)
		strncpy_s(buffer2, hitbox->parentBone.c_str(), sizeof(buffer2));
		buffer2[sizeof(buffer2) - 1] = '\0';  // Ensure null termination

		if (ImGui::InputText("parent name", buffer2, sizeof(buffer2)))
		{
			hitbox->parentBone = std::string(buffer2);  // Update std::string only if user edited the input
		}

		ImGui::DragFloat3("twistMinAngle, twistMaxAngle, swingHalfConeAngle", &hitbox->twistParameters.x, 0.5);
		ImGui::DragFloat3("constraint rotation", &hitbox->constraintRotation.x, 1);

		hitbox->size.x = abs(hitbox->size.x);
		hitbox->size.y = abs(hitbox->size.y);
		hitbox->size.z = abs(hitbox->size.z);

		ImGui::DragFloat("mass", &hitbox->mass, 0.2f,0);

		ImGui::InputFloat("damage multiplier", &hitbox->damageMultiplier);

		ImGui::End();
	}

	vector<const char*> GetAnimationNames()
	{
		vector<const char*> animations;
		animations.reserve(mesh->model->animations.size());
		for (const auto& animation : mesh->model->animations) 
		{ 
			animations.push_back(animation.first.str().c_str());
		}
		return animations;
	}

	vector<const char*> GetAnimationEventsNames(AnimationData* data)
	{
		vector<const char*> events;
		events.reserve(data->animationEvents.size());
		for (const auto& event : data->animationEvents)
		{
			events.push_back(event.eventName.c_str());
		}
		return events;
	}

	void UpdateDebugUI()
	{

		ImGui::Begin("file");

		static char buffer[256];  // Fixed-size buffer (adjust size as needed)
		strncpy_s(buffer, filePath.c_str(), sizeof(buffer));
		buffer[sizeof(buffer) - 1] = '\0';  // Ensure null termination

		if (ImGui::InputText("model path", buffer, sizeof(buffer)))
		{
			filePath = std::string(buffer);  // Update std::string only if user edited the input
		}

		if (ImGui::Button("load model"))
		{
			LoadModel();
		}

		if (ImGui::Button("save model"))
		{
			mesh->SaveMetaToFile();
		}

		ImGui::Spacing();

		ImGui::End();

		ImGui::Begin("hitbox selector");

		ImGui::Spacing();

		vector<const char*> names = GetHitboxNames();

		ImGui::ListBox("hitboxes:", &selectedHitbox, names.data(), names.size());


		if (ImGui::Button("add"))
		{
			mesh->metaData.hitboxes.push_back(HitboxData{ "none" });
		}

		ImGui::SameLine();

		if (ImGui::Button("remove"))
		{
			if (selectedHitbox >= 0 && selectedHitbox < names.size())
			{
				mesh->metaData.hitboxes.erase(mesh->metaData.hitboxes.begin() + selectedHitbox);
			}
		}

		if (ImGui::Button("Start Ragdoll"))
		{
			mesh->StartRagdoll();
		}

		if (ImGui::Button("Stop Ragdoll"))
		{
			mesh->StopRagdoll();
		}

		ImGui::Checkbox("rest pose", &restPose);
		ImGui::DragFloat("pose follow", &mesh->RagdollPoseFollowStrength,0.1f,0,10);

		HitboxData* selectedHitboxRef = nullptr;

		if (selectedHitbox >= 0 && selectedHitbox < names.size())
		{
			selectedHitboxRef = &mesh->metaData.hitboxes[selectedHitbox];
		}

		ImGui::End();

		if (selectedHitboxRef)
		{
			HitboxWindow(selectedHitboxRef);

			auto constraint = mesh->GetConstraintByHitboxName(selectedHitboxRef->boneName);

			Physics::DrawConstraint(constraint);

		}


		ImGui::Begin("Animation Editor");

		ImGui::DragFloat("Speed", &Time::TimeScale, 0.003f, 0, 2);

		auto animationNames = GetAnimationNames();

		if (ImGui::ListBox("animations:", &selectedAnimation, animationNames.data(), animationNames.size()))
		{
			mesh->PlayAnimation(animationNames[selectedAnimation], true, 0.2);

		}

		ImGui::Checkbox("playing", &animationPlaying);

		float animationTime = mesh->GetAnimationTime();

		if (ImGui::SliderFloat("time", &animationTime, 0, mesh->GetAnimationDuration()))
		{
			mesh->SetAnimationTime(animationTime);
		}

		AnimationData* selectedAnimationRef = mesh->currentAnimationData;

		if (selectedAnimationRef)
		{
			auto eventNames = GetAnimationEventsNames(selectedAnimationRef);

			if (ImGui::ListBox("events:", &selectedAnimationEvent, eventNames.data(), eventNames.size()))
			{
				
			}

			if (ImGui::Button("add event"))
			{

				AnimationEvent newEvent;
				newEvent.eventName = "event_" + to_string(floor(animationTime / 10.f));
				newEvent.time = animationTime;

				selectedAnimationRef->animationEvents.push_back(newEvent);
			}

			ImGui::SameLine();

			if (ImGui::Button("remove event"))
			{
				if (selectedAnimationEvent >= 0 && selectedAnimationEvent < selectedAnimationRef->animationEvents.size())
				{
					selectedAnimationRef->animationEvents.erase(selectedAnimationRef->animationEvents.begin() + selectedAnimationEvent);
				}
			}

			if (eventNames.size() > 0)
			{
				AnimationEvent* selectedAnimationEventRef = &selectedAnimationRef->animationEvents[selectedAnimationEvent];

				if (selectedAnimationEventRef)
				{


					ImGui::InputText("event name", &selectedAnimationEventRef->eventName);

					ImGui::DragFloat("event time", &selectedAnimationEventRef->time,0.01,0, mesh->GetAnimationDuration());


					ImGui::InputText("event data 1", &selectedAnimationEventRef->userData1);

					ImGui::InputText("event data 2", &selectedAnimationEventRef->userData2);

					ImGui::InputText("event data 3", &selectedAnimationEventRef->userData3);


				}

			}

		}

		ImGui::End();

	}

};

REGISTER_ENTITY(SkeletalEditorEntity, "skeletal_editor")