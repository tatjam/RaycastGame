#include "DoorEntity.h"



void DoorEntity::callSetState(float val)
{
	state = val;
	
	json j; j["state"] = state;
	sendGenericCallToAll("setState", j);
}

void DoorEntity::callSetProperty()
{
	json j;
	j["openSpeed"] = getOpenSpeed();
	j["closeSpeed"] = getCloseSpeed();
	j["rightwards"] = getRightwards();
	j["openThresold"] = getOpenThresold();
	j["transparent"] = getTransparent();

	sendGenericCallToAll("setState", j);
}

void DoorEntity::onGenericCall(std::string cmd, json args, ENetPeer* peer)
{
	TileEntity::onGenericCall(cmd, args, peer);

	if (cmd == "setProperty")
	{
		setOpenSpeed(args["openSpeed"]);
		setCloseSpeed(args["closeSpeed"]);
		setRightwards(args["rightwards"]);
		setOpenThresold(args["openThresold"]);
		setTransparent(args["transparent"]);
	}
	else if (cmd == "setState") 
	{
		wantedState = args["state"];
	}
}

void DoorEntity::update(float dt)
{
	TileEntity::update(dt);

	if (getProg()->isClient())
	{
		if (state >= getOpenThresold())
		{
			getTile()->walkable = true;
		}
		else
		{
			getTile()->walkable = false;
		}

		if (getTransparent())
		{
			getTile()->transparent = true;
		}
		else
		{
			if (state >= 0.1f)
			{
				getTile()->transparent = true;
			}
			else
			{
				getTile()->transparent = false;
			}
		}

		if (rightwards)
		{
			getTile()->var1 = 128 - state * 128.0f;
		}
		else
		{
			getTile()->var1 = 128 + state * 128.0f;
		}

		if (state != wantedState)
		{
			bool opening = false;
			if (state <= wantedState)
			{
				opening = true;
			}

			if (opening)
			{
				state += dt * getOpenSpeed();
				if (state >= wantedState)
				{
					state = wantedState;
				}
			}
			else
			{
				state -= dt * getCloseSpeed();
				if (state <= wantedState)
				{
					state = wantedState;
				}
			}
		}
	}
}

json DoorEntity::serialize()
{
	json j = TileEntity::serialize();

	j["openSpeed"] = getOpenSpeed();
	j["closeSpeed"] = getCloseSpeed();
	j["rightwards"] = getRightwards();
	j["openThresold"] = getOpenThresold();
	j["transparent"] = getTransparent();

	return j;
}

void DoorEntity::deserialize(json j)
{
	TileEntity::deserialize(j);

	setOpenSpeed(j["openSpeed"]);
	setCloseSpeed(j["closeSpeed"]);
	setRightwards(j["rightwards"]);
	setOpenThresold(j["openThresold"]);
	setTransparent(j["transparent"]);
}


DoorEntity::~DoorEntity()
{
}
