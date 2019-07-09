#pragma once

#include "../../Entity.h"
#include "../../Map.h"
#include "../../Tile.h"
#include "InventorySlot.h"

#include <unordered_set>


class ItemEntity;

// We do this to allow using Vector2i as keys
template <>
struct std::hash<sf::Vector2i>
{
	std::size_t operator()(const sf::Vector2i& k) const
	{
		size_t seed = 0;
		hash_combine(seed, k.x, k.y);
		return seed;
	}
};

// An InventoryEntity can hold items in particular
// arrangements in a grid. The drawing of the grid
// is not done here, as it can be an arbitrary shape
// not even a grid. A good example is the player inventory
// Grid locations can be set to be not usable, or disabled
// NOTE: Inventory sizes should not change, instead tiles should
// be set enabled or disabled, hence inventory size is not synchronized
// and is called on the constructor
//
// Commands:
//
// Generic Calls:
//	- setItem (SERVER<->CLIENT)
//		-> int x
//		-> int y
//		-> uint32_t uid
//	- setTileEnabled (SERVER<->CLIENT)
//		-> int x
//		-> int y
//		-> bool enabled
//	- setSlotType (SERVER<->CLIENT)
//		-> int x
//		-> int y
//		-> int slotType
//
class InventoryEntity : public virtual Entity
{
private:

	sf::Vector2i gridSize;

	std::vector<ItemEntity*> items;
	std::unordered_set<sf::Vector2i> disabledTiles;
	
	// By default every tile is normal, this allows
	// tiles to be allocated to an InventorySlot
	std::unordered_map<sf::Vector2i, InventorySlot> specialTiles;

public:

	virtual void onGenericCall(std::string cmd, json args, ENetPeer* peer) override;

	sf::Vector2i getGridSize(){return gridSize;}

	// Returns true if the tile has an item
	// Out of bound tiles return false
	// Does not include disabled slots
	bool isTileFree(sf::Vector2i tile);

	// Returns true if the tile can fit an item
	// This includes disabled slots
	// Out of bound tiles return false
	bool isTileUsable(sf::Vector2i tile);

	// Can an item, sized "size" fit in the grid assuming
	// that given pos is the upper left tile?
	// Uses isTileUsable
	bool canFit(sf::Vector2i pos, ItemEntity* item);

	// Returns NULL if there is no item there
	ItemEntity* getItem(sf::Vector2i pos);

	// Returns true if it could be placed, otherwise false
	// Automatically synchronized IF THE ITEM COULD BE PLACED and sendNetwork is true
	bool setItem(sf::Vector2i pos, ItemEntity* item, bool sendNetwork = true, ENetPeer* excludedPeer = NULL);
	void setTileEnabled(sf::Vector2i pos, bool val, bool sendNetwork = true, ENetPeer* excludedPeer = NULL);
	bool isTileDisabled(sf::Vector2i pos);

	std::vector<ItemEntity*> getAllItems() { return items; }
	std::vector<ItemEntity*>* getItemsPtr() { return &items; }

	InventorySlot getSlotType(sf::Vector2i pos);

	void setSlotType(sf::Vector2i pos, InventorySlot type, bool sendNetwork = true, ENetPeer* excludedPeer = NULL);

	InventoryEntity(ProgramType* prog, uint32_t uid, sf::Vector2i gridSize) : Entity(prog, uid)
	{
		this->gridSize = gridSize;
		items = std::vector<ItemEntity*>();
		disabledTiles = std::unordered_set<sf::Vector2i>();
		specialTiles = std::unordered_map<sf::Vector2i, InventorySlot>();
	}

	~InventoryEntity();
};

