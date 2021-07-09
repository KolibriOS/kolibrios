#include <kosSyst.h>
#include <kosFile.h>
#include "images.hpp"

//Global const strings
const char HEADER_STRING[] = "Flappy bird";
const char CONTROL_STRING[] = "SPACEBAR TO JUMP";
const char GAMEOVER_STRING[] = "GAMEOVER";
const char ANY_KEY_STRING[] = "Press any key for restart";
const char SELECT_SPEED_STRING[] = "select the speed of the game";
const char FAST_STRING[] = "1 FAST";
const char SLOW_STRING[] = "2 SLOW";

//Global const variables
const int WINDOW_WIDTH = 400;
const int WINDOW_HEIGHT = 400;
const int BORDER_TOP = 24;
const int BORDER_LEFT = 5;
const int BORDER_RIGHT = 5;
const int BORDER_DOWN = 5;

enum GameState
{
	GAMESTATE_MENU,
	GAMESTATE_STARTED,
	GAMESTATE_GAMEOVER
};

struct ScreenSize
{
	int width;
	int height;
};

class Bird
{
public:
	static const int sizeX = 19;
	static const int sizeY = 20;
	static const int x = 100;
	int prev_y;
	int y;
	int acceleration;

	inline void initialize()
	{
		y = WINDOW_HEIGHT / 2;
		acceleration = 0;
	}

	inline void move()
	{
		if (acceleration <= 30)
			acceleration += 2;
		prev_y = y;
		y += acceleration / 10;
	}

	inline void jump()
	{
		acceleration = -50;
	}

	inline void draw()
	{
		kos_PutImage(birdImage, sizeX, sizeY, x, y);
	}
};

class Tube
{
public:
	static const int width = 50;
	static const int gapHeight = 100;
	static const int headHeight = 18;
	int x;
	int gapY;

	inline void randomize()
	{
		x = WINDOW_WIDTH + 1;
		gapY = rtlRand() % 200 + 50;
	}

	inline void move()
	{
		x -= 2;
		if (x < -width - 2)
			randomize();
	}

	void draw()
	{
		//cleanup
		int pixels = (WINDOW_WIDTH - (BORDER_LEFT + BORDER_RIGHT - 1)) - (x + width + 2);
		if (pixels >= -1)
		{
			pixels = (pixels == -1) ? 1 : 2;
			kos_DrawBar(x + width, gapY - headHeight, pixels, headHeight, 0x00FFFF);
			kos_DrawBar(x + width, gapY + gapHeight, pixels, headHeight, 0x00FFFF);
		}

		int offset = x >= 0 ? 0 : -x;
		int trim = x + width >= WINDOW_WIDTH - (BORDER_LEFT + BORDER_RIGHT - 1) ? WINDOW_WIDTH - x - width - (BORDER_LEFT + BORDER_RIGHT - 1) : 0;
		int trimHead = x + width >= WINDOW_WIDTH - (BORDER_LEFT + BORDER_RIGHT - 1) ? WINDOW_WIDTH - x - width - (BORDER_LEFT + BORDER_RIGHT - 1) : 0;

		//top
		for (int y = 0; y < gapY - headHeight; ++y)
			kos_PutImage(tubeBodyImage + offset, width - offset + trim, 1, x + offset, y);
		//head top
		for (int y = gapY - headHeight; y < gapY; ++y)
			kos_PutImage(tubeHeadImage + width * (y - (gapY - headHeight)) + offset, width - offset + trimHead, 1, x + offset, y);
		//head down
		for (int y = gapY + gapHeight; y < gapY + gapHeight + headHeight; ++y)
			kos_PutImage(tubeHeadImage + width * (y - (gapY + gapHeight)) + offset, width - offset + trimHead, 1, x + offset, y);
		//down
		for (int y = gapY + gapHeight + headHeight; y < WINDOW_HEIGHT - (BORDER_TOP + BORDER_DOWN - 1); ++y)
			kos_PutImage(tubeBodyImage + offset, width - offset + trim, 1, x + offset, y);

	}
};

//Global variables
int loopDelay;
GameState gameState;
char scoreString[] = "Score:    ";
bool scoreChanged;
int score;
Bird bird;
int tubeNumber;
Tube tubes[3];
int windowX;
int windowY;

//Function prototypes
void kos_Main();
void startGame();
ScreenSize getScreenSize();
void updateScoreString();
void WriteBorderedText(Word x, Word y, Byte fontType, Dword textColor, const char* textPtr, Dword textLen, Dword borderColor, int borderSize);
inline bool checkAddScore(Tube tube);
inline bool checkCollision(Tube tube);

void drawMenuWindow();
void drawGameWindow();
void redrawGameWindow();
void drawGameoverWindow();

//Functions

