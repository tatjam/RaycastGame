#include "ItemEntity.h"




InventoryEntity * ItemEntity::getInventory()
{
	DCHECK(isInInventory()) << "Tried to get the inventory of a ItemEntity dropped in the world";

	return inventory;
}

void ItemEntity::setInventory(InventoryEntity * inv)
{
	inventory = inv;
}

void ItemEntity::setInWorld(sf::Vector2f pos)
{
	inWorld = true;
	getSprite()->pos = pos;
}

void ItemEntity::setInInventory(sf::Vector2i pos)
{
	inWorld = false;
	inventoryPos = pos;
}


sf::Vector2i ItemEntity::getInventoryPos()
{
	DCHECK(isInInventory()) << "Tried to get the ìnventory position of a ItemEntity dropped in the world";

	return inventoryPos;
}

void ItemEntity::setInventoryPos(sf::Vector2i pos)
{
	DCHECK(isInInventory()) << "Tried to set the ìnventory position of a ItemEntity dropped in the world";

	inventoryPos = pos;
}

sf::Vector2i ItemEntity::getInventorySize()
{
	return sf::Vector2i(1, 1);
}

ItemEntity::~ItemEntity()
{
}
