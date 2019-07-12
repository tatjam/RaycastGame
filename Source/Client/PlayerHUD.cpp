#include "PlayerHUD.h"
#include "Client.h"


// gameHeight is NOT scaled
void PlayerHUD::draw(int screenScale, int uiScale, int gameHeight)
{
	sf::Sprite itemDrawer;

	sf::RenderWindow* win = client->getWindow();

	int endOfGame = gameHeight * screenScale;
	int endOfUI = endOfGame + defaultHUDTex.getSize().y;
	// This will be negative
	int remaining = win->getSize().y - endOfUI;
	int uiPos = endOfGame + remaining;


	defaultHUDSpr.setScale(uiScale, uiScale);
	defaultHUDSpr.setPosition({ 0.0f, float(uiPos) });


	sf::Vector2i realPos = sf::Mouse::getPosition(*win);


	sf::Vector2i pos = realPos;
	pos.y -= uiPos;
	sf::Vector2i rCoord = sf::Vector2i(-1, -1);
	SpecialSlot hoveredSlot = NONE;

	if (pos.x >= BODY_POS.x && pos.x < BODY_OFFSET.x * 2 + BODY_POS.x && pos.y >= BODY_POS.y && pos.y < BODY_OFFSET.y * 3 + BODY_POS.y)
	{
		sf::Vector2i sCoord;

		sCoord.x = floorf((float)(pos.x - BODY_POS.x) / (float)BODY_OFFSET.x);
		sCoord.y = floorf((float)(pos.y - BODY_POS.y) / (float)BODY_OFFSET.y);

		// Check that we are not in inter-tile space
		if ((pos.x - BODY_POS.x - sCoord.x * BODY_OFFSET.x) % BODY_OFFSET.x <= tileWidth && (pos.y - BODY_POS.y - sCoord.y * BODY_OFFSET.x) % BODY_OFFSET.y <= tileWidth)
		{
			rCoord = sCoord;

		}
	}
	else if (pos.x >= BULK_POS.x && pos.y >= BULK_POS.y && pos.x < BULK_POS.x + BULK_OFFSET.x * 8 && pos.y < BULK_POS.y + BULK_OFFSET.y * 2)
	{
		// No need for inter-tiling here
		rCoord.x = floorf((float)(pos.x - BULK_POS.x) / (float)BULK_OFFSET.x) + 2;
		rCoord.y = floorf((float)(pos.y - BULK_POS.y) / (float)BULK_OFFSET.y);
	}
	else if (pos.x >= BELT_POS.x && pos.y >= BELT_POS.y && pos.x < BELT_POS.x + BELT_OFFSET.x * 8 && pos.y < BELT_POS.y + tileWidth)
	{
		// No need for inter-tiling here
		// We use tileWidth instead of BELT_OFFSET.y as this is a Nx1 row
		rCoord.x = floorf((float)(pos.x - BELT_POS.x) / (float)BELT_OFFSET.x) + 2;
		rCoord.y = floorf((float)(pos.y - BELT_POS.y) / (float)tileWidth) + 2;
	}
	else if (pos.x >= HAND_POS.x && pos.y >= HAND_POS.y && pos.x < HAND_POS.x + HAND_OFFSET.x * 2 && pos.y < HAND_POS.y + tileWidth)
	{
		if (pos.x < HAND_POS.x + HAND_OFFSET.x)
		{
			hoveredSlot = L_HAND;
		}
		else
		{
			hoveredSlot = R_HAND;
		}
	}


	// Draw items

	colorer.setSize(sf::Vector2f(float(uiScale * tileWidth), float(uiScale * tileWidth)));
	colorer.setOutlineThickness(0.0f);

	if (client->controlledEntityPtr != NULL)
	{
		InventoryEntity* player = dynamic_cast<InventoryEntity*>(client->controlledEntityPtr);


		ItemEntity* leftHand = player->getItem(SpecialSlot::L_HAND);
		ItemEntity* rightHand = player->getItem(SpecialSlot::R_HAND);

		if (leftHand != NULL)
		{
			sf::Texture leftHandTexture = leftHand->getTextureInHand();
			itemDrawer.setTexture(leftHandTexture);
			sf::IntRect rect;
			rect.left = leftHand->getItemFrame() * leftHandTexture.getSize().y;
			rect.top = 0;
			rect.width = leftHandTexture.getSize().y;
			rect.height = leftHandTexture.getSize().y;

			itemDrawer.setTextureRect(rect);
			itemDrawer.setPosition(leftHand->getHandPos());
			itemDrawer.setScale(leftHand->getHandScale());
			itemDrawer.setColor(leftHand->getHandTint());

			win->draw(itemDrawer);
		}

		if (rightHand != NULL)
		{
			sf::Texture rightHandTexture = rightHand->getTextureInHand();
			itemDrawer.setTexture(rightHandTexture);
			sf::IntRect rect;
			rect.left = rightHand->getItemFrame() * rightHandTexture.getSize().y;
			rect.top = 0;
			rect.width = rightHandTexture.getSize().y;
			rect.height = rightHandTexture.getSize().y;

			itemDrawer.setTextureRect(rect);
			itemDrawer.setPosition(rightHand->getHandPos());
			itemDrawer.setScale(rightHand->getHandScale());
			itemDrawer.setColor(rightHand->getHandTint());

			win->draw(itemDrawer);
		}

		itemDrawer.setColor(sf::Color(255, 255, 255));
		itemDrawer.setScale(1.0f, 1.0f);

		win->draw(defaultHUDSpr);

		for (int x = 0; x < 10; x++)
		{
			for (int y = 0; y < 3; y++)
			{
				sf::Vector2f base;
				sf::Vector2f offset;
				sf::Vector2f pos;

				int xminus = 0;

				if (x <= 1)
				{
					base = sf::Vector2f(float(BODY_POS.x), float(BODY_POS.y));
					offset = sf::Vector2f(float(BODY_OFFSET.x), float(BODY_OFFSET.y));
				}
				else if (x > 1)
				{
					xminus = 2;
					if (y <= 1)
					{
						base = sf::Vector2f(float(BULK_POS.x), float(BULK_POS.y));
						offset = sf::Vector2f(float(BULK_OFFSET.x), float(BULK_OFFSET.y));
					}
					else
					{
						base = sf::Vector2f(float(BELT_POS.x), float(BELT_POS.y));
						offset = sf::Vector2f(float(BELT_OFFSET.x), float(BELT_OFFSET.y));
					}
				}

				pos = sf::Vector2f(base.x + (x - xminus) * offset.x, base.y + y * offset.y + uiPos);
				pos.x = floorf(pos.x);
				pos.y = floorf(pos.y);

				colorer.setPosition(pos);

				if (player->isTileDisabled({ x, y }))
				{
					colorer.setFillColor(DISABLED_COLOR);
					win->draw(colorer);
				}

				if (player->getItem({ x, y }) != NULL)
				{
					colorer.setFillColor(FULL_COLOR);
					win->draw(colorer);

					ItemEntity* item = player->getItem({ x, y });
					sf::Texture itemTexture = item->getTextureInInventory();

					itemDrawer.setTexture(itemTexture);
					sf::IntRect rect;
					rect.left = item->getItemFrame() * itemTexture.getSize().y;
					rect.top = 0;
					rect.width = itemTexture.getSize().y;
					rect.height = itemTexture.getSize().y;

					itemDrawer.setTextureRect(rect);
					itemDrawer.setPosition(colorer.getPosition());

					/*if (item == heldItem)
					{
						itemDrawer.setColor(sf::Color(128, 128, 128, 128));
					}
					else
					{
						itemDrawer.setColor(sf::Color(255, 255, 255, 255));
					}*/

					win->draw(itemDrawer);
				}

				if (x == rCoord.x && y == rCoord.y)
				{
					colorer.setFillColor(sf::Color(255, 255, 255, 50));
					win->draw(colorer);
				}
			}
		}



		if (leftHand != NULL)
		{
			sf::Texture itemTexture = leftHand->getTextureInInventory();
			itemDrawer.setTexture(itemTexture);
			sf::IntRect rect;
			rect.left = leftHand->getItemFrame() * itemTexture.getSize().y;
			rect.top = 0;
			rect.width = itemTexture.getSize().y;
			rect.height = itemTexture.getSize().y;

			itemDrawer.setTextureRect(rect);
			itemDrawer.setPosition(HAND_POS.x, HAND_POS.y + uiPos);
			win->draw(itemDrawer);
		}

		if (rightHand != NULL)
		{
			sf::Texture itemTexture = rightHand->getTextureInInventory();
			itemDrawer.setTexture(itemTexture);
			sf::IntRect rect;
			rect.left = rightHand->getItemFrame() * itemTexture.getSize().y;
			rect.top = 0;
			rect.width = itemTexture.getSize().y;
			rect.height = itemTexture.getSize().y;

			itemDrawer.setTextureRect(rect);
			itemDrawer.setPosition(HAND_POS.x + HAND_OFFSET.x, HAND_POS.y + uiPos);
			win->draw(itemDrawer);
		}

		// Handle item management, and indicate hovered slots
		int hand = 0;

		ItemEntity* over = NULL;

		colorer.setFillColor(sf::Color(255, 255, 255, 50));


		if (hoveredSlot == NONE)
		{
			over = player->getItem(rCoord);
		}
		else if (hoveredSlot == L_HAND)
		{
			over = leftHand;
			colorer.setPosition(HAND_POS.x, HAND_POS.y + uiPos);
			win->draw(colorer);
		}
		else if (hoveredSlot == R_HAND)
		{
			over = rightHand;
			colorer.setPosition(HAND_POS.x + HAND_OFFSET.x, HAND_POS.y + uiPos);
			win->draw(colorer);
		}

		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
		{
			if (!wasLeftDown)
			{
				hand = 1;
			}

			wasLeftDown = true;
		}
		else
		{
			wasLeftDown = false;
		}

		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
		{
			if (!wasRightDown)
			{
				hand = 2;
			}

			wasRightDown = true;
		}
		else
		{
			wasRightDown = false;
		}

	
		if (hand == 1)
		{
			
		}
		else if (hand == 2)
		{
			if (rightHand != NULL && over == NULL)
			{
				if (hoveredSlot == L_HAND && rightHand->isOneHanded())
				{
					player->moveItemToSpecialSlot(rightHand, SpecialSlot::L_HAND);
				}
				else
				{
					if (player->isTileUsable(rCoord) && rightHand->canGoInSlot(player->getSlotType(rCoord)))
					{
						player->moveItem(rightHand, rCoord);
					}
				}
			}

			if (over != NULL && rightHand != NULL)
			{
				// Try to use item on the other item (TODO)
			}

			if (over != NULL && rightHand == NULL)
			{
				if (over->isOneHanded())
				{
					player->moveItemToSpecialSlot(over, SpecialSlot::R_HAND);
				}
				else
				{
					if (leftHand == NULL)
					{
						player->moveItemToSpecialSlot(over, SpecialSlot::B_HAND);
					}
				}
			}
		}

	}
}

