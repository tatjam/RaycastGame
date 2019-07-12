#include "ItemEntity.h"
#include "InventoryEntity.h"



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
	inSpecial = false;
	getSprite()->pos = pos;
}

void ItemEntity::setInInventory(sf::Vector2i pos)
{
	inWorld = false;
	inSpecial = false;
	inventoryPos = pos;
}

void ItemEntity::setInSpecialSlot(SpecialSlot special)
{
	inSpecial = true;
	inWorld = false;
	specialPos = special;
}


void ItemEntity::onGenericCall(std::string cmd, json args, ENetPeer* peer)
{
	if (cmd == "setItemFrame")
	{
		itemFrame = args["frame"];
	}
	else if(cmd == "contextualMenuAction")
	{
		receiveContextualMenuAction(args["action"], peer);
	}
}

void ItemEntity::setItemFrame(size_t frame, bool sendNetwork)
{
	if (sendNetwork && getProg()->isClient())
	{
		LOG(ERROR) << "Tried to call setItemFrame and network it on a client";
		return;
	}

	itemFrame = frame;
	if(sendNetwork)
	{
		json args;
		args["frame"] = itemFrame;
		sendGenericCallToAll("setItemFrame", args);
	}
}

sf::Vector2i ItemEntity::getInventoryPos()
{
	return inventoryPos;
}

void ItemEntity::setInventoryPos(sf::Vector2i pos)
{
	inventoryPos = pos;
}

sf::Vector2i ItemEntity::getInventorySize()
{
	return sf::Vector2i(1, 1);
}

sf::Vector2f ItemEntity::getHandPos()
{
	return handPos;
}

sf::Vector2f ItemEntity::getHandScale()
{
	float scalar = floorf(512.0f / getTextureInHand().getSize().y);

	if (specialPos == L_HAND || specialPos == B_HAND)
	{
		return sf::Vector2f(scalar, scalar);
	} 
	else
	{
		return sf::Vector2f(-scalar, scalar);
	}
}

sf::Color ItemEntity::getHandTint()
{
	SpriteEntity* casted = dynamic_cast<SpriteEntity*>(inventory);
	if (casted != NULL)
	{
		sf::Vector3f light = getWorld()->map->getLight(casted->getSprite()->pos);
		return sf::Color(light.x * 255, light.y * 255, light.z * 255);
	}

	return sf::Color(255, 255, 255);
}

json ItemEntity::serialize()
{
	json j = SpriteEntity::serialize();

	j["isItemInWorld"] = inWorld;
	j["isItemInSpecial"] = inSpecial;

	if (!(inWorld || inSpecial))
	{
		j["inventoryID"] = inventory->uid;
		j["invX"] = inventoryPos.x;
		j["invY"] = inventoryPos.y;

	}

	if (inSpecial)
	{
		j["specialPos"] = specialPos;
	}
	
	j["itemFrame"] = itemFrame;

	return j;
}

void ItemEntity::deserialize(json j)
{
	SpriteEntity::deserialize(j);

	inWorld = j["isItemInWorld"];
	inSpecial = j["isItemInSpecial"];

	if (!(inWorld || inSpecial))
	{
		uint32_t inv_id = j["inventoryID"];
		sf::Vector2i pos = sf::Vector2i(j["invX"], j["invY"]);

		inventory = dynamic_cast<InventoryEntity*>(getWorld()->findEntity(inv_id));
		inventory->setItem(pos, this, false);
	}

	if (inSpecial)
	{
		specialPos = j["specialPos"];
	}

	itemFrame = j["itemFrame"];
}

void ItemEntity::update(float dt)
{
	if (getProg()->isClient())
	{
		if (isInSpecial())
		{
			if (!wasInSpecial)
			{
				handTimer = 0.5f;
			}

			wasInSpecial = true;
		}
		else
		{
			wasInSpecial = false;
		}

		sf::Vector2f base;

		if (specialPos == L_HAND)
		{
			base.x = -(handTimer * 0.5f) * textureInHand.getSize().x * getHandScale().x;
		}
		else if(specialPos == R_HAND)
		{
			base.x = getProg()->getWindow()->getSize().x - (handTimer * 0.5f) * (float)textureInHand.getSize().x * getHandScale().x;
		}
		else if (specialPos == B_HAND)
		{
			base.x = getProg()->getWindow()->getSize().x / 2.0f - textureInHand.getSize().x * getHandScale().x;
		}


		base.y = getProg()->getUIStart() - (1.0f - handTimer) * textureInHand.getSize().y * getHandScale().y;

		handPos = base;

		handTimer -= dt * 1.5f;
		if (handTimer <= 0.0f)
		{
			handTimer = 0.0f;
		}
	}
}

void ItemEntity::sendContextualMenuAction(std::string action)
{
	json args;
	args["action"] = action;
	sendGenericCall(getProg()->getServer(), "contextualMenuAction", args);

	doContextualMenuAction(action);
}

void ItemEntity::receiveContextualMenuAction(std::string action, ENetPeer* from)
{
	if (getProg()->isServer())
	{
		json args;
		args["action"] = action;
		sendGenericCallToAllBut(from, "contextualMenuAction", args);
	}

	doContextualMenuAction(action);
}

ItemEntity::~ItemEntity()
{
}
