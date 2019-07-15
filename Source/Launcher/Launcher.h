#pragma once
#include <enet/enet.h>
#include "../Packet.h"
#include "../ProgramType.h"
#include <TGUI/TGUI.hpp>
#include <Dependency/json.hpp>
#include <Dependency/easylogging++.h>
#include "../Server/ServerInfo.h"
#include <thread>
#include <mutex>
#include "../Client/Client.h"
#include "../Server/Server.h"

#include <memory>

using namespace nlohmann;

struct ServerData
{
	bool inList;

	std::string address;
	uint16_t port;

	ServerInfo info;
	bool hasInfo;
	bool firstGo;
	sf::Clock infoClock;
	bool couldNotConnect;

	ServerData()
	{
		hasInfo = false;
		address = "";
		port = 0;
		inList = true;
		couldNotConnect = false;
		firstGo = true;
	}
};

// The launcher is a GUI to launch the game in its different modes
// and to connect to servers and change settings visually
class Launcher
{
public:
	
	int argc;
	char** argv;

	bool runThread;

	std::mutex mtx;

	tgui::Gui* gui;
	sf::RenderWindow* win;

	json serversJ;

	std::vector<std::shared_ptr<ServerData>> servers;

	std::vector<ServerData> data;

	std::shared_ptr<ServerData> selected;


	void loadWidgets(tgui::Gui& gui);
	void launcherMain(int argc, char** argv);

	void updateServers();
	void loadServersJson();
	void updateServersJson();
	void updateServerInfo();

	void launchGameClient(std::string address);
	void closeLauncher();

	Launcher();
	~Launcher();
};

