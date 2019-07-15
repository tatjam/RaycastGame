#pragma once
#include <SFML/Graphics.hpp>


#include <string>
#include <enet/enet.h>


#include "../Map/World.h"
#include "../Packet.h"

#include "../ProgramType.h"
#include "../Map/Entities/Bases/SpriteEntity.h"

#include "PlayerHUD.h"

class Client : public ProgramType
{
public:

	int uiStart;

	PlayerHUD playerHUD;

	ENetPeer* server;
	ENetHost* client;

	thor::ActionMap<ActionType> actionMap;

	sf::RenderWindow* win;

	World world;

	uint32_t controlledEntity;
	// Useful here on the client to save a few ms while rendering
	// "Upcasted" to a SpriteEntity as it's neccesary for the rendering
	SpriteEntity* controlledEntityPtr;

	virtual void mainFunc(std::unordered_map<std::string, std::string> vals) override;

	bool connect(std::string ip, uint16_t port);
	bool download();
	void play();
	void finish();

	Client();
	~Client();

	virtual int getUIStart()
	{
		return uiStart;
	}


	// Inherited via ProgramType
	virtual World* getWorld() override
	{
		return &world;
	}

	virtual ENetHost* getHost() override
	{
		return client;
	}

	virtual ENetPeer* getServer() override
	{
		return server;
	}

	virtual ConnectedClient* getClient(size_t index) override
	{
		LOG(ERROR) << "Tried to get a client on a client";
		return NULL;
	}

	virtual std::vector<ConnectedClient>* getClients() override
	{
		LOG(ERROR) << "Tried to get clients on a client";
		return NULL;
	}

	virtual bool isServer() override
	{
		return false;
	}

	virtual uint32_t getControlledUID() override
	{
		return controlledEntity;
	}

	virtual sf::RenderWindow* getWindow() override
	{
		return win;
	}

	virtual thor::ActionMap<ActionType>* getActionMap()
	{
		return &actionMap;
	}
};

