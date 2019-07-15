#pragma once

#include "../ProgramType.h"

class Editor : public ProgramType
{
public:

	sf::RenderWindow* win;

	World world;

	thor::ActionMap<ActionType> actionMap;

	Editor();
	~Editor();

	// Inherited via ProgramType, they are pretty much dummy
	// as entities should not do much here
	virtual int getUIStart() override
	{
		return 0;
	}

	virtual World* getWorld() override
	{
		return &world;
	}

	virtual thor::ActionMap<ActionType>* getActionMap() override
	{
		return &actionMap;
	}

	virtual ENetHost* getHost() override
	{
		return NULL;
	}

	virtual ENetPeer* getServer() override
	{
		return NULL;
	}

	virtual ConnectedClient* getClient(size_t index) override
	{
		return NULL;
	}

	virtual std::vector<ConnectedClient>* getClients() override
	{
		return NULL;
	}

	virtual uint32_t getControlledUID() override
	{
		return 0;
	}

	virtual bool isServer() override
	{
		return false;
	}

	std::string chooseMap();

	virtual void mainFunc(std::unordered_map<std::string, std::string> vals) override;
	virtual sf::RenderWindow* getWindow() override
	{
		return NULL;
	}
};

