/*
THE BUS
Copyright (C) 2008, 2012, 2017 Leency
Menu image from Freepik.com
D O N K E Y
Copyright (C) 2008 O.Bogomaz
*/

#ifndef AUTOBUILD
#include "lang.h--"
#endif

#define MEMSIZE 1024 * 60;

#include "..\lib\kolibri.h"
#include "..\lib\random.h"

#include "..\lib\obj\libio_lib.h"
#include "..\lib\obj\libimg_lib.h"
#include "..\lib\patterns\libimg_load_skin.h"

libimg_image menu;
libimg_image road;
libimg_image objects;

int lives=0, level=0, score=0;
int don_x, don_y, don_h, don_w=68, don_type, don_image_y, don_step_y, don_step_y_default=3;
int don_h_mas[8] = { 36,72,36,74,24,64,48,74 };
int bus_x, bus_w=42, bus_y, bus_h=88, bus_y_default=290;

#define SCR_MENU_MAIN   1
#define SCR_GAME        2
#define SCR_PAUSE       3

#define RAND -1

#define WIN_X 516
#define WIN_Y 382

int screen_type=SCR_MENU_MAIN;

#define COLOR_ROAD 0x6D879B;

int active_menu_item=0;

char *MENU_LIST[]={
"New game",
"Control keys",
"About",
"Exit",
0}; 


void DrawObstacle(signed int x, y) { 
	int don_offset_y;
	int image_h;

	if (y >= 0) {
		image_h = don_h;
		don_offset_y = don_image_y;
	}
	else {
		image_h = don_h + y;
		don_offset_y = don_image_y - y;
		y = 0;
	}
	if (y>=don_step_y) DrawBar(x, y-don_step_y, don_w, don_step_y, COLOR_ROAD); //Fill donkey old parts
	if (image_h>0) DrawLibImage(objects.image, x, y, don_w, image_h, 0, don_offset_y); 
}
void DrawBus(dword x, y) { DrawLibImage(objects.image, x, y, bus_w, bus_h, 0, 444); }
void DrawBoom(dword x, y) { DrawLibImage(objects.image, x, y, 78, 66, 0, 536); }
void DrawHighway() { DrawLibImage(road.image, 0,0, WIN_X, WIN_Y, 0, 0); }
void DrawMenuBackground() { DrawLibImage(menu.image, 0, 0, WIN_X, WIN_Y, 0, 0); }

