#include "Launcher.h"
#include <TGUI/TGUI.hpp>
#include "../Utils.h"


#define LAUNCHER_WIDTH 490
#define LAUNCHER_HEIGHT 300

#define EDGE_MARGIN 10



void onTabSelected(tgui::Gui& gui, std::string selectedTab)
{
	// Show the correct panel
	if (selectedTab == "Play")
	{
		gui.get("PlayPanel")->setVisible(true);
		gui.get("HostPanel")->setVisible(false);
		gui.get("ConfigurePanel")->setVisible(false);
	}
	else if (selectedTab == "Host")
	{
		gui.get("PlayPanel")->setVisible(false);
		gui.get("HostPanel")->setVisible(true);
		gui.get("ConfigurePanel")->setVisible(false);
	}
	else if (selectedTab == "Configure")
	{
		gui.get("PlayPanel")->setVisible(false);
		gui.get("HostPanel")->setVisible(false);
		gui.get("ConfigurePanel")->setVisible(true);
	}
}

void loadPlayWidgets(tgui::Gui& gui, tgui::Panel::Ptr panel, tgui::Tabs::Ptr tabs, Launcher* launcher)
{

	auto usernameLabel = tgui::Label::create("Username: ");
	auto passwordLabel = tgui::Label::create("Password: ");


	auto username = tgui::EditBox::create();
	username->setSize(288, 25);
	auto password = tgui::EditBox::create();
	password->setSize(288, 25);

	username->setPosition(EDGE_MARGIN + usernameLabel->getSize().x + EDGE_MARGIN / 2, panel->getFullSize().y - tabs->getFullSize().y - EDGE_MARGIN - username->getSize().y - password->getSize().y - EDGE_MARGIN);
	password->setPosition(EDGE_MARGIN + usernameLabel->getSize().x + EDGE_MARGIN / 2, panel->getFullSize().y - tabs->getFullSize().y - EDGE_MARGIN - password->getSize().y);

	usernameLabel->setPosition(EDGE_MARGIN, username->getPosition().y + EDGE_MARGIN / 2);
	passwordLabel->setPosition(EDGE_MARGIN, password->getPosition().y + EDGE_MARGIN / 2);



	auto serverList = tgui::ListBox::create();
	serverList->setSize(250, 130);
	serverList->setPosition(EDGE_MARGIN, EDGE_MARGIN);

	serverList->connect("ItemSelected", [launcher, &gui]()
	{
		// Select server
		auto list = gui.get<tgui::ListBox>("serverList");
		int id = list->getSelectedItemIndex();

		for (size_t i = 0; i < launcher->servers.size(); i++)
		{
			if (i == id)
			{
				launcher->selected = launcher->servers[i];
				gui.get<tgui::EditBox>("serverIP")->setText(launcher->servers[i]->address + ":" + std::to_string(launcher->servers[i]->port));
				launcher->updateServerInfo();
				break;
			}
		}

	});

	auto serverIP = tgui::EditBox::create();
	serverIP->setSize(serverList->getSize().x, 25);
	serverIP->setPosition(serverList->getPosition().x, serverList->getPosition().y + serverList->getSize().y + EDGE_MARGIN / 2);

	auto removeServer = tgui::Button::create("Remove Server");
	removeServer->setSize(serverList->getSize().x / 2 - EDGE_MARGIN / 4, 25);
	removeServer->setPosition(serverList->getPosition().x, serverIP->getPosition().y + serverIP->getSize().y + EDGE_MARGIN / 2);

	removeServer->connect("pressed", [launcher, &gui]()
	{
		auto editBox = gui.get<tgui::EditBox>("serverIP");
		std::string str = editBox->getText();
		rtrim(str);

		int index = -1;

		for (int i = 0; i < launcher->servers.size(); i++)
		{
			if (launcher->servers[i]->address == getAddress(str) && launcher->servers[i]->port == getPort(str))
			{
				index = i;
				break;
			}
		}

		if (index >= 0)
		{
			launcher->mtx.lock();

			launcher->servers.erase(launcher->servers.begin() + index);

			launcher->mtx.unlock();

			launcher->updateServers();
		}
	});

	auto addServer = tgui::Button::create("Add Server");
	addServer->setSize(removeServer->getSize());
	addServer->setPosition(removeServer->getPosition().x + removeServer->getSize().x + EDGE_MARGIN / 2, removeServer->getPosition().y);


	addServer->connect("pressed", [launcher, &gui]()
	{
		// AddServer code
		auto editBox = gui.get<tgui::EditBox>("serverIP");
		std::string str = editBox->getText();
		rtrim(str);

		bool found = false;


		for (size_t i = 0; i < launcher->servers.size(); i++)
		{
			auto serv = launcher->servers[i];
			if (serv->address == getAddress(str) && serv->port == getPort(str))
			{
				found = true;
				gui.get<tgui::ListBox>("serverList")->setSelectedItemByIndex(i);
				break;
			}
		}

		if (!found)
		{

			auto dat = std::make_shared<ServerData>();
			dat->address = getAddress(str);
			dat->port = getPort(str);

			launcher->mtx.lock();

			launcher->servers.push_back(dat);

			launcher->mtx.unlock();

			launcher->updateServers();

			gui.get<tgui::ListBox>("serverList")->setSelectedItemByIndex(launcher->servers.size() - 1);

		}
	});

	auto launchClient = tgui::Button::create("Connect");
	launchClient->setSize(90, 60);
	launchClient->setPosition(panel->getSize().x - EDGE_MARGIN - launchClient->getSize().x, panel->getFullSize().y - tabs->getFullSize().y - EDGE_MARGIN - launchClient->getSize().y);

	launchClient->connect("pressed", [launcher, panel]()
	{
		std::string address = panel->get<tgui::EditBox>("serverIP")->getText();
		launcher->launchGameClient(address);
	});


	auto serverInfoGroup = tgui::Panel::create();
	serverInfoGroup->setSize(panel->getSize().x - serverList->getSize().x - 3 * EDGE_MARGIN, serverList->getSize().y + serverIP->getSize().y + addServer->getSize().y + EDGE_MARGIN);
	serverInfoGroup->setPosition(serverList->getPosition().x + serverList->getSize().x + EDGE_MARGIN, EDGE_MARGIN);
	//serverInfoGroup->getRenderer()->setBorderColor(tgui::Color(0, 0, 0));
	serverInfoGroup->getRenderer()->setBorders(tgui::Borders(1, 1, 1, 1));
	
	auto serverName = tgui::Label::create();
	serverName->setPosition(EDGE_MARGIN / 2, EDGE_MARGIN / 4);
	serverName->setText("No server selected");
	serverName->getRenderer()->setTextStyle(sf::Text::Bold);

	auto serverMapLabel = tgui::Label::create();
	serverMapLabel->setPosition(EDGE_MARGIN / 2, serverName->getSize().y + serverName->getPosition().y);
	serverMapLabel->setText("- Map: ");
	serverMapLabel->getRenderer()->setTextStyle(sf::Text::Bold);

	auto serverMap = tgui::Label::create();
	serverMap->setPosition(serverMapLabel->getPosition().x + serverMapLabel->getSize().x, serverMapLabel->getPosition().y);
	serverMap->setText("No server selected");

	auto serverPlayersLabel = tgui::Label::create();
	serverPlayersLabel->setPosition(EDGE_MARGIN / 2, serverMapLabel->getPosition().y + serverMapLabel->getSize().y);
	serverPlayersLabel->setText("- Players: ");
	serverPlayersLabel->getRenderer()->setTextStyle(sf::Text::Bold);

	auto serverPlayers = tgui::Label::create();
	serverPlayers->setPosition(serverPlayersLabel->getPosition().x + serverPlayersLabel->getSize().x, serverPlayersLabel->getPosition().y);
	serverPlayers->setText("100 / 100");

	auto serverGameModeLabel = tgui::Label::create();
	serverGameModeLabel->setPosition(EDGE_MARGIN / 2, serverPlayersLabel->getPosition().y + serverPlayersLabel->getSize().y);
	serverGameModeLabel->setText("- Mode: ");
	serverGameModeLabel->getRenderer()->setTextStyle(sf::Text::Bold);

	auto serverGameMode = tgui::Label::create();
	serverGameMode->setPosition(serverGameModeLabel->getPosition().x + serverGameModeLabel->getSize().x, serverGameModeLabel->getPosition().y);
	serverGameMode->setText("Survival");

	auto serverPingLabel = tgui::Label::create();
	serverPingLabel->setPosition(EDGE_MARGIN / 2, serverGameModeLabel->getPosition().y + serverGameModeLabel->getSize().y);
	serverPingLabel->setText("- Ping: ");
	serverPingLabel->getRenderer()->setTextStyle(sf::Text::Bold);

	auto serverPing = tgui::Label::create();
	serverPing->setPosition(serverPingLabel->getPosition().x + serverPingLabel->getSize().x, serverPingLabel->getPosition().y);
	serverPing->setText("100ms");

	auto motd = tgui::Label::create();
	motd->setPosition(-1, serverPingLabel->getPosition().y + serverPingLabel->getSize().y);
	motd->setSize(serverInfoGroup->getSize().x, serverInfoGroup->getSize().y - serverPingLabel->getPosition().y - serverPingLabel->getSize().y);
	motd->setText("Message of the day!\nWelcome to our very beautiful server\n\n\n\n\nText far down below");
	motd->getRenderer()->setBorderColor(sf::Color(89, 89, 95));
	motd->getRenderer()->setBorders({ 1, 1, 1, 1 });
	//motd->getRenderer()->setBackgroundColor(sf::Color(49, 49, 53));
	//motd->getRenderer()->setScrollbarWidth(8.0f);
	motd->setScrollbarPolicy(tgui::Scrollbar::Policy::Always);

	serverInfoGroup->add(serverName, "serverName");
	serverInfoGroup->add(serverMapLabel, "serverMapLabel");
	serverInfoGroup->add(serverMap, "serverMap");
	serverInfoGroup->add(serverPlayersLabel, "serverPlayersLabel");
	serverInfoGroup->add(serverPlayers, "serverPlayers");
	serverInfoGroup->add(serverGameModeLabel, "serverGameModeLabel");
	serverInfoGroup->add(serverGameMode, "serverGameMode");
	serverInfoGroup->add(serverPingLabel, "serverPingLabel");
	serverInfoGroup->add(serverPing, "serverPing");
	serverInfoGroup->add(motd, "motd");

	panel->add(launchClient);
	panel->add(usernameLabel);
	panel->add(passwordLabel);
	panel->add(username);
	panel->add(password);
	panel->add(serverList, "serverList");
	panel->add(serverIP, "serverIP");
	panel->add(addServer);
	panel->add(removeServer);
	panel->add(serverInfoGroup, "serverInfoGroup");

}

