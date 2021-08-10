#pragma warning(disable:28251)
#pragma warning(disable:6385)
#pragma warning(disable:6386)
#pragma warning(disable:6031)

#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>

// To prevent more file dependencies, the font is loaded using a existing one, whose path depends on OS
#ifdef _WIN32
	#include <windows.h>
	#define FONTPATH "C:\\Windows\\Fonts\\Consola.ttf"
	#define main() __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, char* lpCmdLine, int nCmdShow)
#endif

#ifdef __linux__
	#define FONTPATH "/usr/share/fonts/"
	#define main() main(int argc, char* argv[])
#endif

#define UID(x, y) ((unsigned int (x)) + COLS * unsigned int(y))	// Operation to get the 1D index of a pseudo 2D array
#define BACKGROUND {10, 10, 10}			// Deafult background

#define STATES	4						// Number of rotations of the tetrads
#define BLOCKS	4						// Number of blocks of the tetrads
#define TETRADS 7						// Number of tetrads

const unsigned int COLS			= 10;	// Number of game colums
const unsigned int ROWS			= 24;	// Number of game rows
const unsigned int CELL_SIZE	= 36;	// Side lenght of each cell (COLS * ROWS)

#include "../Headers/Box.h"				// Compile-time resources
#include "../Headers/Shapes.h"			// Mappings of each tetrad, including rotation

// Represents a cell in the game
struct square
{
	bool isBlock = false;
	sf::Color color = BACKGROUND;

} *grid;

// Set of squares/cells wich represents a tetrad (should be a class?...)
struct piece
{
	v2 pos = { 4 , 0 };

	int shape_n = rand() % TETRADS;
	int orientation = 0;

	v2* cellsp = shapes[shape_n].states[0];
	int ids[4] = { 0, 0, 0, 0 };

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
				grid[UID(pos.x - 1 + cellsp[i].x, pos.y + cellsp[i].y)].isBlock
				) return;

		pos.x--;
	}
	void moveRight()
	{
		// Collision check
		for (size_t i = 0; i < BLOCKS; i++)
			if (pos.x + cellsp[i].x >= COLS - 1 ||
				grid[UID(pos.x + 1 + cellsp[i].x, pos.y + cellsp[i].y)].isBlock
				) return;

		pos.x++;
	}
	void rotate()
	{
		size_t next_state;

		if (orientation < 3)	next_state = orientation + 1;
		else				next_state = 0;

		for (size_t i = 0; i < BLOCKS; i++)
		{
			if (grid[UID(
				pos.x + shapes[shape_n].states[next_state][i].x,
				pos.y + shapes[shape_n].states[next_state][i].y)].isBlock ||

				pos.x + shapes[shape_n].states[next_state][i].x > COLS - 1 ||
				pos.x + shapes[shape_n].states[next_state][i].x < 0 ||

				pos.y + shapes[shape_n].states[next_state][i].y > ROWS)
			{
				next_state = orientation;
				break;
			}
		}

		orientation = next_state;
		cellsp = shapes[shape_n].states[orientation];
	}
};

////////////////////////////////////////////////////////////////////////

sf::Clock	mclock;		// Clock that passes its value to the timer
sf::Time	mtime;		// Checks if enought time has passed to tick
sf::Event	event;		// Object used to react to events
sf::View	view;		// The only view used, fills the window

int	current	= 0;		// The tetrad index that is being used
int	score	= 0;		// Number of points
int	level	= 0;		// Current level, not used
float TPS	= 1/1.25;	// Tick per second

std::vector<piece> tetrads(1);

void Events(sf::RenderWindow* window);	// Handle all the events
void Tick();	// Ticks the game at the TPS rate

////////////////////////////////////////////////////////////////////////

