#include "Map.h"
#include "Tile.h"
#include "Sprite.h"

sf::Uint8 encodeTexCoords(sf::Vector2f coords)
{
	uint8_t coordX = coords.x * TEX_PREC;
	uint8_t coordY = coords.y * TEX_PREC;

	return (coordX << 4) | coordY;
}

sf::Color getPixelFast(const sf::Uint8* pixels, int x, int y, int width)
{
	sf::Uint8 r = pixels[(y * width + x) * 4 + 0];
	sf::Uint8 g = pixels[(y * width + x) * 4 + 1];
	sf::Uint8 b = pixels[(y * width + x) * 4 + 2];
	sf::Uint8 a = pixels[(y * width + x) * 4 + 3];
	return sf::Color(r, g, b, a);
}

sf::Color mixColor(sf::Color origin, sf::Color overlay)
{
	float alphaFloat = (float)overlay.a / 255.0f;
	sf::Uint8 r = origin.r * (1.0f - alphaFloat) + overlay.r * alphaFloat;
	sf::Uint8 g = origin.g * (1.0f - alphaFloat) + overlay.g * alphaFloat;
	sf::Uint8 b = origin.b * (1.0f - alphaFloat) + overlay.b * alphaFloat;

	return sf::Color(r, g, b);
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

bool Map::drawWall(Tile hit, sf::Vector2i& step, int& side, int realSide, size_t& x, sf::Vector2f& pos, sf::Vector2f& rayDir,
	sf::Vector2i& map, float& perpWallDist, int& height,
	int& width, const sf::Uint8* tilesetPixels, int& tilesetWidth, const sf::Uint8* skyboxPixels)
{
	

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

	int texX = (int)(wallX * tileWidth);
	if (side == 0 && rayDir.x > 0)
	{
		texX = (int)(tileWidth - texX - 1);
	}

	if (side == 1 && rayDir.y < 0)
	{
		texX = (int)(tileWidth - texX - 1);
	}

	sf::Vector3f mult;
	if (realSide == 0)
	{
		mult = hit.northLight;
	}
	else if (realSide == 1)
	{
		mult = hit.eastLight;
	}
	else if (realSide == 2)
	{
		mult = hit.southLight;
	}
	else if (realSide == 3)
	{
		mult = hit.westLight;
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

			if (hit.overlaySides.has((Side)realSide))
			{
				// Note that we use swapped coordinates here, too
				sf::Color overColor = getPixelFast(hit.overlay->getPixelsPtr(), texY, texX, hit.overlay->getSize().x);
				color = mixColor(color, overColor);
			}

			color = sf::Color(color.r * mult.x, color.g * mult.y, color.b * mult.z, color.a);

			// Set coordinate X
			setPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_G, width, map.x);
			// Set coordinate Y
			setPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_B, width, map.y);

			// Clear alpha as this is the first pass
			sf::Uint8 alpha = getPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_A, width);

			// Set wall flag and side 
			BIT_CLEAR(alpha, 0);
			BIT_SET(alpha, 1);
			BIT_CHANGE(alpha, 6, BIT_CHECK(realSide, 0));
			BIT_CHANGE(alpha, 7, BIT_CHECK(realSide, 1));

			setPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_A, width, alpha);

			sf::Vector2f tCoords;
			tCoords.x = (float)texX / (float)tileWidth;
			tCoords.y = (float)texY / (float)tileWidth;

			setPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_R, width, encodeTexCoords(tCoords));


			// Write depth buffer
			depthBuffer[y * width + x] = perpWallDist;

			//target->setPixel(x, y, color);
			setPixelFast(outPixels, x, y, width, color);
		}


	}

	drawFloorAndCeiling(side, x, pos, rayDir, map, wallX, perpWallDist, perpWallDist, drawEnd, height, width, tilesetPixels, tilesetWidth, skyboxPixels);


	// Walls always end drawing, they can't be transparent
	return false;
}

