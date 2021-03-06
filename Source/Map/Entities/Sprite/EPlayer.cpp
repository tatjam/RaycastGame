#include "EPlayer.h"



void EPlayer::updateMovement(float dt)
{
	if (getProg()->isClient())
	{
		sf::Vector2f delta;
		float speed = 2.5f;

		if (getProg()->getWindow()->hasFocus() && isControlled())
		{
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Tab))
			{
				if (!wasTabPressed)
				{
					inFPSControl = !inFPSControl;
					sf::Vector2u mousePos = getProg()->getWindow()->getSize() / 2U;
					mousePrev = sf::Vector2i(mousePos.x, mousePos.y);
				}

				wasTabPressed = true;
			}
			else
			{
				wasTabPressed = false;
			}

			if (inFPSControl)
			{
				getProg()->getWindow()->setMouseCursorGrabbed(true);
				getProg()->getWindow()->setMouseCursorVisible(false);


				sf::Vector2i mouseNow = sf::Mouse::getPosition(*getProg()->getWindow());
				sf::Vector2i mouseDelta = mouseNow - mousePrev;

				float angleChange = (float)mouseDelta.x * 0.0022f;
				getSprite()->angle += angleChange;
				deltaAngle += angleChange;

				aimElevation += (float)mouseDelta.y * 0.003f;

				sf::Vector2u mousePos = getProg()->getWindow()->getSize() / 2U;
				sf::Vector2i mousePosi = sf::Vector2i(mousePos.x, mousePos.y);

				sf::Mouse::setPosition(mousePosi, *getProg()->getWindow());

				mousePrev = mousePosi;


			}
			else
			{
				getProg()->getWindow()->setMouseCursorGrabbed(false);
				getProg()->getWindow()->setMouseCursorVisible(true);
			}


			if (aimElevation < 0.0f)
			{
				aimElevation = 0.0f;
			}

			if (aimElevation > 1.0f)
			{
				aimElevation = 1.0f;
			}


			sf::Vector2f dir; dir.x = sin(getSprite()->angle); dir.y = cos(getSprite()->angle);
			sf::Vector2f perpdir; perpdir.x = sin(getSprite()->angle + PI / 2.0f); perpdir.y = cos(getSprite()->angle + PI / 2.0f);



			if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
			{
				delta += dir;
			}
			if (inFPSControl)
			{

				if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
				{
					delta -= perpdir;
				}
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
				{
					delta += perpdir;
				}
			}
			else
			{

				if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
				{
					delta -= perpdir;
				}
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
				{
					delta += perpdir;
				}


				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
				{
					getSprite()->angle -= PI * 1.2f * dt;
					deltaAngle += PI * 1.2f * dt;
				}

				if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
				{
					getSprite()->angle += PI * 1.2f * dt;
					deltaAngle += PI * 1.2f * dt;
				}

			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
			{
				delta -= dir;
			}


			if (sf::Keyboard::isKeyPressed(sf::Keyboard::PageUp))
			{
				noclip = true;
			}
			else
			{
				noclip = false;
			}
		}

		Map* map = getProg()->getWorld()->map;

		if (delta != sf::Vector2f(0, 0) || map->getTile((int)getSprite()->pos.x, (int)getSprite()->pos.y).walkable == false)
		{
			// Optional, enable for normalized movement (no fast strafing)
			//thor::setLength(delta, 1.0f);

			constexpr float SMALL_MULTIPLIER = 0.05f;
			// We check ahead with a small multiplier
			sf::Vector2f nextPos = getSprite()->pos + delta * speed * SMALL_MULTIPLIER * 1.5f;
			sf::Vector2f nextPosSoft = getSprite()->pos + delta * speed * dt;

			Tile next = map->getTile((int)nextPos.x, (int)nextPos.y);
			Tile actual = map->getTile((int)getSprite()->pos.x, (int)getSprite()->pos.y);

			if (next.walkable || noclip)
			{
				// Nothing here for now
			}
			else
			{


				// This only holds true because we work on a grid!
				sf::Vector2f currPos = getSprite()->pos;
				sf::Vector2i currPosRound = sf::Vector2i((int)currPos.x, (int)currPos.y);
				sf::Vector2i nextPosRound = sf::Vector2i((int)nextPos.x, (int)nextPos.y);
				sf::Vector2i wallNormal = currPosRound - nextPosRound;
				sf::Vector2f wallNormalF = sf::Vector2f((float)wallNormal.x, (float)wallNormal.y);
				if (wallNormalF == sf::Vector2f(0, 0))
				{
					// We are stuck inside a wall!
					delta = sf::Vector2f(0.0f, 0.0f);

					// Move towards nearest side, if it's clear, otherwise find a clear block

					sf::Vector2f subBlock = currPos - sf::Vector2f((float)currPosRound.x, (float)currPosRound.y);
					sf::Vector2f dir0, dir1;
					if (subBlock.x < 0.5f)
					{
						if (subBlock.y < 0.5f)
						{
							dir0 = sf::Vector2f(0.0f, -1.0f);
							dir1 = sf::Vector2f(-1.0f, 0.0f);
						}
						else
						{
							dir0 = sf::Vector2f(0.0f, 1.0f);
							dir1 = sf::Vector2f(-1.0f, 0.0f);
						}
					}
					else
					{
						if (subBlock.y < 0.5f)
						{
							dir0 = sf::Vector2f(0.0f, -1.0f);
							dir1 = sf::Vector2f(1.0f, 0.0f);
						}
						else
						{
							dir0 = sf::Vector2f(0.0f, 1.0f);
							dir1 = sf::Vector2f(1.0f, 0.0f);
						}
					}

					// Check if it's free
					sf::Vector2i targetIf0 = currPosRound + sf::Vector2i((int)dir0.x, (int)dir0.y);
					sf::Vector2i targetIf1 = currPosRound + sf::Vector2i((int)dir1.x, (int)dir1.y);
					if (map->getTile(targetIf0.x, targetIf0.y).walkable)
					{
						delta = dir0;
					}
					else if (map->getTile(targetIf1.x, targetIf1.y).walkable)
					{
						delta = dir1;
					}
					else
					{
						bool up = map->getTile(currPosRound.x, currPosRound.y - 1).walkable;
						bool right = map->getTile(currPosRound.x + 1, currPosRound.y).walkable;
						bool down = map->getTile(currPosRound.x, currPosRound.y + 1).walkable;
						bool left = map->getTile(currPosRound.x - 1, currPosRound.y).walkable;

						if (up)
						{
							delta = sf::Vector2f(0.0f, -1.0f);
						}
						else if (right)
						{
							delta = sf::Vector2f(1.0f, 0.0f);
						}
						else if (left)
						{
							delta = sf::Vector2f(-1.0f, 0.0f);
						}
						else if (down)
						{
							delta = sf::Vector2f(0.0f, 1.0f);
						}
						else
						{
							// DIE (TODO)
						}

					}

				}
				else
				{


					float simil = std::abs(thor::dotProduct(thor::unitVector(wallNormalF), delta));

					// If we are head-on onto a wall we just stop
					// otherwise we add a bit of the "bounce" (normal)
					// Simil will be 1 if we are head on, near 0 if we are glancing
					delta = (1.0f - simil) * delta;
					// Remove the delta component towards the wall
					delta.x *= (1.0f - std::abs(wallNormalF.x));
					delta.y *= (1.0f - std::abs(wallNormalF.y));

					getSprite()->pos += wallNormalF * SMALL_MULTIPLIER * simil * dt;
					deltaPos += wallNormalF * SMALL_MULTIPLIER * simil * dt;
				}


			}


			getSprite()->pos += delta * dt * speed;
			deltaPos += delta * dt * speed;

		}
	}

	if (sendTimer <= 0.0f && (std::abs(deltaAngle) >= EPLAYER_MAX_DELTA_ANGLE || thor::length(deltaPos) >= EPLAYER_MAX_DELTA_POS))
	{
		emitCommandToServer(SET_SPRITE_POSROT, makeSetSpritePosRot(getSprite()->pos, getSprite()->angle));
		sendTimer = EPLAYER_NETUPDATE;
		deltaPos = sf::Vector2f(0.0f, 0.0f);
		deltaAngle = 0.0f;
	}


	sendTimer -= dt;
}

