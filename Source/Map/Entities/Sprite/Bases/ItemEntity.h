#pragma once
#include "../../Bases/SpriteEntity.h"
#include "../../Bases/InventorySlot.h"

class InventoryEntity;


// Items can exist either inside inventories, or as an sprite on the world
// so we inherit SpriteEntity, but it may not actually have a sprite all the time
class ItemEntity : public SpriteEntity
{
private:

	InventoryEntity* inventory;
	bool inWorld;

	sf::Vector2i inventoryPos;

public:

	// Should not be called while in the world
	InventoryEntity* getInventory();
	void setInventory(InventoryEntity* inv);
	
	bool isInInventory() { return !inWorld; }
	bool isInWorld() { return inWorld; }
	void setInWorld(sf::Vector2f pos);
	void setInInventory(sf::Vector2i pos);

	// Should not be called while in the world
	sf::Vector2i getInventoryPos();

	void setInventoryPos(sf::Vector2i pos);

	// Should be overwritten by certain items
	// Can be called anytime
	virtual sf::Vector2i getInventorySize();

	// By default items cannot be worn on special slots,
	// but overriding may change this behaviour
	virtual bool canGoInSlot(InventorySlot slot)
	{
		return slot == InventorySlot::NORMAL;
	}

	// Can the item be held in one hand, or does it require both?
	virtual bool isOneHanded()
	{
		return true;
	}

	ItemEntity(ProgramType* prog, uint32_t uid) : SpriteEntity(prog, uid), Entity(prog, uid)
	{

	}

	~ItemEntity();
};
