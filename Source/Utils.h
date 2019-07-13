#pragma once
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cctype>
#include <locale>


sf::Image rotate90(sf::Image* original);

std::string loadFile(const std::string& path);

// Overwrites previous ALWAYS
void writeFile(const std::string& path, const std::string& contents);

inline bool fileExists(const std::string& path);

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

static inline void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !std::isspace(ch);
	}).base(), s.end());
}
