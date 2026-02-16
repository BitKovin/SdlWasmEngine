#include "InventoryMenu.h"

#include <Entities/Player/Player.hpp>

#include <PauseGameManager.hpp>

#include <ItemsDataBase.h>

void InventoryMenu::Finalize()
{
	Entity::Finalize();

	auto& inventory = Player::Instance->GetInventory();

	DestroyDrawables();

	float totalItems = (float)inventory.size();
	float anglePerItem = (2.0f * M_PI) / totalItems;
	float radius = 0.7f; // Adjust this to change circle size

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

		ItemDbEntry itemData = ItemsDataBase::GetItemData(item.itemID);

		std::string modelPath = itemData.modelPath;

		mesh->LoadFromFile(modelPath);
		mesh->TexturesLocation = modelPath + "/";
		mesh->PreloadAssets();
		mesh->IsViewmodel = true;
		mesh->ViewmodelScaleFactor = 0.2f;
		Drawables.push_back(mesh);

		mesh->FinalizeFrameData();

		auto bounds = mesh->GetBoundingBox();
		vec3 boundsCenter = bounds.Center();

		// Calculate circular position
		float angle = offset * anglePerItem;

		vec3 circleCenter = Camera::position + Camera::Forward() * (radius + 0.52f) + Camera::Up() * -0.15f;
		vec3 circleOffset = Camera::Forward() * (cos(angle) * radius) * -1.0f + Camera::Right() * (sin(angle) * radius);
		vec3 position = circleCenter + circleOffset;

		float rotation = 90;

		if (index == currentSlotIndex)
		{
			rotation += itemRotationTime * 180;
		}

		// Calculate final rotation (same as what will be applied to mesh)
		quat cameraRotation = MathHelper::GetRotationQuaternion(Camera::rotation);
		quat itemRotation = MathHelper::GetRotationQuaternion(vec3(0, rotation, 0));
		quat finalRotation = cameraRotation * itemRotation;

		// Transform bounds center by the rotation
		vec3 rotatedBoundsCenter = finalRotation * boundsCenter;

		// Center the mesh at its rotated bounds center
		mesh->Position = position - rotatedBoundsCenter;
		

		mesh->Rotation = MathHelper::ToYawPitchRoll(finalRotation);
	}

	itemRotationTime += Time::DeltaTimeF;

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

		itemRotationTime = 0;

	}
	else if (Input::GetAction("ui_left")->Pressed() || Input::GetAction("left")->Pressed())
	{
		currentSlotIndex--;
		if (currentSlotIndex < 0)
			currentSlotIndex = Player::Instance->GetInventory().size() - 1;

		itemRotationTime = 0;

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