#include "Map.h"
#include "Tile.h"
#include "Sprite.h"

sf::Color getPixelFast(const sf::Uint8* pixels, int x, int y, int width)
{
	sf::Uint8 r = pixels[(y * width + x) * 4 + 0];
	sf::Uint8 g = pixels[(y * width + x) * 4 + 1];
	sf::Uint8 b = pixels[(y * width + x) * 4 + 2];
	sf::Uint8 a = pixels[(y * width + x) * 4 + 3];
	return sf::Color(r, g, b, a);
}

void setPixelFast(sf::Uint8* target, int x, int y, int width, sf::Color color)
{
	target[(y * width + x) * 4 + 0] = color.r;
	target[(y * width + x) * 4 + 1] = color.g;
	target[(y * width + x) * 4 + 2] = color.b;
	target[(y * width + x) * 4 + 3] = color.a;
}

#define PIXEL_COMP_R 0
#define PIXEL_COMP_G 1
#define PIXEL_COMP_B 2
#define PIXEL_COMP_A 3

void setPixelComponentFast(sf::Uint8* target, int x, int y, int component, int width, sf::Uint8 val)
{
	target[(y * width + x) * 4 + component] = val;
	//target[(y * width + x) * 4 + 3] = 255;
}

sf::Uint8 getPixelComponentFast(sf::Uint8* pixels, int x, int y, int component, int width)
{
	return pixels[(y * width + x) * 4 + component];
}

