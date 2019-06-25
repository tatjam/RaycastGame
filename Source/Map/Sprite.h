#pragma once

struct Sprite
{
	std::vector<sf::Image> frames;
	size_t frame;

	sf::Vector2f pos;
	// Used only on orientable sprites
	float angle;

	float scale;

	float vOffset;

	uint32_t linked_entity;

	// Local to the map, not synchronized, just used for picking
	uint16_t uid;

	Sprite(uint32_t nuid)
	{
		frames = std::vector<sf::Image>();
		frame = 0;
		pos = sf::Vector2f(0.0f, 0.0f);
		angle = 0.0f;
		scale = 1.0f;
		vOffset = 0.0f;
		linked_entity = 0;
		uid = nuid;
	}
};