bool Map::drawThin(Tile hit, sf::Vector2f rayOverride, sf::Vector2f rayDirectorOverride,sf::Vector2i& step, int& side, int realSide, size_t& x, sf::Vector2f& pos,
	sf::Vector2f& rayDir, sf::Vector2f& camDir, sf::Vector2i& map, float& perpWallDist, int& height, int& width, 
	const sf::Uint8* tilesetPixels, int& tilesetWidth, const sf::Uint8* skyboxPixels)
{
	// We don't really need any complex stuff as this is line - straight line intersection
	// So find the intersection point, find distance, draw column and draw floor / ceiling
	// Many rays will miss, so this can return true very easily


	// wall indicates the exact position of the ray hit on the "square"
	// surrounding the thin wall
	float wall;

	if (side == 0)
	{
		wall = pos.y + perpWallDist * rayDir.y;
	}
	else
	{
		wall = pos.x + perpWallDist * rayDir.x;
	}

	wall -= floor(wall);


	sf::Vector2f rayPos;

	if (rayOverride.x >= 0.0f && rayOverride.y >= 0.0f)
	{
		rayPos = rayOverride;
	}
	else
	{
		// NESW
		if (realSide == 0)
		{
			rayPos.x = wall;
			rayPos.y = 0.0f;
		}
		else if (realSide == 1)
		{
			rayPos.x = 1.0f;
			rayPos.y = wall;
		}
		else if (realSide == 2)
		{
			rayPos.x = wall;
			rayPos.y = 1.0f;
		}
		else
		{
			rayPos.x = 0.0f;
			rayPos.y = wall;
		}
	}
	
	sf::Vector2f rayDirector = (rayPos + sf::Vector2f(map.x, map.y)) - pos;
	if (rayOverride.x >= 0.0f && rayOverride.y >= 0.0f)
	{
		rayDirector = rayDirectorOverride;
	}

	sf::Vector2f wallDirector;
	if (hit.var0 == 0)
	{
		wallDirector = sf::Vector2f(1.0f, 0.0f);
	}
	else
	{
		wallDirector = sf::Vector2f(0.0f, 1.0f);
	}

	// Ax + By = C, v = (-B, A)
	sf::Vector3f rayL;
	rayL.x = rayDirector.y; 
	rayL.y = -rayDirector.x;
	rayL.z = rayL.x * rayPos.x + rayL.y * rayPos.y;

	sf::Vector2f wallP;
	if (hit.var0 == 0)
	{
		wallP = sf::Vector2f(0.0f, (float)hit.var2 / 255.0f);
	}
	else
	{
		wallP = sf::Vector2f((float)hit.var2 / 255.0f, 0.0f);
	}

	sf::Vector3f wallL;
	wallL.x = wallDirector.y;
	wallL.y = wallDirector.x;
	wallL.z = wallL.x * wallP.x + wallL.y * wallP.y;

	// Cramer's rule
	float den = (rayL.x * wallL.y - rayL.y * wallL.x);
	if (den == 0.0f)
	{
		// Parallel lines
		return true;
	}
	float xIntercept = (rayL.z * wallL.y - rayL.y * wallL.z) / den;
	float yIntercept = (rayL.x * wallL.z - rayL.z * wallL.x) / den;

	// xIntercept is on the line rayPos->rayDir, so 
	// if k is positive it means it's in our front
	float kX = (xIntercept - rayPos.x) / rayDir.x;
	float kY = (yIntercept - rayPos.y) / rayDir.y;

	if (kX < 0 || kY < 0)
	{
		// We missed the wall
		return true;
	}

	float texXDisplace = (((float)hit.var1 / 255.0f) - 0.5f) * 2.0f;

	float minX = 0.0f, minY = 0.0f, maxX = 1.0f, maxY = 1.0f;
	if (hit.var0 == 0)
	{
		minX += texXDisplace;
		maxX += texXDisplace;
	}
	else
	{
		minY += texXDisplace;
		maxY += texXDisplace;
	}

	minX = std::min(std::max(minX, 0.0f), 1.0f); maxX = std::min(std::max(maxX, 0.0f), 1.0f);
	minY = std::min(std::max(minY, 0.0f), 1.0f); maxY = std::min(std::max(maxY, 0.0f), 1.0f);

	if (xIntercept < minX || xIntercept > maxX || yIntercept < minY || yIntercept > maxY)
	{
		// We missed the wall
		return true;
	}
	else
	{

		// Distance is not euler distance but perpendicular distance, projected ontop of the direction vector
		float realWallDist = std::abs(thor::dotProduct(sf::Vector2f(xIntercept, yIntercept) + sf::Vector2f(map.x, map.y) - pos, camDir));

		int lineHeight = (int)(height / realWallDist);

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

		if (drawEnd < drawStart)
		{
			return true;
		}

		int texX;
		if (hit.var0 == 0)
		{
			texX = (int)(xIntercept * tileWidth);
		}
		else
		{
			texX = (int)(yIntercept * tileWidth);
		}

		bool hitTransparent = false;

		// Lighting
		
		// If we are "open" we use the block light directly
		// Open means var1 is not 0 (128), there is a gap
		sf::Vector3f mult;
		if (hit.var1 == 128 && hit.var2 != 0 && hit.var2 != 255)
		{
			if (hit.var0 == 1)
			{
				// y-Oriented
				if (pos.x < map.x + (float)hit.var2 / 255.0f)
				{
					// Seen from the west
					mult = hit.westLight;
				}
				else
				{
					// Seen from the east
					mult = hit.eastLight;
				}
			}
			else
			{
				// X-Oriented
				if (pos.y < map.y + (float)hit.var2 / 255.0f)
				{
					// Seen from the north
					mult = hit.northLight;
				}
				else
				{
					// Seen from the south
					mult = hit.southLight;
				}
			}
		}
		else
		{
			mult = hit.light;

		}

		for (size_t y = drawStart; y <= drawEnd; y++)
		{
			float currentDepth = depthBuffer[y * width + x];
			if (realWallDist < currentDepth)
			{
				int d = y * 256 - (int)height * 128 + lineHeight * 128; // We use these factors to avoid floats
				int texY = ((d * tileWidth) / lineHeight) / 256;

				//sf::Color color = tileset.getPixel(texX, texY);
				// We rotate the texture 90º clockwise (change x for y)
				int texXoff = texXDisplace * tileWidth;
				if (texXoff >= tileWidth)
				{
					texXoff = tileWidth - 1;
				}
				sf::Color color = getPixelFast(tilesetPixels, texY, texX + tileWidth * hit.texID - texXoff, tilesetWidth);
				// We don't support overlays

				color.r *= mult.x; color.g *= mult.y; color.b *= mult.z;

				if (color.a < 255)
				{
					hitTransparent = true;
				}
				else
				{
					// Set coordinate X
					setPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_G, width, map.x);
					// Set coordinate Y
					setPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_B, width, map.y);

					// Clear alpha as this is the first pass
					sf::Uint8 alpha = getPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_A, width);

					// Set wall flag and side 
					BIT_CLEAR(alpha, 0);
					BIT_SET(alpha, 1);
					BIT_CHANGE(alpha, 6, BIT_CHECK(realSide, 0));
					BIT_CHANGE(alpha, 7, BIT_CHECK(realSide, 1));

					setPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_A, width, alpha);

					sf::Vector2f tCoords;
					tCoords.x = (float)texX / (float)tileWidth;
					tCoords.y = (float)texY / (float)tileWidth;

					setPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_R, width, encodeTexCoords(tCoords));

					setPixelFast(outPixels, x, y, width, color);

					depthBuffer[y * width + x] = realWallDist;
				}
			}
		}

		float realWall = wall;
		
		drawFloorAndCeiling(side, x, pos, rayDir, map, realWall, perpWallDist, realWallDist, drawEnd, height, width, tilesetPixels, tilesetWidth, skyboxPixels);
		 
		if (hitTransparent)
		{
			// We must keep going for this column
			return true;
		}
		else
		{
			return false;
		}
	}

}


bool Map::drawColumn(Tile hit, sf::Vector2i& step, int& side, int realSide, size_t& x, sf::Vector2f& pos, sf::Vector2f& rayDir, sf::Vector2f& camDir, sf::Vector2i& map,
	float& perpWallDist, int& height, int& width, const sf::Uint8* tilesetPixels, int& tilesetWidth, const sf::Uint8* skyboxPixels)
{
	// Line-circle intersection

	// We express the ray as y=mx+c
	// or as a vertical line (x=n)
	// The circle is centered on (0.5, 0.5)


	// Obtain the ray's equation
	float wall;

	if (side == 0)
	{
		wall = pos.y + perpWallDist * rayDir.y;
	}
	else
	{
		wall = pos.x + perpWallDist * rayDir.x;
	}

	wall -= floor(wall);


	sf::Vector2f rayPos;

	// NESW
	if (realSide == 0)
	{
		rayPos.x = wall;
		rayPos.y = 0.0f;
	}
	else if (realSide == 1)
	{
		rayPos.x = 1.0f;
		rayPos.y = wall;
	}
	else if (realSide == 2)
	{
		rayPos.x = wall;
		rayPos.y = 1.0f;
	}
	else
	{
		rayPos.x = 0.0f;
		rayPos.y = wall;
	}
	
	sf::Vector2f rayDirector = (rayPos + sf::Vector2f(map.x, map.y)) - pos;


	// Circle
	// p = CenterX
	// q = CenterY
	// r = radius
	float p = 0.5f; float q = 0.5f;
	float r = (float)hit.var0 / 512.0f;

	float xSol, ySol;

	if (std::abs(rayDirector.x) >= 0.001f)
	{
		// Normal line
		// y = mx + c
		float m = rayDirector.y / rayDirector.x;

		// c = y - mx, being (x, y) = rayPos
		float c = rayPos.y - m * rayPos.x;

		// Solve a quadratic
		// Ax^2 + Bx + C = 0
		float A = m * m + 1.0f;
		float B = 2.0f * (m * c - m * q - p);
		float C = q * q - r * r + p * p - 2 * c * q + c * c;

		// Determinant
		float det = B * B - 4.0f * A * C;

		if (det < 0.0f)
		{
			// We missed the circle
			return true;
		}

		if (det == 0.0f)
		{
			xSol = -B / (2.0f * A);
			ySol = m * xSol + c;
		}
		else if (det > 0.0f)
		{
			// We have two solutions, we must choose the closest one
			float x1 = (-B + sqrtf(det)) / (2.0f * A);
			float x2 = (-B - sqrtf(det)) / (2.0f * A);
			float y1 = m * x1 + c;
			float y2 = m * x2 + c;

			sf::Vector2f s1 = sf::Vector2f(x1, y1);
			sf::Vector2f s2 = sf::Vector2f(x2, y2);

			if (thor::length(rayPos - s1) >= thor::length(rayPos - s2))
			{
				xSol = x2;
				ySol = y2;
			}
			else
			{
				xSol = x1;
				ySol = y1;
			}
		}
	}
	else
	{
		// Vertical line
		// We solve for y as we know x
		float k = rayPos.x;
		xSol = k;

		float A = 1.0f;
		float B = -2.0f * q;
		float C = p * p + q * q - r * r - 2 * k * p + k * k;

		float det = B * B - 4.0f * A * C;
		if (det < 0.0f)
		{
			// We miss
			return true;
		}
		else if (det == 0.0f)
		{
			ySol = -B / (2.0f * A);
		}
		else
		{
			float y1 = (-B + sqrt(det)) / (2.0f * A);
			float y2 = (-B - sqrt(det)) / (2.0f * A);

			sf::Vector2f s1 = sf::Vector2f(xSol, y1);
			sf::Vector2f s2 = sf::Vector2f(xSol, y2);

			if (thor::length(rayPos - s1) >= thor::length(rayPos - s2))
			{
				ySol = y2;
			}
			else
			{
				ySol = y1;
			}
		}

	}

	float realWallDist = std::abs(thor::dotProduct(sf::Vector2f(xSol, ySol) + sf::Vector2f(map.x, map.y) - pos, camDir));

	int lineHeight = (int)(height / realWallDist);

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

	float angle = ((atan2(ySol - 0.5f, xSol - 0.5f) / PI) + 1.0f) / 2.0f;

	sf::Vector3f light;

	if (hit.transparent)
	{
		light = hit.light;
	}
	else
	{
		// 0 = Center, -1 = Opposie, 1 = Alongside
		float xNrm = (xSol - 0.5f) / r;
		float yNrm = (ySol - 0.5f) / r;

		float southFactor = std::max(yNrm, 0.0f);
		float eastFactor = std::max(xNrm, 0.0f);
		float northFactor = std::max(-yNrm, 0.0f);
		float westFactor = std::max(-xNrm, 0.0f);

		light = hit.northLight * northFactor;
		light += hit.southLight * southFactor;
		light += hit.eastLight * eastFactor;
		light += hit.westLight * westFactor;

		light.x = std::max(std::min(light.x, 1.0f), 0.0f);
		light.y = std::max(std::min(light.y, 1.0f), 0.0f);
		light.z = std::max(std::min(light.z, 1.0f), 0.0f);
	}

	for (size_t y = drawStart + 1; y <= drawEnd; y++)
	{
		float currentDepth = depthBuffer[y * width + x];
		if (realWallDist < currentDepth)
		{
			int d = y * 256 - (int)height * 128 + lineHeight * 128; // We use these factors to avoid floats
			int texY = ((d * tileWidth) / lineHeight) / 256;
			int texX = angle * tileWidth;

			sf::Color color = getPixelFast(tilesetPixels, texY, texX + hit.texID * tileWidth, tilesetWidth);

			// We don't support overlays

			color = sf::Color(color.r * light.x, color.g * light.y, color.b * light.z);

			setPixelFast(outPixels, x, y, width, color);


			// Set coordinate X
			setPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_G, width, map.x);
			// Set coordinate Y
			setPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_B, width, map.y);

			// Clear alpha as this is the first pass
			sf::Uint8 alpha = getPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_A, width);

			// Set wall flag and side 
			BIT_CLEAR(alpha, 0);
			BIT_SET(alpha, 1);
			BIT_CHANGE(alpha, 6, BIT_CHECK(realSide, 0));
			BIT_CHANGE(alpha, 7, BIT_CHECK(realSide, 1));

			setPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_A, width, alpha);

			sf::Vector2f tCoords;
			tCoords.x = (float)texX / (float)tileWidth;
			tCoords.y = (float)texY / (float)tileWidth;

			setPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_R, width, encodeTexCoords(tCoords));


			depthBuffer[y * width + x] = realWallDist;
		}
	}

	drawFloorAndCeiling(side, x, pos, rayDir, map, wall, perpWallDist, realWallDist, drawEnd, height, width, tilesetPixels, tilesetWidth, skyboxPixels);

	return false;
}

