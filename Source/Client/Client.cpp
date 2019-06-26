#include "Client.h"




void Client::mainFunc(int argc, char** argv)
{
	std::string ip = "localhost";
	uint16_t port = 1234;

	// Try to get IP to conncet to
	if (argc >= 3)
	{
		std::string as_str = argv[2];
		ip = as_str;
	}

	LOG(INFO) << "Connecting to " << ip << ":" << port;

	client = NULL;
	client = enet_host_create(NULL, 1, 1, 0, 0);

	if (client == NULL)
	{
		LOG(ERROR) << "Could not create ENET client";
		return;
	}

	if (connect(ip, port))
	{
		if (download())
		{
			play();
		}
	}
	else
	{
		return;
	}

	LOG(INFO) << "Disconnecting from server";

	sf::Clock clock;
	
	enet_peer_disconnect(server, 0);

	// Send disconnect message and update for a little bit
	while (true)
	{
		ENetEvent event;
		enet_host_service(client, &event, 0);

		if (clock.getElapsedTime().asSeconds() >= 1.0f)
		{
			break;
		}
	}

	enet_host_destroy(client);
}

bool Client::connect(std::string ip, uint16_t port)
{

	// TODO: Allow user to input
	ENetAddress address;
	enet_address_set_host(&address, ip.c_str());
	address.port = port;

	server = enet_host_connect(client, &address, 2, 0);

	ENetEvent event;

	if (enet_host_service(client, &event, 5000) > 0 &&
		event.type == ENET_EVENT_TYPE_CONNECT)
	{
		LOG(INFO) << "Connected successfully";
		return true;
	}
	else
	{
		LOG(ERROR) << "Could not connect, timed out";
		return false;
	}
}

bool Client::download()
{
	LOG(INFO) << "Beggining download";

	ENetEvent event;

	bool downloading = true;

	while (true)
	{

		enet_host_service(client, &event, 0);

		if (event.type == ENET_EVENT_TYPE_RECEIVE)
		{
			Packet packet = Packet(event.packet);
			// Receive packet
			if (downloading)
			{
				if (packet.getType() == MSG_WORLD)
				{
					LOG(INFO) << "Received big world package";
					packet.popByte(); //< Pop off the identifier
					world.deserialize(packet.popJson(), this);
				} 
				else if (packet.getType() == MSG_FINISH_DOWNLOAD)
				{
					LOG(INFO) << "Finished download";
					downloading = false;
				}
			}
			else
			{
				LOG(INFO) << "Packet " << packet.getType();
				// While downloading a limited ammount of packet types can be received
				if (packet.getType() == MSG_START)
				{
					// Start game, we received start acknowledge
					LOG(INFO) << "Starting";
					world.start();
					return true;
				}
				else if (packet.getType() == MSG_NEW_ENTITY)
				{
					// Create a new entity, we can receive these while in download
					// as new clients connect
					packet.popByte();
					world.receiveNewEntity(packet, this);
				}

			}

			enet_packet_destroy(event.packet);

		}
		else if (event.type == ENET_EVENT_TYPE_DISCONNECT)
		{
			LOG(INFO) << "Disconnected. The server is already playing";
			return false;
		}
	}

	return true;
}

