#pragma once

#include "../Packet.h"

#include <vector>
#include <stdint.h>


#include "Map.h"


class Entity;
class ProgramType;

// A World exists in both the server and the client
// but the client has very little authority over it
class World
{
private:
	bool started;

public:

	float lightingInterval;
	float lightingTimer;

	Map* map;

	// Used for synchronized entities
	// Counts upwards from 1 (Minimum ID is 1)
	// Technical limit of up to UINT32_MAX / 2 entities (Will never be reached)
	uint32_t uid;
	// Used for client-only entities (particles and such)
	// Counts downwards from UINT32_MAX - 1
	// Technical limit of up to UINT32_MAX / 2 client entities (Will never be reached)
	uint32_t clientUID;

	std::vector<Entity*> entities;

	// You must pop the 'C' first!
	void handleCommand(Packet packet, ENetPeer* peer);

	uint32_t getUID()
	{
		uid++;
		return uid;
	}

	uint32_t getClientUID()
	{
		clientUID--;
		return clientUID;
	}

	json serialize();
	void deserialize(json data, ProgramType* prog);

	void start();
	void update(float dt);
	
	// Can only be called on the server
	Entity* createGlobalEntity(uint8_t type, ProgramType* prog);

	// Can only be called on the client
	Entity* createClientEntity(uint8_t type, ProgramType* prog);

	Entity* receiveNewEntity(Packet pak, ProgramType* prog);

	Entity* findEntity(uint32_t uid);
	Sprite* findSprite(uint32_t sprite_id);

	// Saves some lines of code, returns NULL if not found or
	// if typecast failed
	template<typename T> 
	T* findEntity(uint32_t uid)
	{
		Entity* asEnt = findEntity(uid);
		return dynamic_cast<T*>(asEnt);
	}

	World();
	~World();
};

