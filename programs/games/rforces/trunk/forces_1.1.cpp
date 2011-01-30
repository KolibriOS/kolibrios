/* Rocket Forces
 * Filename: rforces.cpp
 * Version 0.1
 * Copyright (c) Serial 2007
 */


#include <stdlib.h>

#include "kosSyst.h"
#include "kosFile.h"
#include "mymath.h"
#include "properties.h"
#include "objects.h"
#include "rforces.h"


const char header[] = GAME_NAME;
cCursor *cursor;
cGun *gun = new cGun;
cCross **crosses = new cCross*[R_COUNT];
cRocket **rockets = new cRocket*[R_COUNT];
cBomb **bombs = new cBomb*[B_COUNT];
cExplode **explodes = new cExplode*[R_COUNT + B_COUNT]; 
cBuilding *house = new cBuilding();
Dword *cur_handle;
int score = 0, health = 100;

struct MouseState
{
	int x, y, lbclick;	
	Dword buttons;
} ms;


void kos_Main()
{
	Dword frame_start, frame_end;
	OnStart();
	Menu();
	for (;;)
	{
		frame_start = kos_GetTime();
		switch (kos_CheckForEvent())
		{
		case 1:
			DrawWindow();
			break;
		case 2:	// key pressed, read it and ignore
			Byte keyCode;
			kos_GetKey(keyCode);
			if (keyCode == 27)
			{
				OnExit();
			}
			break;
		case 3: // button pressed; we have only one button, close
			OnExit();
			break;
		case 6: // событие от мыши (нажатие на кнопку мыши или перемещение; сбрасывается при прочтении) 
			OnMouseMove();
			if (ms.lbclick == 1)
			{
				OnLMBClick();
			}
			break;
		default:
			OnMouseMove();
			break;
		}
		DrawBombs();
		DrawRocketsAndCrosses();
		DrawExplodes();
		frame_end = kos_GetTime();
		if (frame_end - frame_start < FRAME_TIME)
		{
			kos_Pause(FRAME_TIME - (frame_end - frame_start));
		}
		if (health <= 0)
		{
			OnExit();
		}
	}
}

void DrawWindow()
{
	kos_WindowRedrawStatus(1);
	kos_DefineAndDrawWindow(10, 40, WINDOW_WIDTH + 8, WINDOW_HEIGHT + 25, 0x33, BG_COLOR, 0, 0, (Dword)header);
	kos_WindowRedrawStatus(2);

	kos_WindowRedrawStatus(1); /// DEL!!!!11

	OnMouseMove();

	// Draw buildings
	for (int i = 20; i < 5 * 50; i += 50)
	{
		house->Draw(i, 467, H_COLOR);
	}
	for (int i = 8 * 50; i < 13 * 50; i += 50)
	{
		house->Draw(i, 467, H_COLOR);
	}

}

void DrawBombs()
{
	for (int i = 0; i < B_COUNT; i++)
	{
		if (bombs[i]->IsEnabled() == 0)
		{
			int rnd;
			rnd = rtlRand() % B_POSSIBILITY;
			if (rnd == 1)
			{
				rnd = 10 + rtlRand() % 620;
				bombs[i]->Enable(rnd, 0, 4, 9, rnd + 2, 0);
			}
		}
		else
		{
			if (bombs[i]->cy > gun->cy + 5)
			{
				health -= 5;
				if (explodes[R_COUNT + i]->IsEnabled() == 1)
				{
					explodes[R_COUNT + i]->Disable(BG_COLOR);
				}
				explodes[R_COUNT + i]->Enable(bombs[i]->cx, bombs[i]->cy);
				bombs[i]->Disable(BG_COLOR);
			}
			else
			{
				bombs[i]->cy += B_SPEED;
				bombs[i]->DrawAngle(bombs[i]->cx, 639, B_COLOR);
			}
		}
	}
}

