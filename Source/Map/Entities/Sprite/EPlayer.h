#pragma once
#include "../Bases/SpriteEntity.h"
#include "../Bases/InventoryEntity.h"

// 20 packets per second
#define EPLAYER_NETFRAMERATE 8
#define EPLAYER_NETUPDATE (1.0f / EPLAYER_NETFRAMERATE)

#define EPLAYER_MAX_DELTA_POS 0.05f
#define EPLAYER_MAX_DELTA_ANGLE (PI / 20.0f)  //< 9�

// TODO: Control methods
class EPlayer : public SpriteEntity, public InventoryEntity
{
private:
	bool noclip;

	sf::Vector2f inputs;
	float angleInput;

	sf::Vector2i mousePrev;

	bool wasTabPressed;

	void updateMovement(float dt);
	void updateInventory(float dt);

public:

	// Used when in FPS control to know how high are we looking
	// Not synchronized at all
	// 0 = top of screen, 1 = bottom of screen
	float aimElevation;

	// Are we using FPS controls or aim-and-click controls?
	bool inFPSControl;

	// Used to limit the ammount of information we send
	sf::Vector2f deltaPos;
	float deltaAngle;

	// Every how much time are network updates sent
	float sendTimer;

	virtual EntityType getEntityType() { return PLAYER; }

	virtual void update(float dt) override;
	virtual void start() override;

	EPlayer(ProgramType* prog, uint32_t uid);
	~EPlayer();
};