void EPlayer::updateInventory(float dt)
{
	// TODO: Change this again :)
	/*bool hasShirt = !isTileFree({ 0, 1 });
	bool hasBackpack = !isTileFree({ 1, 1 });
	bool hasPants = !isTileFree({ 0, 2 });
	bool hasBelt = !isTileFree({ 1, 2 });*/
	bool hasShirt = isTileFree({ 0, 1 });
	bool hasBackpack = isTileFree({ 1, 1 });
	bool hasPants = isTileFree({ 0, 2 });
	bool hasBelt = isTileFree({ 1, 2 }); 

	setTileEnabled({ 2, 0 }, hasShirt, false);
	setTileEnabled({ 3, 0 }, hasShirt, false);
	setTileEnabled({ 2, 1 }, hasShirt, false);
	setTileEnabled({ 3, 1 }, hasShirt, false);


	setTileEnabled({ 4, 0 }, hasBackpack, false);
	setTileEnabled({ 5, 0 }, hasBackpack, false);
	setTileEnabled({ 6, 0 }, hasBackpack, false);
	setTileEnabled({ 7, 0 }, hasBackpack, false);
	setTileEnabled({ 8, 0 }, hasBackpack, false);
	setTileEnabled({ 9, 0 }, hasBackpack, false);
	setTileEnabled({ 4, 1 }, hasBackpack, false);
	setTileEnabled({ 5, 1 }, hasBackpack, false);
	setTileEnabled({ 6, 1 }, hasBackpack, false);
	setTileEnabled({ 7, 1 }, hasBackpack, false);
	setTileEnabled({ 8, 1 }, hasBackpack, false);
	setTileEnabled({ 9, 1 }, hasBackpack, false);

	setTileEnabled({ 2, 2 }, hasPants, false);
	setTileEnabled({ 3, 2 }, hasPants, false);

	setTileEnabled({ 4, 2 }, hasBelt, false);
	setTileEnabled({ 5, 2 }, hasBelt, false);
	setTileEnabled({ 6, 2 }, hasBelt, false);
	setTileEnabled({ 7, 2 }, hasBelt, false);
	setTileEnabled({ 8, 2 }, hasBelt, false);
	setTileEnabled({ 9, 2 }, hasBelt, false);
}