void DrawRocketsAndCrosses()
{
	double a;
	for (int i = 0; i < R_COUNT; i++)
	{
		if (crosses[i]->IsEnabled() == 1)
		{
			if (sqrt(((long int) (crosses[i]->x - rockets[i]->cx) * (crosses[i]->x - rockets[i]->cx)) + ((long int) (crosses[i]->y - rockets[i]->cy) * (crosses[i]->y - rockets[i]->cy))) < 5)
			{
				if (explodes[i]->IsEnabled() == 1)
				{
					explodes[i]->Disable(BG_COLOR);
				}
				explodes[i]->Enable(crosses[i]->x, crosses[i]->y);
				crosses[i]->Disable(BG_COLOR);
				rockets[i]->Disable(BG_COLOR);
			}
			else
			{
				crosses[i]->Draw(CROSS_COLOR);
				if (rockets[i]->cx - crosses[i]->x == 0)
				{
					a = M_PI / 2;
				}
				else
				{
					a = atan((double)(rockets[i]->cy - crosses[i]->y) / (double)(rockets[i]->cx - crosses[i]->x));
					if (rockets[i]->cx - crosses[i]->x < 0) a += M_PI;
				}
				rockets[i]->cx = round_int(rockets[i]->cx - R_SPEED * cos(a));
				rockets[i]->cy = round_int(rockets[i]->cy - R_SPEED * sin(a));
				rockets[i]->DrawAngle(crosses[i]->x, crosses[i]->y, R_COLOR);
			}
		}
	}
}

void DrawExplodes()
{
	for (int i = 0; i < R_COUNT + B_COUNT; i++)
	{
		if (explodes[i]->IsEnabled() == 1)
		{
			explodes[i]->DrawNext(EXP_COLOR);
			for (int j = 0; j < B_COUNT; j++)
			{
				if ( bombs[j]->IsEnabled() == 1 &&
					 bombs[j]->cx > explodes[i]->cx - explodes[i]->step - 1 && bombs[j]->cx < explodes[i]->cx + explodes[i]->step + 1 &&
					 bombs[j]->cy + 5 > explodes[i]->cy - explodes[i]->step - 1 && bombs[j]->cy + 5 < explodes[i]->cy + explodes[i]->step + 1
					)
				{
					score += B_COUNT + 2;
					if (explodes[R_COUNT + j]->IsEnabled() == 1)
					{
						explodes[R_COUNT + j]->Disable(BG_COLOR);
					}
					explodes[R_COUNT + j]->Enable(bombs[j]->cx, bombs[j]->cy);
					bombs[j]->Disable(BG_COLOR);
				}
			}
		}
	}
}

void OnMouseMove()
{
	Dword old_buttons = ms.buttons;
	kos_GetMouseWindowXY(ms.x, ms.y);
	kos_GetMouseButtonsState(ms.buttons);
	if ((old_buttons & 0x00000001) == 0 && (ms.buttons & 0x00000001) == 1)
	{
		ms.lbclick = 1;
	}
	else
	{
		ms.lbclick = 0;
	}


	kos_WriteTextToWindow(8, 10, 0, TEXT_COLOR, "Population:    %", 16);
	kos_DisplayNumberToWindowBg(health, 3, 79, 10, TEXT_COLOR, BG_COLOR, nbDecimal, false);
	kos_WriteTextToWindow(8, 22, 0, TEXT_COLOR, "Score:", 6);
	kos_DisplayNumberToWindowBg(score, 4, 49, 22, TEXT_COLOR, BG_COLOR, nbDecimal, false);

	if (ms.x >= 0 && ms.x < WINDOW_WIDTH &&  ms.y >= 0 && ms.y < WINDOW_HEIGHT)
	{
		gun->DrawAngle(ms.x, ms.y, G_COLOR);
	}
	
	if (HARDWARE_CURSOR == 0)
	{
		cursor->Draw(ms.x, ms.y, CUR_COLOR);
	}

	/*if (DEBUG == 1)
	{
		kos_DisplayNumberToWindowBg(ms.x, 3, WINDOW_WIDTH - 30, 10, TEXT_COLOR, BG_COLOR, nbDecimal, false);
		kos_DisplayNumberToWindowBg(ms.y, 3, WINDOW_WIDTH - 30, 22, TEXT_COLOR, BG_COLOR, nbDecimal, false);
		kos_DisplayNumberToWindowBg(ms.buttons, 1, WINDOW_WIDTH - 30, 34, TEXT_COLOR, BG_COLOR, nbDecimal, false);
	}*/

}

