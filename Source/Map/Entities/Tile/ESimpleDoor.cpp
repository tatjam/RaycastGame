#include "ESimpleDoor.h"




void ESimpleDoor::onGenericCall(std::string cmd, json args, ENetPeer* peer)
{
	DoorEntity::onGenericCall(cmd, args, peer);

	if (cmd == "doorOpen" && getProg()->isServer())
	{
		if (getState() >= 0.5f)
		{
			callSetState(0.0f);
		}
		else
		{
			// We don't open fully
			callSetState(0.7f);
		}
		
	}
}

bool ESimpleDoor::onUserHover(SpriteEntity * player, Side side, sf::Vector2f pixel)
{
	if (side != Side::CEILING && side != Side::FLOOR)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool ESimpleDoor::onUserClick(SpriteEntity* player, Side side, sf::Vector2f pixel)
{
	if (side != Side::CEILING && side != Side::FLOOR)
	{
		sendGenericCall(getProg()->getServer(), "doorOpen");
		return false;
	}
	else
	{
		return true;
	}
}

ESimpleDoor::~ESimpleDoor()
{
}