void Tick()
{
	// Downwards movement
	if (!tetrads[current].moveDown())
	{
		tetrads[current].blocked = true;
		tetrads[current].getIDs();
		piece t;
		tetrads.push_back(t);
		current++;
	}

	std::vector<int> lines;	// Keeps the full lines index
	bool bLine = true;		// Lets skip if line isnt full

	// Check for filled lines
	for (size_t y = 0; y < ROWS; y++) {
		for (size_t x = 0; x < COLS; x++) {
			if (!grid[UID(x, y)].isBlock)
			{
				bLine = false; 
				break;
			}
		}
		if (bLine) lines.push_back(y);
		bLine = true;
	}

	// Skip line-deleting loops if necessary
	if (lines.size() == 0) goto skip;

	// Second iteration to remove lines & move down
	for (size_t y = 0; y < lines.size(); y++)
		for (size_t x = 0; x < COLS; x++)
			grid[UID(x, lines[y])] = { false, BACKGROUND };

	// Third iteration to move down i times
	for (size_t i = 0; i < lines.size(); i++)
		for (size_t x = 0; x < COLS; x++)
			for (size_t y = ROWS-1; y > 0; y--)
				grid[UID(x, y)] = grid[UID(x, y-1)];


	// Score table
	switch (lines.size())
	{
	case 1:
		score += 40;
		break;
	case 2:
		score += 100;
		break;
	case 3:
		score += 300;
		break;
	case 4:
		score += 1200;
		break;
	default:
		break;
	}

// Right now there is no extra ticking functionality, so skip = return
skip:
	return;
}

////////////////////////////////////////////////////////////////////////

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
				// If can move down, increment score by 1
				if(tetrads[current].moveDown()) score++;
			}
			else if (event.key.code == sf::Keyboard::Up)
			{
				tetrads[current].rotate();
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////

int main()
{
#if _DEBUG && _WIN32
	AllocConsole();
	freopen("CONOUT$", "w", stdout); // Black magic
#endif

	// Set pseudo-random generation seed (probably I will switch to a better pseudo-generation)
	srand(time(NULL));

	// Window initalization

	sf::RenderWindow window(sf::VideoMode(COLS * CELL_SIZE, ROWS * CELL_SIZE), "vex::Tetris", sf::Style::Close | sf::Style::Titlebar);
	window.setIcon(icon.width, icon.height, icon.pixel_data);
	window.setVerticalSyncEnabled(true);
	view = window.getDefaultView();

	// Resource-realted

	sf::Image	img;			img.create(box.width, box.height, box.pixel_data);
	sf::Texture box_texture;	box_texture.loadFromImage(img);
	sf::Font	font;			font.loadFromFile(FONTPATH);

	sf::Text text;
	text.setFont(font);
	text.setCharacterSize(20);
	text.setFillColor(sf::Color(200, 200, 200));
	text.setPosition(8, 6);
	text.setString("Score: " + std::to_string(score));

	sf::RectangleShape box;
	box.setTexture(&box_texture);
	box.setSize(sf::Vector2f(CELL_SIZE, CELL_SIZE));

	// Object allocation + construction
	grid = new square[COLS * ROWS];

	/* tetrads => Dynamically allocation (init at scope) */
	

	// Game loop
	while (window.isOpen())
	{
		// Erase screen
		window.clear();
		window.setView(view);

		// Events handle
		Events(&window);

		// Checks for ticking ()
		mtime = mclock.getElapsedTime();
		if (mtime.asSeconds() >= TPS)
		{
			mclock.restart();
			Tick();
		}

		// Update current tetrad
		tetrads[current].getIDs();

		text.setString("Score: " + std::to_string(score));
		
		// Drawing loop
		for (size_t x = 0; x < COLS; x++) {
			for (size_t y = 0; y < ROWS; y++) {

				box.setPosition(x * CELL_SIZE, y * CELL_SIZE);
				box.setFillColor(grid[UID(x, y)].color);

				window.draw(box);
			}
		}

		window.draw(text);

		// Just display content to the window
		window.display();
	}

	// Cleaning objects
	std::vector<piece>().swap(tetrads);	// Empties the tetrads vector and "deallocates" memory
	delete[] grid;						// Destroy grid array

	return 0;
}