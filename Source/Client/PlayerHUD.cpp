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

		bool leftDown = false, rightDown = false;

		if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
		{
			if (!wasLeftDown)
			{
				leftDown = true;
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
				rightDown = true;
			}

			wasRightDown = true;
		}
		else
		{
			wasRightDown = false;
		}


		// Handle item management, and indicate hovered slots
		if (inContextualMenu)
		{
			drawContextualMenu(leftDown, rightDown, player);
		}
		else
		{
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



			
			handleHand(leftDown, rightDown, rCoord, hoveredSlot, player, leftHand, rightHand, over);
		}
	}
}

void PlayerHUD::drawContextualMenu(bool leftDown, bool rightDown, InventoryEntity* inv)
{
	sf::RenderWindow* win = client->getWindow();

	std::vector<std::string> options;
	options.push_back("Swap");
	
	if (contextualItem != NULL)
	{
		options.push_back("Drop");
		std::string oppositeHand;
		if (contextualItem->getSpecialSlot() == L_HAND)
		{
			oppositeHand = "right hand";
		}
		else
		{
			oppositeHand = "left hand";
		}

		options.push_back("Use on " + oppositeHand);

		std::vector<std::string> fromItem = contextualItem->getContextualMenuEntries();
		options.insert(options.end(), fromItem.begin(), fromItem.end());
	}

	sf::RectangleShape overlay;
	overlay.setFillColor(CONTEXTUAL_OVERLAY_COLOR);
	overlay.setOutlineColor(CONTEXTUAL_STROKE_COLOR);
	overlay.setOutlineThickness(CONTEXTUAL_STROKE_WIDTH);

	float requiredHeight = CONTEXTUAL_HEIGHT_PER_ITEM * options.size();
	overlay.setSize(sf::Vector2f(CONTEXTUAL_WIDTH, requiredHeight));

	overlay.setPosition(sf::Vector2f(CONTEXTUAL_POS) - sf::Vector2f(0, requiredHeight) + sf::Vector2f(0, client->getUIStart()));

	win->draw(overlay);

	thor::Arrow separator;
	separator.setStyle(thor::Arrow::Style::Line);
	separator.setColor(CONTEXTUAL_SEPARATOR_COLOR);
	separator.setThickness(1.0f);

	sf::Vector2i mouse = sf::Mouse::getPosition(*win);


	sf::RectangleShape chosenOverlay;
	chosenOverlay.setFillColor(CONTEXTUAL_HOVER_COLOR);
	chosenOverlay.setSize(sf::Vector2f(overlay.getSize().x, CONTEXTUAL_HEIGHT_PER_ITEM));
	chosenOverlay.setOutlineThickness(0.0f);

	int chosen = -1;

	sf::Text text;
	text.setFont(client->getFonts()["consola.ttf"]);
	text.setCharacterSize(16);

	for (size_t i = 0; i < options.size(); i++)
	{
		sf::Vector2i min;
		min.x = overlay.getPosition().x;
		min.y = overlay.getPosition().y + CONTEXTUAL_HEIGHT_PER_ITEM * i;
		sf::Vector2i max;
		max.x = min.x + overlay.getSize().x;
		max.y = min.y + CONTEXTUAL_HEIGHT_PER_ITEM;

		// TODO: Center text?
		text.setString(options[i]);
		text.setPosition(min.x + 2, min.y);
		win->draw(text);

		if (i != options.size() - 1)
		{
			separator.setPosition(min.x, max.y);
			separator.setDirection(max.x - min.x, 0.0f);
			win->draw(separator);
		}

		if (mouse.x >= min.x && mouse.y >= min.y && mouse.x < max.x && mouse.y < max.y)
		{
			chosenOverlay.setPosition(min.x, min.y);
			win->draw(chosenOverlay);
			chosen = i;
		}
	}

	if (rightDown)
	{
		inContextualMenu = false;
	}

	if (leftDown)
	{
		if (chosen >= 0)
		{
			std::string str = options[chosen];

			if (str == "Swap")
			{
				ItemEntity* leftHand = inv->getItem(SpecialSlot::L_HAND);
				ItemEntity* rightHand = inv->getItem(SpecialSlot::R_HAND);

				if (leftHand != NULL)
				{
					leftHand->setInSpecialSlot(SpecialSlot::R_HAND);
				}

				if (rightHand != NULL)
				{
					rightHand->setInSpecialSlot(SpecialSlot::L_HAND);
				}

			}
			else if (str == "Drop")
			{

			}
			else if (str == "Use on right hand")
			{

			}
			else if (str == "Use on left hand")
			{

			}
			else
			{
				// Send to item
				if (contextualItem != NULL)
				{
					contextualItem->sendContextualMenuAction(str);
				}
			}
		

		}

		inContextualMenu = false;

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

		if (hovered == L_HAND || hovered == R_HAND)
		{
			// Special behaviour for these, left click
			// uses hovered on self, right-click
			// shows contextual menu

			if (leftClick && over != NULL)
			{
				over->useOnItem(over);
			}

			if (rightClick)
			{
				contextualItem = over;
				inContextualMenu = true;
			}
		}
		else
		{

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
					// Swap items maybe? TODO
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
				if (leftClick)
				{
					player->moveItemToSpecialSlot(over, L_HAND);
				}
				else if (rightClick)
				{
					player->moveItemToSpecialSlot(over, R_HAND);
				}

			}
		}
	}

}

void PlayerHUD::useItem(ItemEntity* item, EPlayer* player)
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
