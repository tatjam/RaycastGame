#include "Editor.h"



Editor::Editor()
{
}


Editor::~Editor()
{
}

void Editor::mainFunc(std::unordered_map<std::string, std::string> vals)
{
	std::string mapToEdit;

	auto it = vals.find("map");
	if (it == vals.end())
	{

	}
	win = new sf::RenderWindow(sf::VideoMode(1024, 700), "Raycaster Editor");

	while (win->isOpen())
	{
		sf::Event ev;
		while (win->pollEvent(ev))
		{
			if (ev.type == sf::Event::Closed)
			{
				win->close();
			}
		}

		win->clear();

		win->display();
	}
}
