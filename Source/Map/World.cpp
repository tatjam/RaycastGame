#include "World.h"
#include "Entity.h"


void World::handleCommand(Packet packet, ENetPeer* peer)
{
	// Command, send to target entity
	uint32_t target = packet.popUID();

	for (size_t i = 0; i < entities.size(); i++)
	{
		if (entities[i]->uid == target)
		{
			entities[i]->receivePacket(packet, peer);
		}
	}
}

json World::serialize()
{
	json root;

	root["map"] = map->serialize();

	root["uid"] = uid;

	// Client entities are not stored, we don't need to
	// store the clientUID
	for (size_t i = 0; i < entities.size(); i++)
	{
		if (!entities[i]->isClientOnly())
		{
			json ent = entities[i]->serialize();
			root["entities"].push_back(ent);
		}
	}

	return root;
}

void World::deserialize(json data, ProgramType* prog)
{
	if (map != NULL)
	{
		delete map;
		map = NULL;
	}

	if (map == NULL)
	{
		map = new Map(0, 0);
		map->deserialize(data["map"]);
	}

	uid = data["uid"];

	// Load entities
	entities.clear();
	json ents = data["entities"];
	for (json ent : ents)
	{
		Entity* entity = Entity::createFromType(ent["type"], prog, ent["uid"]);
		entity->deserialize(ent);
		entities.push_back(entity);
	}
}

void World::start()
{
	LOG(INFO) << "Starting world";
	started = true;

	for (size_t i = 0; i < entities.size(); i++)
	{
		entities[i]->start();
	}



}

void World::update(float dt)
{
	bool light = false;

	lightingTimer -= dt;
	if (lightingTimer < 0.0f)
	{
		lightingTimer = lightingInterval;
		light = true;
	}

	map->update(dt, light);

	for (size_t i = 0; i < entities.size(); i++)
	{
		entities[i]->update(dt);
	}


}

Entity* World::createGlobalEntity(uint8_t type, ProgramType* prog)
{
	LOG(INFO) << "Creating global entity";

	if (prog->isClient())
	{
		LOG(ERROR) << "Cannot create a global entity on a client";
	}
	else
	{
		Entity* nEntity = Entity::createFromType((Entity::EntityType)type, prog, getUID());
		entities.push_back(nEntity);

		// Send the message
		Packet pak; 
		pak.pushByte(type);
		pak.pushUID(nEntity->uid);
		pak.pushByte(MSG_NEW_ENTITY);

		auto clients = prog->getClients();
		for (size_t i = 0; i < clients->size(); i++)
		{
			pak.send(clients->at(i).peer);
		}

		if (started)
		{
			nEntity->start();
		}

		return nEntity;
	}
}

Entity* World::createClientEntity(uint8_t type, ProgramType* prog)
{
	LOG(INFO) << "Creating local client entity";

	if (prog->isServer())
	{
		LOG(ERROR) << "Cannot create a client entity on a server";
	}
	else
	{
		Entity* nEntity = Entity::createFromType((Entity::EntityType)type, prog, getClientUID());
		entities.push_back(nEntity);

		if (started)
		{
			nEntity->start();
		}

		return nEntity;
	}
}

Entity* World::receiveNewEntity(Packet pak, ProgramType* prog)
{

	uint32_t nuid = pak.popUID();
	Entity::EntityType type = (Entity::EntityType)pak.popByte();
	Entity* nEntity = Entity::createFromType(type, prog, nuid);

	LOG(INFO) << "Received new entity, UID: " << nuid << ", TYPE: " << type;

	for (size_t i = 0; i < entities.size(); i++)
	{
		if (entities[i]->uid == nuid)
		{
			if (entities[i]->getEntityType() != type)
			{
				// This should never happen
				LOG(ERROR) << "Desynchronization. Received an entity which already existed on client, but types were different";
				return entities[i];
			}

			// This can happen during world download
			LOG(INFO) << "Entity already existed on client, no need to create it";
			return entities[i];
		}
	}

	if (started)
	{
		nEntity->start();
	}

	entities.push_back(nEntity);

	// It's nice to keep the UID pointer working
	if (nuid > uid)
	{
		uid = nuid;
	}
}

Entity* World::findEntity(uint32_t uid)
{
	if (uid == 0)
	{
		return NULL;
	}

	for (size_t i = 0; i < entities.size(); i++)
	{
		if (entities[i]->uid == uid)
		{
			return entities[i];
		}
	}

	return NULL;
}

Sprite* World::findSprite(uint32_t sprite_id)
{
	for (size_t i = 0; i < map->sprites.size(); i++)
	{
		if (map->sprites[i]->id == sprite_id)
		{
			return map->sprites[i];
		}
	}

	return NULL;
}

World::World()
{
	uid = 0;
	clientUID = UINT32_MAX;
	map = NULL;
	started = false;
	lightingInterval = 0.05f;
	lightingTimer = lightingInterval;
}


World::~World()
{
}
