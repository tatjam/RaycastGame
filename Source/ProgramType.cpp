#include "ProgramType.h"
#include <experimental/filesystem>

void ProgramType::loadAllResources(const std::string& path)
{
	using recursive_directory_iterator = std::experimental::filesystem::recursive_directory_iterator;
	for (const auto& dirEntry : recursive_directory_iterator(path))
	{
		if (dirEntry.path().extension().string() == ".png")
		{
			std::string nPath = dirEntry.path().string();

			// Remove "path"
			std::string id = dirEntry.path().string();
			id = id.substr(path.size());

			// Fix separators to use the standard / (POSIX)
			std::replace(id.begin(), id.end(), '\\', '/');

			images.acquire(id, thor::Resources::fromFile<sf::Image>(nPath));
		}
	}
}
