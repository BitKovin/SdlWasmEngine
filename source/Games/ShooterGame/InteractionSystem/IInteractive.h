#pragma once


#include <string>

class Entity;

class IInteractive
{
public:
	

	virtual bool CanBeInteracted() { return true; };
	virtual bool CanBeInteractedSecondary() { return true; };

	virtual bool HasSecondaryInteraction(){ return false; }

	virtual void Interact(Entity* interactingEntity) {}
	virtual void InteractSecondary(Entity* interactingEntity){}

	virtual float GetSecondaryInteractionHoldTime() { return 1; }

	virtual void InterruptedSecondaryInteractionHold(float progress) {};
	virtual void PerformingSecondaryInteractionHold(float progress) {};

};

