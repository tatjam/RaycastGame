#pragma once
#include "../../Entity.h"
#include "../../Map.h"
#include "../../Tile.h"

class SpriteEntity;

class TileEntity : public virtual Entity
{
private:

	// Used to save CPU cycles
	Tile* tile;

	size_t tileX, tileY;

public:

	virtual void start() override;

	// Called every frame while the user hovers
	// the block with the cursor
	// pixel is in texture coordinates (0->1) but horribly imprecise
	// Return true if the interaction icon should show
	// Only called on the client!
	virtual bool onUserHover(SpriteEntity* player, Side side, sf::Vector2f pixel);

	// Called the first frame the user clicks on us
	// pixel is in texture coordinates (0->1) but imprecise, in steps of 1/16
	// Return true to play a deny sound
	// Only called on the client!
	virtual bool onUserClick(SpriteEntity* player, Side side, sf::Vector2f pixel);

	// Called the frame the user releases the mouse
	// pixel is in texture coordinates (0->1) but imprecise, in steps of 1/16
	// Return true to play a deny sound
	// Only called on the client!
	virtual bool onUserRelease(SpriteEntity* player, Side side, sf::Vector2f pixel);

	Tile* getTile() { return tile; }
	sf::Vector2i getTilePos() { return sf::Vector2i((int)tileX, (int)tileY); }

	void setTilePos(sf::Vector2i pos);

	void setTilePos(int x, int y) 
	{
		setTilePos(sf::Vector2i(x, y));
	}

	virtual json serialize() override;
	virtual void deserialize(json j) override;

	TileEntity(ProgramType* prog, uint32_t uid) : Entity(prog, uid) { tile = NULL; }
	~TileEntity() {}
};