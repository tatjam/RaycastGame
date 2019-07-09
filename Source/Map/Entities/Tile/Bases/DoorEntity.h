#pragma once

#include "../../Bases/TileEntity.h"
// Base class for tiles which open and close
// The server has tothal authority over opening
// and closing, and mirrors the open/close command
// to clients, which animate the door and play any
// effects
//
// Commands:
//
// Generic Calls:
//	- setProperty (SERVER->CLIENT)
//		-> float openSpeed
//		-> float closeSpeed
//		-> bool rightwards
//		-> bool transparent
//		-> float openThresold
//	- setState (SERVER->CLIENT)
//		-> float state
//

class DoorEntity : public TileEntity
{
private:

	// 0 = closed, 1 = open
	float state;
	float wantedState;

	float openSpeed;
	float closeSpeed;

	// Used when animating the door, does it open to the
	// right or to the left? tile.var1
	bool rightwards;

	// Is the door transparent even when closed?
	bool transparent;
	
	// At what point is the door walkable?
	float openThresold;

public:

	// Can only be called on the server, client 
	// doors automatically interpolate
	void callSetState(float val);
	void callSetProperty();

	virtual void onGenericCall(std::string cmd, json args, ENetPeer* peer) override;

	void setOpenSpeed(float val){openSpeed = val;}
	float getOpenSpeed(){return openSpeed;}

	void setCloseSpeed(float val){closeSpeed = val;}
	float getCloseSpeed() { return closeSpeed; }

	void setRightwards(bool val) { rightwards = val; }
	bool getRightwards() { return rightwards; }

	void setOpenThresold(float val) { openThresold = val; }
	float getOpenThresold() { return openThresold; }

	void setTransparent(bool val) { transparent = val; }
	bool getTransparent() { return transparent; }

	float getState() { return state; }

	virtual void update(float dt) override;

	virtual json serialize() override;
	virtual void deserialize(json j) override;


	DoorEntity(ProgramType* prog, uint32_t uid) : TileEntity(prog, uid), Entity(prog, uid)
	{
		state = 0.0f; wantedState = 0.0f;
	}

	~DoorEntity();
};

