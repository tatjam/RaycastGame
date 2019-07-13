#include "Server.h"
#include "../Map/Entity.h"
#include "../Map/Entities/AllEntities.h"

void Server::mainFunc(int argc, char** argv)
{
	ENetAddress address;
	server = NULL;

	address.host = ENET_HOST_ANY;
	address.port = 1234;

	server = enet_host_create(&address, 32, 1, 0, 0);

	if (server == NULL)
	{
		//LOG(ERROR) << "Could not create server";
		return;
	}

	bool running = true;

	ENetEvent event;

	std::string buffer;

	createDefaultWorld();

	sf::Clock dtc;
	sf::Time dtt;
	float dt = 0.01f;

	// Server only needs sparse lighting updates
	world.lightingInterval = 1.0f;

	while (running)
	{
		enet_host_service(server, &event, 0);

		if (event.type == ENET_EVENT_TYPE_CONNECT)
		{
			LOG(INFO) << "Received new client (may be ping)";
			// Do nothing just yet

		}
		else if (event.type == ENET_EVENT_TYPE_DISCONNECT)
		{
			// Disconnect client
			LOG(INFO) << "Client disconnected";

			int index = -1;

			for (size_t i = 0; i < clients.size(); i++)
			{
				if (clients[i].peer == event.peer)
				{
					index = i;
				}
			}

			if (index >= 0)
			{
				clients.erase(clients.begin() + index);
			}

			if (clients.size() == 0 && playing)
			{
				LOG(INFO) << "All clients disconnected (while ingame), closing";
				break;
			}
		}
		else if (event.type == ENET_EVENT_TYPE_RECEIVE)
		{
			// Receive packet
			Packet pak = Packet(event.packet);
			if (pak.getType() == MSG_COMMAND)
			{
				pak.popByte();
				world.handleCommand(pak, event.peer);
			}
			else if (pak.getType() == MSG_PLAY)
			{
				LOG(INFO) << "Connection requested playing";

				if (playing)
				{
					LOG(INFO) << "Denied, game is in progress"; 
					// TODO: Hot joining
					enet_peer_disconnect(event.peer, 0);
				}
				else
				{

					// Accept new client
					ConnectedClient newClient = ConnectedClient();
					newClient.peer = event.peer;
					newClient.needsDownload = true;

					// Give him an entity
					// We do this here to prevent the client from
					// getting an entity which will be overwritten by the download
					EPlayer* playerEnt = dynamic_cast<EPlayer*>(world.createGlobalEntity(Entity::PLAYER, this));
					newClient.entityControlled = playerEnt->uid;

					ItemEntity* flashlightEnt = dynamic_cast<ItemEntity*>(world.createGlobalEntity(Entity::ITEM_FLASHLIGHT, this));

					playerEnt->setItem({ 3, 0 }, flashlightEnt, true, newClient.peer);

					clients.push_back(newClient);
				}

			}
			else if (pak.getType() == MSG_GET_DATA)
			{
				LOG(INFO) << "Connection requested server data";

				// Send data packet
				Packet out;
				ServerInfo outInfo;

				outInfo.name = "Tatjam's Server";
				outInfo.motd = "Welcome to Tatjam's server!";
				outInfo.maxPlayers = 20;
				outInfo.connectedPlayers = 10;
				outInfo.gameMode = "Survival";
				outInfo.mapName = "Deep Station I";

				out.pushJson(outInfo.serialize());
				out.send(event.peer);
			}
			else if (pak.getType() == MSG_PING)
			{
				Packet out;
				out.pushByte(MSG_PING);
				out.send(event.peer);
			}

			enet_packet_destroy(event.packet);
		}

		// Handle commands
		if (_kbhit())
		{
			char c = _getch();
			if (c == '\n' || c == '\r')
			{
				// Execute command
				LOG(INFO) << "Received command: " << buffer;
				if (buffer == "quit" || buffer == "q")
				{
					running = false;
				}

				buffer = "";
			}
			else if (c != '\0')
			{
				buffer.push_back(c);
			}

			if (c != '\n' && c != '\r' && c != '\0')
			{
				LOG(INFO) << "Input buffer: " << buffer;
			}
		}

		if (playing)
		{
			world.update(dt);

			dtt = dtc.restart();
			dt = dtt.asSeconds();

		}
		else
		{ 
			int count = 0;
			for (size_t i = 0; i < clients.size(); i++)
			{
				if (clients[i].needsDownload)
				{
					downloadTo(&clients[i]);
				}
				else
				{
					count++;
				}
			}

			if (count >= clients.size() && count >= targetPlayers)
			{
				playing = true;
				for (size_t i = 0; i < clients.size(); i++)
				{
					// Send start message
					Packet pak = Packet(); pak.pushByte(MSG_START);
					pak.send(clients[i].peer);

					// Also set their controlled entities here
					// to save some frames once the game is started
					pak = Packet();
					pak.pushUID(clients[i].entityControlled);
					pak.pushByte(MSG_CONTROL);
					pak.send(clients[i].peer);
				}

				world.start();
			}
		}

		
	}

	LOG(INFO) << "Disconnecting all clients";

	for (size_t i = 0; i < clients.size(); i++)
	{
		enet_peer_disconnect(clients[i].peer, 0);
	}

	LOG(INFO) << "Waiting a bit before closing";

	sf::Clock clock;

	while (true)
	{
		// Run this a little bit more to allow clients to disconnect
		enet_host_service(server, &event, 0);

		if (clock.getElapsedTime().asSeconds() >= 1.0f)
		{
			break;
		}
	}

	LOG(INFO) << "Closing";

	enet_host_destroy(server);
}

