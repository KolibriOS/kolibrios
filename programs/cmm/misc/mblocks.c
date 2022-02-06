/*
   Memory Blocks for KolibriOS v1.2
        Leency&Veliant Edition
              2008-2020
*/

#define MEMSIZE 1024 * 20
#define ENTRY_POINT #main

#include "..\lib\gui.h"
#include "..\lib\random.h"

#define BTN_CLOSED 0
#define BTN_PRESSED 1
#define BTN_OPEN 2

#define CELL_SIZE 43
#define PANEL_Y CELL_SIZE+4*6 + 4
#define PANEL_H 36
#define WIN_W CELL_SIZE+4*10 + 4
#define WIN_H PANEL_Y+PANEL_H

#define ROWS 6
#define COLS 10
#define COUNT ROWS*COLS

#ifdef LANG_RUS
	#define LABEL_NEW_GAME "Новая игра";
#else
	#define LABEL_NEW_GAME " New game";
#endif

int bitstat[COUNT], bitpict[COUNT];
dword butonsx[COUNT], butonsy[COUNT];
dword firstbit, secondbit;
int count;

void main()
{   
	dword id;

	mem_init();
	NewGame();

	loop() switch(@WaitEvent())
	{
		case evKey:
			//if (@GetKeyScancode()==SCAN_CODE_F2) NewGame();
			break;
			
		case evButton:
			id = @GetButtonID();
			if (id==1) @ExitProcess();
			else if (id==5) NewGame();
			else {
					id -= 100;
					if (bitstat[id] == BTN_CLOSED)
					{
						if (firstbit != 0x0BAD)
						{
							if (secondbit != 0x0BAD)
							{
								if (bitpict[firstbit] == bitpict[secondbit])
									bitstat[firstbit] = bitstat[secondbit] = BTN_OPEN;
								else
									bitstat[firstbit] = bitstat[secondbit] = BTN_CLOSED;
								ReDraw_Game_Button(firstbit);
								ReDraw_Game_Button(secondbit);
								secondbit = 0x0BAD;
								firstbit = id;
								count++;
							} else if (firstbit != id) {
								secondbit = id;
								count++;
							}
						} else {
							firstbit = id;
							count++;
						}
					}
					bitstat[id] = BTN_PRESSED;
					ReDraw_Game_Button(id);
					Draw_Count();
			}
			break;

		case evReDraw:
			sc.get();
			DefineAndDrawWindow(215,100,WIN_W + 9,WIN_H+4+GetSkinHeight(),
				0x34,0xC0C0C0,"Memory Blocks",0);
			Draw_Panel();
			Draw_Game_Pole();
	}
}

void NewGame()
{
	int off;
	int i;

	FOR (i = 0; i < COUNT; i++)
	{
		bitstat[i] = 0;
		bitpict[i] = 0;
	}

	count = 0;
	firstbit = secondbit = 0x0BAD;
	FOR (i = 0; i < COUNT/2; i++)
	{
		do off = random(COUNT); while (bitpict[off] != 0);
		bitpict[off] = i;
		do off = random(COUNT); while (bitpict[off] != 0);
		bitpict[off] = i;
	}
	Draw_Game_Pole();
	Draw_Panel();
}

void Draw_Game_Pole()
{
	int i;
	byte j;
	for (j = 0; j < COLS; j++)	for (i = 0; i < ROWS; i++)
	{
			butonsx[j*ROWS+i] = CELL_SIZE+4 * j + 4; //save coordinates to avoid 
			butonsy[j*ROWS+i] = CELL_SIZE+4 * i + 4; //their recalculation after
			ReDraw_Game_Button(j*ROWS + i);
	}
}

void ReDraw_Game_Button(int id)
{
	dword xx, yy;
	xx = butonsx[id];
	yy = butonsy[id];
	DefineButton(xx, yy, CELL_SIZE, CELL_SIZE, 100 + BT_HIDE + id, 0);
	DrawRectangle3D(xx, yy, CELL_SIZE, CELL_SIZE, 0x94AECE, 0x94AECE);//border
	switch (bitstat[id])
	{
		case BTN_CLOSED:
			DrawRectangle3D(xx + 1, yy + 1, CELL_SIZE-2, CELL_SIZE-2, 0xFFFFFF, 0xDEDEDE);//bump
			DrawBar(xx + 2, yy + 2, CELL_SIZE-3, CELL_SIZE-3, 0xBDC7D6);//background
			return;
		case BTN_PRESSED:
			DrawWideRectangle(xx + 1, yy + 1, CELL_SIZE-1, CELL_SIZE-1, 2, 0x94DB00);//border green
			DrawBar(xx + 3, yy + 3, CELL_SIZE-5, CELL_SIZE-5, 0xFFFfff);//background
			BREAK;
		case BTN_OPEN:
			DrawBar(xx+1, yy+1, CELL_SIZE-1, CELL_SIZE-1, 0xFFFfff);//background
	}
	draw_icon_32(xx+6, yy+6, 0xffFFFfff, bitpict[id]+51); //skip first 51 icons as they are boring for game
}

void Draw_Panel()
{
	DrawBar(0, PANEL_Y, WIN_W, 1, sc.dark);
	DrawBar(0, PANEL_Y+1, WIN_W, 1, sc.light);
	DrawBar(0, PANEL_Y+2, WIN_W, PANEL_H-2, sc.work);
	DefineButton(9, PANEL_Y+5, 102, 26, 5, sc.button);
	WriteText(20, PANEL_Y+11, 0x90, sc.button_text, LABEL_NEW_GAME);
	Draw_Count();
}

void Draw_Count()
{
	EDI = sc.work; //writing a number with bg
	WriteNumber(WIN_W-32, PANEL_Y + 12, 0xD0, sc.work_text, 3<<16, count);
}




stop:
