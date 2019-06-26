#include "TileEntity.h"

void TileEntity::start()
{
	Entity::start();

	if (tile == NULL)
	{
		Map* map = getWorld()->map;

		setTilePos(tileX, tileY);
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

void TileEntity::setTilePos(sf::Vector2i pos)
{
	if (tile != NULL)
	{
		tile->linked_entity = 0;
	}

	tileX = pos.x;
	tileY = pos.y;
	tile = &getWorld()->map->tiles[tileY * getWorld()->map->map_width + tileX];

	tile->linked_entity = uid;
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