void OnLMBClick()
{
	if (ms.y < gun->cy - 10)
	{
		double a;
		int j = -1;
		for (int i = 0; i < R_COUNT; i++)
		{
			if (crosses[i]->IsEnabled() == 0)
			{
				if (j >= -1) j = i;
			}
			else if (ms.x > crosses[i]->x - 10 && ms.x < crosses[i]->x + 10 && ms.y > crosses[i]->y - 10 && ms.y < crosses[i]->y + 10)
			{
				j = -2;
				break;
			}
		}
		if (j >= 0)
		{
			if (score > 0) score -= 1;
			crosses[j]->Enable(ms.x, ms.y);
			if (gun->cx - ms.x == 0)
			{
				a = M_PI/2;
			}
			else
			{
				a = atan((double)gun->cy - ms.y / (double) gun->cx - ms.x);
				if (gun->cx - ms.x < 0) a += M_PI;
			}
			rockets[j]->Enable(round_int(gun->cx - 15 * cos(a)) - 2, round_int(gun->cy - 15 * sin(a)) - 5, 3, 6, round_int(gun->cx - 15 * cos(a)), round_int(gun->cy - 15 * sin(a)));
		}
	}
}

void OnRMBClick()
{
}

void ChangeCursor()
{
	Dword *cur = new Dword[1024];
	for (int i = 0; i < 1024; i++)
	{
		cur[i] = 0x00000000;
	}
	if (HARDWARE_CURSOR == 1)
	{
		Dword cur_color = 0xFF000000 | CUR_COLOR;
		cur[0 * 32 + 5] = cur_color;
		cur[1 * 32 + 5] = cur_color;
		cur[2 * 32 + 5] = cur_color;
		cur[2 * 32 + 3] = cur_color;
		cur[2 * 32 + 4] = cur_color;
		cur[2 * 32 + 6] = cur_color;
		cur[3 * 32 + 2] = cur_color;
		cur[4 * 32 + 2] = cur_color;
		cur[5 * 32 + 2] = cur_color;
		cur[5 * 32 + 1] = cur_color;
		cur[5 * 32 + 0] = cur_color;

		cur[5 * 32 + 5] = cur_color;

		cur[8 * 32 + 4] = cur_color;
		cur[8 * 32 + 5] = cur_color;
		cur[8 * 32 + 6] = cur_color;
		cur[8 * 32 + 7] = cur_color;
		cur[9 * 32 + 5] = cur_color;
		cur[10 * 32 + 5] = cur_color;
		cur[7 * 32 + 8] = cur_color;
		cur[6 * 32 + 8] = cur_color;
		cur[5 * 32 + 8] = cur_color;
		cur[5 * 32 + 9] = cur_color;
		cur[5 * 32 + 10] = cur_color;
	}
	cur_handle = kos_LoadMouseCursor(cur, 0x05050002);
	delete[] cur;
	kos_SetMouseCursor(cur_handle);
}

void Menu()
{
	NewGame();
}

void NewGame()
{
	gun->DrawAngle((WINDOW_WIDTH / 2) - 5, WINDOW_HEIGHT - 20, G_COLOR);
}

void OnStart()
{
	if (HARDWARE_CURSOR == 0)
	{
		cursor = new cCursor();
	}
	ChangeCursor();

	gun->Enable((WINDOW_WIDTH / 2) - 10, WINDOW_HEIGHT - 30, 10, 20, (WINDOW_WIDTH / 2) - 5, WINDOW_HEIGHT - 20); 

	for (int i = 0; i < R_COUNT; i++)
	{
		crosses[i] = new cCross();
		rockets[i] = new cRocket();
	}
	for (int i = 0; i < B_COUNT; i++)
	{
		bombs[i] = new cBomb();
	}
	for (int i = 0; i < R_COUNT + B_COUNT; i++)
	{
		explodes[i] = new cExplode();
	}

	rtlSrand(kos_GetTime());

	DrawWindow();
	kos_SetMaskForEvents(39);
}

void OnExit()
{
	kos_WriteTextToWindow(WINDOW_WIDTH / 2 - 35, WINDOW_HEIGHT / 2 - 10, 0, TEXT_COLOR, "Game Over", 9);

	//while(kos_WaitForEvent() != 2);
	kos_Pause(200);

	/*kos_DeleteMouseCursor(cur_handle);
	
	for (int i = 0; i < R_COUNT; i++)
	{
		delete crosses[i];
		delete rockets[i];
	}
	for (int i = 0; i < B_COUNT; i++)
	{
		delete bombs[i];
	}
	for (int i = 0; i < R_COUNT + B_COUNT; i++)
	{
		delete explodes[i];
	}
	delete[] crosses;
	delete[] rockets;
	delete[] bombs;
	delete[] explodes;

	delete gun;
	delete house;
	if (HARDWARE_CURSOR == 0)
	{
		delete cursor;
	}*/

	kos_ExitApp();
}
