#include "EPlayer.h"



void EPlayer::update(float dt)
{
	SpriteEntity::update(dt);

	if (getProg()->isClient())
	{
		if (getProg()->getWindow()->hasFocus() && isControlled())
		{

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			{
				getSprite()->angle -= PI * 1.2f * dt;
				deltaAngle += PI * 1.2f * dt;
			}

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			{
				getSprite()->angle += PI * 1.2f * dt;
				deltaAngle += PI * 1.2f * dt;
			}

			sf::Vector2f dir; dir.x = sin(getSprite()->angle); dir.y = cos(getSprite()->angle);
			sf::Vector2f perpdir; perpdir.x = sin(getSprite()->angle + PI / 2.0f); perpdir.y = cos(getSprite()->angle + PI / 2.0f);

			float speed = 2.0f;

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
			{
				getSprite()->pos += dir * dt * speed;
				deltaPos += dir * dt * speed;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
			{
				getSprite()->pos -= perpdir * dt * speed;
				deltaPos -= perpdir * dt * speed;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
			{
				getSprite()->pos -= dir * dt * speed;
				deltaPos -= dir * dt * speed;
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
			{
				getSprite()->pos += perpdir * dt * speed;
				deltaPos += perpdir * dt * speed;
			}
		}

		if (sendTimer <= 0.0f && (deltaAngle >= EPLAYER_MAX_DELTA_ANGLE || thor::length(deltaPos) >= EPLAYER_MAX_DELTA_POS))
		{
			emitCommandToServer(SET_SPRITE_POSROT, makeSetSpritePosRot(getSprite()->pos, getSprite()->angle));
			sendTimer = EPLAYER_NETUPDATE;
			deltaPos = sf::Vector2f(0.0f, 0.0f);
			deltaAngle = 0.0f;
		}
	}

	sendTimer -= dt;
}

void EPlayer::start()
{
	SpriteEntity::start();
	setInterpSpeed(EPLAYER_NETFRAMERATE);
}

EPlayer::~EPlayer()
{
}
