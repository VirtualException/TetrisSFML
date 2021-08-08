#pragma warning(disable:28251)
#pragma warning(disable:6385)
#pragma warning(disable:6386)
#pragma warning(disable:6031)

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <vector>
#include <windows.h>
#include <winuser.h>

#define UID(x, y)  unsigned int(((x) + COLS * (y)))
#define BACKGROUND {10, 10, 10}

#define STATES	4
#define BLOCKS	4
#define TETRADS 7

const unsigned int COLS = 10;
const unsigned int ROWS = 24;
const unsigned int CELL_SIZE = 36;

#include "../Headers/Box.h"

struct square
{
	bool isBlock = false;
	sf::Color color = BACKGROUND;

} *grid;

#include "../Headers/Shapes.h"

struct piece
{
	v2 pos = { int(6) , int(0) };

	int shape_n = rand() % TETRADS;
	int orientation = 0;

	v2 *cellsp = shapes[shape_n].states[0];
	int ids[4] = {0, 0, 0, 0};

	bool blocked = 0;
	sf::Color color = shapes[shape_n].col;

	void getIDs()
	{
		// Updates tetrad blocks
		for (size_t i = 0; i < BLOCKS; i++)
		{
			grid[ids[i]] = { false, BACKGROUND };
			ids[i] = UID(pos.x + cellsp[i].x, pos.y + cellsp[i].y);
			grid[ids[i]] = { blocked, color };
		}
	}

	bool checkDown()
	{
		for (size_t i = 0; i < BLOCKS; i++)
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
		// Collision check
		for (size_t i = 0; i < BLOCKS; i++)
			if (pos.x + cellsp[i].x <= 0 ||
				grid[UID(pos.x -1 + cellsp[i].x, pos.y + cellsp[i].y)].isBlock
				) return;
		
		pos.x--;
	}
	void moveRight()
	{
		// Collision check
		for (size_t i = 0; i < BLOCKS; i++) 
			if (pos.x + cellsp[i].x >= COLS-1 || 
				grid[UID(pos.x+1 + cellsp[i].x, pos.y + cellsp[i].y)].isBlock
				) return;
		
		pos.x++;
	}
	void rotate()
	{
		orientation = ++orientation % STATES;

		int next_state = 0;

		if (orientation < 3) next_state++;
		else next_state = 0;

		for (size_t i = 0; i < BLOCKS; i++)
		{
			if (grid[UID(
					pos.x + shapes[shape_n].states[next_state][i].x,
					pos.y + shapes[shape_n].states[next_state][i].y)].isBlock ||

					pos.x + shapes[shape_n].states[next_state][i].x > COLS ||
					pos.x + shapes[shape_n].states[next_state][i].x < 0	||

					pos.y + shapes[shape_n].states[next_state][i].y > ROWS)
			{
				orientation--; return;
			}
		}

		cellsp = shapes[shape_n].states[orientation];
	}

} *tetrads;

sf::Clock mclock;
sf::Time mtime;
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
		if (!grid[UID(x, ROWS-1)].isBlock)
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

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpCmdLine, int nCmdShow)
{
#if !NDEBUG && WIN32 // Allows use stdout (usefull when debuggin)
	AllocConsole();
	freopen("CONOUT$", "w", stdout); // Black magic
#endif

	srand(time(NULL));

	// Window initalization

	sf::RenderWindow window(sf::VideoMode(COLS * CELL_SIZE, ROWS * CELL_SIZE), "vex::Tetris", sf::Style::Close | sf::Style::Titlebar);
	window.setIcon(icon.width, icon.height, icon.pixel_data);
	window.setVerticalSyncEnabled(true);
	view = window.getDefaultView();

	// Resource-realted

	sf::Image	img;			img.create(box.width, box.height, box.pixel_data);
	sf::Texture box_texture;	box_texture.loadFromImage(img);

	sf::RectangleShape box;
	box.setTexture(&box_texture);
	box.setSize(sf::Vector2f(CELL_SIZE, CELL_SIZE));

	// Obj allocation
	grid	= new square[COLS * ROWS];
	tetrads = new piece[255];

	while (window.isOpen())
	{
		// Erase screen
		window.clear();
		window.setView(view);

		// Events handle
		Events(&window);

		// Checks for ticking ()
		mtime = mclock.getElapsedTime();
		if (mtime.asSeconds() >= 1/1.25)
		{
			mclock.restart();
			Tick();
		}

		// Update current tetrad
		tetrads[current].getIDs();
		
		// Drawing loop
		for (size_t x = 0; x < COLS; x++) {
			for (size_t y = 0; y < ROWS; y++) {

				box.setPosition(x * CELL_SIZE, y * CELL_SIZE);
				box.setFillColor(grid[UID(x, y)].color);

				window.draw(box);
			}
		}

		// Just display content to the window
		window.display();
	}

	// Cleaning heap
	delete[] grid;
	delete[] tetrads;

	return 0;
}