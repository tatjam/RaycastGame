#include "Packet.h"



Packet::Packet()
{
}

Packet::Packet(ENetPacket* packet)
{
	for (size_t i = 0; i < packet->dataLength; i++)
	{
		pushByte(packet->data[i]);
	}
}


Packet::~Packet()
{
}

void Packet::pushByte(uint8_t data)
{
	bytes.push_back(data);
}

uint8_t Packet::popByte()
{
	uint8_t out = bytes[bytes.size() - 1];
	bytes.pop_back();
	return out;
}

void Packet::pushUID(uint32_t data)
{
	uint8_t a, b, c, d;
	a = (data >> 0 ) & 0xFF;
	b = (data >> 8 ) & 0xFF;
	c = (data >> 16) & 0xFF;
	d = (data >> 24) & 0xFF;
	pushByte(a);
	pushByte(b);
	pushByte(c);
	pushByte(d);
}

uint32_t Packet::popUID()
{
	uint32_t out  ;

	uint32_t d = (uint32_t)popByte();
	uint32_t c = (uint32_t)popByte();
	uint32_t b = (uint32_t)popByte();
	uint32_t a = (uint32_t)popByte();

	out = (a << 0) | (b << 8) | (c << 16) | (d << 24);

	return out;
}

void Packet::pushFloat(float data)
{
	unsigned char const * p = reinterpret_cast<unsigned char const *>(&data);

	for (std::size_t i = 0; i != sizeof(float); ++i)
	{
		pushByte(p[i]);
	}
}

float Packet::popFloat()
{
	uint32_t asInt = popUID();
	return *reinterpret_cast<float*>(&asInt);
}

void Packet::pushString(std::string data)
{
	// First push all characters
	for (size_t i = 0; i < data.size(); i++)
	{
		pushByte(data[i]);
	}

	// Then push size
	pushUID(data.size());
}

std::string Packet::popString()
{
	uint32_t toRead = popUID();
	std::string out; out.resize(toRead);

	// We need to reverse the string so we do it in one go!
	for (size_t i = 0; i < toRead; i++)
	{
		out[toRead - i - 1] = popByte();
	}

	return out;
}

void Packet::pushJson(json data)
{
	std::vector<uint8_t> bytes = json::to_msgpack(data);
	for (size_t i = 0; i < bytes.size(); i++)
	{
		pushByte(bytes[i]);
	}

	// Size
	pushUID(bytes.size());
}

json Packet::popJson()
{
	uint32_t size = popUID();

	std::vector<uint8_t> data; data.resize(size);
	for (size_t i = 0; i < size; i++)
	{
		data[size - i - 1] = popByte();
	}

	json out = json::from_msgpack(data);

	return out;
}

uint8_t Packet::getType()
{
	return bytes[bytes.size() - 1];
}

void* Packet::getBytes()
{
	return bytes.data();
}

size_t Packet::getSize()
{
	return bytes.size();
}

void Packet::send(ENetPeer* peer, bool reliable)
{
	ENetPacket* epacket;
	if (reliable)
	{
		epacket = enet_packet_create(getBytes(), getSize(), ENET_PACKET_FLAG_RELIABLE);
	}
	else
	{
		epacket = enet_packet_create(getBytes(), getSize(), 0);
	}

	enet_peer_send(peer, 0, epacket);
}
