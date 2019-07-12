#pragma once

#include "../Bases/ItemEntity.h"

// Commands:
//
// Generic Calls:
// 	- setLightOn (CLIENT<-SERVER)
//		-> bool value
//
class EFlashlight : public ItemEntity
{
private:

	bool lightOn;
	Light light;
	bool isLightAdded;

public:


	virtual EntityType getEntityType() { return ITEM_FLASHLIGHT; }

	virtual std::vector<std::string> getContextualMenuEntries()
	{
		return { "Toggle" };
	}

	virtual void doContextualMenuAction(std::string action);

	virtual void update(float dt);

	EFlashlight(ProgramType* prog, uint32_t uid);
	~EFlashlight();
};

