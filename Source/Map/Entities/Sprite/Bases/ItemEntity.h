#pragma once
#include "../../Bases/SpriteEntity.h"

// Items can exist either inside inventories, or as an sprite on the world
// so we inherit SpriteEntity, but it may not actually have a sprite all the time
class ItemEntity : public SpriteEntity
{
public:
	ItemEntity(ProgramType* prog, uint32_t uid) : SpriteEntity(prog, uid)
	{

	}

	~ItemEntity();
};

