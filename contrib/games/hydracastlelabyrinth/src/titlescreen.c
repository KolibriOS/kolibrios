#include "titlescreen.h"
#include "game.h"
#include <stdio.h>
#include "text.h"

int tempsave = 0;
int cursor = 0;

void titleScreenSetup();

int titleScreenStep();
void titleScreenDraw();

#ifdef EMSCRIPTEN
int titleEMStep()
{
	PHL_MainLoop();
	//Get input
	PHL_ScanInput();
	
	//Titlescreen step
	int result = titleScreenStep();
	
	//Draw titlescreen
	PHL_StartDrawing();
		
	titleScreenDraw();

	if (result != -1)
		PHL_DrawRect(0, 0, 640, 480, PHL_NewRGB(0, 0, 0));

	PHL_EndDrawing();		

	return result;
}
#endif

int titleScreen()
{
	titleScreenSetup();
	
	char loop = 1;
	int result = -1;
	
	while (PHL_MainLoop() && loop == 1)
	{
		//__asm__("int3");	
		//Get input
		PHL_ScanInput();
		
		//Titlescreen step
		result = titleScreenStep();
		
		//Draw titlescreen
		PHL_StartDrawing();
			
		titleScreenDraw();

		if (result != -1) {
			loop = 0;
			//Force screen to black
			PHL_DrawRect(0, 0, 640, 480, PHL_NewRGB(0, 0, 0));
		}
		PHL_EndDrawing();		
	}
	
	return result;	
}

void titleScreenSetup()
{	
	cursor = 0;
	
	//Move cursor if save file exists
	if ( fileExists(savemap) ) {
		cursor = 1;
	}	
	
	//Check if temp save file exists
	tempsave = 0;
	if ( fileExists(savename) ) {
		#ifndef EMSCRIPTEN
		tempsave = 1;
		#endif
		cursor = 1;		
	}
}

int titleScreenStep()
{
	//Move cursor
	if (btnDown.pressed == 1 || btnSelect.pressed == 1) {
		cursor += 1;
		if (cursor > 3) {
			cursor = 0;
		}
		PHL_PlaySound(sounds[sndPi01], 1);
	}
	
	if (btnUp.pressed == 1) {
		cursor -= 1;
		if (cursor < 0) {
			cursor = 3;
		}
		PHL_PlaySound(sounds[sndPi01], 1);
	}
	
	//Selection
	if (btnAccept.pressed == 1 || btnStart.pressed == 1) {
		PHL_PlaySound(sounds[sndOk], 1);
		return cursor;
	}
	
	return -1;
}

void titleScreenDraw()
{
	//Blackdrop
	PHL_DrawRect(0, 0, 640, 480, PHL_NewRGB(0, 0, 0));
	
	if (tempsave == 0) {
		//Title image
		PHL_DrawSurfacePart(168, 72, 0, 0, 304, 168, images[imgTitle01]);
	}else{
		//Save error message
		drawTextCentered(saveError[0], 320, 80);
		drawTextCentered(saveError[1], 320, 80 + 50);
		drawTextCentered(saveError[2], 320, 80 + 96);
	}
	
	//Cursor
	PHL_DrawSurfacePart(228, 264 + (cursor * 32), 4, 176, 184, 32, images[imgTitle01]);
	
	//Text
	PHL_DrawTextBold("NEW GAME", 256, 272, YELLOW);
	PHL_DrawTextBold("LOAD GAME", 248, 304, YELLOW);
	PHL_DrawTextBold("OPTIONS", 264, 336, YELLOW);
	PHL_DrawTextBold("EXIT", 288, 368, YELLOW);
	PHL_DrawTextBold("(C) 2011 E.HASHIMOTO", 160, 410, WHITE);
	PHL_DrawTextBold("GPL2 PTITSEB", 224, 442, WHITE);
}