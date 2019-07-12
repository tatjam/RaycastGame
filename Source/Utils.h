#pragma once
#include <SFML/Graphics.hpp>

sf::Image rotate90(sf::Image* original);



inline void hash_combine(std::size_t& seed) { }

template <typename T, typename... Rest>
inline void hash_combine(std::size_t& seed, const T& v, Rest... rest) {
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	hash_combine(seed, rest...);
}

// We do this to allow using Vector2i as keys
template <>
struct std::hash<sf::Vector2i>
{
	std::size_t operator()(const sf::Vector2i& k) const
	{
		size_t seed = 0;
		hash_combine(seed, k.x, k.y);
		return seed;
	}
};