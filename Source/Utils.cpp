#include "Utils.h"

sf::Image rotate90(sf::Image* original)
{
sf::Image out;
size_t orWidth = original->getSize().x;
size_t orHeight = original->getSize().y;
sf::Uint8* pixels = (sf::Uint8*)malloc(orWidth * orHeight * 4 * sizeof(sf::Uint8));
const sf::Uint8* orPixels = original->getPixelsPtr();

for (size_t orY = 0; orY < orHeight; orY++)
{
	for (size_t orX = 0; orX < orWidth; orX++)
	{
		// To rotate we change x for y
		size_t x = orY;
		size_t y = orX;

		pixels[(y * orHeight + x) * 4 + 0] = orPixels[(orY * orWidth + orX) * 4 + 0];
		pixels[(y * orHeight + x) * 4 + 1] = orPixels[(orY * orWidth + orX) * 4 + 1];
		pixels[(y * orHeight + x) * 4 + 2] = orPixels[(orY * orWidth + orX) * 4 + 2];
		pixels[(y * orHeight + x) * 4 + 3] = orPixels[(orY * orWidth + orX) * 4 + 3];
	}
}

out.create(orHeight, orWidth, pixels);
free(pixels);
return out;
}