void EPlayer::update(float dt)
{
	SpriteEntity::update(dt);
	InventoryEntity::update(dt);

	updateMovement(dt);
	updateInventory(dt);
}

void EPlayer::start()
{
	SpriteEntity::start();
	addSprite();
	getSprite()->pos = sf::Vector2f(2.0f, 2.0f);

	setInterpSpeed(EPLAYER_NETFRAMERATE);
}

EPlayer::EPlayer(ProgramType* prog, uint32_t uid) : SpriteEntity(prog, uid), InventoryEntity(prog, uid, { 10, 3 }), Entity(prog, uid)
{
	sendTimer = EPLAYER_NETUPDATE; noclip = false;

	// Setup the inventory (we don't sync as this is called on the constructor)
	setSlotType({ 0, 0 }, InventorySlot::HEAD, false);
	setSlotType({ 0, 1 }, InventorySlot::TORSO, false);
	setSlotType({ 0, 2 }, InventorySlot::LEGS, false);
	setSlotType({ 1, 0 }, InventorySlot::GLASSES, false);
	setSlotType({ 1, 1 }, InventorySlot::BACKPACK, false);
	setSlotType({ 1, 2 }, InventorySlot::BELT, false);

	deltaPos = sf::Vector2f(1000.0f, 1000.0f);
}

EPlayer::~EPlayer()
{
}
