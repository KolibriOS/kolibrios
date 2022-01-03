#include "inventory.h"
#include "PHL.h"
#include "game.h"
#include "text.h"

int cursorX = 0;
int cursorY = 0;

#ifdef EMSCRIPTEN
static char tempDark;
void inventorySetup()
{
	tempDark = roomDarkness;
	roomDarkness = 0;
	
	PHL_PlaySound(sounds[sndPi04], CHN_SOUND);
}
int inventoryEMStep()
{
	int result = -1;
	PHL_MainLoop();

	PHL_StartDrawing();
	PHL_ScanInput();
	
	if (inventoryStep() == 1) {
		result = 0;
	}
	
	inventoryDraw();
	
	PHL_EndDrawing();
	
	if(!result)
		roomDarkness = tempDark;
	return result;
}
#else

void inventory()
{
	char tempDark = roomDarkness;
	roomDarkness = 0;
	
	char loop = 1;
	PHL_PlaySound(sounds[sndPi04], CHN_SOUND);
	while (PHL_MainLoop() && loop == 1)
	{
		PHL_StartDrawing();
		PHL_ScanInput();
		
		if (inventoryStep() == 1) {
			loop = 0;
		}
		
		inventoryDraw();
		
		PHL_EndDrawing();
	}
	
	roomDarkness = tempDark;
}
#endif
int inventoryStep()
{		
	secretCountdown();
	
	//Input
	char playsnd = 0;
	
	if (btnRight.pressed - btnLeft.pressed != 0) {
		cursorX += btnRight.pressed - btnLeft.pressed;
		playsnd = 1;
	}
	if (btnDown.pressed - btnUp.pressed != 0) {
		cursorY += btnDown.pressed - btnUp.pressed;
		playsnd = 1;
	}
	
	if (playsnd == 1) {
		PHL_PlaySound(sounds[sndPi01], CHN_SOUND);
	}
	
	//Limit cursor
	if (cursorX < 0) { cursorX = 6; }
	if (cursorX > 6) { cursorX = 0; }
	if (cursorY < 0) { cursorY = 3; }
	if (cursorY > 3) { cursorY = 0; }

	if (btnStart.pressed == 1 || btnDecline.pressed == 1)
	{		
		return 1;
	}
	
	return 0;
}

void inventoryDraw()
{		
	//Black background
	PHL_DrawRect(0, 0, 640, 480, PHL_NewRGB(0, 0, 0));
	
	//Labels
	PHL_DrawTextBold("SUB WEAPON", 16, 16, YELLOW);
	PHL_DrawTextBold("ITEM", 16, 96, YELLOW);
	PHL_DrawTextBold("KEY", 16, 320, YELLOW);
	
	//Blue rectangles
	int i, a, cx, cy;
	//Weapons
	for (i = 0; i < 5; i++) {
		PHL_DrawRect(18 + (48 * i), 34, 44, 44, PHL_NewRGB(255, 255, 255));
		PHL_DrawRect(20 + (48 * i), 36, 40, 40, PHL_NewRGB(119, 166, 219));
		if (hasWeapon[i] == 1) {
			cx = (i + 1) * 40;
		}else{
			cx = 0;
		}
		PHL_DrawSurfacePart(20 + (48 * i), 36, cx, 0, 40, 40, images[imgItems]);
	}
	//Items
	int count = 0;
	int imageOrder[28] = { 13, 17, 16, 15, 8, 10, 9,
						   18, 4, 6, 5, 7, 3, 11,
						   14, 12, 1, 2, 22, 26, 27,
						   21, 25, 28, 23, 20, 19, 24 };
					   
	for (i = 0; i <  4; i++) {
		for (a = 0; a < 7; a++) {
			PHL_DrawRect(18 + (48 * a), 114 + (48 * i), 44, 44, PHL_NewRGB(255, 255, 255));
			PHL_DrawRect(20 + (48 * a), 116 + (48 * i), 40, 40, PHL_NewRGB(119, 166, 219));
			
			if (hasItem[count] == 0) {
				cx = 0;
				cy = 0;
			}else{
				cy = 0;
				cx = (imageOrder[count] + 6) * 40;
				while (cx >= 640) {
					cy += 40;
					cx -= 640;
				}
			}
			
			PHL_DrawSurfacePart(20 + (48 * a), 116 + (48 * i), cx, cy, 40, 40, images[imgItems]);
			count++;
		}
	}
	//Keys
	for (i = 0; i < 8; i++) {
		PHL_DrawRect(18 + (48 * i), 338, 44, 44, PHL_NewRGB(255, 255, 255));
		PHL_DrawRect(20 + (48 * i), 340, 40, 40, PHL_NewRGB(119, 166, 219));
		if (hasKey[i] == 1) {
			cx = 120 + (i * 40);
			cy = 80;
		}else{
			cx = 0;
			cy = 0;
		}
		PHL_DrawSurfacePart(20 + (48 * i), 340, cx, cy, 40, 40, images[imgItems]);
	}
	
	//Text box
	PHL_DrawRect(16, 400, 606, 46, PHL_NewRGB(255, 255, 255));
	PHL_DrawRect(18, 402, 602, 42, PHL_NewRGB(0, 0, 255));
	
	//Text
	if (hasItem[cursorX + (cursorY * 7)] == 1) {
		int drawX = 32, drawY = 402;
		
		//Draw item name
		drawX = drawText(itemName[cursorX + (cursorY * 7) + 5], drawX, drawY);
		//Draw item description
		drawX = drawCharacter(6, 0, drawX, drawY);
		drawText(itemDescription[cursorX + (cursorY * 7)], drawX, drawY);
	}
	
	//Cursor
	PHL_DrawSurfacePart(16 + (cursorX * 48), 112 + (cursorY * 48), 0, 96, 48, 48, images[imgHud]);
}