void startGame()
{
	kos_SetMaskForEvents(0x7); /// 111 in binary

	bird.initialize();

	score = 0;
	memset((Byte*)scoreString + 6, ' ', 3);
	updateScoreString();

	tubeNumber = 1;
	tubes[0].randomize();

	gameState = GAMESTATE_STARTED;
	drawGameWindow();
}

ScreenSize getScreenSize()
{
	Dword result;
	__asm {
		push 14		//System function 14
		pop eax
		int 0x40
		mov result, eax
	}
	ScreenSize screenSize;
	screenSize.height = (result & 0xFFFF) + 1;	//last two bytes
	screenSize.width = (result >> 16) + 1;		//first two bytes
	return screenSize;
}

void kos_Main()
{
	rtlSrand( kos_GetSystemClock() );

	//Centring window
	ScreenSize screenSize = getScreenSize();
	windowX = (screenSize.width - WINDOW_WIDTH) / 2;
	windowY = (screenSize.height - WINDOW_HEIGHT) / 2;

	gameState = GAMESTATE_MENU;

	kos_SetMaskForEvents(0x27); // 100111 in binary

	while( true )
	{
		switch (gameState)
		{
		case GAMESTATE_STARTED:
			kos_Pause(loopDelay);

			bird.move();

			//Adding new tubes
			if ((tubeNumber == 1 || tubeNumber == 2) && (tubes[tubeNumber - 1].x < (WINDOW_WIDTH - WINDOW_WIDTH / 3)))
				tubes[tubeNumber++].randomize();

			//Processing all tubes
			scoreChanged = false;
			for (int i = 0; i < tubeNumber; ++i)
			{
				//Adding score
				if (checkAddScore(tubes[i]))
				{
					++score;
					scoreChanged = true;
				}

				//Check collision with bird
				if (checkCollision(tubes[i]))
				{
					gameState = GAMESTATE_GAMEOVER;
					continue;
				}

				//Move tube
				tubes[i].move();
			}

			if (scoreChanged)
				updateScoreString();

			//Cheking the bird is too high or low 
			if (bird.y + bird.sizeY > WINDOW_HEIGHT - (BORDER_TOP + BORDER_DOWN - 1) || bird.y < 0)
			{
				gameState = GAMESTATE_GAMEOVER;
				continue;
			}

			redrawGameWindow();

			switch (kos_CheckForEvent())
			{
			case 1:
				drawGameWindow();
				break;

			case 2: // key pressed
				Byte keyCode;
				kos_GetKey(keyCode);
				if (keyCode == 32) //if pressed key is spacebar
					bird.jump();
				break;

			case 3: // button pressed; we have only one button, close
				kos_ExitApp();
			}
			break;

		case GAMESTATE_GAMEOVER:
			drawGameoverWindow();

			switch (kos_WaitForEvent())
			{
			case 1:
				drawGameoverWindow();
				break;

			case 2:
				startGame();
				break;

			case 3:
				kos_ExitApp();
			}
			break;

		case GAMESTATE_MENU:
			switch (kos_WaitForEvent())
			{
			case 1:
				drawMenuWindow();
				break;

			case 2:
				Byte keyCode;
				kos_GetKey(keyCode);
				if (keyCode == 0x31 || keyCode == 0x61)	//1 or NumPad1
				{
					loopDelay = 1;
					startGame();
				}
				else if (keyCode == 0x32 || keyCode == 0x62) //2 or NumPad2
				{
					loopDelay = 2;
					startGame();
				}
				break;

			case 3:
				kos_ExitApp();

			case 6:
				Dword result;
				__asm {
					push 37 //Function 37 - work with mouse
					pop eax
					mov ebx, 3 //Subfunction 3 - states and events of the mouse buttons 
					int 0x40
					mov result, eax
				}
				result &= 0x100; //bit 8 is set = left button is pressed
				if ( result )
				{
					Dword coordinates;
					__asm {
						push 37	//Function 37 - work with mouse
						pop eax
						mov ebx, 1 //Subfunction 1 - coordinates of the mouse relative to the window 
						int		0x40
						mov coordinates, eax
					}
					int clickX = coordinates >> 16;
					int clickY = coordinates & 0xFFFF;
					if (clickX >= 100 && clickX < 390 && clickY >= 170 && clickY < 230)
					{
						loopDelay = 1;
						startGame();
					}
					else if (clickX >= 100 && clickX < 390 && clickY >= 270 && clickY < 330)
					{
						loopDelay = 2;
						startGame();
					}
				}
				break;
			}
			break;
		}
	}
}

