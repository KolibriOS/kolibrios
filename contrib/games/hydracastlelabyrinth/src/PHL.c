#include "PHL.h"
#include <stdio.h>
#include <string.h>
#include "qda.h"
#include "game.h"

int WHITE, RED, YELLOW;

void PHL_Init()
{	
	PHL_GraphicsInit();
	PHL_AudioInit(); // DBG

	#ifdef _3DS
		Result rc = romfsInit();
		/*if (rc) {
			printf("romfsInit: %08lX\n", rc);
			//while(1){}
		}
		else
		{
			printf("\nromfs Init Successful!\n");
		}*/
	#endif

	WHITE = 0;
	RED = 1;
	YELLOW = 2;
}

void PHL_Deinit()
{
	PHL_AudioClose();
	PHL_GraphicsExit();
	
	#ifdef _3DS
		romfsExit();
	#endif
}

//Extracts bmps from the bmp.qda archive file
PHL_Surface PHL_LoadQDA(char* fname)
{	
	PHL_Surface surf;
	
	int numofsheets = 29;
	
	for (int i = 0; i < numofsheets; i++)
	{
		if (strcmp(fname, (char*)headers[i].fileName) == 0) { //Match found
			//printf("\nMatch Found: %s", fname);
			surf = PHL_LoadBMP(i);
			i = numofsheets; //End search
		}
	}
	
	return surf;
}

void PHL_DrawTextBold(char* txt, int dx, int dy, int col)
{
	int i, cx, cy;
	
	for (i = 0; i < strlen(txt); i++)
	{
		cx = (txt[i] - 32) * 16;
		cy = 32 * col;
		
		while (cx >= 512) {
			cx -= 512;
			cy += 16;
		}
		
		PHL_DrawSurfacePart(dx + (16 * i), dy, cx, cy, 16, 16, images[imgBoldFont]);
	}
}

void PHL_DrawTextBoldCentered(char* txt, int dx, int dy, int col)
{
	if (dy < 640 && dy > -16) {
		int stringW = strlen(txt) * 16;
		
		PHL_DrawTextBold(txt, dx - (stringW / 2), dy, col);
	}
}
