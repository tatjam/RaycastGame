#pragma once

#include <vector>
#include "../Map/World.h"

#include <enet/enet.h>
#include <conio.h>
#include <Dependency/easylogging++.h>

#include <SFML/System.hpp>

#include "../ProgramType.h"



class Server : public ProgramType
{
public:

	bool playing;
	int targetPlayers;

	ENetHost* server;

	World world;

	std::vector<ConnectedClient> clients;

	virtual int getUIStart()
	{
		LOG(ERROR) << "Tried to getUIStart on the server, which is headless";
		return 0;
	}

	virtual void mainFunc(int argc, char** argv) override;

	virtual bool isServer() override
	{
		return true;
	}

	virtual World* getWorld() override
	{
		return &world;
	}

	void createDefaultWorld();

	void downloadTo(ConnectedClient* target);

	virtual ENetHost* getHost() override
	{
		return server;
	}

	virtual ENetPeer* getServer() override
	{
		LOG(ERROR) << "Tried to getServer on the server";
		return NULL;
	}

	virtual ConnectedClient* getClient(size_t index) override
	{
		return &clients[index];
	}

	virtual std::vector<ConnectedClient>* getClients() override
	{
		return &clients;
	}

	// The Server never controls anything
	virtual uint32_t getControlledUID() override
	{
		return 0;
	}

	virtual sf::RenderWindow* getWindow() override
	{
		LOG(ERROR) << "Tried to get window on the server, which is headless";
		return NULL;
	}

	virtual thor::ActionMap<ActionType>* getActionMap()
	{
		LOG(ERROR) << "Tried to get the action map on the server, which is headless";
		return NULL;
	}

	Server();
	~Server();




};

