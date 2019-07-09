#include "PlayerHUD.h"
#include "Client.h"

// gameHeight is NOT scaled
void PlayerHUD::draw(int screenScale, int uiScale, int gameHeight)
{


	sf::RenderWindow* win = client->getWindow();

	int endOfGame = gameHeight * screenScale;
	int endOfUI = endOfGame + defaultHUDTex.getSize().y;
	// This will be negative
	int remaining = win->getSize().y - endOfUI;
	int uiPos = endOfGame + remaining;


	defaultHUDSpr.setScale(uiScale, uiScale);
	defaultHUDSpr.setPosition({ 0.0f, float(uiPos) });


	sf::Vector2i pos = sf::Mouse::getPosition(*win);
	pos.y -= uiPos;
	sf::Vector2i rCoord = sf::Vector2i(-1, -1);

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

	win->draw(defaultHUDSpr);

	// Draw items

	colorer.setSize(sf::Vector2f(float(uiScale * tileWidth), float(uiScale * tileWidth)));
	colorer.setOutlineThickness(0.0f);

	if (client->controlledEntityPtr != NULL)
	{
		InventoryEntity* player = dynamic_cast<InventoryEntity*>(client->controlledEntityPtr);
		if (player != NULL)
		{
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
					else if (player->getItem({ x, y }) != NULL)
					{
						colorer.setFillColor(FULL_COLOR);
						win->draw(colorer);
					}

					if (x == rCoord.x && y == rCoord.y)
					{
						colorer.setFillColor(sf::Color(255, 255, 255, 50));
						win->draw(colorer);
					}
				}
			}
		}
	}
	

}

void PlayerHUD::update(float dt)
{

}

PlayerHUD::PlayerHUD(Client* cl)
{
	client = cl;
	defaultHUDTex.loadFromFile("Assets/hud.png");
	defaultHUDSpr.setTexture(defaultHUDTex);

}

PlayerHUD::~PlayerHUD()
{
}