void Server::createDefaultWorld()
{
	world.map = new Map(16, 16, 1);

	for (size_t y = 0; y < world.map->map_height; y++)
	{
		for (size_t x = 0; x < world.map->map_width; x++)
		{
			world.map->tiles[y * world.map->map_width + x].ceilingID = 3;

			if (x == 5 && y == 5)
			{
				world.map->tiles[y * world.map->map_width + x].texID = 8;
				world.map->tiles[y * world.map->map_width + x].tileType = Tile::THIN;
				world.map->tiles[y * world.map->map_width + x].var2 = 128;
				world.map->tiles[y * world.map->map_width + x].var1 = 128;
				world.map->tiles[y * world.map->map_width + x].var0 = 1;
				world.map->tiles[y * world.map->map_width + x].walkable = false;
				world.map->tiles[y * world.map->map_width + x].transparent = false;
			}

			if ((x == 5 && y == 4) || (x == 5 && y == 6))
			{
				world.map->tiles[y * world.map->map_width + x].texID = 4;
				world.map->tiles[y * world.map->map_width + x].tileType = Tile::WALL;
				world.map->tiles[y * world.map->map_width + x].walkable = false;
				world.map->tiles[y * world.map->map_width + x].transparent = false;
			}

			/*if (x % 6 == 0 && y % 6 == 0)
			{
				world.map->tiles[y * world.map->map_width + x].ceilingID = 0;
				world.map->tiles[y * world.map->map_width + x].floorID = 4;
			}*/


			if (x == 8 && y == 8)
			{
				world.map->tiles[y * world.map->map_width + x].texID = 8;
				world.map->tiles[y * world.map->map_width + x].tileType = Tile::THIN;
				world.map->tiles[y * world.map->map_width + x].var2 = 128;
				world.map->tiles[y * world.map->map_width + x].var1 = 128;
				world.map->tiles[y * world.map->map_width + x].var0 = 0;
				world.map->tiles[y * world.map->map_width + x].walkable = false;
				world.map->tiles[y * world.map->map_width + x].transparent = false;
			}

			if ((x == 9 && y == 8) || (x == 7 && y == 8) || (x == 10 && y == 8) || (x == 6 && y == 8) || (x == 11 && y == 8) || (x == 5 && y == 8) || (x == 12 && y == 8) || (x == 4 && y == 8) || (x == 12 && y == 12))
			{
				world.map->tiles[y * world.map->map_width + x].texID = 4;
				world.map->tiles[y * world.map->map_width + x].tileType = Tile::WALL;
				world.map->tiles[y * world.map->map_width + x].walkable = false;
				world.map->tiles[y * world.map->map_width + x].transparent = false;
			}

			if (x == 8 && y == 4)
			{
				world.map->tiles[y * world.map->map_width + x].texID = 3;
				world.map->tiles[y * world.map->map_width + x].tileType = Tile::COLUMN;
				world.map->tiles[y * world.map->map_width + x].walkable = false;
				world.map->tiles[y * world.map->map_width + x].transparent = false;
				world.map->tiles[y * world.map->map_width + x].var0 = 255;
			}
		}
	}

	ESimpleDoor* door = dynamic_cast<ESimpleDoor*>(world.createGlobalEntity(Entity::DOOR_SIMPLE, this));
	door->setTilePos(5, 5);

	ESimpleDoor* door2 = dynamic_cast<ESimpleDoor*>(world.createGlobalEntity(Entity::DOOR_SIMPLE, this));
	door2->setTilePos(8, 8);
}

void Server::downloadTo(ConnectedClient* target)
{
	Packet worldPak = Packet();
	worldPak.pushJson(world.serialize());
	worldPak.pushByte(MSG_WORLD);
	worldPak.send(target->peer);

	// Finish download, and send the message
	Packet packet = Packet(); packet.pushByte(MSG_FINISH_DOWNLOAD);
	packet.send(target->peer);
	target->needsDownload = false;

	LOG(INFO) << "Finished downloading to client";
}

Server::Server()
{
	playing = false;
	targetPlayers = 1;
}


Server::~Server()
{
}
