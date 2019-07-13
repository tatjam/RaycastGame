#pragma once

#include <SFML/Graphics.hpp>
#include <enet/enet.h>

#include <Dependency/easylogging++.h>
#include "Server/Server.h"
#include "Client/Client.h"
#include "Launcher/Launcher.h"

#include <string>


INITIALIZE_EASYLOGGINGPP


int main(int argc, char** argv)
{

	el::Configurations defaultConf;

	defaultConf.setToDefault();

	bool server = false;
	if (argc >= 2 && strcmp(argv[1], "server") == 0)
	{
		server = true;
	}	
	
	if (server)
	{
		remove("./server.log");
		defaultConf.parseFromText("*GLOBAL:\n FILENAME = ./server.log\n FORMAT =  [%datetime{%h:%m:%s:%g}] [%level] %msg");
	}
	else
	{
		remove("./client.log");
		defaultConf.parseFromText("*GLOBAL:\n FILENAME = ./client.log\n FORMAT =  [%datetime{%h:%m:%s:%g}] [%level] %msg");
	}


	el::Loggers::reconfigureAllLoggers(defaultConf);
	el::Loggers::addFlag(el::LoggingFlag::ColoredTerminalOutput);

	START_EASYLOGGINGPP(argc, argv);

	LOG(INFO) << "Initializing";


	if (enet_initialize() != 0)
	{
		LOG(ERROR) << "Could not initialize enet";

	}

	ProgramType* prog = NULL;

	if (argc >= 2 && strcmp(argv[1], "server") == 0)
	{
		LOG(INFO) << "Launching server";
		prog = new Server();
		prog->mainFunc(argc, argv);
	}
	else if(argc >= 2 && strcmp(argv[1], "client") == 0)
	{
		LOG(INFO) << "Delaying to allow server to start";
		sf::sleep(sf::seconds(1.5f));
		LOG(INFO) << "Launching client";
		prog = new Client();
		prog->mainFunc(argc, argv);
	}
	else
	{
		// Launcher
		Launcher launcher;
		launcher.launcherMain(argc, argv);

		// The launcher will probably launch an instance of the game
		// so fear not, we are out
	}


	if (prog != NULL)
	{
		delete prog;
	}

	enet_deinitialize();

	return 0;
}