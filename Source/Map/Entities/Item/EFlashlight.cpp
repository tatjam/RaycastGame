#include "EFlashlight.h"



EFlashlight::EFlashlight(ProgramType* prog, uint32_t uid) : ItemEntity(prog, uid), Entity(prog, uid)
{
	setImageInHand(&getProg()->getImages()["items/flashlight/hand.png"]);
	setImageInInventory(&getProg()->getImages()["items/flashlight/inventory.png"]);

	setItemFrame(0);
}


EFlashlight::~EFlashlight()
{
}
