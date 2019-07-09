#pragma once
#include "../../Entity.h"
#include "../../Map.h"
// Implements:
// 
// SET_SPRITE_POSROT
// SET_SPRITE_DETAIL
// SET_INTERPOLATION
//
// Uses interpolation to smooth out the sprite's movement unless disabled
// via
//
// SET_INTERPOLATION 
//
// to a 0 value
//
// It creates the sprite it uses
// If a player controls this entity the server
// will receive SET_SPRITE_POSROT from it
// It's up for any override to actually 
// set the sprite textures, this only creates an empty sprite
class SpriteEntity : public virtual Entity
{
private:

	Sprite sprite;

	sf::Vector2f wantedPos;
	sf::Vector2f oldPos;

	float interpSpeed;
	float interpTimer;

	bool spriteInMap;

public:
	
	void cmdSetSpritePosRot(sf::Vector2f pos, float angle, ENetPeer* peer);
	Packet makeSetSpritePosRot(sf::Vector2f pos, float angle);

	virtual void receiveCommand(uint8_t command_id, Packet packet, ENetPeer* peer) override;

	// Must be set to something similar
	// to the update interval of the network
	void setInterpSpeed(float speed)
	{
		interpSpeed = speed;
	}

	float getInterpSpeed()
	{
		return interpSpeed;
	}

	virtual Sprite* getSprite()
	{
		return &sprite;
	}

	// Adds the sprite to the map, if it has no uid it gets it here
	void addSprite();

	// Removes the sprite from the world, but keeps the uid
	void removeSprite();

	bool isSpriteInMap() { return spriteInMap; }

	virtual void start() override;
	virtual void update(float dt) override;

	SpriteEntity(ProgramType* prog, uint32_t uid) : Entity(prog, uid), sprite(0) { interpTimer = 0.0f; interpSpeed = 1.0f; spriteInMap = false; }
	~SpriteEntity() {}
};

