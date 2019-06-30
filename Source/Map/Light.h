#pragma once
#include <SFML/System.hpp>

// Lights cast light into a map
struct Light
{
	enum LightType
	{
		// Simplest, just "sets"
		// the light at its position
		POINT,

		// Raycasts an area, creating
		// shadows. It has a maximum area
		AREA,

		// Same as before but only on a given
		// direction. (Straight line!)
		SPOT,

	};


	LightType type;

	sf::Vector2f pos;
	sf::Vector3f light;

	// Only on AREA and SPOT
	float maxDist;

	// Only on SPOT
	float direction;

	// For softer lights, set to a high value, for stronger, to a lower
	float attenuation;

	uint16_t id;

	Light(uint16_t id)
	{
		type = POINT;
		pos = sf::Vector2f(0.5f, 0.5f);
		light = sf::Vector3f(1.0f, 1.0f, 1.0f);
		attenuation = 1.0f;
		this->id = id;
	}
};