void main()
{
	randomize();
	StartNewGame();

	load_dll(libio,  #libio_init,1);
	load_dll(libimg, #libimg_init,1);
	Libimg_LoadImage(#menu, "menu.png");
	Libimg_LoadImage(#road, "road.png");
	Libimg_LoadImage(#objects, "objects.png");
	
	loop()
	{
		WaitEventTimeout(1);

		switch(EAX & 0xFF)
		{
			case evKey: 
				GetKeys();
				if (key_scancode == SCAN_CODE_ESC)
				{
					if (screen_type==SCR_GAME) SetScreen(SCR_MENU_MAIN);
					else if (screen_type==SCR_MENU_MAIN) ExitProcess();
				}
				if (key_scancode == SCAN_CODE_DOWN) && (screen_type==SCR_MENU_MAIN)
				{
					if (active_menu_item<>3) active_menu_item++; ELSE active_menu_item=0;
					DrawMenuList();
				}
				if (key_scancode == SCAN_CODE_UP) && (screen_type==SCR_MENU_MAIN)
				{
					if (active_menu_item<>0) active_menu_item--; ELSE active_menu_item=3;
					DrawMenuList();
				}
				if (key_scancode == SCAN_CODE_ENTER) && (screen_type==SCR_MENU_MAIN)
				{
					if (active_menu_item==0)
					{
						StartNewGame();
						SetScreen(SCR_GAME);
					}
					if (active_menu_item==1) notify("'The Bus\nControl keys:\nLeft, Right, Space\nPress P key for pause'tI");
					if (active_menu_item==2) notify("'The Bus\nVersion v0.9\nAuthor: Leency\nMenu image from Freepik.com'tI");
					if (active_menu_item==3) ExitProcess();	
				}				
				if (key_scancode == SCAN_CODE_SPACE) && (screen_type==SCR_GAME)
				{
					DrawBar(bus_x*80+200, bus_y, bus_w, bus_h+1, COLOR_ROAD);
					if (bus_x==1) bus_x=0; else bus_x=1;
				}
				if (key_scancode == SCAN_CODE_LEFT) && (screen_type==SCR_GAME)
				{
					if (bus_x==0) break;
					DrawBar(bus_x*80+200, bus_y, bus_w, bus_h+1, COLOR_ROAD);
					bus_x=0;
				}
				if (key_scancode == SCAN_CODE_RIGHT) && (screen_type==SCR_GAME)
				{
					if (bus_x==1) break;
					DrawBar(bus_x*80+200, bus_y, bus_w, bus_h+1, COLOR_ROAD);
					bus_x=1;
				}
				if (key_scancode == SCAN_CODE_KEY_P)
				{
					if (screen_type==SCR_MENU_MAIN) break;
					else if (screen_type==SCR_GAME) SetScreen(SCR_PAUSE); 
					else if (screen_type==SCR_PAUSE) SetScreen(SCR_GAME); 
				}
				break;
				
			case evReDraw:
				DefineAndDrawWindow(250,150,WIN_X-1,WIN_Y-1,0x01,0,"The Bus",0); //0x74 is also possible if you fix bottom border
				DrawScreen();
				break;
				
			case evButton:
				ExitProcess();
				break;
				
			default:
				if (screen_type==SCR_GAME) 
				{
					if ((don_x == bus_x)&&(don_y + don_h > bus_y )&&(don_y < bus_y + don_h )) {
						lives--;
						DrawBus(bus_x*80+200,bus_y);
						DrawBoom(bus_x*80+180,302);
						pause(150);
						GetNewObstacle(RAND);
						DrawScreen();
					}
	
					if (lives==0) {
						DrawGameOverMessage();
						break;
					}

					don_y += don_step_y;

					if (don_y - don_step_y >= WIN_Y)
					{
						GetNewObstacle(RAND);
						score++;
						bus_y -= don_step_y+1;
						DrawBar(bus_x*80+200, bus_y+bus_h, bus_w, don_step_y, COLOR_ROAD);
						WriteScore();
					}

					if (score) && (score % 15 == 0) 
					{
						score++;
						NewLevel();
						DrawScreen();
						don_step_y++;
					}

					DrawRoad();
				}
		}
	}
}

void NewLevel()
{
	level++;
	bus_y = bus_y_default;
}

void StartNewGame()
{
	lives=3;
	level=0;
	score=0;
	bus_y = bus_y_default;
	don_step_y = don_step_y_default;
	GetNewObstacle(RAND);
}

void WriteScore() {
	DrawLibImage(road.image, 20, 166, 120, 24, 20, 164);
	WriteText(20, 140, 0x81, 0xFFFFFF, "Score");
	WriteText(20, 166, 0x81, 0xFFFFFF, itoa(score));
}

void SetScreen(dword _screen_type) {
	screen_type = _screen_type;
	DrawScreen();
}

void DrawScreen()
{
	int i;
	if (screen_type==SCR_MENU_MAIN) 
	{
		DrawMenuBackground();
		WriteTextB(20, 20, 0x82, 0xE8783F, "THE BUS");
		DrawMenuList();
	}
	if (screen_type==SCR_GAME) || (screen_type==SCR_PAUSE) 
	{
		DrawHighway();
		WriteText(20, 20,  0x81, 0xFFFFFF, "Lives");
		WriteText(20, 46,  0x81, 0xFFFFFF, itoa(lives));
		WriteText(20, 80,  0x81, 0xFFFFFF, "Level");
		WriteText(20, 106, 0x81, 0xFFFFFF, itoa(level));
		WriteScore();
		DrawRoad();
		if (screen_type==SCR_PAUSE) {
			DrawBar(0,0,140,60,0xFF0000);
			WriteText(10,14,0x83,0xFFFfff,"PAUSE");			
		}
	}
}


void DrawMenuList()
{
	int j;
	for (j=0; j<4; j++) DrawMenuItem(j, j);
}

void DrawMenuItem(int item_n, text_n)
{
	dword color;
	if (active_menu_item==item_n) color = 0xFF0000; else color = 0xFFffff;
	WriteText(20+2, item_n*56+116+2, 0x81, 0xAAAaaa, MENU_LIST[text_n]);
	WriteText(20, item_n*56+116, 0x81, color, MENU_LIST[text_n]);
}


void DrawGameOverMessage()
{
	DrawBar(0, 0, WIN_X, WIN_Y, 0xF3E1BD);
	WriteText(40, 40, 0x81, 0xA48C74, "GAME OVER");
	WriteText(40, 75, 0x81, 0xA48C74, "FINAL SCORE");
	WriteTextB(40, 140, 0x85, 0xA48C74, itoa(score));
	pause(350);	
	active_menu_item=0;
	SetScreen(SCR_MENU_MAIN);	
}

void GetNewObstacle(int N)
{
	int i;
	don_x = random(2);
	if (N==RAND) don_type = random(7); else don_type = N;
	don_h = don_h_mas[don_type];
	don_y = -don_h;
	don_image_y = 0;
	for (i = 0; i < don_type; i++) don_image_y += don_h_mas[i]+2; //calculate image y offset for current obstacle
}

#define LINE_LENGTH 10
int line_y=0;
void DrawLineSeparator()
{
	int y;
	if (screen_type == SCR_GAME) line_y += don_step_y;
	//the beginning of the white dashed line between two roadways
	if (line_y>=20) {
		line_y=0;
	}
	else 
	{ 
		DrawBar(258, 0, 2, line_y, COLOR_ROAD); 
		DrawBar(258, 0, 2, line_y-LINE_LENGTH, 0xDDE9F2); 
	}
	for (y=0; y<WIN_Y-20; y+=20) //white dashed line between two roadways
	{
		DrawBar(258, line_y+y, 2, LINE_LENGTH, 0xDDE9F2);
		DrawBar(258, line_y+y+LINE_LENGTH, 2, LINE_LENGTH, COLOR_ROAD); 
	}
}

void DrawRoad()
{
	DrawLineSeparator();
	DrawObstacle(don_w+10*don_x+186,don_y);
	DrawBus(bus_x*80+200,bus_y);
}





stop:
