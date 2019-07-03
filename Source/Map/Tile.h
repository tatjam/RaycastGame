#pragma once

#include <Dependency/json.hpp>
#include <SFML/Graphics.hpp>

using namespace nlohmann;


enum Side
{
	NORTH,
	EAST,
	SOUTH,
	WEST,
	CEILING,
	FLOOR
};

struct SideList
{
	bool north, east, south, west, ceiling, floor;

	bool has(Side side)
	{
		if (side == NORTH)
		{
			return north;
		}
		else if (side == EAST)
		{
			return east;
		}
		else if (side == SOUTH)
		{
			return south;
		}
		else if (side == WEST)
		{
			return west;
		}
		else if (side == FLOOR)
		{
			return floor;
		}
		else if (side == CEILING)
		{
			return ceiling;
		}
		else
		{
			return false;
		}
	}
};


struct Tile
{
	enum TileType
	{
		EMPTY,

		// 1x1 Block
		WALL,

		// Thin wall
		// var0 -> 0: X oriented, 1: Y oriented
		// var1 -> Offset Along direction 0->255 (0->128 < 0, 128->255 > 0, 0 = 128)
		// var2 -> Offset Along normal 0->255
		THIN,

		// A column, circular. Maps the texture around it
		// var0 -> Radius, converted to a float so (0->255) = (0->1)
		// var1 -> Texture Offset, converted to a float (0->255) = (0->1)
		// var2 -> Center location
		COLUMN,




	};

	int texID;
	TileType tileType;
	// Can LIGHT go through this tile?
	bool transparent;
	uint8_t var0, var1, var2;

	bool walkable;

	int floorID;
	// 0 = Skybox, not really used in the game, but implemented
	int ceilingID;

	bool reflectiveFloor;
	bool reflectiveCeiling;

	// Light for floor and ceiling
	sf::Vector3f light;
	sf::Vector3f prevLight;
	sf::Vector3f startLight;

	// Only on non empties
	sf::Vector3f westLight;		// -X
	sf::Vector3f eastLight;		// +X 
	sf::Vector3f northLight;	// -Y
	sf::Vector3f southLight;	// +Y

	uint32_t linked_entity;

	// overlay must be sized exactly tileWidth*tileWidth

	SideList overlaySides;
	sf::Image* overlay;
	
	Tile()
	{
		texID = 0;
		tileType = EMPTY;
		transparent = true;
		var0 = 0; var1 = 0; var2 = 0;
		floorID = 0; ceilingID = 0;
		reflectiveFloor = false; reflectiveCeiling = false;
		linked_entity = 0;
		walkable = true;
		overlay = NULL;
		overlaySides = { 0 };
	}

	void setAllLights(sf::Vector3f val)
	{
		light = val;
		prevLight = val;
		startLight = val;
	}

	json serialize();
	void deserialize(json data);
};