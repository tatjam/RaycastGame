#pragma once
#include <SFML/Graphics.hpp>


#include "../Packet.h"
#include <Dependency/easylogging++.h>

#include <enet/enet.h>
#include "../ProgramType.h"

class Player;

class Entity
{
private:

	ProgramType* program;
	
public:

	// Note: Arguments are given in
	// PUSH order, so POP them backwards
	// GENERIC_CALL(_ARGS) is used to save
	// some of the CommandID space, for example
	// from trivial, once-in-a-while calls from doors
	// and for very complex messages which require
	// many arguments of arbitrary types, or arrays
	enum CommandID
	{
		SET_SPRITE_POSROT,
		// float xPos
		// float yPos
		// float angle

		SET_SPRITE_DETAIL,
		// float scale
		// float vOffset

		SET_INTERPOLATION,
		// float ammount

		GENERIC_CALL,
		// string name

		GENERIC_CALL_ARGS,
		// string name
		// json args
	};

	// You must register here an entity! (For serialization)
	enum EntityType
	{
		UNIMPLEMENTED,	//< These should not exist in any save
		PLAYER,
	};

	uint32_t uid;

	// Sends a command to a peer, or the server (use emitCommandToServer instead)
	void emitCommand(uint8_t command_id, Packet packet, ENetPeer* peer, bool reliable = true);

	// Sends a command to the server, only makes sense on the client
	void emitCommandToServer(uint8_t command_id, Packet packet, bool reliable = true);

	// Broadcasts a command to all connected clients, only makes sense on the server
	void emitCommandToAll(uint8_t command_id, Packet packet, bool reliable = true);

	// Emits a command to all connected clients, except one.
	void emitCommandToAllBut(uint8_t command_id, Packet packet, ENetPeer* exception, bool reliable = true);

	// args can be empty
	virtual void onGenericCall(std::string cmd, json args, ENetPeer* peer);
	// args can be empty
	void sendGenericCall(ENetPeer* peer, std::string cmd, json args = json());

	// Dispatches the command to either the client handler or the server handle
	void receivePacket(Packet packet, ENetPeer* peer);

	virtual void receiveCommand(uint8_t command_id, Packet packet, ENetPeer* peer);

	virtual EntityType getEntityType() { return UNIMPLEMENTED; }

	// Called every frame, on the client frame-rate will be erratic
	// on the server it will probably be more stable
	// dt in seconds
	virtual void update(float dt) {}
	// Called just after the match starts, set up stuff here
	virtual void start() {}
	// Called either when the entity is destroyed or when the match ends
	virtual void finish() {}

	bool isControlled()
	{
		return uid == program->getControlledUID();
	}

	ProgramType* getProg()
	{
		return program;
	}

	World* getWorld()
	{
		return program->getWorld();
	}

	virtual json serialize() 
	{
		json out;
		out["uid"] = uid;
		out["type"] = getEntityType();
		return out;
	}

	// Called only after we have properly initialized the entity
	// to its proper type using createFromType!
	virtual void deserialize(json data) 
	{

	}

	// Is this entity a client only entity
	bool isClientOnly()
	{
		return uid >= UINT32_MAX / 2;
	}

	Entity(ProgramType* progrma, uint32_t uid);
	static Entity* createFromType(EntityType type, ProgramType* server, uint32_t uid);
	~Entity();
};