void Client::play()
{

#ifdef _DEBUG
	int renderWidth = 256;
	int renderHeight = 128;
	int screenScale = 4;
#else
	int renderWidth = 512;
	int renderHeight = 256;
	int screenScale = 2;
#endif
	int uiHeight = 256;
	int topUiHeight = 32;

	win = new sf::RenderWindow(sf::VideoMode(renderWidth * screenScale, renderHeight * screenScale + uiHeight + topUiHeight), "Raycasting game");

	sf::Font font = sf::Font();
	font.loadFromFile("Assets/consola.ttf");

	sf::Image target; target.create(renderWidth, renderHeight);
	sf::Texture targetTex;
	sf::Sprite targetSpr;

	sf::Clock dtc;
	sf::Time dtt;
	float dt = 0.0003f;
	float t = 0.0f;

	// Mouse coordinates in scaled space
	// Used for picking
	sf::Vector2i gameMouseCoords;
	bool mouseInGame = false;

	sf::Text debugText = sf::Text();
	debugText.setFont(font);
	debugText.setCharacterSize(16);

	while (win->isOpen())
	{
		sf::Event event;
		while (win->pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				win->close();
		}


		// Handle network
		ENetEvent netevent;
		enet_host_service(client, &netevent, 0);

		if (netevent.type == ENET_EVENT_TYPE_DISCONNECT)
		{
			// We disconnected for x reason, state it and quit
			LOG(INFO) << "Lost connection with server";
			break;
		}
		else if (netevent.type == ENET_EVENT_TYPE_RECEIVE)
		{
			// Receive packet
			Packet pak = Packet(netevent.packet);
			if (pak.getType() == MSG_CONTROL)
			{
				pak.popByte();
				// Sets the controlled entity
				uint32_t uid = pak.popUID();
				controlledEntity = uid;
				controlledEntityPtr = (SpriteEntity*)world.findEntity(uid);

				LOG(INFO) << "Now controlling entity " << uid;
			}
			else if (pak.getType() == MSG_COMMAND)
			{
				// Command
				pak.popByte();
				world.handleCommand(pak, netevent.peer);
			}
			else if (pak.getType() == MSG_NEW_ENTITY)
			{
				// Create a new entity
				pak.popByte();
				world.receiveNewEntity(pak, this);
			}

			enet_packet_destroy(netevent.packet);
		}

		if (win->hasFocus())
		{
			gameMouseCoords = sf::Mouse::getPosition(*win);
			gameMouseCoords.y -= topUiHeight;
			gameMouseCoords /= screenScale;


			if (gameMouseCoords.y >= 0 && gameMouseCoords.y < renderHeight && gameMouseCoords.x >= 0 && gameMouseCoords.x < renderWidth)
			{
				mouseInGame = true;
			}
			else
			{
				mouseInGame = false;
			}

			
		}

		// Not really neccesary
		win->clear();

		world.update(dt);
		if (controlledEntityPtr != NULL)
		{
			world.map->draw(&target, controlledEntityPtr->getSprite()->pos, controlledEntityPtr->getSprite()->angle, 1.0f);

			targetTex.loadFromImage(target);
			targetSpr.setTexture(targetTex);
			targetSpr.setScale((float)screenScale, (float)screenScale);
			targetSpr.setPosition(0, (float)topUiHeight);

			win->draw(targetSpr);

			if (mouseInGame)
			{
				sf::Color buffer = world.map->buffers.getPixel(gameMouseCoords.x, gameMouseCoords.y);

				sf::String string = "MPOS: (" + std::to_string(gameMouseCoords.x) + ", " + std::to_string(gameMouseCoords.y) + ") | ";

				if (BIT_CHECK(buffer.a, 0))
				{
					string += "Sprite ID ";
					uint16_t id = ((uint16_t)buffer.g << 8) | buffer.b;
					string += std::to_string(id);
				}
				else
				{
					string += "RPOS: (" + std::to_string(buffer.g) + ", " + std::to_string(buffer.b) + ") | ";
					string += "Flags: (";

					if (BIT_CHECK(buffer.a, 1))
					{
						string += "Wall ";

						int side = 0;
						if (BIT_CHECK(buffer.a, 6))
						{
							BIT_SET(side, 0);
						}

						if (BIT_CHECK(buffer.a, 7))
						{
							BIT_SET(side, 1);
						}

						if (side == 0)
						{
							string += "North ";
						}
						else if (side == 1)
						{
							string += "East ";
						}
						else if (side == 2)
						{
							string += "South ";
						}
						else
						{
							string += "West ";
						}
					}
					else
					{
						if (BIT_CHECK(buffer.a, 4))
						{
							string += "Skybox ";
						}
						else
						{
							string += "Normal ";
						}

						if (BIT_CHECK(buffer.a, 5))
						{
							string += "Ceiling ";
						}
						else
						{
							string += "Floor ";
						}
					}

					string += ")";
				}

				debugText.setString(string);
				win->draw(debugText);
			}
		}

		win->display();

		dtt = dtc.restart();
		dt = dtt.asSeconds();
		t += dt;

		win->setTitle("Raycaster | FPS: " + std::to_string(1.0f / dt));
	}
}

Client::Client()
{
	controlledEntity = 0;
	controlledEntityPtr = NULL;
}


Client::~Client()
{
}

