#pragma once

#include "../Bases/TileEntity.h"

// Base class for tiles which open and close
// The server has tothal authority over opening
// and closing, and mirrors the open/close command
// to clients, which animate the door and play any
// effects
class EDoor : public TileEntity
{
private:

	// 0 = closed, 1 = open
	float state;

	float openSpeed;
	float closeSpeed;

	// Used when animating the door, does it open to the
	// right or to the left? tile.var1
	bool rightwards;

public:

	// Can only be called on the server
	void toggleState();

	void setOpenSpeed(float val)
	{
		openSpeed = val;
	}

	void setCloseSpeed(float val)
	{
		closeSpeed = val;
	}

	EDoor();
	~EDoor();
};

