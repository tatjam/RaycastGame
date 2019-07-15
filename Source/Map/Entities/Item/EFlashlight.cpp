#include "EFlashlight.h"
#include "../Bases/SpriteEntity.h"
#include "../Bases/InventoryEntity.h"

void EFlashlight::doContextualMenuAction(std::string action)
{
	// Everything is automagically synchronized from ItemEntity so...
	if (action == "Toggle")
	{
		lightOn = !lightOn;
	}
	
}

void EFlashlight::update(float dt)
{
	ItemEntity::update(dt);

	if (lightOn)
	{
		setItemFrame(1);
	}
	else
	{
		setItemFrame(0);
	}

	light.enabled = isHeldInHands() && lightOn;

	SpriteEntity* holderSprite = dynamic_cast<SpriteEntity*>(getInventory());

	if (holderSprite != NULL)
	{
		light.pos = holderSprite->getSprite()->pos;
		light.direction = holderSprite->getSprite()->angle;
	}
}

EFlashlight::EFlashlight(ProgramType* prog, uint32_t uid) : ItemEntity(prog, uid), Entity(prog, uid), light(getWorld()->map->getLightUID())
{
	setImageInHand(&getProg()->getImages()["items/flashlight/hand.png"]);
	setImageInInventory(&getProg()->getImages()["items/flashlight/inventory.png"]);

	setItemFrame(0);

	light.type = Light::SPOT;
	light.amplitude = PI / 8.0f;
	light.maxDist = 8.0f;

	getWorld()->map->lights.push_back(&light);

	lightOn = false;
}


EFlashlight::~EFlashlight()
{
}