sf::Vector3f maxVector(sf::Vector3f a, sf::Vector3f b)
{
	if (thor::squaredLength(a) > thor::squaredLength(b))
	{
		return a;
	}
	else
	{
		return b;
	}
}

void Map::drawFloorAndCeiling(int& side, size_t& x, sf::Vector2f& pos, sf::Vector2f& rayDir, 
	sf::Vector2i& map, float& wallX, float& perpWallDist, float distShadow, int& drawEnd, int& height, 
	int& width, const sf::Uint8* tilesetPixels, int& tilesetWidth, const sf::Uint8* skyboxPixels)
{
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

	float distWall, currentDist;
	distWall = perpWallDist;

	if (drawEnd < 0)
	{
		drawEnd = height;
	}

	for (int y = drawEnd + 1; y < height; y++)
	{
		float currentDepthFloor = depthBuffer[y * width + x];
		float currentDepthCeiling = depthBuffer[(height - y) * width + x];

		currentDist = (float)height / (2.0f * (float)y - (float)height);
		float weight = (currentDist) / (distWall);
		// Shadows, when used with complex wall shapes, break unless we use
		// the actual, real distance to the wall.
		// But if we were to use that distance for everything then the ground 
		// textures appear offset, so this is the solution
		float weightPlayer = (currentDist) / (distShadow);
		sf::Vector2f currentFloor;
		currentFloor.x = weight * floorWall.x + (1.0f - weight) * pos.x;
		currentFloor.y = weight * floorWall.y + (1.0f - weight) * pos.y;

		sf::Vector2i floorTex;
		floorTex.x = int(currentFloor.x * tileWidth) % (int)tileWidth;
		floorTex.y = int(currentFloor.y * tileWidth) % (int)tileWidth;


		// Add a bit of a shadow
		float shadow = std::abs(1.0f - weightPlayer);

		shadow = std::max(std::min(shadow, 0.8f), 0.0f);
		Tile at = getTile((int)currentFloor.x, (int)currentFloor.y);
		sf::Color color;

		sf::Color reflect;

		if (at.reflectiveFloor || at.reflectiveCeiling)
		{
			reflect = getPixelFast(outPixels, x, drawEnd - (y - drawEnd), width);
		}

		sf::Vector3f light;

		// Special case for closed thin walls (doors)
		if(at.tileType == Tile::THIN)
		{
			sf::Vector3f curSideLight;
			sf::Vector3f oppSideLight;
			float dist;

			if(at.var0 == 0)
			{
				// X-aligned, compare Y
				dist = currentFloor.y - floor(currentFloor.y) - at.var2 / 255.0f;
				if (dist < 0.0f)
				{
					curSideLight = at.northLight;
					oppSideLight = at.southLight;

				}
				else 
				{
					curSideLight = at.southLight;
					oppSideLight = at.northLight;
				}
			}
			else
			{
				dist = currentFloor.x - floor(currentFloor.y) - at.var2 / 255.0f;
				// Y-aligned, compare X
				if (dist < 0.0f)
				{
					curSideLight = at.westLight;
					oppSideLight = at.eastLight;
				
				}
				else
				{
					curSideLight = at.eastLight;
					oppSideLight = at.westLight;
				}
			}

			if (at.var1 == 128)
			{
				light = curSideLight;
			}
			else
			{
				dist = -dist;

				if (dist < 0.0f)
				{
					dist = -dist;
				}

				//dist *= 0.5f;
				dist += 0.5f;

				//dist = 0.5f * dist;
				
				// Interpolate between the two
				
				float rInterp = curSideLight.x * dist + oppSideLight.x * (1.0f - dist);
				float gInterp = curSideLight.y * dist + oppSideLight.y * (1.0f - dist);
				float bInterp = curSideLight.z * dist + oppSideLight.z * (1.0f - dist);
			

				light = sf::Vector3f(rInterp, gInterp, bInterp);

			}
		}
		else if (at.tileType == Tile::COLUMN && at.transparent == false)
		{
			// Maximum one
			light = maxVector(maxVector(at.northLight, at.southLight), maxVector(at.eastLight, at.westLight));
		}
		else
		{
			// We don't do linear interpolation here because it's laggy and doesn't look extremely good
			//light = getLight(currentFloor);
			light = at.light;
		}

		if (currentDist < currentDepthFloor)
		{
			color = getPixelFast(tilesetPixels, floorTex.y, floorTex.x + tileWidth * at.floorID, tilesetWidth);

			if (at.overlaySides.has(Side::FLOOR))
			{
				sf::Color overColor = getPixelFast(at.overlay->getPixelsPtr(), floorTex.y, tileWidth - floorTex.x + 1, at.overlay->getSize().x);
				color = mixColor(color, overColor);
			}

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


			color = sf::Color(color.r * light.x, color.g * light.y, color.b * light.z);

			setPixelFast(outPixels, x, y, width, color);

		}

		if (currentDist < currentDepthCeiling)
		{
			if (at.ceilingID == 0)
			{
				// Skybox
				//color = getPixelFast(skyboxPixels, 0, std::min(height - y, (int)skybox.getSize().y - 1), 1);
				color = sf::Color(140, 232, 246);
			}
			else
			{
				color = getPixelFast(tilesetPixels, floorTex.y, floorTex.x + tileWidth * at.ceilingID, tilesetWidth);

				if (at.overlaySides.has(Side::CEILING))
				{
					sf::Color overColor = getPixelFast(at.overlay->getPixelsPtr(), floorTex.y, floorTex.x, at.overlay->getSize().x);
					color = mixColor(color, overColor);
				}

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

				color = sf::Color(color.r * light.x, color.g * light.y, color.b * light.z);
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

		if (currentDist < currentDepthCeiling)
		{

			// Set coordinate X
			setPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_G, width, (int)currentFloor.x);
			// Set coordinate Y
			setPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_B, width, (int)currentFloor.y);
			// Set flags
			setPixelComponentFast(outBufferPixels, x, y, PIXEL_COMP_A, width, alpha);

		}

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


		if (currentDist < currentDepthFloor)
		{
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
}

void drawThreadFunc(Map* mapPtr, sf::Vector2f direction, sf::Vector2f screenPlane, float viewPlaneDist, int width, int height, sf::Vector2f pos, 
	const sf::Uint8* tilesetPixels, int tilesetWidth, const sf::Uint8* skyboxPixels, size_t startX, size_t endX)
{
	for (size_t x = startX; x < endX; x++)
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

		bool run = true;

		if (mapPtr->getTile(map.x, map.y).tileType == Tile::THIN)
		{
			sf::Vector2f rayOverride = pos - sf::Vector2f(map.x, map.y);
			sf::Vector2f rayDirOverride = rayDir;

			// We need to draw it because you can get inside them
			mapPtr->drawThin(mapPtr->getTile(map.x, map.y), rayOverride, rayDirOverride, step, side, 0, x, pos, rayDir, direction, map, perpWallDist, height, width, tilesetPixels, tilesetWidth, skyboxPixels);
		}



		while (run)
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

			hit = mapPtr->getTile(map.x, map.y);

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

			if (side == 0)
			{
				perpWallDist = ((float)map.x - pos.x + (1 - (float)step.x) / 2.0f) / rayDir.x;
			}
			else
			{
				perpWallDist = ((float)map.y - pos.y + (1 - (float)step.y) / 2.0f) / rayDir.y;
			}

			if (hit.tileType == Tile::WALL)
			{
				run = mapPtr->drawWall(hit, step, side, realSide, x, pos, rayDir, map, perpWallDist, height, width, tilesetPixels, tilesetWidth, skyboxPixels);
			}
			else if (hit.tileType == Tile::THIN)
			{
				run = mapPtr->drawThin(hit, sf::Vector2f(-1.0f, -1.0f), sf::Vector2f(-1.0f, -1.0f), step, side, realSide, x, pos, rayDir, direction, map, perpWallDist, height, width, tilesetPixels, tilesetWidth, skyboxPixels);
			}
			else if (hit.tileType == Tile::COLUMN)
			{
				run = mapPtr->drawColumn(hit, step, side, realSide, x, pos, rayDir, direction, map, perpWallDist, height, width, tilesetPixels, tilesetWidth, skyboxPixels);
			}
		}


	}

	//LOG(INFO) << "Thread done";
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

	if (MAP_THREAD_COUNT == 1)
	{
		drawThreadFunc(this, direction, screenPlane, viewPlaneDist, width, height, pos, tilesetPixels, tilesetWidth, skyboxPixels, 0, width);
	}
	else
	{

		std::vector<std::thread*> threads;

		// Launch all threads
		for (size_t i = 0; i < MAP_THREAD_COUNT; i++)
		{

			size_t startX = i * (width / MAP_THREAD_COUNT);
			size_t endX = std::min((i + 1) * (width / MAP_THREAD_COUNT), (size_t)width);
			//LOG(INFO) << "Thread " << i << " works from " << startX << " to " << endX;
			std::thread* nthread = new std::thread(drawThreadFunc, this, direction, screenPlane, viewPlaneDist, width, height, pos, tilesetPixels, tilesetWidth, skyboxPixels, startX, endX);

			threads.push_back(nthread);
		}

		//LOG(INFO) << "All threads done";

		for (size_t i = 0; i < MAP_THREAD_COUNT; i++)
		{
			threads[i]->join();
			delete threads[i];
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

		sf::Vector3f light = getLight(sprites[i]->pos);
		
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
						color = sf::Color(color.r * light.x, color.g * light.y, color.b * light.z, color.a);

						if (color.a != 0)
						{
							setPixelFast(outPixels, x, y, width, color);

							depthBuffer[y * width + x] = transform.y;

							sf::Uint8 highByte = (sprites[i]->id >> 8) & 0xFF;
							sf::Uint8 lowByte = (sprites[i]->id >> 0) & 0xFF;
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

Tile * Map::getTilePtr(int x, int y)
{
	return &tiles[y * map_width + x];
}

void Map::updateLighting()
{

	

	// Apply skylight
	for (size_t y = 0; y < (size_t)map_height; y++)
	{
		for (size_t x = 0; x < (size_t)map_width; x++)
		{
			Tile* tile = &tiles[y * map_width + x];
			if (tile->ceilingID == 0 && (tile->tileType != Tile::WALL))
			{
				tile->setAllLights(sf::Vector3f(1.0f, 1.0f, 1.0f));
			}
			else
			{
				tile->setAllLights(sf::Vector3f(0.0f, 0.0f, 0.0f));
			}
		}
	}
	
	// Apply lights
	for (size_t i = 0; i < lights.size(); i++)
	{
		if (!(lights[i]->pos.x >= 1.0f && lights[i]->pos.y >= 1.0f && lights[i]->pos.x < (float)map_width && lights[i]->pos.y < (float)map_height))
		{
			continue;
		}

		if (lights[i]->type == Light::POINT)
		{
			// We interpolate in all directions
			// to smooth everything out

			sf::Vector2f subPos = lights[i]->pos - sf::Vector2f(floorf(lights[i]->pos.x), floorf(lights[i]->pos.y));
			int pX = (int)lights[i]->pos.x; int pY = (int)lights[i]->pos.y;

			float att = lights[i]->attenuation;

			float cDistInv = 1.0f;
			float uDistInv = std::min(1.0f / (thor::squaredLength(subPos - sf::Vector2f(0.5f, -0.5f)) * att), 1.0f);
			float rDistInv = std::min(1.0f / (thor::squaredLength(subPos - sf::Vector2f(1.5f, 0.5f)) * att), 1.0f);
			float dDistInv = std::min(1.0f / (thor::squaredLength(subPos - sf::Vector2f(0.5f, 1.5f)) * att), 1.0f);
			float lDistInv = std::min(1.0f / (thor::squaredLength(subPos - sf::Vector2f(-0.5f, 0.5f)) * att), 1.0f);
			float urDistInv = std::min(1.0f / (thor::squaredLength(subPos - sf::Vector2f(1.5f, -0.5f)) * att), 1.0f);
			float ulDistInv = std::min(1.0f / (thor::squaredLength(subPos - sf::Vector2f(-0.5f, -0.5f)) * att), 1.0f);
			float drDistInv = std::min(1.0f / (thor::squaredLength(subPos - sf::Vector2f(1.5f, 1.5f)) * att), 1.0f);
			float dlDistInv = std::min(1.0f / (thor::squaredLength(subPos - sf::Vector2f(-0.5f, 1.5f)) * att), 1.0f);

			tiles[(pY + 0) * map_width + (pX + 0)].setAllLights(lights[i]->light * cDistInv);
			tiles[(pY - 1) * map_width + (pX + 0)].setAllLights(lights[i]->light * uDistInv);
			tiles[(pY + 0) * map_width + (pX + 1)].setAllLights(lights[i]->light * rDistInv);
			tiles[(pY + 1) * map_width + (pX + 0)].setAllLights(lights[i]->light * dDistInv);
			tiles[(pY + 0) * map_width + (pX - 1)].setAllLights(lights[i]->light * lDistInv);
			tiles[(pY - 1) * map_width + (pX + 1)].setAllLights(lights[i]->light * urDistInv);
			tiles[(pY - 1) * map_width + (pX - 1)].setAllLights(lights[i]->light * ulDistInv);
			tiles[(pY + 1) * map_width + (pX + 1)].setAllLights(lights[i]->light * drDistInv);
			tiles[(pY + 1) * map_width + (pX - 1)].setAllLights(lights[i]->light * dlDistInv);
		}
		else if (lights[i]->type == Light::AREA)
		{
			for (float theta = 0; theta < 2.0f * PI; theta += 0.25f)
			{
				// Direction vector
				sf::Vector2f direction;
				direction.x = sin(theta);
				direction.y = cos(theta);

				sf::Vector2f rayDir = direction;

				sf::Vector2i map = sf::Vector2i((int)lights[i]->pos.x, (int)lights[i]->pos.y);

				sf::Vector2f sideDist;

				sf::Vector2f deltaDist;
				deltaDist.x = std::abs(1.0f / rayDir.x);
				deltaDist.y = std::abs(1.0f / rayDir.y);

				sf::Vector2i step;

				Tile* hit = getTilePtr(map.x, map.y);
				int side = 0; //< Side of the hit

				// Set the origin light
				hit->setAllLights(lights[i]->light * 1.0f);

				// Calculate step and sideDist
				if (rayDir.x < 0)
				{
					step.x = -1;
					sideDist.x = (lights[i]->pos.x - map.x) * deltaDist.x;
				}
				else
				{
					step.x = 1;
					sideDist.x = (map.x + 1.0f - lights[i]->pos.x) * deltaDist.x;
				}

				if (rayDir.y < 0)
				{
					step.y = -1;
					sideDist.y = (lights[i]->pos.y - map.y) * deltaDist.y;
				}
				else
				{
					step.y = 1;
					sideDist.y = (map.y + 1.0f - lights[i]->pos.y) * deltaDist.y;
				}

				bool run = true;


				while (run)
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

					float dist = thor::length(sf::Vector2f(map.x + 0.5f, map.y + 0.5f) - lights[i]->pos);

					if (map.x >= map_width || map.y >= map_height || map.x < 0 || map.y < 0 || dist > lights[i]->maxDist)
					{
						run = false;
						break;
					}

					float power = (-dist + lights[i]->maxDist) / lights[i]->maxDist;

					hit = getTilePtr(map.x, map.y);

					if (hit->transparent)
					{
						hit->setAllLights(lights[i]->light * power);
					}
					else
					{
						run = false;
					}
				}


			}
		}
	}
	

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
				// or we are not a full block
				if (tile->transparent)
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

					sf::Vector3f starting = tile->light;
					float startingLength = thor::length(starting);
					sf::Vector3f surroundingAverage = sf::Vector3f(0.0f, 0.0f, 0.0f);
					int rcount = 0, gcount = 0, bcount = 0;

					sf::Vector3f uv = sf::Vector3f(u.prevLight.x, u.prevLight.y, u.prevLight.z);
					sf::Vector3f dv = sf::Vector3f(d.prevLight.x, d.prevLight.y, d.prevLight.z);
					sf::Vector3f rv = sf::Vector3f(r.prevLight.x, r.prevLight.y, r.prevLight.z);
					sf::Vector3f lv = sf::Vector3f(l.prevLight.x, l.prevLight.y, l.prevLight.z);
					sf::Vector3f sv = sf::Vector3f(tile->prevLight.x, tile->prevLight.y, tile->prevLight.z);
					
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
					//if (thor::length(surroundingAverage) > startingLength)
					//{
						tile->light = sf::Vector3f(std::min(surroundingAverage.x, 1.0f), std::min(surroundingAverage.y, 1.0f), std::min(surroundingAverage.z, 1.0f));
					//}
					// If we are exposed to sunlight, saturate
					//if (tile->startLight != sf::Vector3f(0.0f, 0.0f, 0.0f))
					//{
					//	tile->light = tile->startLight;
					//}
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

			if (tile->tileType == Tile::THIN && (tile->var2 == 0 || tile->var2 == 255))
			{
				// The tile light is the opposite side of the closed wall
				if (tile->var0 == 0)
				{
					// X-oriented
					if (tile->var2 == 0)
					{
						tile->light = d.light;
					}
					else
					{
						tile->light = u.light;
					}
				}
				else
				{
					// Y-oriented
					if (tile->var2 == 0)
					{
						tile->light = r.light;
					}
					else
					{
						tile->light = l.light;
					}
				}
			}
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

void getLightHelper(Tile tile, sf::Vector3f* additive, float* divider, sf::Vector2f subPos, sf::Vector2f tilePos)
{
	if (tile.transparent)
	{
		sf::Vector2f diff = subPos - tilePos;
		float dist = std::max(thor::squaredLength(diff), 1.0f);
		float inverseDist = std::min(1.0f / dist, 1.0f);
		*additive += tile.light * inverseDist;
		*divider += inverseDist;
	}
}

sf::Vector3f Map::getLight(sf::Vector2f pos)
{

	int pX = (int)pos.x;
	int pY = (int)pos.y;

	sf::Vector2f subPos = pos - sf::Vector2f(floorf(pos.x), floorf(pos.y));

	sf::Vector3f additive = sf::Vector3f(0.0f, 0.0f, 0.0f);
	float divider = 0.0f;

	Tile tile = getTile(pX, pY);

	getLightHelper(tile, &additive, &divider, subPos, sf::Vector2f(0.5f, 0.5f));

	tile = getTile(pX, pY - 1);
	getLightHelper(tile, &additive, &divider, subPos, sf::Vector2f(0.5f, -0.5f));

	tile = getTile(pX, pY + 1);
	getLightHelper(tile, &additive, &divider, subPos, sf::Vector2f(0.5f, 1.5f));

	tile = getTile(pX - 1, pY);
	getLightHelper(tile, &additive, &divider, subPos, sf::Vector2f(-0.5f, 0.5f));

	tile = getTile(pX + 1, pY);
	getLightHelper(tile, &additive, &divider, subPos, sf::Vector2f(1.5f, 0.5f));

	// Diagonal
	/*tile = getTile(pX - 1, pY - 1);
	getLightHelper(tile, &additive, &divider, subPos, sf::Vector2f(-0.5f, -0.5f));

	tile = getTile(pX + 1, pY - 1);
	getLightHelper(tile, &additive, &divider, subPos, sf::Vector2f(1.5f, -0.5f));

	tile = getTile(pX - 1, pY + 1);
	getLightHelper(tile, &additive, &divider, subPos, sf::Vector2f(-0.5f, 1.5f));

	tile = getTile(pX + 1, pY + 1);
	getLightHelper(tile, &additive, &divider, subPos, sf::Vector2f(1.5f, 1.5f));*/

	return additive / divider;

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
				tiles[y * map_width + x].transparent = false;
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