void loadHostWidgets(tgui::Gui& gui, tgui::Panel::Ptr panel)
{
	auto mapToUse = tgui::TextBox::create();

	auto launchServer = tgui::Button::create("Launch Server");
	launchServer->setPosition(panel->getSize().x - launchServer->getSize().x, "80%");
	panel->add(launchServer);
}

void loadConfigureWidgets(tgui::Gui& gui, tgui::Panel::Ptr panel)
{

}


void Launcher::loadWidgets(tgui::Gui& gui)
{
	auto tabs = tgui::Tabs::create();
	tabs->add("Play");
	tabs->add("Host");
	tabs->add("Configure");
	tabs->setPosition(0, 0);
	gui.add(tabs);

	tgui::Panel::Ptr panel1 = tgui::Panel::create();
	panel1->setSize(LAUNCHER_WIDTH, LAUNCHER_HEIGHT);
	panel1->setPosition(tabs->getPosition().x, tabs->getPosition().y + tabs->getFullSize().y);
	gui.add(panel1, "PlayPanel");

	// Create the second panel (by copying of first one)
	tgui::Panel::Ptr panel2 = tgui::Panel::copy(panel1);
	gui.add(panel2, "HostPanel");

	tgui::Panel::Ptr panel3 = tgui::Panel::copy(panel1);
	gui.add(panel3, "ConfigurePanel");

	tabs->connect("TabSelected", onTabSelected, std::ref(gui));

	loadPlayWidgets(gui, panel1, tabs, this);
	loadHostWidgets(gui, panel2);
	loadConfigureWidgets(gui, panel3);

	tabs->select("Play");
	panel2->setVisible(false);
	panel3->setVisible(false);
}

void checkerThread(Launcher* launcher)
{
	int i = 0;

	while (launcher->runThread)
	{
		std::shared_ptr<ServerData> toWork = NULL;

		launcher->mtx.lock();

		if (i < launcher->servers.size())
		{
			if ((launcher->servers[i]->firstGo == true || launcher->servers[i]->infoClock.getElapsedTime().asSeconds() >= 5.0f) && launcher->servers[i]->inList)
			{
				toWork = launcher->servers[i];
				launcher->servers[i]->firstGo = false;
			}
		}

		i++;

		if (i >= launcher->servers.size())
		{
			i = 0;
		}

		launcher->mtx.unlock();

		if (toWork == NULL)
		{
			sf::sleep(sf::seconds(1.0f));
		}
		else
		{
			// Connect to the server and ask for server info,
			// then close connection and write to the serverdata
			ENetAddress address;
			enet_address_set_host(&address, toWork->address.c_str());
			address.port = toWork->port;

			ENetHost* client = enet_host_create(NULL, 1, 1, 0, 0);
			ENetPeer* server = enet_host_connect(client, &address, 2, 0);

			ENetEvent event;

			bool could = false;

			json inData;

			if (enet_host_service(client, &event, 1000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT)
			{
				// Send want data packet
				Packet pak;
				pak.pushByte(MSG_GET_DATA);
				pak.send(event.peer);

				if (enet_host_service(client, &event, 1000) > 0 && event.type == ENET_EVENT_TYPE_RECEIVE)
				{
					Packet received = Packet(event.packet);
					inData = received.popJson();
					could = true;
				}

				enet_peer_disconnect_now(server, 0);				
			}

			launcher->mtx.lock();

			if (could)
			{
				toWork->hasInfo = true;
				toWork->infoClock.restart();
				toWork->info = ServerInfo();
				toWork->info.deserialize(inData);
			}
			else
			{
				toWork->hasInfo = false;
			}


			launcher->mtx.unlock();
			launcher->updateServers();

		}

	}
}

void Launcher::launcherMain(int argc, char** argv)
{

	win = new sf::RenderWindow(sf::VideoMode(LAUNCHER_WIDTH, LAUNCHER_HEIGHT), "Raycaster Launcher");
	//win->setFramerateLimit(30);
	gui = new tgui::Gui(*win);

	std::string serversFile;

	serversFile = loadFile("Launcher/servers.json");

	if (serversFile == "")
	{
		serversFile = "{}";
	}

	serversJ = json::parse(serversFile);

	tgui::Theme theme{ "Assets/ui/Black.txt" };
	tgui::Theme::setDefault(&theme);

	loadWidgets(*gui);


	loadServersJson();
	updateServers();



	runThread = true;
	std::thread checker = std::thread(checkerThread, this);

	while (win->isOpen())
	{
		sf::Event ev;
		while (win->pollEvent(ev))
		{
			if (ev.type == sf::Event::Closed)
			{
				win->close();
			}

			gui->handleEvent(ev);
		}

		win->clear(sf::Color(37, 37, 37));

		try
		{
			gui->draw();
		}
		catch(...)
		{
			LOG(ERROR) << "Error drawing GUI";
		}

		win->display();
	}

	runThread = false;
	checker.join();
}

void Launcher::updateServers()
{
	mtx.lock();

	auto listBox = gui->get<tgui::ListBox>("serverList");
	int oldIndex = listBox->getSelectedItemIndex();

	bool sizeChange = listBox->getItemCount() != servers.size();

	if (sizeChange)
	{
		listBox->removeAllItems();
		
		for (size_t i = 0; i < servers.size(); i++)
		{
			listBox->addItem("Dummy");
		}
	}

	for (size_t i = 0; i < servers.size(); i++)
	{
		std::string compound = "";

		if (servers[i]->hasInfo)
		{
			compound += servers[i]->info.name;
		}
		else
		{
			compound += servers[i]->address + ":" + std::to_string(servers[i]->port);
		}

		listBox->changeItemByIndex(i, compound);
	}

	if (oldIndex >= servers.size())
	{
		oldIndex = servers.size() - 1;
	}

	if (oldIndex >= 0 && !gui->get<tgui::EditBox>("serverIP")->isFocused())
	{
		listBox->setSelectedItemByIndex(oldIndex);
	}
	else
	{
		listBox->setFocused(false);
	}

	mtx.unlock();


	updateServersJson();
	updateServerInfo();
}

void Launcher::loadServersJson()
{
	mtx.lock();

	for (auto j : serversJ["servers"])
	{
		std::shared_ptr<ServerData> nData = std::make_shared<ServerData>();
		nData->address = j["address"].get<std::string>();
		nData->port = j["port"];
		servers.push_back(nData);
	}

	mtx.unlock();

	updateServers();
}

void Launcher::updateServersJson()
{
	mtx.lock();

	serversJ.clear();

	for (size_t i = 0; i < servers.size(); i++)
	{
		json j;

		j["address"] = servers[i]->address;
		j["port"] = servers[i]->port;

		serversJ["servers"].push_back(j);
	}

	std::string asString = serversJ.dump();

	writeFile("Launcher/servers.json", asString);

	mtx.unlock();
}

void Launcher::updateServerInfo()
{
	if (selected)
	{
		auto panel = gui->get<tgui::Panel>("serverInfoGroup");

		auto serverName = panel->get<tgui::Label>("serverName");
		auto serverMap = panel->get<tgui::Label>("serverMap");
		auto serverPlayers = panel->get<tgui::Label>("serverPlayers");
		auto serverGameMode = panel->get<tgui::Label>("serverGameMode");
		auto motd = panel->get<tgui::Label>("motd");


		if (selected->hasInfo)
		{
			serverName->setText(selected->info.name);
			serverMap->setText(selected->info.mapName);
			serverPlayers->setText(std::to_string(selected->info.connectedPlayers) + "/" + std::to_string(selected->info.maxPlayers));
			serverGameMode->setText(selected->info.gameMode);
			motd->setText(selected->info.motd);
		}
		else
		{
			serverName->setText("Trying to get server info");
			serverMap->setText("");
			serverPlayers->setText("");
			serverGameMode->setText("");
			motd->setText("Welcome to my server!\nRules:\n- Rule 1\n- Rule 2\n- Rule 3\n\n\n\n\n- Far below");
		}
	}
	
}

void Launcher::launchGameClient(std::string address)
{
	Client* client = new Client();

	std::unordered_map<std::string, std::string> vals;
	vals["type"] = "client";
	vals["addr"] = address;

	closeLauncher();

	client->mainFunc(vals);

	delete client;

}

void Launcher::closeLauncher()
{
	win->close();
	runThread = false;

}

Launcher::Launcher()
{
}


Launcher::~Launcher()
{
}
