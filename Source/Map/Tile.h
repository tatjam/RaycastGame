#pragma once

#include <Dependency/json.hpp>
#include <SFML/Graphics.hpp>

using namespace nlohmann;


struct Tile
{
	enum TileType
	{
		EMPTY,
		// 1x1 Block
		WALL,
		// Thin wall
		// var0 -> 0: X oriented, 1: Y oriented
		// var1 -> Offset Along direction 0->128
		// var2 -> Offset Along normal 0->128
		THIN,
		// A 1x1 column
		// var0 -> 0: 1x1, 1: 0.5x0.5, 2: 0.25x0.25
		// Always centered
		COLUMN,
	};

	int texID;
	TileType tileType;
	// Set to true if this use a texture with alpha
	// as the ray will continue
	bool transparent;
	int var0, var1, var2;

	bool walkable;

	int floorID;
	// 0 here means skybox (TODO
	int ceilingID;

	bool reflectiveFloor;
	bool reflectiveCeiling;

	// Light for floor and ceiling
	sf::Color light;
	sf::Color prevLight;
	sf::Color startLight;

	// Only on non empties
	sf::Color westLight;	// -X
	sf::Color eastLight;	// +X 
	sf::Color northLight;	// -Y
	sf::Color southLight;	// +Y

	uint32_t linked_entity;

	Tile()
	{
		texID = 0;
		tileType = EMPTY;
		transparent = false;
		var0 = 0; var1 = 0; var2 = 0;
		floorID = 0; ceilingID = 0;
		reflectiveFloor = false; reflectiveCeiling = false;
		linked_entity = 0;
		walkable = true;
	}

	json serialize();
	void deserialize(json data);
};