void drawGameWindow()
{
	kos_DefineAndDrawWindow(windowX, windowY, WINDOW_WIDTH, WINDOW_HEIGHT, 0x33, 0x00FFFF, 0, 0, (Dword)HEADER_STRING);
	bird.draw();
	for (int i = 0; i < tubeNumber; ++i)
		tubes[i].draw();
	kos_WriteTextToWindow(10, 10, 0x81, 0x000000, scoreString, 0);
	kos_WriteTextToWindow(10, 30, 0x81, 0x000000, CONTROL_STRING, 0);
}
void redrawGameWindow()
{
	//cleaning the screen
	if (scoreChanged)
		kos_DrawBar(80, 10, 50, 15, 0x00FFFF);
	if (bird.y > bird.prev_y)
		kos_DrawBar(bird.x, bird.prev_y, bird.sizeX, bird.y - bird.prev_y, 0x00FFFF);
	else
		kos_DrawBar(bird.x, bird.y + bird.sizeY, bird.sizeX, bird.prev_y - bird.y, 0x00FFFF);

	bird.draw();
	for (int i = 0; i < tubeNumber; ++i)
		tubes[i].draw();

	kos_WriteTextToWindow(10, 10, 0x81, 0x000000, scoreString, 0);
	kos_WriteTextToWindow(10, 30, 0x81, 0x000000, CONTROL_STRING, 0);
}

void drawGameoverWindow()
{
	kos_DefineAndDrawWindow(windowX, windowY, WINDOW_WIDTH, WINDOW_HEIGHT, 0x33, 0x000000, 0, 0, (Dword)HEADER_STRING);
	kos_WriteTextToWindow(125, 50, 0x82, 0xFFFFFF, GAMEOVER_STRING, 0);
	kos_WriteTextToWindow(135, 100, 0x81, 0xFFFFFF, scoreString, 0);
	kos_WriteTextToWindow(50, 150, 0x81, 0xFFFFFF, ANY_KEY_STRING, 0);
}

void WriteBorderedText(Word x, Word y, Byte fontType, Dword textColor, const char *textPtr, Dword textLen, Dword borderColor, int borderSize)
{
	kos_WriteTextToWindow(x - borderSize, y - borderSize, fontType, borderColor, textPtr, textLen);
	kos_WriteTextToWindow(x - borderSize, y + borderSize, fontType, borderColor, textPtr, textLen);
	kos_WriteTextToWindow(x + borderSize, y - borderSize, fontType, borderColor, textPtr, textLen);
	kos_WriteTextToWindow(x + borderSize, y + borderSize, fontType, borderColor, textPtr, textLen);
	kos_WriteTextToWindow(x, y, fontType, textColor, textPtr, textLen);
}

void drawMenuWindow()
{
	kos_DefineAndDrawWindow(windowX, windowY, WINDOW_WIDTH, WINDOW_HEIGHT, 0x33, 0x00FFFF, 0, 0, (Dword)HEADER_STRING);
	
	WriteBorderedText(85, 40, 0x4, 0xFFFFFF, HEADER_STRING, 6, 0x000000, 2);
	WriteBorderedText(185, 80, 0x84, 0xFFFFFF, HEADER_STRING + 7, 0, 0x000000, 2);

	RGB* pos = &tubeHeadImage[0];
	for (int x = 100 - 1; x >= 100 - Tube::headHeight; --x)
		for (int y = 170; y < 170 + Tube::width; ++y)
		{
			kos_PutPixel(x, y, (pos->r << 16) + (pos->g << 8) + (pos->b));	//first tube
			kos_PutPixel(x, y+100, (pos->r << 16) + (pos->g << 8) + (pos->b)); //second tube
			++pos;
		}

	//First button
	for(int x = 100; x < WINDOW_WIDTH - (BORDER_LEFT + BORDER_RIGHT - 1); ++x)
		kos_PutImage(tubeBodyImage, 1, Tube::width, x, 170);
	WriteBorderedText(140, 185, 0x82, 0x000000, FAST_STRING, 0, 0xFFFFFF, 1);
	
	//Second button
	for (int x = 100; x < WINDOW_WIDTH - (BORDER_LEFT + BORDER_RIGHT - 1); ++x)
		kos_PutImage(tubeBodyImage, 1, Tube::width, x, 270);
	WriteBorderedText(140, 285, 0x82, 0x000000, SLOW_STRING, 0, 0xFFFFFF, 1);
}

inline bool checkCollision(Tube tube)
{
	return ((tube.x <= (bird.x + bird.sizeX) && tube.x + tube.width >= bird.x)
		&& (bird.y <= tube.gapY || bird.y + bird.sizeY >= tube.gapY + tube.gapHeight));
}

inline bool checkAddScore(Tube tube)
{
	//int diff = bird.x - (tube.x + tube.width);
	//return diff == 0 || diff == 1;
	return ((bird.x - (tube.x + tube.width)) >> 1) == 0;
}

void updateScoreString()
{
	int temp = score;
	int index = 9;
	do {
		scoreString[index--] = temp % 10 + '0';
		temp /= 10;
	} while (temp > 0);
}