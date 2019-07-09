#pragma once

#include <SFML/Graphics.hpp>
#include "../Map/Entities/Bases/InventoryEntity.h"

class Client;

// Handles the UI during normal gameplay
// including the inventory, and any interface
// which may come up later on, such as entities
// interfaces or visors.
// We hardcode the coordinates of the different
// item boxes
class PlayerHUD
{
private:

	Client* client;

	sf::Sprite defaultHUDSpr;
	sf::Texture defaultHUDTex;

	sf::RectangleShape colorer;
	
	const sf::Color DISABLED_COLOR = sf::Color(20, 20, 20);
	const sf::Color FULL_COLOR = sf::Color(20, 20, 20);

	const int tileWidth = 64;

	// Texture coordinates (in pixels) for the different boxes
	const sf::Vector2i BODY_POS = sf::Vector2i(51, 35);
	const sf::Vector2i BODY_OFFSET = sf::Vector2i(82, 82);

	const sf::Vector2i BULK_POS = sf::Vector2i(233, 35);
	const sf::Vector2i BULK_OFFSET = sf::Vector2i(65, 65);
	
	const sf::Vector2i BELT_POS = sf::Vector2i(233, 191);
	const sf::Vector2i BELT_OFFSET = sf::Vector2i(65, 0);

	const sf::Vector2i HAND_POS = sf::Vector2i(819, 26);
	const sf::Vector2i HAND_OFFSET = sf::Vector2i(65, 0);

	sf::Vector2i hoveredTile;

public:

	// Called after the game has been drawn to the screen 
	// You are given the different scalars
	// gameHeight is NOT scaled
	void draw(int screenScale, int uiScale, int gameHeight);

	// Called AFTER the world update
	void update(float dt);

	PlayerHUD(Client* cl);
	~PlayerHUD();
};

