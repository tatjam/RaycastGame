#pragma once
#include "Bases/DoorEntity.h"

class ESimpleDoor : public DoorEntity
{
public:

	virtual void onGenericCall(std::string cmd, json args, ENetPeer* peer) override;

	virtual bool onUserHover(SpriteEntity* player, Side side, sf::Vector2f pixel) override;
	virtual bool onUserClick(SpriteEntity* player, Side side, sf::Vector2f pixel) override;

	virtual EntityType getEntityType() { return DOOR_SIMPLE; }

	ESimpleDoor(ProgramType* prog, uint32_t uid) : DoorEntity(prog, uid), Entity(prog, uid)
	{
		setOpenSpeed(1.0f);
		setCloseSpeed(1.0f);
		setOpenThresold(0.5f);
		setRightwards(false);
		setTransparent(false);
	}

	~ESimpleDoor();
};

