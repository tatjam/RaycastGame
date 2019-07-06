#pragma once

#include "../../Entity.h"
#include "../../Map.h"
#include "../../Tile.h"

class ItemEntity;

// An InventoryEntity can hold items in particular
// arrangements in a grid. The drawing of the grid
// is not done here.
class InventoryEntity : public Entity
{
private:

	sf::Vector2i gridSize;

	std::vector<ItemEntity*> items;

public:

	sf::Vector2i getGridSize(){return gridSize;}

	void setGridSize(sf::Vector2i val){gridSize = val;}

	// Returns true if the tile can be used, false if not
	// Out of bound tiles return false, too
	bool isTileFree(sf::Vector2i tile);

	// Can an item, sized "size" fit in the grid assuming
	// that given pos is the upper left tile?
	bool canFit(sf::Vector2i pos, ItemEntity* item);

	// Returns NULL if there is no item there
	ItemEntity* getItem(sf::Vector2i pos);

	// Returns true if it could be placed, otherwise false
	bool setItem(sf::Vector2i pos, ItemEntity* item);

	std::vector<ItemEntity*> getAllItems();
	std::vector<ItemEntity*>* getItemsPtr() { return &items; }

	InventoryEntity(ProgramType* prog, uint32_t uid) : Entity(prog, uid)
	{

	}

	~InventoryEntity();
};

