#pragma warning(disable:28251)
#pragma warning(disable:6385)
#pragma warning(disable:6386)
#pragma warning(disable:6031)

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <Windows.h>
#include <dwmapi.h>
#include <winuser.h>

#define UID(x, y)  unsigned int(((x) + COLS * (y)))
#define BACKGROUND {10, 10, 10}

const unsigned int COLS = 14;
const unsigned int ROWS = 26;
const unsigned int CELL_SIZE = 34;

struct square
{
	bool isBlock = false;
	sf::Color color = BACKGROUND;

} *grid;

#include "../Headers/Shapes.h"

struct piece
{
	v2 pos = { 6 , 0 };

	int shape_n = rand() % 4;
	int orientation = 0;

	v2 *cellsp = shapes[shape_n].states[0];

	int ids[4] = {0, 0, 0, 0};
	bool blocked = 0;

	sf::Color color = shapes[shape_n].col;

	void getIDs()
	{
		for (size_t i = 0; i < 4; i++)
		{
			ids[i] = UID(pos.x + cellsp[i].x, pos.y + cellsp[i].y);
			grid[ids[i]] = { blocked, color };
		}
	}

	bool checkDown()
	{
		for (size_t i = 0; i < 4; i++)
		{
			if (grid[ids[i] + COLS].isBlock || pos.y + 1 + cellsp[i].y >= ROWS) return false;
		}
		return true;
	}
	bool moveDown()
	{
		if (checkDown())
		{
			pos.y++;
			return true;
		}
		else return false;
	}
	void moveLeft()
	{
		for (size_t i = 0; i < 4; i++)
			if (pos.x + cellsp[i].x <= 0 ||
				grid[UID(pos.x -1 + cellsp[i].x, pos.y + cellsp[i].y)].isBlock
				) return;
		
		pos.x--;
	}
	void moveRight()
	{
		for (size_t i = 0; i < 4; i++) 
			if (pos.x + cellsp[i].x >= COLS-1 || 
				grid[UID(pos.x+1 + cellsp[i].x, pos.y + cellsp[i].y)].isBlock
				) return;
		
		pos.x++;
	}
	void rotate()
	{
		if (orientation < 3) orientation++;
		else orientation = 0;

		cellsp = shapes[shape_n].states[orientation];
	}

} *tetrads;

sf::Event event;
sf::View view;

int current = 0;

void Events(sf::RenderWindow* window);
void Tick();

void Tick()
{
	// Downwards movement
	if (!tetrads[current].moveDown())
	{
		tetrads[current].blocked = true;
		tetrads[current].getIDs();
		current++;
	}

	// Check for filled lines
	for (size_t x = 0; x < COLS; x++)
	{
		if (!grid[UID(x, 25)].isBlock)
		{
			goto skip;
		}
	}
	// Second iteration to remove & move down
	for (size_t x = 0; x < COLS; x++)
	{
		grid[UID(x, 25)] = {false, BACKGROUND};
	}

	for (size_t x = 0; x < COLS; x++)
	{
		for (size_t y = ROWS-1; y > 0; y--)
		{
			grid[UID(x, y)] = grid[UID(x, y-1)];
		}
	}

skip:

	tetrads[current].getIDs();

	return;
}

void Events(sf::RenderWindow* window)
{
	while (window->pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
		{
			window->close();
		}
		else if (event.type == sf::Event::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Left)
			{
				tetrads[current].moveLeft();
			}
			else if (event.key.code == sf::Keyboard::Right)
			{
				tetrads[current].moveRight();
			}
			else if (event.key.code == sf::Keyboard::Down)
			{
				tetrads[current].moveDown();
			}
			else if (event.key.code == sf::Keyboard::Up)
			{
				tetrads[current].rotate();
			}
		}
	}
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
#if !NDEBUG
	AllocConsole();
	freopen("CONOUT$", "w", stdout);	// lol
#endif

	srand(time(NULL));

	sf::RenderWindow window(sf::VideoMode(COLS * CELL_SIZE, ROWS * CELL_SIZE), "vex::Tetris", sf::Style::Close | sf::Style::Titlebar);
	window.setVerticalSyncEnabled(true);

	view = window.getDefaultView();

	sf::Texture box_texture;
	if (box_texture.loadFromFile("./Resources/box.png")) box_texture.setSmooth(true);

	sf::RectangleShape box;
	box.setTexture(&box_texture);
	box.setSize(sf::Vector2f(CELL_SIZE, CELL_SIZE));

	sf::Clock clock;
	sf::Time time;

	grid = new square[COLS * ROWS];
	tetrads = new piece[255];
	//tetrads[0].exists = true;

	while (window.isOpen())
	{
		window.clear();
		Events(&window);
		window.setView(view);

		tetrads[current].getIDs();

		time = clock.getElapsedTime();
		if (time.asSeconds() >= 0.8)
		{
			clock.restart();
			Tick();
		}
		
		for (size_t x = 0; x < COLS; x++) {
			for (size_t y = 0; y < ROWS; y++) {

				box.setPosition(x * CELL_SIZE, y * CELL_SIZE);
				box.setFillColor(grid[UID(x, y)].color);

				if (!grid[UID(x, y)].isBlock)
				{
					grid[UID(x, y)].color = BACKGROUND;
				}

				window.draw(box);
			}
		}

		window.display();
	}

	delete[] grid;
	delete[] tetrads;

	return 0;
}