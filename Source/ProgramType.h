#pragma once
#include <Thor/Resources.hpp>
#include <Thor/Input.hpp>

#include "Map/World.h"

// Arguments in PUSH order

#define MSG_COMMAND 'C'
// Can be received both by server and client
// byte commandID
// UID emiiterUID

#define MSG_CONTROL 'E'
// Can only be received by clients
// UID uid

#define MSG_START 'S'
// Can only be received by clients
// None

#define MSG_FINISH_DOWNLOAD 'F'
// Can only be received by clients
// None

#define MSG_WORLD 'W'
// Can only be received by clients
// JSON data

#define MSG_NEW_ENTITY 'N'
// Can only be received by clients
// byte entityType
// UID uid

struct ConnectedClient
{
	ENetPeer* peer;
	bool needsDownload;
	uint32_t entityControlled;
};

enum ActionType
{
	ON_LEFT_DOWN,
	ON_LEFT_HOLD,
	ON_LEFT_UP,
	ON_RIGHT_DOWN,
	ON_RIGHT_HOLD,
	ON_RIGHT_UP
};


class ProgramType
{
private:

	thor::ResourceHolder<sf::Image, std::string> images;
	thor::ResourceHolder<sf::Font, std::string> fonts;

public:

	// The pixel at which the UI starts
	virtual int getUIStart() = 0;

	virtual World* getWorld() = 0;
	
	virtual thor::ActionMap<ActionType>* getActionMap() = 0;

	virtual ENetHost* getHost() = 0;

	virtual ENetPeer* getServer() = 0;

	virtual ConnectedClient* getClient(size_t index) = 0;

	virtual std::vector<ConnectedClient>* getClients() = 0;

	virtual uint32_t getControlledUID() = 0;

	virtual bool isServer() = 0;
	bool isClient()
	{
		return !isServer();
	}

	virtual void mainFunc(int argc, char** argv) = 0;

	virtual sf::RenderWindow* getWindow() = 0;

	thor::ResourceHolder<sf::Image, std::string>& getImages() { return images; }

	thor::ResourceHolder<sf::Font, std::string>& getFonts() { return fonts; }

	// Loads resources from the Assets folder, giving ids by filesystem location relative
	// to Assets
	// Ex: ./Assets/items/flashlight/hand.png becomes items/flashlight/hand.png
	void loadAllResources(const std::string& path = "./Assets/");

	ProgramType()
	{
		loadAllResources();
	}

};