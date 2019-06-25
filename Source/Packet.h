#pragma once
#include <vector>
#include <string>
#include <stdint.h>
#include <enet/enet.h>
#include <Dependency/json.hpp>
#include <Dependency/easylogging++.h>

using namespace nlohmann;


class Packet
{
private:
	std::vector<uint8_t> bytes;

public:

	void pushByte(uint8_t data);
	uint8_t popByte();

	void pushUID(uint32_t data);
	uint32_t popUID();

	// TODO: Support endianess?? No idea how to do this
	void pushFloat(float data);
	float popFloat();

	void pushString(std::string data);
	std::string popString();

	void pushJson(json data);
	json popJson();

	// Gets the type of the packet without popping it
	uint8_t getType();

	void* getBytes();
	size_t getSize();

	void send(ENetPeer* peer, bool reliable = true);

	Packet();
	Packet(ENetPacket* packet);
	~Packet();
};


