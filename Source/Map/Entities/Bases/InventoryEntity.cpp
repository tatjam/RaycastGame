#include "InventoryEntity.h"
#include "../Sprite/Bases/ItemEntity.h"



InventoryEntity::~InventoryEntity()
{
}

void InventoryEntity::onGenericCall(std::string cmd, json args, ENetPeer* peer)
{
	Entity::onGenericCall(cmd, args, peer);

	bool sendNetwork = false;
	ENetPeer* excludedPeer = NULL;

	if (getProg()->isServer())
	{
		// The server has received a command from a client and must synchronize it
		sendNetwork = true;
		excludedPeer = peer;
	}


	if (cmd == "setItem")
	{
		sf::Vector2i pos = sf::Vector2i(args["x"], args["y"]);
		uint32_t uid = args["uid"];

		setItem(pos, dynamic_cast<ItemEntity*>(getWorld()->findEntity(uid)), sendNetwork, excludedPeer);
	}
	else if (cmd == "setTileEnabled")
	{
		sf::Vector2i pos = sf::Vector2i(args["x"], args["y"]);
		bool val = args["uid"];

		setTileEnabled(pos, val, sendNetwork, excludedPeer);
	}
	else if(cmd == "setSlotType")
	{
		sf::Vector2i pos = sf::Vector2i(args["x"], args["y"]);
		InventorySlot val = args["uid"];

		setSlotType(pos, val, sendNetwork, excludedPeer);
	}
	else if (cmd == "removeItem")
	{
		uint32_t uid = args["uid"];
		sf::Vector2f pos = sf::Vector2f(args["x"], args["y"]);

		removeItem(dynamic_cast<ItemEntity*>(getWorld()->findEntity(uid)), pos, sendNetwork, excludedPeer);
	}
}

bool InventoryEntity::isTileFree(sf::Vector2i tile)
{
	if (tile.x >= gridSize.x || tile.y >= gridSize.y || tile.x < 0 || tile.y < 0)
	{
		return false;
	}

	for (size_t i = 0; i < items.size(); i++)
	{
		sf::Vector2i size = items[i]->getInventorySize();
		sf::Vector2i pos = items[i]->getInventoryPos();

		// AABB check to see if any item intercepts
		if (tile.x >= pos.x && tile.x <= pos.x + size.x && tile.y >= pos.y && tile.y <= pos.y + size.y)
		{
			return false;
		}
	}

	return true;
}

bool InventoryEntity::isTileUsable(sf::Vector2i tile)
{
	bool fromDisabled = isTileDisabled(tile);
	if(fromDisabled)
	{
		return false;
	}
	else
	{
		return isTileFree(tile);
	}
}

bool InventoryEntity::canFit(sf::Vector2i pos, ItemEntity* item)
{
	sf::Vector2i size = item->getInventorySize();
	for (int xpos = pos.x; xpos < pos.x + size.x; xpos++)
	{
		for (int ypos = pos.y; ypos < pos.y + size.y; ypos++)
		{
			bool isUsable = isTileUsable(sf::Vector2i(xpos, ypos));
			if (!isUsable)
			{
				return false;
			}
		}
	}

	return true;
}

ItemEntity* InventoryEntity::getItem(sf::Vector2i tile)
{
	for (size_t i = 0; i < items.size(); i++)
	{
		sf::Vector2i size = items[i]->getInventorySize();
		sf::Vector2i pos = items[i]->getInventoryPos();

		// AABB check to see if any item intercepts
		if (tile.x >= pos.x && tile.x <= pos.x + size.x && tile.y >= pos.y && tile.y <= pos.y + size.y)
		{
			return items[i];
		}
	}

	return NULL;
}

bool InventoryEntity::setItem(sf::Vector2i pos, ItemEntity* item, bool sendNetwork, ENetPeer* excludedPeer)
{
	if(canFit(pos, item))
	{
		item->setInventory(this);
		item->setInInventory(pos);

		if (sendNetwork)
		{
			json args;
			args["x"] = pos.x;
			args["y"] = pos.y;
			args["uid"] = item->uid;

			if (excludedPeer != NULL)
			{
				sendGenericCallToAllBut(excludedPeer, "setItem", args);
			}
			else
			{
				// We are the client so send to server
				sendGenericCall(getProg()->getServer(), "setItem", args);
			}
		}

		onInventoryItemAdded();

		return true;
	}
	else
	{
		return false;
	}
}


void InventoryEntity::setTileEnabled(sf::Vector2i pos, bool val, bool sendNetwork, ENetPeer* excluded)
{
	bool could = false;

	if (val)
	{
		if (isTileDisabled(pos))
		{
			disabledTiles.erase(pos);
			could = true;
		}
	}
	else
	{
		if (!isTileDisabled(pos))
		{
			disabledTiles.insert(pos);
			could = true;
		}
	}

	if (could && sendNetwork)
	{
		json args;
		args["x"] = pos.x;
		args["y"] = pos.y;
		args["val"] = val;

		if (excluded != NULL)
		{
			sendGenericCallToAllBut(excluded, "setTileEnabled", args);
		}
		else
		{
			// We are the client so send to server
			sendGenericCall(getProg()->getServer(), "setTileEnabled", args);
		}
	}
}

bool InventoryEntity::isTileDisabled(sf::Vector2i pos)
{
	// We return true if found
	return disabledTiles.find(pos) != disabledTiles.end();
}

void InventoryEntity::removeItem(ItemEntity* item, sf::Vector2f worldPos, bool sendNetwork, ENetPeer * excludedPeer)
{
	int index = -1;
	for (int i = 0; i < items.size(); i++)
	{
		if (items[i] == item)
		{
			index = i;
		}
	}

	if (index >= 0)
	{
		items.erase(items.begin() + index);
		item->setInWorld(worldPos);

		if (sendNetwork)
		{
			json args;
			args["id"] = item->uid;
			args["x"] = worldPos.x;
			args["y"] = worldPos.y;

			if (excludedPeer != NULL)
			{
				sendGenericCallToAllBut(excludedPeer, "removeItem", args);
			}
			else
			{
				// We are the client so send to server
				sendGenericCall(getProg()->getServer(), "removeItem", args);
			}
		}

		onInventoryItemRemoved();
	}
}

InventorySlot InventoryEntity::getSlotType(sf::Vector2i pos)
{
	auto it = specialTiles.find(pos);
	if (it == specialTiles.end())
	{
		return InventorySlot::NORMAL;
	}
	else
	{
		return it->second;
	}
}

void InventoryEntity::setSlotType(sf::Vector2i pos, InventorySlot type, bool sendNetwork, ENetPeer* excludedPeer)
{
	if (type == InventorySlot::NORMAL)
	{
		auto it = specialTiles.find(pos);
		if (it != specialTiles.end())
		{
			// We simply remove if we want to set to normal
			specialTiles.erase(it);
		}
	}
	else
	{
		specialTiles[pos] = type;
	}

	if(sendNetwork)
	{ 
		json args;
		args["x"] = pos.x;
		args["y"] = pos.y;
		args["val"] = type;

		if (excludedPeer != NULL)
		{
			sendGenericCallToAllBut(excludedPeer, "setSlotType", args);
		}
		else
		{
			// We are the client so send to server
			sendGenericCall(getProg()->getServer(), "setSlotType", args);
		}
	}
}