void Map::draw(sf::Image* target, sf::Vector2f pos, float angle, float viewPlaneDist)
{

	const sf::Uint8* tilesetPixels = tileset.getPixelsPtr();
	const sf::Uint8* skyboxPixels = skybox.getPixelsPtr();
	int tilesetWidth = tileset.getSize().x;


	int width = target->getSize().x;
	int height = target->getSize().y;

	// Reallocate buffers if neccesary
	if (depthBuffer == NULL)
	{
		depthBuffer = (float*)malloc(sizeof(float) * width * height);
	}
	else if (width != prev_width || height != prev_height)
	{
		depthBuffer = (float*)realloc(depthBuffer, sizeof(float) * width * height);
	}

	if (outPixels == NULL)
	{
		outPixels = (sf::Uint8*)malloc(4 * sizeof(sf::Uint8) * width * height);
	}
	else if (width != prev_width || height != prev_height)
	{
		outPixels = (sf::Uint8*)realloc(outPixels, 4 * sizeof(sf::Uint8) * width * height);
	}

	if (outBufferPixels == NULL)
	{
		outBufferPixels = (sf::Uint8*)malloc(4 * sizeof(sf::Uint8) * width * height);
	}
	else if (width != prev_width || height != prev_height)
	{
		outBufferPixels = (sf::Uint8*)realloc(outBufferPixels, 4 * sizeof(sf::Uint8) * width * height);
	}

	// Clear z-buffer to very high values
	for (int i = 0; i < width * height; i++)
	{
		depthBuffer[i] = 999999.0f;
	}

	// Direction vector
	sf::Vector2f direction;
	direction.x = sin(angle);
	direction.y = cos(angle);

	sf::Vector2f screenPlane;

	// Screen plane is perpendicular to direction, plus 90º (pi/2 rad)
	// Also it's moved forward by viewPlaneDist
	screenPlane.x = sin(angle + PI * 0.5f);
	screenPlane.y = cos(angle + PI * 0.5f);

	for (size_t x = 0; x < target->getSize().x; x++)
	{
		float cameraX = (float)(2 * x / (float)width) - 1;
		sf::Vector2f rayDir = direction * viewPlaneDist + screenPlane * cameraX;
		
		sf::Vector2i map = sf::Vector2i((int)pos.x, (int)pos.y);

		sf::Vector2f sideDist;

		sf::Vector2f deltaDist; 
		deltaDist.x = std::abs(1.0f / rayDir.x);
		deltaDist.y = std::abs(1.0f / rayDir.y);
		float perpWallDist;

		sf::Vector2i step;

		Tile hit = Tile();
		int side = 0; //< Side of the hit
		
		// Calculate step and sideDist
		if (rayDir.x < 0)
		{
			step.x = -1;
			sideDist.x = (pos.x - map.x) * deltaDist.x;
		}
		else
		{
			step.x = 1;
			sideDist.x = (map.x + 1.0f - pos.x) * deltaDist.x;
		}

		if (rayDir.y < 0)
		{
			step.y = -1;
			sideDist.y = (pos.y - map.y) * deltaDist.y;
		}
		else
		{
			step.y = 1;
			sideDist.y = (map.y + 1.0f - pos.y) * deltaDist.y;
		}

		while (hit.tileType == Tile::EMPTY)
		{
			if (sideDist.x < sideDist.y)
			{
				sideDist.x += deltaDist.x; 
				map.x += step.x;
				side = 0;
			}
			else
			{
				sideDist.y += deltaDist.y;
				map.y += step.y;
				side = 1;
			}

			hit = getTile(map.x, map.y);
		}

		if (side == 0)
		{
			perpWallDist = ((float)map.x - pos.x + (1 - (float)step.x) / 2.0f) / rayDir.x;
		}
		else
		{
			perpWallDist = ((float)map.y - pos.y + (1 - (float)step.y) / 2.0f) / rayDir.y;
		}

		int lineHeight = (int)(height / perpWallDist);

		int drawStart = (int)(-(float)lineHeight / 2 + height / 2);
		if (drawStart < 0)
		{
			drawStart = 0;
		}

		int drawEnd = (int)((float)lineHeight / 2 + height / 2);
		if (drawEnd >= height)
		{
			drawEnd = height - 1;
		}


		float wallX;
		if (side == 0)
		{
			wallX = pos.y + perpWallDist * rayDir.y;
		}
		else
		{
			wallX = pos.x + perpWallDist * rayDir.x;
		}

		wallX -= floor(wallX);

		// 0 = N, 1 = E, 2 = S, 3 = W
		int realSide = 0;
		if (side == 1)
		{
			if (rayDir.y >= 0)
			{
				realSide = 0;
			}
			else
			{
				realSide = 2;
			}
		}

		if (side == 0)
		{
			if (rayDir.x >= 0)
			{
				realSide = 3;
			}
			else
			{
				realSide = 1;
			}
		}

		int texX = (int)(wallX * tileWidth);
		if (side == 0 && rayDir.x > 0)
		{
			texX = (int)(tileWidth - texX - 1);
		}

		if (side == 1 && rayDir.y < 0)
		{
			texX = (int)(tileWidth - texX - 1);
		}

		for (int y = drawStart; y <= drawEnd; y++)
		{
			float currentDepth = depthBuffer[y * width + x];
			if (perpWallDist < currentDepth)
			{
				int d = y * 256 - (int)height * 128 + lineHeight * 128; // We use these factors to avoid floats
				int texY = ((d * tileWidth) / lineHeight) / 256;

				//sf::Color color = tileset.getPixel(texX, texY);
				// We rotate the texture 90º clockwise (change x for y)
				sf::Color color = getPixelFast(tilesetPixels, texY, texX + tileWidth * hit.texID, tilesetWidth);

				if (realSide == 0)
				{
					color = sf::Color(color.r * ((float)hit.northLight.r / 255.0f), color.g * ((float)hit.northLight.g / 255.0f), color.b * ((float)hit.northLight.b / 255.0f));
				}
				else if (realSide == 1)
				{
					color = sf::Color(color.r * ((float)hit.eastLight.r / 255.0f), color.g * ((float)hit.eastLight.g / 255.0f), color.b * ((float)hit.eastLight.b / 255.0f));
				}
				else if (realSide == 2)
				{
					color = sf::Color(color.r * ((float)hit.southLight.r / 255.0f), color.g * ((float)hit.southLight.g / 255.0f), color.b * ((float)hit.southLight.b / 255.0f));
				}
				else if (realSide == 3)
				{
					color = sf::Color(color.r * ((float)hit.westLight.r / 255.0f), color.g * ((float)hit.westLight.g / 255.0f), color.b * ((float)hit.westLight.b / 255.0f));
				}

				// Set coordinate X
				setPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_G, width, map.x);
				// Set coordinate Y
				setPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_B, width, map.y);

				// Clear alpha as this is the first pass
				sf::Uint8 alpha = 0;

				// Set wall flag and side 
				BIT_CLEAR(alpha, 0);
				BIT_SET(alpha, 1);
				BIT_CHANGE(alpha, 6, BIT_CHECK(realSide, 0));
				BIT_CHANGE(alpha, 7, BIT_CHECK(realSide, 1));

				setPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_A, width, alpha);

				// Write depth buffer
				depthBuffer[y * width + x] = perpWallDist;

				//target->setPixel(x, y, color);
				setPixelFast(outPixels, x, y, width, color);
			}


		}

		// Draw floor and ceiling
		sf::Vector2f floorWall;

		if (side == 0 && rayDir.x > 0)
		{
			floorWall.x = (float)map.x;
			floorWall.y = (float)map.y + wallX;
		}
		else if (side == 0 && rayDir.x < 0)
		{
			floorWall.x = (float)map.x + 1.0f;
			floorWall.y = (float)map.y + wallX;
		}
		else if (side == 1 && rayDir.y > 0)
		{
			floorWall.x = (float)map.x + wallX;
			floorWall.y = (float)map.y;
		}
		else
		{
			floorWall.x = (float)map.x + wallX;
			floorWall.y = (float)map.y + 1.0f;
		}

		float distWall, distPlayer, currentDist;
		distWall = perpWallDist;
		distPlayer = 0.0;

		if (drawEnd < 0)
		{
			drawEnd = height;
		}

		for (int y = drawEnd + 1; y < height; y++)
		{
			float currentDepthFloor = depthBuffer[y * width + x];
			float currentDepthCeiling = depthBuffer[(height - y) * width + x];

			currentDist = (float)height / (2.0f * (float)y - (float)height);
			float weight = (currentDist - distPlayer) / (distWall - distPlayer);
			sf::Vector2f currentFloor;
			currentFloor.x = weight * floorWall.x + (1.0 - weight) * pos.x;
			currentFloor.y = weight * floorWall.y + (1.0 - weight) * pos.y;
			
			sf::Vector2i floorTex;
			floorTex.x = int(currentFloor.x * tileWidth) % (int)tileWidth;
			floorTex.y = int(currentFloor.y * tileWidth) % (int)tileWidth;

			sf::Color reflect = getPixelFast(outPixels, x, drawEnd - (y - drawEnd), width);

			// Add a bit of a shadow
			float shadow = std::abs(1.0f - weight);

			shadow = std::max(std::min(shadow, 0.8f), 0.0f);
			Tile at = getTile((int)currentFloor.x, (int)currentFloor.y);
			sf::Color color;

			if (currentDist < currentDepthFloor)
			{
				color = getPixelFast(tilesetPixels, floorTex.y, floorTex.x + tileWidth * at.floorID, tilesetWidth);

				if (at.reflectiveFloor)
				{
					float r = reflect.r * (1.0f - shadow) + color.r;
					float g = reflect.g * (1.0f - shadow) + color.g;
					float b = reflect.b * (1.0f - shadow) + color.b;
					if (r > 255.0f) { r = 255.0f; }
					if (g > 255.0f) { g = 255.0f; }
					if (b > 255.0f) { b = 255.0f; }
					color = sf::Color(r, g, b);
				}
				else
				{
					float newShadow = shadow;
					if (newShadow >= MAP_SHADOW_FADE)
					{
						newShadow = MAP_SHADOW_FADE;
					}

					newShadow *= 1.0f / MAP_SHADOW_FADE;
					newShadow += MAP_SHADOW_INTENSE;
					if (newShadow >= 1.0f)
					{
						newShadow = 1.0f;
					}

					color = sf::Color(color.r * newShadow, color.g * newShadow, color.b * newShadow);
				}


				color = sf::Color(color.r * (at.light.r / 255.0f), color.g * (at.light.g / 255.0f), color.b * (at.light.b / 255.0f));

				setPixelFast(outPixels, x, y, width, color);

			}

			if (currentDist < currentDepthCeiling)
			{
				if (at.ceilingID == 0)
				{
					// Skybox
					color = getPixelFast(skyboxPixels, 0, std::min(height - y, (int)skybox.getSize().y - 1), 1);
				}
				else
				{
					color = getPixelFast(tilesetPixels, floorTex.y, floorTex.x + tileWidth * at.ceilingID, tilesetWidth);

					if (at.reflectiveCeiling)
					{
						float r = reflect.r * (1.0f - shadow) + color.r;
						float g = reflect.g * (1.0f - shadow) + color.g;
						float b = reflect.b * (1.0f - shadow) + color.b;
						if (r > 255.0f) { r = 255.0f; }
						if (g > 255.0f) { g = 255.0f; }
						if (b > 255.0f) { b = 255.0f; }
						color = sf::Color(r, g, b);
					}
					else
					{
						float newShadow = shadow;
						if (newShadow >= MAP_SHADOW_FADE)
						{
							newShadow = MAP_SHADOW_FADE;
						}

						newShadow *= 1.0f / MAP_SHADOW_FADE;
						newShadow += MAP_SHADOW_INTENSE;
						if (newShadow >= 1.0f)
						{
							newShadow = 1.0f;
						}

						color = sf::Color(color.r * newShadow, color.g * newShadow, color.b * newShadow);
					}

					color = sf::Color(color.r * (at.light.r / 255.0f), color.g * (at.light.g / 255.0f), color.b * (at.light.b / 255.0f));
				}

				setPixelFast(outPixels, x, height - y, width, color);
			}

			sf::Uint8 alpha = getPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_A, width);


			// Clear wall and sprite flag
			BIT_CLEAR(alpha, 0);
			BIT_CLEAR(alpha, 1);

			// We are not ceiling
			BIT_CLEAR(alpha, 4);
			BIT_CLEAR(alpha, 5);


			// Set coordinate X
			setPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_G, width, (int)currentFloor.x);
			// Set coordinate Y
			setPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_B, width, (int)currentFloor.y);
			// Set flags
			setPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_A, width, alpha);


			// Mirror (Floor now)

			// Set ceiling
			BIT_SET(alpha, 5);

			// Set skybox flag
			if (at.ceilingID == 0)
			{
				BIT_SET(alpha, 4);
			}
			else
			{
				BIT_CLEAR(alpha, 4);
			}



			// Set coordinate X
			setPixelComponentFast(outBufferPixels, x, height - y, PIXEL_COMP_G, width, (int)currentFloor.x);
			// Set coordinate Y
			setPixelComponentFast(outBufferPixels, x, height - y, PIXEL_COMP_B, width, (int)currentFloor.y);
			// Set flags
			setPixelComponentFast(outBufferPixels, x, height - y, PIXEL_COMP_A, width, alpha);

			// Set depth buffer
			depthBuffer[y * width + x] = currentDist;

			if (at.ceilingID != 0)
			{
				depthBuffer[(height - y) * width + x] = currentDist;
			}

		}
	}

	// Draw sprites

	for (size_t i = 0; i < sprites.size(); i++)
	{

		if (sprites[i]->frames.size() <= sprites[i]->frame)
		{
			// Sprite is invisible
			continue;
		}

		sf::Image frame = sprites[i]->frames[sprites[i]->frame];
		int imgWidth = frame.getSize().x;
		int imgHeight = frame.getSize().y;


		// Relative to player position
		sf::Vector2f projected = sprites[i]->pos - pos;
		// Transform with the inverse camera matrix
		float invDet = 1.0f / (screenPlane.x * direction.y - direction.x * screenPlane.y);

		sf::Vector2f transform;
		transform.x = invDet * (direction.y * projected.x - direction.x * projected.y);
		transform.y = invDet * (-screenPlane.y * projected.x + screenPlane.x * projected.y);

		int spriteScreenX = (int)((width / 2) * (1.0f + transform.x / transform.y));

		// Vertical
		int spriteHeight = std::abs((int)((float)height / transform.y));

		int vMoveScreen = (int)((sprites[i]->vOffset * imgHeight) / transform.y);


		int drawStartY = -spriteHeight / 2 + height / 2 + vMoveScreen;
		if (drawStartY < 0)
		{
			drawStartY = 0;
		}
		int drawEndY = spriteHeight / 2 + height / 2 + vMoveScreen;
		if (drawEndY >= height)
		{
			drawEndY = height - 1;
		}

		// Horizontal
		int spriteWidth = abs((int)((float)height / transform.y));
		int drawStartX = -spriteWidth / 2 + spriteScreenX;
		if (drawStartX < 0)
		{
			drawStartX = 0;
		}
		int drawEndX = spriteWidth / 2 + spriteScreenX;
		if (drawEndX >= width)
		{
			drawEndX = width - 1;
		}

		Tile at = Tile();

		if (sprites[i]->pos.x > 0 && sprites[i]->pos.y > 0 && sprites[i]->pos.x <= map_width && sprites[i]->pos.y <= map_height)
		{
			at = tiles[(int)sprites[i]->pos.y * map_width + (int)sprites[i]->pos.x]; 	
		}

		// 0 = 1 Orientation  (F, Default)
		// 1 = 4 Orientations (F, R, B, L)
		// 2 = 8 Orientations (F, FR, R, RB, B, BL, L)
		int spriteType = 0;

		int realWidth = imgHeight;
		int realHeight = imgHeight;

		if (imgWidth == imgHeight)
		{
			spriteType = 0;
		}
		else if (imgWidth == 4 * imgHeight)
		{
			spriteType = 1;
		}
		else if(imgWidth == 8 * imgHeight)
		{
			spriteType = 2;
		}

		
		int orientation = 1;

		sf::Vector2f diff = sprites[i]->pos - pos;

		float enangle = atan2(diff.y, diff.x) + fmod(sprites[i]->angle + 0.5f * PI, 2.0f * PI);

		if (enangle < 0)
		{
			enangle = (2 * PI) + enangle;
		}

		if (enangle > 2 * PI)
		{
			enangle = enangle - (2 * PI);
		}

		if (spriteType == 1)
		{

		}
		else
		{
			if (enangle <= PI / 4 && enangle > 0)
			{
				orientation = 0;
			}
			else if (enangle > (1 * PI) / 4.0f && enangle <= (2 * PI) / 4.0f)
			{
				orientation = 1;
			}
			else if (enangle > (2 * PI) / 4.0f && enangle <= (3 * PI) / 4.0f)
			{
				orientation = 2;
			}
			else if (enangle > (3 * PI) / 4.0f && enangle <= (4 * PI) / 4.0f)
			{
				orientation = 3;
			}
			else if (enangle > (4 * PI) / 4.0f && enangle <= (5 * PI) / 4.0f)
			{
				orientation = 4;
			}
			else if (enangle > (5 * PI) / 4.0f && enangle <= (6 * PI) / 4.0f)
			{
				orientation = 5;
			}
			else if (enangle > (6 * PI) / 4.0f && enangle <= (7 * PI) / 4.0f)
			{
				orientation = 6;
			}
			else
			{
				orientation = 7;
			}
		}

		for (int x = drawStartX; x < drawEndX; x++)
		{
			int texX = (int)(256 * (x - (-spriteWidth / 2 + spriteScreenX)) * realWidth / spriteWidth) / 256;
			texX += orientation * realWidth;

			// We only draw if it's in front of us, it's on screen and it's visible in the depth buffer
			if (transform.y > 0 && x >= 0 && x < width)
			{
				for (int y = drawStartY; y < drawEndY; y++)
				{
					// Depth buffer check here
					if (transform.y < depthBuffer[y * width + x])
					{
						int d = (y - vMoveScreen) * 256 - height * 128 + spriteHeight * 128;
						int texY = ((d * realHeight) / spriteHeight) / 256;

						// No need for fast read functions here as it's a small ammount
						sf::Color color = frame.getPixel(texX, texY);
						color = sf::Color(color.r * ((float)at.light.r / 255.0f), color.g * ((float)at.light.g / 255.0f), color.b * ((float)at.light.b / 255.0f), color.a);

						if (color.a != 0)
						{
							setPixelFast(outPixels, x, y, width, color);

							depthBuffer[y * width + x] = transform.y;

							sf::Uint8 highByte = (sprites[i]->uid >> 8) & 0xFF;
							sf::Uint8 lowByte = (sprites[i]->uid >> 0) & 0xFF;
							sf::Uint8 flags = getPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_A, width);

							BIT_SET(flags, 0);

							// Set Flags and sprite UID
							setPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_G, width, highByte);
							setPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_B, width, lowByte);
							setPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_A, width, flags);
						}
					}
				}
			}
		}
	}

	target->create(width, height, outPixels);
	buffers.create(width, height, outBufferPixels);

	prev_width = width;
	prev_height = height;
}

