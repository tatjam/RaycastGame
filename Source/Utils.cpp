#include "Utils.h"
#include <string>
#include <fstream>
#include <streambuf>
#include <stdio.h>

std::string getAddress(const std::string address)
{
	return address.substr(0, address.find_last_of(':'));
}

int getPort(const std::string address, int def)
{
	std::string sub = address.substr(address.find_last_of(':') + 1);

	if (sub == address)
	{
		return def;
	}

	try
	{
		int out = std::stoi(sub);

		return out;
	}
	catch (std::invalid_argument a)
	{
		return def;
	}
	catch (std::out_of_range a)
	{
		return def;
	}
}

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

std::string loadFile(const std::string& path)
{
	std::ifstream t(path);
	std::string str((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());

	return str;
}

void writeFile(const std::string& path, const std::string& contents)
{
	if (fileExists(path))
	{
		remove(path.c_str());
	}

	std::ofstream out(path);
	out << contents;
	out.close();
}

inline bool fileExists(const std::string& path)
{
	std::ifstream f(path.c_str());
	return f.good();
}
