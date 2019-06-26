#pragma once
#include "Bases/SpriteEntity.h"

// 20 packets per second
#define EPLAYER_NETFRAMERATE 8
#define EPLAYER_NETUPDATE (1.0f / EPLAYER_NETFRAMERATE)

#define EPLAYER_MAX_DELTA_POS 0.05f
#define EPLAYER_MAX_DELTA_ANGLE (PI / 4.5f)

class EPlayer : public SpriteEntity
{
private:
	bool noclip;

public:

	// Used to limit the ammount of information we send
	sf::Vector2f deltaPos;
	float deltaAngle;

	// Every how much time are network updates sent
	float sendTimer;

	virtual EntityType getEntityType() { return PLAYER; }

	virtual void update(float dt) override;
	virtual void start() override;

	EPlayer(ProgramType* prog, uint32_t uid) : SpriteEntity(prog, uid) { sendTimer = EPLAYER_NETUPDATE; noclip = false; }
	~EPlayer();
};