Tile Map::getTile(int x, int y)
{
	Tile out;

	if (x < 0 || y < 0 || x >= map_width || y >= map_height)
	{
		out.tileType = Tile::WALL;
		out.walkable = false;
		return out;
	}
	else
	{
		return tiles[y * map_width + x];
	}
}

void Map::updateLighting()
{
	// Delete all lights and apply skylight
	for (size_t y = 0; y < (size_t)map_height; y++)
	{
		for (size_t x = 0; x < (size_t)map_width; x++)
		{
			Tile* tile = &tiles[y * map_width + x];
			if (tile->ceilingID == 0 && (tile->transparent || tile->tileType == Tile::EMPTY))
			{
				tile->light = sf::Color(255, 255, 255);
			}
			else if (x == 4 && y == 4)
			{
				tile->light = sf::Color(128, 128, 128);
			}
			else
			{
				tile->light = sf::Color(0, 0, 0);
			}

			tile->prevLight = tile->light;
			tile->startLight = tile->light;
		}
	}
	
	// Apply entity lights
	// TODO

	// Propagate
	for (size_t i = 0; i < MAP_LIGHT_PROPAGATION; i++)
	{
		for (size_t y = 0; y < (size_t)map_height; y++)
		{
			for (size_t x = 0; x < (size_t)map_width; x++)
			{

				Tile* tile = &tiles[y * map_width + x];
				if (tile->ceilingID == 0)
				{
					continue;
				}

				// We only propagate if we are transparent
				if (tile->transparent || tile->tileType == Tile::EMPTY)
				{

					Tile u = Tile(), d = Tile(), r = Tile(), l = Tile();

					if (y > 0)
					{
						u = tiles[(y - 1) * map_width + (x + 0)];
					}
					if (y < (size_t)map_height - 1)
					{
						d = tiles[(y + 1) * map_width + (x + 0)];
					}

					if (x < (size_t)map_width - 1)
					{
						r = tiles[(y + 0) * map_width + (x + 1)];
					}
					if (x > 0)
					{
						l = tiles[(y + 0) * map_width + (x - 1)];
					}

					sf::Vector3f starting = sf::Vector3f(tile->light.r, tile->light.g, tile->light.b);
					float startingLength = thor::length(starting);
					sf::Vector3f surroundingAverage = sf::Vector3f(0.0f, 0.0f, 0.0f);
					int rcount = 0, gcount = 0, bcount = 0;

					sf::Vector3f uv = sf::Vector3f(u.prevLight.r, u.prevLight.g, u.prevLight.b);
					sf::Vector3f dv = sf::Vector3f(d.prevLight.r, d.prevLight.g, d.prevLight.b);
					sf::Vector3f rv = sf::Vector3f(r.prevLight.r, r.prevLight.g, r.prevLight.b);
					sf::Vector3f lv = sf::Vector3f(l.prevLight.r, l.prevLight.g, l.light.b);
					sf::Vector3f sv = sf::Vector3f(tile->prevLight.r, tile->prevLight.g, tile->prevLight.b);
					
					/*if (thor::length(uv) >= startingLength) {surroundingAverage += uv; count++;}
					if (thor::length(dv) >= startingLength) { surroundingAverage += dv; count++; }
					if (thor::length(rv) >= startingLength) { surroundingAverage += rv; count++; }
					if (thor::length(lv) >= startingLength) { surroundingAverage += lv; count++; }
					if (thor::length(sv) >= startingLength) { surroundingAverage += sv; count++; }

					if (count > 0)
					{
						surroundingAverage /= (float)count;
					}*/

					if (uv.x >= starting.x) { surroundingAverage.x += uv.x; rcount++; }
					if (dv.x >= starting.x) { surroundingAverage.x += dv.x; rcount++; }
					if (rv.x >= starting.x) { surroundingAverage.x += rv.x; rcount++; }
					if (lv.x >= starting.x) { surroundingAverage.x += lv.x; rcount++; }
					if (sv.x >= starting.x) { surroundingAverage.x += sv.x; rcount++; }

					if (uv.y >= starting.y) { surroundingAverage.y += uv.y; gcount++; }
					if (dv.y >= starting.y) { surroundingAverage.y += dv.y; gcount++; }
					if (rv.y >= starting.y) { surroundingAverage.y += rv.y; gcount++; }
					if (lv.y >= starting.y) { surroundingAverage.y += lv.y; gcount++; }
					if (sv.y >= starting.y) { surroundingAverage.y += sv.y; gcount++; }

					if (uv.z >= starting.z) { surroundingAverage.z += uv.z; bcount++; }
					if (dv.z >= starting.z) { surroundingAverage.z += dv.z; bcount++; }
					if (rv.z >= starting.z) { surroundingAverage.z += rv.z; bcount++; }
					if (lv.z >= starting.z) { surroundingAverage.z += lv.z; bcount++; }
					if (sv.z >= starting.z) { surroundingAverage.z += sv.z; bcount++; }

					surroundingAverage.x /= (float)rcount;
					surroundingAverage.y /= (float)gcount;
					surroundingAverage.z /= (float)bcount;

					// We cannot lose light
					if (thor::length(surroundingAverage) > startingLength)
					{
						tile->light = sf::Color(std::min(surroundingAverage.x, 255.0f), std::min(surroundingAverage.y, 255.0f), std::min(surroundingAverage.z, 255.0f));
					}
					// If we are exposed to sunlight, saturate
					if (tile->startLight != sf::Color(0, 0, 0))
					{
						tile->light = tile->startLight;
					}
				}
			}
		}

		for (size_t y = 0; y < (size_t)map_height; y++)
		{
			for (size_t x = 0; x < (size_t)map_width; x++)
			{
				Tile* tile = &tiles[y * map_width + x];
				tile->prevLight = tile->light;
			}
		}
	}
	// Apply into walls

	for (size_t y = 0; y < (size_t)map_height; y++)
	{
		for (size_t x = 0; x < (size_t)map_width; x++)
		{
			Tile* tile = &tiles[y * map_width + x];
			tile->prevLight = tile->light;

			Tile u = Tile(), d = Tile(), r = Tile(), l = Tile();

			if (y > 0)
			{
				u = tiles[(y - 1) * map_width + (x + 0)];
			}
			if (y < (size_t)map_height - 1)
			{
				d = tiles[(y + 1) * map_width + (x + 0)];
			}

			if (x < (size_t)map_width - 1)
			{
				r = tiles[(y + 0) * map_width + (x + 1)];
			}
			if (x > 0)
			{
				l = tiles[(y + 0) * map_width + (x - 1)];
			}

			tile->northLight = u.light;
			tile->eastLight = r.light;
			tile->southLight = d.light;
			tile->westLight = l.light;

		}
	}
}