void PlayerHUD::handleHand(bool leftClick, bool rightClick, sf::Vector2i hoveredCoords, SpecialSlot hovered, InventoryEntity* player, ItemEntity* leftHand, ItemEntity* rightHand, ItemEntity* over)
{
	SpecialSlot opposite;
	if (hovered == R_HAND)
	{
		opposite = L_HAND;
	}
	else 
	{
		opposite = R_HAND;
	}

	EPlayer* asPlayer = dynamic_cast<EPlayer*>(player);

	if (asPlayer->inFPSControl)
	{
		if (leftClick)
		{
			useItem(leftHand, asPlayer);
		}

		if (rightClick)
		{
			useItem(rightHand, asPlayer);
		}
	}
	else
	{
		ItemEntity* source = NULL;
		ItemEntity* opposite = NULL;
		if (leftClick)
		{
			source = leftHand;
			opposite = rightHand;
		}
		
		if (rightClick)
		{
			source = rightHand;
			opposite = leftHand;
		}


		if (over == NULL && source != NULL)
		{
			if (hovered == NORMAL)
			{
				// Try to set the hand item there
				if (player->isTileUsable(hoveredCoords) && source->canGoInSlot(player->getSlotType(hoveredCoords)))
				{
					player->moveItem(source, hoveredCoords);
				}
			}
			else
			{
				if (hovered == R_HAND)
				{
					// Move to right hand
					player->moveItemToSpecialSlot(source, R_HAND);
				}
			}
		}

		if (over != NULL && source != NULL)
		{
			if (hovered == NORMAL)
			{

			}
			else
			{
				if (hovered == L_HAND || hovered == R_HAND)
				{
					source->useOnItem(over);
				}
			}
		}

		if (over != NULL && source == NULL)
		{
			// Move item to hand
			player->moveItemToSpecialSlot(over, hovered);
		}
	}

}

void PlayerHUD::useItem(ItemEntity * item, EPlayer * player)
{
}

void PlayerHUD::update(float dt)
{

}

PlayerHUD::PlayerHUD(Client* cl)
{
	client = cl;
	defaultHUDTex.loadFromImage(client->getImages()["hud.png"]);
	defaultHUDSpr.setTexture(defaultHUDTex);

	wasLeftDown = false;
	wasRightDown = false;
}

PlayerHUD::~PlayerHUD()
{
}
