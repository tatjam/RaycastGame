#pragma once
#include <SFML/Graphics.hpp>
#include <Thor/Vectors.hpp>
#include <Dependency/easylogging++.h>
#include <Dependency/json.hpp>
#include "Sprite.h"
#include "Tile.h"

#ifndef PI
#define PI 3.14159265358979323f
#endif

#define MAP_SHADOW_FADE 0.4f
#define MAP_SHADOW_INTENSE 0.333f
#define MAP_LIGHT_PROPAGATION 4

#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1ULL<<(b)))
#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b)))) 
#define BIT_CHANGE(a, b, v) ((a ^= (-v ^ a) & (1UL << b)))

using namespace nlohmann;



class Map
{
private:

	sf::Uint8* outPixels;
	sf::Uint8* outBufferPixels;
	int prev_width, prev_height;

	uint16_t spriteUID;
public:

	// r = Reserved
	// g = Pick Buffer Coord X (0->255) or Sprite High byte
	// b = Pick Buffer Coord Y (0->255) or Sprite Low byte
	// a = Extra Data: Byte decomposes into -> 
	//		76543210
	//		76	-> Side of the wall, if a wall
	//		5	-> If set, ceiling, otherwise floor (only if ceiling or floor)
	//		4   -> If set, skybox, otherwise normal ceiling (only if ceiling or floor)
	//		3   -> Reserved
	//      2   -> Reserved
	//		1   -> Wall flag, if set this is a wall, otherwise a (celing / floor)
	//		0	-> Sprite flag: if set g and b are a sprite IDs instead of coordinates
	sf::Image buffers;

	float* depthBuffer;

	int map_width;
	int map_height;

	std::vector<Tile> tiles;
	std::vector<Sprite*> sprites;

	sf::Image tileset;
	sf::Image skybox;
	int tileWidth;



	// Renders the map to a target image from given position and direction
	void draw(sf::Image* target, sf::Vector2f pos, float angle, float viewPlaneDistance = 0.66f);

	Tile getTile(int x, int y);

	void updateLighting();

	void update(float dt, bool lighting = true);

	uint16_t getSpriteUID()
	{
		spriteUID++;
		return spriteUID;
	}

	json serialize();
	void deserialize(json data);

	Map(size_t width, size_t height);
	~Map();
};

