#include "SpriteEntity.h"

void SpriteEntity::cmdSetSpritePosRot(sf::Vector2f pos, float angle, ENetPeer* peer)
{
	// Prevent the server from overriding player input
	if (isControlled())
	{
		return;
	}

	sprite.angle = angle;

	if (getProg()->isServer())
	{
		// We need to send this to all clients
		emitCommandToAllBut(CommandID::SET_SPRITE_POSROT, makeSetSpritePosRot(pos, angle), peer);
		sprite.pos = pos;

	}
	else
	{

		// We will do interpolation
		oldPos = sprite.pos;
		wantedPos = pos;
		interpTimer = 0.0f;


	}
}

Packet SpriteEntity::makeSetSpritePosRot(sf::Vector2f pos, float angle)
{
	Packet out;

	out.pushFloat(pos.x);
	out.pushFloat(pos.y);
	out.pushFloat(angle);

	return out;
}


void SpriteEntity::receiveCommand(uint8_t command_id, Packet packet, ENetPeer* peer)
{
	Entity::receiveCommand(command_id, packet, peer);

	if (command_id == CommandID::SET_SPRITE_POSROT)
	{
		float angle = packet.popFloat();
		float yPos = packet.popFloat();
		float xPos = packet.popFloat();
		sf::Vector2f pos = sf::Vector2f(xPos, yPos);

		cmdSetSpritePosRot(pos, angle, peer);
	}
}

void SpriteEntity::start()
{
	LOG(INFO) << "Sprite entity start!";

	Entity::start();

	Map* map = getProg()->getWorld()->map;

	sprite = Sprite(map->getSpriteUID());

	sf::Image demo = sf::Image();
	demo.loadFromFile("Assets/sprite.png");

	if (!isControlled())
	{
		sprite.frames.push_back(demo);
		sprite.frame = 0;
	}

	sprite.pos = sf::Vector2f(4.5f, 3.5f);
	wantedPos = sprite.pos;
	oldPos = sprite.pos;

	map->sprites.push_back(getSprite());
}

void SpriteEntity::update(float dt)
{
	Entity::update(dt);

	if (getProg()->isClient())
	{
		if (!isControlled())
		{
			//LOG(INFO) << "Interpolating " << uid;

			// Interpolation only for non-controlled entities
			sprite.pos = wantedPos * interpTimer + oldPos * (1.0f - interpTimer);

			interpTimer += dt * interpSpeed;
			if (interpTimer > 1.0f)
			{
				interpTimer = 1.0f;
			}
		}
	}
}
