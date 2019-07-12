#include "InventoryEntity.h"
#include "../Bases/ItemEntity.h"



InventoryEntity::~InventoryEntity()
{
}

void InventoryEntity::onGenericCall(std::string cmd, json args, ENetPeer* peer)
{
	Entity::onGenericCall(cmd, args, peer);

	//LOG(INFO) << args.dump();

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

		LOG(INFO) << "SET ITEM "; 

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
	else if (cmd == "moveItem")
	{
		uint32_t uid = args["uid"];
		sf::Vector2i pos = sf::Vector2i(args["x"], args["y"]);
		SpecialSlot slot = (SpecialSlot)(args["slot"].get<int>());

		if (slot == NORMAL)
		{
			moveItem(dynamic_cast<ItemEntity*>(getWorld()->findEntity(uid)), pos, sendNetwork, excludedPeer);
		}
		else
		{
			moveItemToSpecialSlot(dynamic_cast<ItemEntity*>(getWorld()->findEntity(uid)), slot, sendNetwork, excludedPeer);
		}
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
		if (items[i]->isInInventory())
		{
			sf::Vector2i size = items[i]->getInventorySize();
			sf::Vector2i pos = items[i]->getInventoryPos();

			// AABB check to see if any item intercepts
			if (tile.x >= pos.x && tile.x <= pos.x + size.x && tile.y >= pos.y && tile.y <= pos.y + size.y)
			{
				return false;
			}
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
	if (tile.x < 0 || tile.y < 0)
	{
		// Avoid getting in-slot items
		return NULL;
	}

	for (size_t i = 0; i < items.size(); i++)
	{
		sf::Vector2i size = items[i]->getInventorySize();
		sf::Vector2i pos = items[i]->getInventoryPos();

		// AABB check to see if any item intercepts
		if (tile.x >= pos.x && tile.x < pos.x + size.x && tile.y >= pos.y && tile.y < pos.y + size.y)
		{
			return items[i];
		}
	}

	return NULL;
}

ItemEntity* InventoryEntity::getItem(SpecialSlot slot)
{
	for (size_t i = 0; i < items.size(); i++)
	{
		if (slot == R_HAND || slot == L_HAND)
		{
			if (items[i]->getSpecialSlot() == slot || items[i]->getSpecialSlot() == B_HAND)
			{
				return items[i];
			}
		}
		else
		{
			if (items[i]->getSpecialSlot() == slot)
			{
				return items[i];
			}
		}
	}

	return NULL;
}

bool InventoryEntity::setItem(sf::Vector2i pos, ItemEntity* item, bool sendNetwork, ENetPeer* excludedPeer)
{
	if(canFit(pos, item))
	{
		for (size_t i = 0; i < items.size(); i++)
		{
			if (items[i] == item)
			{
				LOG(INFO) << "WTF!?";
			}
		}


		items.push_back(item);
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
			args["uid"] = item->uid;
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

void InventoryEntity::moveItem(ItemEntity* item, sf::Vector2i pos, bool sendNetwork, ENetPeer * excludedPeer)
{
	if (getItem(pos) == NULL)
	{
		sf::Vector2i from = item->getInventoryPos();
		SpecialSlot oldSlot = item->getSpecialSlot();

		item->setInInventory(pos);

		if (sendNetwork)
		{
			json args;
			args["uid"] = item->uid;
			args["x"] = pos.x;
			args["y"] = pos.y;
			args["slot"] = NORMAL;

			if (excludedPeer != NULL)
			{
				sendGenericCallToAllBut(excludedPeer, "moveItem", args);
			}
			else
			{
				// We are the client so send to server
				sendGenericCall(getProg()->getServer(), "moveItem", args);
			}
		}

		onInventoryItemMoved(item, from, oldSlot);
	}
}

void InventoryEntity::moveItemToSpecialSlot(ItemEntity* item, SpecialSlot slot, bool sendNetwork, ENetPeer * excludedPeer)
{
	if (slot != NORMAL)
	{
		sf::Vector2i from = item->getInventoryPos();
		SpecialSlot oldSlot = item->getSpecialSlot();

		item->setInventoryPos({ -1, -1 });
		item->setInSpecialSlot(slot);

		if (sendNetwork)
		{
			json args;
			args["uid"] = item->uid;
			args["x"] = -1;
			args["y"] = -1;
			args["slot"] = slot;

			if (excludedPeer != NULL)
			{
				sendGenericCallToAllBut(excludedPeer, "moveItem", args);
			}
			else
			{
				// We are the client so send to server
				sendGenericCall(getProg()->getServer(), "moveItem", args);
			}
		}

		onInventoryItemMoved(item, from, oldSlot);
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

void InventoryEntity::update(float dt)
{
	Entity::update(dt);

	std::string server;
	if (getProg()->isClient())
	{
		server = "client";
	}
	else
	{
		server = "server";
	}

	LOG(INFO) << "Enumerate items in " << server << " id: " << uid;

	LOG(INFO) << "---------------------------------";

	for (size_t i = 0; i < items.size(); i++)
	{
		std::string slot = "";
		if (items[i]->isInInventory())
		{
			slot = std::to_string(items[i]->getInventoryPos().x);
			slot += ", ";
			slot += std::to_string(items[i]->getInventoryPos().y);
		}
		else
		{
			if (items[i]->getSpecialSlot() == L_HAND)
			{
				slot = "L_HAND";
			}
			else if (items[i]->getSpecialSlot() == R_HAND)
			{
				slot = "R_HAND";
			}
			else
			{
				slot = "OTHER";
			}
		}

		LOG(INFO) << "Item: " << items[i]->uid << " in (" << slot << ")";
	}

	LOG(INFO) << "---------------------------------";

}

json InventoryEntity::serialize()
{
	json j = Entity::serialize();

	j["gridSizeX"] = gridSize.x;
	j["gridSizeY"] = gridSize.y;

	// We don't serialize items as they are responsible for adding themselves to inventories

	return j;
}

void InventoryEntity::deserialize(json j)
{
	Entity::deserialize(j);

	gridSize.x = j["gridSizeX"];
	gridSize.y = j["gridSizeY"];

	// We don't deserialize items as they are responsible for adding themselves to inventories
	
}
