#include "TileEntity.h"

void TileEntity::start()
{
	Entity::start();

	if (tile == NULL)
	{
		Map* map = getWorld()->map;

		tile = &map->tiles[tileY * map->map_width + tileX];
		tile->linked_entity = uid;
	}
	else
	{
		tile->linked_entity = uid;
	}
}

bool TileEntity::onUserHover(SpriteEntity * player, Side side, sf::Vector2f pixel)
{
	return false;
}

bool TileEntity::onUserClick(SpriteEntity * player, Side side, sf::Vector2f pixel)
{
	return false;
}

bool TileEntity::onUserRelease(SpriteEntity * player, Side side, sf::Vector2f pixel)
{
	return false;
}

json TileEntity::serialize()
{
	json root = Entity::serialize();

	root["tileX"] = tileX;
	root["tileY"] = tileY;

	return root;
}

void TileEntity::deserialize(json j)
{
	Entity::deserialize(j);
	tileX = j["tileX"];
	tileY = j["tileY"];
}