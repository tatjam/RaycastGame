#pragma once
#include <SFML/Graphics.hpp>
#include <Thor/Vectors.hpp>
#include <Dependency/easylogging++.h>
#include <Dependency/json.hpp>
#include "Sprite.h"
#include "Tile.h"
#include "Light.h"
#include "../Utils.h"
#include <thread>

#ifndef PI
#define PI 3.14159265358979323f
#endif

#define MAP_SHADOW_FADE 0.4f
#define MAP_SHADOW_INTENSE 0.333f
#define MAP_LIGHT_PROPAGATION 2

#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))
#define BIT_FLIP(a,b) ((a) ^= (1ULL<<(b)))
#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b)))) 
#define BIT_CHANGE(a, b, v) ((a ^= (-v ^ a) & (1UL << b)))

#define TEX_PREC 16.0f

#define MAP_THREAD_COUNT 1


using namespace nlohmann;

#define DRAW_SKIP 1


class Map
{
private:

	int drawNum = 0;

	sf::Uint8* outPixels;
	sf::Uint8* outBufferPixels;
	int prev_width, prev_height;

	uint16_t spriteUID;
	uint16_t lightUID;



public:

	// All these draw functions return true
	// if we must continue casting the ray
	// They take a huge ammount of arguments because they are kind of "inlined"
	// hoping the compiler will actually just copy-paste them into draw
	bool drawWall(Tile hit, sf::Vector2i& step, int& side, int realSide, size_t& x, sf::Vector2f& pos, sf::Vector2f& rayDir,
		sf::Vector2i& map, float& perpWallDist, int& height,
		int& width, const sf::Uint8* tilesetPixels, int& tilesetWidth, const sf::Uint8* skyboxPixels);


	bool drawThin(Tile hit, sf::Vector2f rayOverride, sf::Vector2f rayDirOverride, sf::Vector2i& step, int& side, int realSide, size_t& x,
		sf::Vector2f& pos, sf::Vector2f& rayDir, sf::Vector2f& camDir, sf::Vector2i& map, float& perpWallDist, int& height,
		int& width, const sf::Uint8* tilesetPixels, int& tilesetWidth, const sf::Uint8* skyboxPixels);

	bool drawColumn(Tile hit, sf::Vector2i& step, int& side, int realSide, size_t& x,
		sf::Vector2f& pos, sf::Vector2f& rayDir, sf::Vector2f& camDir, sf::Vector2i& map, float& perpWallDist, int& height,
		int& width, const sf::Uint8* tilesetPixels, int& tilesetWidth, const sf::Uint8* skyboxPixels);

	// Floor and ceiling drawing is done 
	// always, which is a bit of a perfomance issue
	// but not too bad as transparency is rare
	void drawFloorAndCeiling(int& side, size_t& x, sf::Vector2f& pos, sf::Vector2f& rayDir,
		sf::Vector2i& map, float& wallX, float& perpWallDist, float distReal, int& drawEnd, int& height,
		int& width, const sf::Uint8* tilesetPixels, int& tilesetWidth, const sf::Uint8* skyboxPixels);



	// r = Rough texture position, to be seen as a float
	//		76543210
	//		7654	-> X-position (0->16).
	//		3210	-> Y-position (0->16)
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
	std::vector<Light*> lights;

	sf::Image tileset;
	sf::Image skybox;
	int tileWidth;



	// Renders the map to a target image from given position and direction
	void draw(sf::Image* target, sf::Vector2f pos, float angle, float viewPlaneDistance = 0.66f);

	Tile getTile(int x, int y);
	// Has no safety checks
	Tile* getTilePtr(int x, int y);

	void updateLighting();

	void update(float dt, bool lighting = true);

	// Interpolates light linearly between surrounding 
	// tiles. Used for rendering sprites
	sf::Vector3f getLight(sf::Vector2f pos);

	sf::Vector3f getLight(float x, float y)
	{
		return getLight(sf::Vector2f(x, y));
	}

	uint16_t getSpriteUID()
	{
		spriteUID++;
		return spriteUID;
	}

	uint16_t getLightUID()
	{
		lightUID++;
		return lightUID;
	}

	json serialize();
	void deserialize(json data);

	Map(size_t width, size_t height);
	~Map();
};

