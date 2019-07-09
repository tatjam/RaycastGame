#include "Entity.h"

#include "../Client/Client.h"
#include "../Server/Server.h"
#include "Entities/EPlayer.h"
#include "Entities/Tile/ESimpleDoor.h"

void Entity::emitCommand(uint8_t command_id, Packet packet, ENetPeer * peer, bool reliable)
{
	packet.pushByte(command_id);
	packet.pushUID(uid); //< UID of the emitter
	packet.pushByte(MSG_COMMAND); // < Packet identifier, C for commands

	packet.send(peer, reliable);
}

void Entity::emitCommandToServer(uint8_t command_id, Packet packet, bool reliable)
{

	if (program->isServer())
	{
		LOG(ERROR) << "Tried to send a command to server being the server";
	}
	else
	{
		emitCommand(command_id, packet, program->getServer(), reliable);
	}
}

void Entity::emitCommandToAll(uint8_t command_id, Packet packet, bool reliable)
{
	if (program->isServer())
	{
		for (size_t i = 0; i < program->getClients()->size(); i++)
		{
			emitCommand(command_id, packet, program->getClients()->at(i).peer, reliable);
		}
		
	}
	else
	{
		LOG(ERROR) << "Tried to send a command to all clients without being the server";
	}
}

void Entity::emitCommandToAllBut(uint8_t command_id, Packet packet, ENetPeer* exception, bool reliable)
{
	if (program->isServer())
	{
		auto clients = program->getClients();
		for (size_t i = 0; i < clients->size(); i++)
		{
			if (clients->at(i).peer->connectID != exception->connectID)
			{
				emitCommand(command_id, packet, clients->at(i).peer, reliable);
			}
		}

	}
	else
	{
		LOG(ERROR) << "Tried to send a command to all clients without being the server";
	}
}

void Entity::onGenericCall(std::string cmd, json args, ENetPeer * peer)
{
}

void Entity::sendGenericCall(ENetPeer* peer, std::string cmd, json args)
{
	Packet pak;
	pak.pushString(cmd);

	if (!args.empty())
	{
		pak.pushJson(args);
	}

	if (args.empty())
	{
		emitCommand(GENERIC_CALL, pak, peer);
	}
	else
	{
		emitCommand(GENERIC_CALL_ARGS, pak, peer);
	}
	
}

void Entity::sendGenericCallToAll(std::string cmd, json args)
{
	if (getProg()->isClient())
	{
		LOG(ERROR) << "Tried to send a generic call to all being a client.";
		return;
	}

	auto clients = getProg()->getClients();
	for (auto client : *clients)
	{
		sendGenericCall(client.peer, cmd, args);
	}
}

void Entity::sendGenericCallToAllBut(ENetPeer* exception, std::string cmd, json args)
{
	if (getProg()->isClient())
	{
		LOG(ERROR) << "Tried to send a generic call to all being a client.";
		return;
	}

	auto clients = getProg()->getClients();
	for (auto client : *clients)
	{
		if (client.peer->connectID != exception->connectID)
		{
			sendGenericCall(client.peer, cmd, args);
		}
	}
}

void Entity::receivePacket(Packet packet, ENetPeer* peer)
{
	uint8_t command_id = packet.popByte();

	receiveCommand(command_id, packet, peer);
}

void Entity::receiveCommand(uint8_t command_id, Packet packet, ENetPeer* peer)
{
	if (command_id == GENERIC_CALL)
	{
		std::string cmd = packet.popString();
		onGenericCall(cmd, json(), peer);
	}
	else if (command_id == GENERIC_CALL_ARGS)
	{
		json j = packet.popJson();
		std::string cmd = packet.popString();
		onGenericCall(cmd, j, peer);
	}
}


Entity::Entity(ProgramType* prog, uint32_t uid)
{
	this->program = prog;
	this->uid = uid;
}



Entity* Entity::createFromType(EntityType type, ProgramType* prog, uint32_t uid)
{
	if (type == UNIMPLEMENTED)
	{
		LOG(ERROR) << "Tried to create an UNIMPLEMENTED entity";
	}
	else if (type == PLAYER)
	{
		return new EPlayer(prog, uid);
	}
	else if (type == DOOR_SIMPLE)
	{
		return new ESimpleDoor(prog, uid);
	}
	else
	{
		LOG(ERROR) << "Attempted to create unimplemented or invalid entity type";
	}

	return NULL;
}


Entity::~Entity()
{
}
