#pragma once

#include <SFML/Graphics.hpp>
#include <enet/enet.h>

#include <Dependency/easylogging++.h>
#include "Server/Server.h"
#include "Client/Client.h"
#include "Editor/Editor.h"
#include "Launcher/Launcher.h"



#include <string>
#include <Dependency/CLI11.hpp>

INITIALIZE_EASYLOGGINGPP


int main(int argc, char** argv)
{
	CLI::App app{ "Raycaster game super-binary (includes server, client, launcher, editor...)" };

	// Add options for the general program
	std::string type;
	app.add_option("-t, --type", type, "Type of program to launch, can be 'server', 'client' or empty for the launcher");
	
	// Options for the client
	std::string addr;
	app.add_option("--address", addr, "Address to connect the client to, if the default port is not used, write it after the address separated by ':'");


	try 
	{
		app.parse(argc, argv);
	}
	catch (const CLI::ParseError &e) 
	{
		return app.exit(e);
	}

	std::unordered_map<std::string, std::string> vals;
	vals["type"] = type;
	vals["addr"] = addr;

	el::Configurations defaultConf;

	defaultConf.setToDefault();
	
	if (type == "server")
	{
		remove("./server.log");
		defaultConf.parseFromText("*GLOBAL:\n FILENAME = ./server.log\n FORMAT =  [%datetime{%h:%m:%s:%g}] [%level] %msg");
	}
	
	if(type == "client")
	{
		remove("./client.log");
		defaultConf.parseFromText("*GLOBAL:\n FILENAME = ./client.log\n FORMAT =  [%datetime{%h:%m:%s:%g}] [%level] %msg");
	}

	if (type == "" || type == "launcher")
	{
		remove("./launcher.log");
		defaultConf.parseFromText("*GLOBAL:\n FILENAME = ./launcher.log\n FORMAT =  [%datetime{%h:%m:%s:%g}] [%level] %msg");
	}

	if (type == "editor")
	{
		remove("./editor.log");
		defaultConf.parseFromText("*GLOBAL:\n FILENAME = ./editor.log\n FORMAT =  [%datetime{%h:%m:%s:%g}] [%level] %msg");
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

	if (type == "server")
	{
		LOG(INFO) << "Launching server";
		prog = new Server();
		prog->mainFunc(vals);
	}
	else if(type == "client")
	{
		LOG(INFO) << "Delaying to allow server to start";
		sf::sleep(sf::seconds(1.5f));
		LOG(INFO) << "Launching client";
		prog = new Client();
		prog->mainFunc(vals);
	}
	else if (type == "editor")
	{
		prog = new Editor();
		prog->mainFunc(vals);
	}
	else
	{
		LOG(INFO) << "Launching launcher";

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