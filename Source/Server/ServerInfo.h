#pragma once
#include <string>
#include <Dependency/json.hpp>

using namespace nlohmann;

struct ServerInfo
{
	std::string name;
	std::string motd;
	std::string gameMode;
	std::string mapName;
	int connectedPlayers;
	int maxPlayers;

	float ping;

	json serialize()
	{
		json j;
		j["name"] = name;
		j["motd"] = motd;
		j["gameMode"] = gameMode;
		j["connectedPlayers"] = connectedPlayers;
		j["maxPlayers"] = maxPlayers;
		j["mapName"] = mapName;

		return j;
	}

	void deserialize(json j)
	{
		name = j["name"].get<std::string>();
		motd = j["motd"].get<std::string>();
		mapName = j["mapName"].get<std::string>();
		gameMode = j["gameMode"].get<std::string>();
		connectedPlayers = j["connectedPlayers"];
		maxPlayers = j["maxPlayers"];
	}
};
