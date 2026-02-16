#pragma once

#include <Entity.h>

class InventoryMenu : public Entity
{

	void Finalize() override;

	void Start() override;

	void Update() override;

	void Destroy() override;

	float slotPosition = 0;
	int currentSlotIndex = 0;
	int oldSlot = 0;
	float itemRotationTime = 0;

	int GetCurrentVisualSlotIndex();

};