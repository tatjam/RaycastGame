#pragma once

enum InventorySlot
{
	NORMAL,
	HEAD,
	TORSO,
	LEGS,
	GLASSES,
	BACKPACK,
	BELT,
};

// Only used for items held by a player
enum SpecialSlot
{
	NONE,
	L_HAND,
	R_HAND,
	B_HAND, //< Takes both hands
	VISOR
};