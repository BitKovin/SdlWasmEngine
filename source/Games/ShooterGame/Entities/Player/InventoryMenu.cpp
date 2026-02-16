#include "InventoryMenu.h"

#include <Entities/Player/Player.hpp>

#include <PauseGameManager.hpp>

void InventoryMenu::Finalize()
{

	Entity::Finalize();

	auto& inventory = Player::Instance->GetInventory();

	DestroyDrawables();

	float totalItems = (float)inventory.size();
	float anglePerItem = (2.0f * M_PI) / totalItems;
	float radius = 0.25f; // Adjust this to change circle size

	if (totalItems > 4)
	{
		radius += radius / 4 * (totalItems - 4); // Increase radius for more items to prevent overlap
	}

	// Calculate target position using shortest circular path
	float targetPosition = (float)currentSlotIndex;

	// Adjust target to be closest to current slotPosition (shortest path around circle)
	while (targetPosition - slotPosition > totalItems / 2.0f) {
		targetPosition -= totalItems;
	}
	while (slotPosition - targetPosition > totalItems / 2.0f) {
		targetPosition += totalItems;
	}

	slotPosition = MathHelper::Interp(slotPosition, targetPosition, Time::DeltaTimeF, 10.0f);

	int index = -1;

	for (auto& item : inventory)
	{

		index++;

		float offset = index - slotPosition;

		SkeletalMesh* mesh = new SkeletalMesh(this);

		std::string modelPath = "GameData/models/weapons/glock.glb";

		mesh->LoadFromFile(modelPath);
		mesh->TexturesLocation = modelPath + "/";
		mesh->PreloadAssets();
		mesh->IsViewmodel = true;
		mesh->ViewmodelScaleFactor = 0.2f;
		Drawables.push_back(mesh);

		// Calculate circular position
		float angle = offset * anglePerItem;

		vec3 circleCenter = Camera::position + Camera::Forward() * (radius + 0.45f) + Camera::Up() * -0.15f;
		vec3 circleOffset = Camera::Forward() * (cos(angle) * radius) * -1.0f + Camera::Right() * (sin(angle) * radius);
		vec3 position = circleCenter + circleOffset;

		mesh->Position = position;

		mesh->Rotation = Camera::rotation;


	}

	oldSlot = currentSlotIndex;

}

void InventoryMenu::Start()
{

	Entity::Start();

	oldSlot = GetCurrentVisualSlotIndex();
	currentSlotIndex = oldSlot;
	slotPosition = (float)oldSlot;

	PauseGameManager::SetGamePausedGameplay(true);

	UpdateWhenPaused = true;

}

void InventoryMenu::Update()
{
	Position = Camera::position;

	if (Input::GetAction("ui_cancel")->Pressed() || Input::GetAction("inventory")->Pressed())
	{
		Destroy();
		return;
	}

	if (Input::GetAction("ui_confirm")->Pressed())
	{
		Player::Instance->SwitchToInventoryItem(Player::Instance->inventory[currentSlotIndex].uid);
		Destroy();
		return;
	}

	if (Input::GetAction("ui_right")->Pressed() || Input::GetAction("right")->Pressed())
	{
		currentSlotIndex++;
		if (currentSlotIndex >= Player::Instance->GetInventory().size())
			currentSlotIndex = 0;

	}
	else if (Input::GetAction("ui_left")->Pressed() || Input::GetAction("left")->Pressed())
	{
		currentSlotIndex--;
		if (currentSlotIndex < 0)
			currentSlotIndex = Player::Instance->GetInventory().size() - 1;
	}

}

void InventoryMenu::Destroy()
{

	Entity::Destroy();
	PauseGameManager::SetGamePausedGameplay(false);

}

int InventoryMenu::GetCurrentVisualSlotIndex()
{

	int index = Player::Instance->GetInventorySlotIdByUUID(Player::Instance->currentInventoryUUID);
	if (index == -1)
	{
		index = Player::Instance->GetInventorySlotIdByUUID(Player::Instance->currentMainWeaponUUID);
	}
	if (index == -1)
	{
		index = Player::Instance->GetInventorySlotIdByUUID(Player::Instance->currentOffhandWeaponUUID);
	}
	if (index == -1)
	{
		//currentSlotIndex = Player::Instance->GetInventorySlotIdByUUID(Player::Instance->lastInventoryUUID);
	}
	if (index == -1)
	{
		index = 0;
	}

	return index;

}

REGISTER_ENTITY(InventoryMenu, "inventory_menu")