void Map::update(float dt, bool lighting)
{
	// TODO: Separate constant lights and dynamic for better perfomance
	if (lighting)
	{
		updateLighting();
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

Map::Map(size_t width, size_t height)
{
	this->map_width = width;
	this->map_height = height;

	tiles.resize(width * height, Tile());

	sf::Image tilesetBuff;
	tilesetBuff.loadFromFile("Assets/tileset.png");

	tileset = rotate90(&tilesetBuff);

	tileWidth = tileset.getSize().x;
	

	skybox.loadFromFile("Assets/skybox.png");

	// We give some edges to the world
	for (size_t y = 0; y < (size_t)map_height; y++)
	{
		for (size_t x = 0; x < (size_t)map_width; x++)
		{
			if (x == 0 || y == 0 || y >= (size_t)map_height - 1 || x >= (size_t)map_width - 1)
			{
				tiles[y * map_width + x].tileType = Tile::WALL;
				tiles[y * map_width + x].texID = 6;
				tiles[y * map_width + x].walkable = false;
			}
		}
	}

	depthBuffer = NULL;
	outPixels = NULL;
	outBufferPixels = NULL;
}


Map::~Map()
{
}

json Map::serialize()
{
	json root = json();

	root["width"] = map_width;
	root["height"] = map_height;

	json tilesj;

	for (size_t i = 0; i < tiles.size(); i++)
	{
		tilesj[i] = tiles[i].serialize();
	}

	root["tiles"] = tilesj;

	return root;
}

void Map::deserialize(json data)
{
	map_width = data["width"];
	map_height = data["height"];

	tiles.resize(map_width * map_height);

	for (size_t i = 0; i < (size_t)map_width * (size_t)map_height; i++)
	{
		tiles[i].deserialize(data["tiles"][i]);
	}
}

json Tile::serialize()
{
	json root;

	root["texID"] = texID;
	root["type"] = tileType;
	root["transparent"] = transparent;
	root["var0"] = var0;
	root["var1"] = var1;
	root["var2"] = var2;
	root["floorID"] = floorID;
	root["ceilingID"] = ceilingID;
	root["reflectiveCeiling"] = reflectiveCeiling;
	root["reflectiveFloor"] = reflectiveFloor;
	root["walkable"] = walkable;

	// Lights don't need to be included, they are computed
	// Neither do entities as these add themselves to a tile
	// on game start
	// Same for sprites, they are created by an entity on start

	return root;
}

void Tile::deserialize(json data)
{
	texID = data["texID"];
	tileType = data["type"];
	transparent = data["transparent"];
	var0 = data["var0"];
	var1 = data["var1"];
	var2 = data["var2"];
	floorID = data["floorID"];
	ceilingID = data["ceilingID"];
	reflectiveCeiling = data["reflectiveCeiling"];
	reflectiveFloor = data["reflectiveFloor"];
	walkable = data["walkable"];
}
