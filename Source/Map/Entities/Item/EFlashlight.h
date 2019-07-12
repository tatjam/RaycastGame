#pragma once

#include "../Bases/ItemEntity.h"


class EFlashlight : public ItemEntity
{
public:

	virtual EntityType getEntityType() { return ITEM_FLASHLIGHT; }


	EFlashlight(ProgramType* prog, uint32_t uid);
	~EFlashlight();
};

