#include "game.h"
#include "hero.h"
#include "PHL.h"
#include "qda.h"
#include "ini.h"
#include "titlescreen.h"
#include "options.h"
#include "inventory.h"
#include "object.h"
#include "effect.h"
#include "text.h"
#include "stagedata.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

int gameStep();
void gameDraw(char doDrawHud);

void freeArrays();
void drawHud();
int getTileType(int valx, int valy);

void loadUncommonImages();

char forceGameExit = 0;

int drawhp;
int NumOfSounds = 43;
int NumOfImages = 14;

char autoSave = 1;
int levelStartFlag = 0;

char tilesetStrings[9][12] = {"stage01.bmp",
							  "stage02.bmp",
							  "stage02.bmp",
							  "stage03.bmp",
							  "stage04.bmp",							
							  "stage03.bmp",
							  "stage02.bmp",							  
							  "stage05.bmp",							  
							  "stage08.bmp" };
							
char musicStrings[9][14] = { "midi/main01",
							 "midi/main02",
							 "midi/main02",
							 "midi/main05",
							 "midi/main03",							 
							 "midi/main05",							 
							 "midi/main02",							 
							 "midi/main04",							 
							 "midi/main06" };
							
int collisionTiles[16][12];
PHL_Surface images[15];
PHL_Sound	sounds[43];
PHL_Music 	bgmMusic;
PHL_Music	bgmSecret;
PHL_Music	bgmGameover;
Object* objects[MAX_OBJECTS];
Effect* effects[MAX_EFFECTS];
Weapon* weapons[MAX_WEAPONS];
Enemy* enemies[MAX_ENEMIES];
Platform* platforms[MAX_PLATFORMS];

int secretTimer;
unsigned long playTime;

Door* lastDoor;
int quakeTimer;
int bellFlag;
int bossFlag;
int bossDefeatedFlag;
int roomSecret;
char roomDarkness;

int itemGotX;
int itemGotY;

PHL_Background background, foreground;

unsigned char hasWeapon[5];
unsigned char hasItem[28];
unsigned char hasKey[8];

unsigned char flags[60];

double cutInTimer = 240;
int transitionTimer = 0;
int level = 0;
int screenX = 5,
	screenY = 2;

#ifdef _SDL
char savename[4096];
char savemap[4096];
#endif

#ifdef EMSCRIPTEN
extern int fileSynched;
int em_state = -2;
void em_loop_fn(void* arg)
{
	if(!PHL_MainLoop()) {
        emscripten_cancel_main_loop();
	}

	int result;
	switch (em_state) {
		case -2: if(fileSynched) em_state++;
				 break;
		case -1: em_state++;
				// need to delay loading of init to let synchof files happens
				iniInit();
				//Load Resources
				loadText();	
				loadResources();
				 break;
		case 0:	
				PHL_StopMusic();
				titleScreenSetup();
				++em_state;
				break;
		case 1: result = titleEMStep();
				if (result == 3) {
					em_state = 100;
				} else if(result!=-1) {
					if(result==2) 
						em_state = 60;
					else {
						//Reset game state
						gameSetup();
						
						//Load Game
						if (result == 1)
						{
							if (fileExists(savename) == 1) {
								loadSave(savename);
							}else if (fileExists(savemap) == 1) {
								loadSave(savemap);					
							}
						}
						++em_state;
					}
				}
				break;
		case 2:
				//Update resources, depending on level
				loadUncommonImages();
				PHL_FreeSurface(images[imgTiles]);
				images[imgTiles] = PHL_LoadQDA(tilesetStrings[level]);
				
				PHL_FreeMusic(bgmMusic);
				bgmMusic = PHL_LoadMusic(musicStrings[level], 1);

				loadScreen();
				em_state = 10;
				break;
		case 10:	// main game loop
				PHL_MainLoop();
				PHL_ScanInput();	
				result = gameStep();
				if(!result)
					em_state = 20;
				else {
					if(result!=-1)
						em_state = result;
					else {
						PHL_StartDrawing();					
						gameDraw(1);					
						PHL_EndDrawing();
					}
				}
				break;
		case 20:	// game ended
			roomDarkness = 0;	
			freeArrays();
			
			//Erase temp save if it exists
			if (fileExists(savename))
			{
				remove(savename);
				#ifdef EMSCRIPTEN
				EM_ASM(
					FS.syncfs(false,function () {
						Module.print("File sych'd")
					});
				);
				#endif
			}
			em_state = 0;
			break;
		case 30:	// option menu
				optionsSetup(0);
				++em_state;
				// fall thru
		case 31:
				result = optionsEMStep();
				
				//Reset Game
				if (result == 1)
					em_state = 20;
				else if (result == 3) {
					em_state = 100;
				} else if (result!=-1)
					em_state = 10;
				break;
		case 40:
				inventorySetup();
				++em_state;
		case 41:
				result = inventoryEMStep();
				if(result==0)
					em_state = 10;
				break;
		case 50:
				result = getItemEMStep();
				if(result==0)
					em_state = 10;
				break;
		case 60:	// option menu
				optionsSetup(1);
				++em_state;
				// fall thru
		case 61:
				result = optionsEMStep();
				
				if (result!=-1)
					em_state = 0;
				break;
		case 100:
				/*
				//Free Resources
				textFree();
				freeResources();
				
				//Deinit services
				PHL_Deinit();
				// end
				emscripten_cancel_main_loop();
				*/
				em_state = 0;	// no quitting, as it make no sense in a browser (just kill the tab)
				break;
	}

}
#endif

void game()
{
#ifdef _SDL
	#if defined(__amigaos4__) || defined(__MORPHOS__)
	const char* home = "PROGDIR:";
	#elif defined(EMSCRIPTEN)
	const char* home = "hcl_data/";
	#elif defined(_KOLIBRI)
	const char* home = KOS_TMP_DIR;
	#else
	const char* home = getenv("HOME");
	#endif
	if(home)
	{
		strcpy(savename, home);
		#if defined(__amigaos4__) || defined(__MORPHOS__)
		strcat(savename, ".hydracastlelabyrinth/");
		#elif !defined(EMSCRIPTEN)
		strcat(savename, "/.hydracastlelabyrinth/");
		#endif
		strcpy(savemap, savename);
		strcat(savename, "save.tmp");
		strcat(savemap, "save.map");
	} else {
		strcpy(savename, "data/save.tmp");
		strcpy(savemap, savemap);
	}
#endif
	//Setup services
	printf("DBG:0\n");
	PHL_Init();
	if(1!=initQDA())
	{
		printf("DBG:QDA FAILED\n");
	}
	printf("DBG: 1\n");	
	textInit();
	printf("DBG: 2\n");
	#ifdef EMSCRIPTEN
	emscripten_set_main_loop_arg(em_loop_fn, NULL, -1, 1);
	#else
	iniInit();
	printf("DBG: 3\n");
	//Load Resources
	loadText();	
	printf("DBG: 4\n");
	loadResources();
	printf("DBG: 5\n");


	while (PHL_MainLoop())
	{		
		//Titlescreen
		int titleScreenResult = titleScreen();
		printf("DBG: titleScreen()\n");
		//Exit game
		if (titleScreenResult == 3) {
			PHL_GameQuit();
		}

		// Options
		else if(titleScreenResult == 2) {
			int optionsResult = options(1);
			
			//Exit Game
			if (optionsResult == 3) {
				PHL_GameQuit();
			}
		}
		//Game Start
		else{
			//Reset game state
			gameSetup();
			
			//Load Game
			if (titleScreenResult == 1)
			{
				if (fileExists(savename) == 1) {
					loadSave(savename);
				}else if (fileExists(savemap) == 1) {
					loadSave(savemap);					
				}
			}
			
			//Update resources, depending on level
			loadUncommonImages();
			
			/*printf("\nTiles are ");
			if (images[imgTiles].pxdata == NULL) {
				printf("not loaded.");
			}else{
				printf("loaded.");
			}*/
			PHL_FreeSurface(images[imgTiles]);
			images[imgTiles] = PHL_LoadQDA(tilesetStrings[level]);
			
			PHL_FreeMusic(bgmMusic);
			bgmMusic = PHL_LoadMusic(musicStrings[level], 1);

			loadScreen();
			
			//In game main loop
			char gameLoop = 1;
			
			while (PHL_MainLoop() == 1 && gameLoop == 1) {
				PHL_ScanInput();
				
				int gameResult = gameStep();
				
				if (gameResult != -1) {
					gameLoop = 0;
				}
				
				if (gameLoop == 1) {
					PHL_StartDrawing();					
					gameDraw(1);					
					PHL_EndDrawing();
				}
			}
			
			//Game end (return to titlescreen)
			roomDarkness = 0;	
			freeArrays();
			
			//Erase temp save if it exists
			if (fileExists(savename))
			{
				#ifdef _SDL
				remove(savename);
				#else
				char fullPath[128];
				strcpy(fullPath, "");
				#ifdef _3DS
				strcat(fullPath, "sdmc:/3ds/appdata/HydraCastleLabyrinth/");
				#endif
				strcat(fullPath, savename);
				remove(fullPath);
				#endif
				#ifdef EMSCRIPTEN
				EM_ASM(
					FS.syncfs(false,function () {
						Module.print("File sych'd")
					});
				);
				#endif
			}
		}
	}
	
	//Free Resources
	textFree();
	freeResources();
	
	//Deinit services
	PHL_Deinit();
	#endif
}

void loadImages()
{
	images[imgTiles] 		= PHL_LoadQDA(tilesetStrings[level]);
	images[imgEnemies] 		= PHL_LoadQDA("ene01.bmp");
	images[imgHud] 			= PHL_LoadQDA("status.bmp");
	images[imgMisc20] 		= PHL_LoadQDA("chr20.BMP");
	images[imgMisc32] 		= PHL_LoadQDA("chr32.BMP");
	images[imgHero] 		= PHL_LoadQDA("mychr.bmp");
	images[imgItems] 		= PHL_LoadQDA("items.bmp");
	images[imgExplosion] 	= PHL_LoadQDA("chr64.BMP");
	images[imgBoss] 		= PHL_LoadQDA("boss01.bmp");
	//images[imgMisc2040] 	= PHL_LoadQDA("chr20x40.BMP");
	images[imgFontKana] 	= PHL_LoadQDA("font8x8-kana.bmp");
	images[imgBoldFont] 	= PHL_LoadQDA("font8x8-01.bmp");
	//images[imgDark] 		= PHL_LoadQDA("dark.bmp");
	//images[imgMisc6020] 	= PHL_LoadQDA("chr60x20.bmp");
	//images[imgHud].colorKey = PHL_NewRGB(0, 0, 0);
	//PHL_SetColorKey(images[imgHud], 0, 0, 0);
	images[imgTitle01] 		= PHL_LoadQDA("title01.BMP");
}
	
void loadResources()
{	
	//Loading Images
	loadImages();
	puts("DBG loadResources1");
	//Load Sounds	
	sounds[sndBee01] 	= PHL_LoadSound("wav/bee01.wav");	
	sounds[sndBell01] 	= PHL_LoadSound("wav/bell01.wav");
	sounds[sndBom01] 	= PHL_LoadSound("wav/bom01.wav");
	sounds[sndBom02] 	= PHL_LoadSound("wav/bom02.wav");
	sounds[sndBom03] 	= PHL_LoadSound("wav/bom03.wav");
	sounds[sndDoor00] 	= PHL_LoadSound("wav/door00.wav");
	sounds[sndFire01] 	= PHL_LoadSound("wav/fire01.wav");
	sounds[sndGas01] 	= PHL_LoadSound("wav/gas01.wav");
	sounds[sndGet01] 	= PHL_LoadSound("wav/get01.wav");
	sounds[sndGet02] 	= PHL_LoadSound("wav/get02.wav");
	sounds[sndHit01] 	= PHL_LoadSound("wav/hit01.wav");
	sounds[sndHit02] 	= PHL_LoadSound("wav/hit02.wav");
	sounds[sndHit03] 	= PHL_LoadSound("wav/hit03.wav");
	sounds[sndHit04] 	= PHL_LoadSound("wav/hit04.wav");
	sounds[sndHit05] 	= PHL_LoadSound("wav/hit05.wav");
	sounds[sndHit06] 	= PHL_LoadSound("wav/hit06.wav");
	sounds[sndHit07] 	= PHL_LoadSound("wav/hit07.wav");	
	sounds[sndJump01] 	= PHL_LoadSound("wav/jump01.wav");
	sounds[sndJump02] 	= PHL_LoadSound("wav/jump02.wav");
	sounds[sndNg] 		= PHL_LoadSound("wav/ng.wav");
	sounds[sndOk]	 	= PHL_LoadSound("wav/ok.wav");
	sounds[sndPi01] 	= PHL_LoadSound("wav/pi01.wav");
	sounds[sndPi02] 	= PHL_LoadSound("wav/pi02.wav");
	sounds[sndPi03] 	= PHL_LoadSound("wav/pi03.wav");
	sounds[sndPi04] 	= PHL_LoadSound("wav/pi04.wav");
	sounds[sndPi05] 	= PHL_LoadSound("wav/pi05.wav");
	sounds[sndPi06] 	= PHL_LoadSound("wav/pi06.wav");
	sounds[sndPi07] 	= PHL_LoadSound("wav/pi07.wav");
	sounds[sndPi08] 	= PHL_LoadSound("wav/pi08.wav");
	sounds[sndPi09] 	= PHL_LoadSound("wav/pi09.wav");
	sounds[sndPi10] 	= PHL_LoadSound("wav/pi10.wav");
	sounds[sndPower01] 	= PHL_LoadSound("wav/power01.wav");
	sounds[sndPower02] 	= PHL_LoadSound("wav/power02.wav");
	sounds[sndShot01]  	= PHL_LoadSound("wav/shot01.wav");
	sounds[sndShot02]  	= PHL_LoadSound("wav/shot02.wav");
	sounds[sndShot03]  	= PHL_LoadSound("wav/shot03.wav");
	sounds[sndShot04]  	= PHL_LoadSound("wav/shot04.wav");
	sounds[sndShot05]  	= PHL_LoadSound("wav/shot05.wav");
	sounds[sndShot06]  	= PHL_LoadSound("wav/shot06.wav");
	sounds[sndShot07]  	= PHL_LoadSound("wav/shot07.wav");
	sounds[sndStep01]  	= PHL_LoadSound("wav/step01.wav");
	sounds[sndWater01] 	= PHL_LoadSound("wav/water01.wav");
	sounds[sndWolf01]  	= PHL_LoadSound("wav/wolf01.wav");
	puts("DBG loadResources2");
	//Load Music
	bgmSecret = PHL_LoadMusic("midi/nazo", 0);
	puts("DBG loadResources3");
	bgmGameover = PHL_LoadMusic("midi/gameover", 0);
	puts("DBG loadResources4");
}

void freeImages()
{
	int i;

	//Free graphics
	for (i = 0; i < NumOfImages; i++) {
		PHL_FreeSurface(images[i]);
	}
}

void freeResources()
{
	//Free sounds
	int i;
	for (i = 0; i < NumOfSounds; i++) {
		PHL_FreeSound(sounds[i]);
	}
	
	//Free Music
	PHL_FreeMusic(bgmMusic);
	PHL_FreeMusic(bgmGameover);
	PHL_FreeMusic(bgmSecret);

	freeImages();
}

void gameSetup()
{	
	//Reset Flags
	{
		quakeTimer = 0;
		secretTimer = 0;
		roomDarkness = 0;
		
		bellFlag = 0;
		bossFlag = 0;
		bossDefeatedFlag = 0;
		
		int i;
		for (i = 0; i < 60; i++) {
			flags[i] = 0;
		}
	}
	
	//Save Data
	{
		playTime = 0;
		
		//Inventory	
		int i;
		for (i = 0; i < 5; i++) {
			hasWeapon[i] = 0;
		}
		
		for (i = 0; i < 28; i++) {
			hasItem[i] = 0;
		}
		
		for (i = 0; i< 8; i++) {
			hasKey[i] = 0;
		}
	}
		
	//Room Data
	{
		roomSecret = 0;
		level = 0;
		screenX = 5;
		screenY = 2;
	}
	
	//Hero Setup
	{
		heroSetup();
		drawhp = herohp;
	}	
	
	//Reset object arrays
	freeArrays();
	
	//Setup screen transition
	cutInTimer = 240;
}

int gameStep()
{	
	//Manage Timers
	{
		playTime += 1;
		
		secretCountdown();
			
		if (quakeTimer > 0) {
			quakeTimer -= 1;
		}
		
		if (cutInTimer > 0) {
			cutInTimer -= 5;
			
			//Play music when the transition ends
			if (cutInTimer <= 0 && bossDefeatedFlag == 0 && bossFlag == 0) {
				PHL_PlayMusic(bgmMusic);
			}
		}
	}
	
	//Hero step
	{
		//End game if hero died
		if (heroStep() == 1) {		
			return 0;
		}
	}
	
	//Menu button presses
	{
		if (getHeroState() <= 5 && cutInTimer <= 0) {
			if (btnSelect.pressed == 1)
			{
				#ifdef EMSCRIPTEN
				optionsSetup(0);
				return 31;
				#else						
				int optionsResult = options(0);
				
				//Reset Game
				if (optionsResult == 1) {
					return 0;
				}
				//Exit Game
				if (optionsResult == 3) {
					PHL_GameQuit();
					return 1;
				}
				#endif
			}else if (btnStart.pressed == 1) {	
				#ifdef EMSCRIPTEN
				return 40;
				#else
				inventory();
				#endif
			}
		}
	}
	
	//Objects steps
	{
		int i;
		for (i = 0; i < MAX_PLATFORMS; i++) {
			if (platforms[i] != NULL) {
				platformStep(platforms[i]);
			}
		}
		
		for (i = 0; i < MAX_OBJECTS; i++) {
			if (objects[i] != NULL) {
				objects[i]->objectStep(objects[i]->data);
			}
		}
		
		for (i = 0; i < MAX_WEAPONS; i++) {
			if (weapons[i] != NULL) {
				weaponStep(weapons[i]);
			}
		}
		
		for (i = 0; i < MAX_EFFECTS; i++) {
			if (effects[i] != NULL) {
				effectStep(effects[i]);
			}
		}

		for (i = 0; i < MAX_ENEMIES; i++) {
			if (enemies[i] != NULL) {
				enemies[i]->enemyStep(enemies[i]->data);
			}
		}
	}
	
	if (forceGameExit == 1) {
		forceGameExit = 0;
		return 0;
	}
	
	return -1;
}

void gameDraw(char doDrawHud)
{	
	PHL_DrawBackground(background, foreground);
	
	int i;
	//Draw water/lava top effects
	for (i = 0; i < MAX_EFFECTS; i++) {
		if (effects[i] != NULL) {
			if (effects[i]->depth == -1) {
				effectDraw(effects[i]);
			}
		}
	}
	
	for (i = 0; i < MAX_PLATFORMS; i++) {
		if (platforms[i] != NULL) {
			platformDraw(platforms[i]);
		}
	}
	
	for (i = 0; i < MAX_OBJECTS; i++) {
		if (objects[i] != NULL) {
			objects[i]->objectDraw(objects[i]->data);
		}
	}
	
	for (i = 0; i < MAX_WEAPONS; i++) {
		if (weapons[i] != NULL) {
			weaponDraw(weapons[i]);
		}
	}	
	
	//Draw effects under
	for (i = 0; i < MAX_EFFECTS; i++) {
		if (effects[i] != NULL) {
			if (effects[i]->depth == 0) {
				effectDraw(effects[i]);
			}
		}
	}
	
	//Draw enemies backwards, so bullets and such are underneath their spawners
	//for (i = MAX_ENEMIES - 1; i >= 0; i--) {
	for (i = 0; i < MAX_ENEMIES; i++) {
		if (enemies[i] != NULL) {
			enemies[i]->enemyDraw(enemies[i]->data);
		}
	}
	
	//Not Death, draw death later
	if (getHeroState() != 8) {
		heroDraw();
	}
	
	//Draw effects over
	for (i = 0; i < MAX_EFFECTS; i++) {
		if (effects[i] != NULL) {
			if (effects[i]->depth == 1) {
				effectDraw(effects[i]);
			}
		}
	}
	
	//Draw Darkness
	if (roomDarkness == 1) {
		int cornerX = herox - 160,
			cornerY = heroy + 20 - 160;
		
		PHL_DrawSurfacePart(cornerX, cornerY, 320 * hasItem[18], 0, 320, 320, images[imgDark]);
		
		//Top darkness rectangle
		if (cornerY > 0) {
			PHL_DrawRect(0, 0, 640, cornerY, PHL_NewRGB(10, 0, 0));
		}
		//Bottom darkness rectangle
		if (cornerY + 320 < 480) {
			PHL_DrawRect(0, cornerY + 320, 640, 480, PHL_NewRGB(10, 0, 0));
		}
		
		//Left rectangle
		if (cornerX > 0) {
			PHL_DrawRect(0, cornerY, cornerX, 320, PHL_NewRGB(10, 0, 0));
		}
		//Right rectangle
		if (cornerX + 320 < 640) {
			PHL_DrawRect(cornerX + 320, cornerY, 640 - cornerX + 320, 320, PHL_NewRGB(10, 0, 0));
		}		
	}
	
	//Draw death over darkness
	if (getHeroState() == 8) {
		heroDraw();
	}
	
	if (doDrawHud == 1) {
		drawHud();
	}
	
	//cut-in transition
	{
		if (cutInTimer > 0)	{
			PHL_DrawRect(0, 0, 640, cutInTimer, PHL_NewRGB(0, 0, 0));
			PHL_DrawRect(0, 240 + (240 - cutInTimer), 640, 480, PHL_NewRGB(0, 0, 0));		
		}
	}
	
}
#ifdef EMSCRIPTEN
static int em_itemNum;
static char getItemTimer;
void getItemSetup(int itemNum)
{
	setHeroState(6);
	setHeroImageIndex(0);
	
	char getItemTimer = 0;
}
int getItemEMStep()
{
	int itemNum = em_itemNum;
	char loop = 1;
	PHL_MainLoop();
#else
void getItem(int itemNum)
{
	setHeroState(6);
	setHeroImageIndex(0);
	
	char getItemTimer = 0;
	char loop = 1;
	
	while (PHL_MainLoop() && loop == 1)
#endif
	{
		secretCountdown();
		//Get Item Step		
		if (getItemTimer == 0) {
			setHeroImageIndex(getHeroImageIndex() + 0.3);
			if (getHeroImageIndex() > 3) {
				getItemTimer = 1;
			}
		}else if (getItemTimer == 1) {
			//Wait for input
			PHL_ScanInput();
			
			if (btnAccept.pressed == 1 || btnFaceDown.pressed == 1 || btnFaceRight.pressed == 1 ||
				btnFaceUp.pressed == 1 || btnFaceLeft.pressed == 1 || btnStart.pressed == 1) {
				getItemTimer = 2;
			}
		}else if (getItemTimer == 2) {
			setHeroImageIndex(getHeroImageIndex() + 0.3);
			if (getHeroImageIndex() >= 7) {
				loop = 0;
				
				setHeroState(0);
				setHeroImageIndex(0);
			}
		}		
		
		//Get Item Draw
		{
			PHL_StartDrawing();
			
			gameDraw(1);
			
			if (getHeroImageIndex() >= 3) {
				char tempDarkness = roomDarkness;
				roomDarkness = 0;
				
				PHL_DrawRect(140, 208, 360, 64, PHL_NewRGB(255, 255, 255));
				PHL_DrawRect(142, 210, 356, 60, PHL_NewRGB(0, 20, 0));
				
				PHL_DrawRect(148, 216, 48, 48, PHL_NewRGB(255, 255, 255));
				PHL_DrawRect(152, 220, 40, 40, PHL_NewRGB(119, 166, 219));
				//Image
				PHL_DrawSurfacePart(152, 220, itemGotX, itemGotY, 40, 40, images[imgItems]);
				//Text
				{
					int drawX = 196, drawY = 216;
					int twoLayers = 0;
					if (itemName[itemNum]->length + found->length + 2 > 17) {
						twoLayers = 1;
						drawY -= 8;
					}
					drawX = drawCharacter(17, 2, drawX, drawY);
					drawX = drawText(itemName[itemNum], drawX, drawY);
					drawX = drawCharacter(18, 2, drawX, drawY);
					if (twoLayers == 1) {
						drawX = 204;
						drawY += 24;
					}
					drawText(found, drawX, drawY);
				}
				
				roomDarkness = tempDarkness;
			}
						
			PHL_EndDrawing();
		}
	}	
#ifdef EMSCRIPTEN
	return loop;
#endif
}

void saveScreen()
{
	PHL_PlaySound(sounds[sndPower02], CHN_SOUND);
	herohp = maxhp;
	setHeroHsp(0);
	
	int saveTimer = 60;
	char loop = 1;
	
	while (PHL_MainLoop() && loop == 1)
	{
		PHL_StartDrawing();

		gameDraw(1);
		
		PHL_DrawRect(140, 208, 360, 64, PHL_NewRGB(255, 255, 255));
		PHL_DrawRect(142, 210, 356, 60, PHL_NewRGB(0, 0, 255));
		drawTextCentered(saving, 320, 216);
		
		saveTimer -= 1;
		if (saveTimer <= 0) {
			loop = 0;
		}
		
		PHL_EndDrawing();
	}
	
	if (writeSave(savemap) == 1)
	{
		if (fileExists(savename))
		{
			char fullPath[128];
			strcpy(fullPath, "");
			#ifdef _3DS
				strcat(fullPath, "sdmc:/3ds/appdata/HydraCastleLabyrinth/");
			#endif
			strcat(fullPath, savename);
			remove(fullPath);
			#ifdef EMSCRIPTEN
			EM_ASM(
				FS.syncfs(false,function () {
					Module.print("File sych'd")
				});
			);
			#endif
		}
	}
}

//Result screen and credits
void gameEnding()
{
	int timer = 0;
	char exitLoop = 0;
	
	//Result screen
	{
		PHL_StopMusic();
		PHL_FreeMusic(bgmMusic);
		bgmMusic = PHL_LoadMusic("midi/allclear", 0);
		PHL_PlayMusic(bgmMusic);
		
		//Calculate completion percentage
		char treasureString[11];
		{
			int itemCount = 0;
			int ALLITEMS = 41;
			
			int i;
			for (i = 0; i < 5; i++) {
				itemCount += hasWeapon[i];
			}
			
			for (i = 0; i < 28; i++) {
				itemCount += hasItem[i];
			}
			
			for (i = 0; i < 8; i++) {
				itemCount += hasKey[i];
			}
			
			sprintf(treasureString, "%d%%", itemCount * 100 / ALLITEMS);
		}
		
		//Calculate time		
		char timeString[12];
		{
			int hours = playTime / 216000;
			int minutes = (playTime % 216000) / 3600;
			int seconds = ((playTime % 216000) % 3600) / 60;
			
			sprintf(timeString, "%02d:%02d:%02d", hours, minutes, seconds);
		}		
		
		int transTimer = 0;
		
		while (PHL_MainLoop() && exitLoop == 0)
		{
			timer += 1;
			if (timer >= 500) {
				transTimer += 8;
			}
			
			if (transTimer >= 360) {
				exitLoop = 1;
			}
			
			//Animate Effects
			int i;
			for (i = 0; i < MAX_EFFECTS; i++) {
				if (effects[i] != NULL) {
					effectStep(effects[i]);
				}
			}
			
			PHL_StartDrawing();
			
			gameDraw(0);
			
			PHL_DrawTextBoldCentered("--- ALL CLEAR! ---", 320, 64, YELLOW);
			
			PHL_DrawTextBoldCentered("TIME", 320, 128, YELLOW);
			PHL_DrawTextBoldCentered(timeString, 320, 144, WHITE);
			
			PHL_DrawTextBoldCentered("TREASURE", 320, 192, YELLOW);
			PHL_DrawTextBoldCentered(treasureString, 320, 208, WHITE);
			
			//transition
			if (transTimer > 0)	{
				PHL_DrawRect(0, 0, 640, transTimer, PHL_NewRGB(0, 0, 0));
				PHL_DrawRect(0, 240 + (240 - transTimer), 640, 480, PHL_NewRGB(0, 0, 0));		
			}
			
			PHL_EndDrawing();			
		}
	}

	//Credits
	{
		timer = 0;
		exitLoop = 0;
		
		PHL_StopMusic();
		PHL_FreeMusic(bgmMusic);
		bgmMusic = PHL_LoadMusic("midi/ending", 0);
		PHL_PlayMusic(bgmMusic);
		
		int timer = 0;
		double viewY = 0;
		int maxViewY = 2200;
		double imageIndex = 0;
		
		while (PHL_MainLoop() && exitLoop == 0)
		{			
			timer += 1;
			if (timer >= 2220) {
				exitLoop = 1;
			}
			
			viewY += 1;
			if (viewY >= maxViewY - 480) {
				viewY = maxViewY - 480;
			}
			
			imageIndex += 0.1;
			if (imageIndex >= 2) {
				imageIndex -= 2;
			}
			
			PHL_StartDrawing();
			
			PHL_DrawRect(0, 0, 640, 480, PHL_NewRGB(0, 0, 0));
			
			if (exitLoop == 0) {
				PHL_DrawTextBoldCentered("- STAFF -", 320, 480 - viewY, YELLOW);
				
				PHL_DrawTextBoldCentered("SPRITES", 320, 560 - viewY, YELLOW);
				PHL_DrawTextBoldCentered("BUSTER", 320, 576 - viewY, WHITE);
				
				PHL_DrawTextBoldCentered("PROGRAM", 320, 640 - viewY, YELLOW);
				PHL_DrawTextBoldCentered("BUSTER", 320, 656 - viewY, WHITE);
				
				PHL_DrawTextBoldCentered("MUSIC", 320, 720 - viewY, YELLOW);
				PHL_DrawTextBoldCentered("MATAJUUROU", 320, 736 - viewY, WHITE);
				
				PHL_DrawTextBoldCentered("TEST PLAYER", 320, 800 - viewY, YELLOW);
				PHL_DrawTextBoldCentered("ZAC", 320, 816 - viewY, WHITE);
							
				PHL_DrawTextBoldCentered("- SPECIAL THANKS -", 320, 912 - viewY, YELLOW);			
							
				PHL_DrawTextBoldCentered("QUADRUPLE D", 320, 992 - viewY, YELLOW);
				PHL_DrawTextBoldCentered("SANDMAN", 320, 1008 - viewY, WHITE);
										
				PHL_DrawTextBoldCentered("KBGM", 320, 1072 - viewY, YELLOW);
				PHL_DrawTextBoldCentered("KR.SHIN", 320, 1088 - viewY, WHITE);		
							
				PHL_DrawTextBoldCentered("KBGMPLAYER", 320, 1152 - viewY, YELLOW);
				PHL_DrawTextBoldCentered("NARUTO", 320, 1168 - viewY, WHITE);
							
				PHL_DrawTextBoldCentered("SOUND EFFECT", 320, 1232 - viewY, YELLOW);
				PHL_DrawTextBoldCentered("OSABISHIYUUKI", 320, 1248 - viewY, WHITE);
							
				PHL_DrawTextBoldCentered("EDGE", 320, 1312 - viewY, YELLOW);
				PHL_DrawTextBoldCentered("TAKABO", 320, 1328 - viewY, WHITE);
				
				PHL_DrawTextBoldCentered("THE END", 320, maxViewY - 284 - viewY, YELLOW);
				PHL_DrawSurfacePart(300, maxViewY - 256 - viewY, (int)imageIndex * 40, 280, 40, 80, images[imgHero]);
			}
			
			PHL_EndDrawing();
		}
		
	}
	
	forceGameExit = 1;
	
}

//Black screen between screens
void screenTransition()
{
	char timer = 15;
	
	while (PHL_MainLoop() && timer > 0)
	{
		PHL_StartDrawing();
		
		PHL_DrawRect(0, 0, 640, 480, PHL_NewRGB(0, 0, 0));
		timer -= 1;
		
		PHL_EndDrawing();
	}
	
	if (autoSave == 1) {
		writeSave(savename);
	}
}

void enterDoor()
{	
	//Is not leaving boss room prematurely
	bossFlag = 0;

	level = lastDoor->warplevel;
	
	screenX = lastDoor->warpcoords % 12;
	screenY = lastDoor->warpcoords / 12;
	
	herox = lastDoor->warpx;
	heroy = lastDoor->warpy;

	PHL_StopMusic();
	PHL_FreeMusic(bgmMusic);

	if (level == 0) {
		//Free uncommon images
		PHL_FreeSurface(images[imgMisc2040]);
		PHL_FreeSurface(images[imgMisc6020]);
		PHL_FreeSurface(images[imgDark]);
	}else{
		bgmMusic = PHL_LoadMusic("midi/start", 0);
		PHL_PlayMusic(bgmMusic);
		
		int timer = 125;		
		while (PHL_MainLoop() && timer > 0)
		{
			timer -= 1;
			
			PHL_StartDrawing();
			
			PHL_DrawRect(0, 0, 640, 480, PHL_NewRGB(0, 0, 0));
			drawTextCentered(dungeon[level - 1], 320, 216);
			
			PHL_EndDrawing();
		}
		
		PHL_StopMusic();
		PHL_FreeMusic(bgmMusic);
		
		loadUncommonImages();
	}
	
	//Reload tileset
	PHL_FreeSurface(images[imgTiles]);
	images[imgTiles] = PHL_LoadQDA(tilesetStrings[level]);
	
	bgmMusic = PHL_LoadMusic(musicStrings[level], 1);
	
	changeScreen(0, 0);
	
	PHL_PlayMusic(bgmMusic);
}

void loadScreen()
{
	//Stop music if you leave a boss room early
	if (bossFlag == 1) {
		PHL_StopMusic();
		PHL_FreeMusic(bgmMusic);
		
		bgmMusic = PHL_LoadMusic(musicStrings[level], 1);
		PHL_PlayMusic(bgmMusic);
	}
	
	bossDefeatedFlag = bossFlag = 0;
	roomDarkness = 0;
	
	screenTransition();
	
	int fileNum = stage[level][(screenY * 12) + screenX];
	
	//Cycle through this process twice. Once for the backgroud, and one for the foreground
	int cycle = 0;
	for (cycle = 0; cycle < 2; cycle++) 
	{
		//Build file string
		char toChar[4];
		sprintf(toChar, "%03d", fileNum);
		
		char dest[80];
		strcpy(dest, "");
		#ifdef _3DS
			strcat(dest, "romfs:/map/");
		#elif defined(__amigaos4__) || defined(__MORPHOS__)
			strcat(dest, "PROGDIR:data/map/");
		#elif defined(_SDL)
			strcat(dest, "data/map/");
		#else
			strcat(dest, "romfs/map/");
		#endif
		strcat(dest, toChar);
		
		//load background on first pass
		if (cycle == 0) {
			strcat(dest, "a");
		}
		strcat(dest, ".map");
		
		//Read file
		FILE* file;
		if ((file = fopen(dest, "rb"))) 
		{
			char* memblock;
			int size;
			
			fseek(file, 0, SEEK_END);
			size = ftell(file);
			memblock = (char*)malloc(size);
			fseek(file, 0, SEEK_SET);
			if(fread(memblock, 1, size, file) != size)
				printf("Warning, could not read %s correctly\n", dest);
			fclose(file);
			
			//Load data
			int count = 162; //Level data starts 118
			int xx, yy;
			int valx = 0, valy = 0;
			int raw;
			for (yy = 0; yy < 12; yy++) {
				for (xx = 0; xx < 16; xx++) {
					raw = (unsigned)memblock[count];
					valx = raw & 0x0F;
					valy = raw & 0xF0;
					valy >>= 4;
					
					if (cycle == 0) {
						background.tileX[xx][yy] = valx;
						background.tileY[xx][yy] = valy;
					}else if (cycle == 1) {
						foreground.tileX[xx][yy] = valx;
						foreground.tileY[xx][yy] = valy;
						
						collisionTiles[xx][yy] = getTileType(valx, valy);
						//Breakable blocks
						if (valy == 11 && (valx == 0 || valx == 1 || valx == 2)) {
							int secret = 0;
							if (valx == 2) {
								secret = 1;
							}
							createDestroyable(xx * 40, yy * 40, secret);
						}
						
						//Lava
						if (valx == 2 && valy == 1) {
							createEffect(10, xx * 40, yy * 40);
							foreground.tileX[xx][yy] = 0;
							foreground.tileY[xx][yy] = 0;
						}
						
						//Water
						if (valx == 6 && valy == 1) {
							createEffect(11, xx * 40, yy * 40);
							foreground.tileX[xx][yy] = 0;
							foreground.tileY[xx][yy] = 0;
						}
					}
					
					count += 2;
				}
				count += 12;
			}
			
			free(memblock);
			
		}else{
			PHL_ErrorScreen("Map file was not found");
		}		
	}
	PHL_UpdateBackground(background, foreground);
	
	//Load file
	//Build file string		
	char toChar[4];
	sprintf(toChar, "%03d", fileNum);
	
	char dest[30];
	#ifdef _3DS
		strcpy(dest, "romfs:/obj/");
	#elif defined(__amigaos4__) || defined(__MORPHOS__)
		strcpy(dest, "PROGDIR:data/obj/");
	#elif defined(_SDL)
		strcpy(dest, "data/obj/");
	#else
		strcpy(dest, "romfs/obj/");
	#endif
	
	
	//Add a 0 if needed
	/*
	if (fileNum < 100) {
		strcat(dest, "0");
	}
	*/
	strcat(dest, toChar);
	strcat(dest, ".dat");
	
	FILE* file;
	if ((file = fopen(dest, "rb"))) {
		unsigned char* memblock;
		int size;
			
		fseek(file, 0, SEEK_END);
		size = ftell(file);
		memblock = (unsigned char*)malloc(size);
		fseek(file, 0, SEEK_SET);
		if(fread(memblock, 1, size, file) != size)
			printf("Warning: could not read %s correctly\n", dest);
		
		int count = 0;
		while (count < size) {
			int type = memblock[count];
			
			if (type <= 10)
			{			
				if (type == 0 || type == 9) { //Blue/Red Slime
					createSlime(memblock[count + 1] * 20, memblock[count + 2] * 20, memblock[count + 3], memblock[count + 4]);
				}
				else if (type == 1) { //Bat (grey/red)
					createBat(memblock[count + 1] * 20, memblock[count + 2] * 20, memblock[count + 3]);
				}
				else if (type == 2) { //Slug
					createSlug(memblock[count + 1] * 20, memblock[count + 2] * 20, memblock[count + 3]);
				}
				else if (type == 3) { //Knight
					createKnight(memblock[count + 1] * 20, memblock[count + 2] * 20, memblock[count + 3]);
				}
				else if (type == 4) { //Rhyno head
					createHead(0, memblock[count + 1] * 20, memblock[count + 2] * 20, memblock[count + 3], memblock[count+4], memblock[count+5]);
				}
				else if (type == 5) { //Dragon head
					createHead(2, memblock[count + 1] * 20, memblock[count + 2] * 20, memblock[count + 3], memblock[count+4], memblock[count+5]);
				}
				else if (type == 6) { //Goblin/medusa head
					createHead(1, memblock[count + 1] * 20, memblock[count + 2] * 20, memblock[count + 3], memblock[count+4], memblock[count+5]);
				}
				else if (type == 7) { //Demon head
					createHead(3, memblock[count + 1] * 20, memblock[count + 2] * 20, memblock[count + 3], memblock[count+4], memblock[count+5]);
				}
				else if (type == 10) { //Fireball head
					createHead(4, memblock[count+1] * 20, memblock[count+2] * 20, 1, memblock[count+3], memblock[count+4]);
				}
			}
			
			else if (type <= 20)
			{
				if (type == 11) { //Poison Gas
					createGas(memblock[count+1] * 20, memblock[count+2] * 20, memblock[count+3]);
				}
				else if (type == 12) { //Flying skull
					createSkull(memblock[count+1] * 20, memblock[count+2] * 20);
				}
				else if (type == 13) { //Fish
					createFish(memblock[count+1] * 20, memblock[count+2] * 20, memblock[count+3]);
				}
				else if (type == 14) { //Water Jumper
					createWaterJumper(memblock[count+1] * 20, memblock[count+2] * 20, memblock[count+3], memblock[count+4], memblock[count+5]);
				}
				else if (type == 15) { //Podoboo
					createPodoboo(memblock[count+1] * 20, memblock[count+2] * 20, memblock[count+3], memblock[count+4]);
				}
				else if (type == 16) { //Thwomp
					createThwomp(memblock[count+1] * 20, memblock[count+2] * 20, memblock[count+3], memblock[count+4], memblock[count+5], memblock[count+6]);
				}
				else if (type == 17) { //Skeleton
					createSkeleton(memblock[count+1] * 20, memblock[count+2] * 20, memblock[count+3]);
				}
				else if (type == 18) { //Ghoul
					createGhoul(memblock[count+1]*20, memblock[count+2]*20, memblock[count+3]);
				}
				else if (type == 19) { //Seal
					createSeal(memblock[count+1]*20, memblock[count+2]*20);
				}
				else if (type == 20) { //Jellyfish
					createJellyfish(memblock[count+1]*20, memblock[count+2]*20);
				}
			}
			
			else if (type <= 30)
			{
				if (type == 21) { //Wizard
					createWizard(memblock[count+1]*20, memblock[count+2]*20);
				}
				else if (type == 22) { //Pendulum
					createPendulum(memblock[count+1]*20, memblock[count+2]*20, memblock[count+3]);
				}
				else if (type == 24) { //Bee
					createBee(memblock[count+1]*20, memblock[count+2]*20, memblock[count+3]);
				}
				else if (type == 25) { //Air Jar
					//createJar(memblock[count+1]*20, memblock[count+2]*20, memblock[count+3], memblock[count+4]);
					createHead(5, memblock[count + 1] * 20, memblock[count + 2] * 20, 0, memblock[count+3], memblock[count+4]);
				}
				else if (type == 26) { //Boar
					createBoar(memblock[count+1]*20, memblock[count+2]*20);
				}
				else if (type == 27) { //Fire Wheel
					createFirewheel(memblock[count+1]*20, memblock[count+2]*20, memblock[count+3]);
				}
				else if (type == 28) { //Rock Golem
					createGolem(memblock[count+1]*20, memblock[count+2]*20, memblock[count+3]);
				}
				else if (type == 29) { //Poison Knight
					createPoisonknight(memblock[count+1]*20, memblock[count+2]*20);
				}
				else if (type == 30) { //Electricity doggy
					createDog(memblock[count+1]*20, memblock[count+2]*20);
				}
			}
			
			else if (type < 40)
			{
				if (type == 31) {
					createBoomknight(memblock[count+1]*20, memblock[count+2]*20);
				}
				else if (type == 32) {
					createPumpkinenemy(memblock[count+1]*20, memblock[count+2]*20);
				}
			}
			
			else if (type < 50)
			{
				//Bosses
				if (type == 40) {
					createDodo(memblock[count+1] * 20, memblock[count+2] * 20, memblock[count+3]);
				}
				else if (type == 41) {
					createBatboss(memblock[count+1] * 20, memblock[count+2] * 20);
				}
				else if (type == 42) {
					createCrab(memblock[count+1] * 20, memblock[count+2] * 20);
				}
				else if (type == 43) {
					createGyra(memblock[count+1] * 20, memblock[count+2] * 20);
				}
				else if (type == 44) {
					createLolidra(memblock[count+1] * 20, memblock[count+2] * 20);
				}
				else if (type == 45) {
					createDevil(memblock[count+1] * 20, memblock[count+2] * 20);
				}
				else if (type == 46) {
					createGarm(memblock[count+1] * 20, memblock[count+2] * 20);
				}
				else if (type == 47) {
					createHydra(memblock[count+1] * 20);
				}
			}
			
			else if (type <= 60)
			{
				//Objects
				if (type == 50) { //Moving platforms
					createPlatform(0, memblock[count + 1] * 20, memblock[count + 2] * 20, memblock[count + 3] * 20, memblock[count + 4] * 20, memblock[count + 5], memblock[count+6]);
				}
				else if (type == 51) { //Loose block
					createPlatform(1, memblock[count + 1] * 20, memblock[count + 2] * 20, 0, 0, 0, memblock[count+3]);
				}
				else if (type == 52) { //Locked Block
					createLockBlock(memblock[count+1] * 20, memblock[count+2] * 20, memblock[count+3]);
				}
				else if (type == 53) { //Gate
					createGate(memblock[count+1]*20, memblock[count+2]*20, memblock[count+3]);
				}
				else if (type == 54) { //Statue
					createStatue(memblock[count+1]*20, memblock[count+2]*20, memblock[count+3]);
				}
				else if (type == 55) { //Megaman block
					createPlatform(2, memblock[count + 1] * 20, memblock[count + 2] * 20, memblock[count + 3], memblock[count + 4], 0, 0);
				}
				else if (type == 56) { //Electric gate
					createShockgate(memblock[count + 1] * 20, memblock[count + 2] * 20, memblock[count + 3]);
				}
				else if (type == 57) { //Hydra platform
					createPlatform(3, memblock[count + 1] * 20, memblock[count + 2] * 20, 0, 0, 0, 0);
				}
			}
			
			else if (type < 70)
			{
				
			}
			
			else/* if (type <= 80)*/
			{
				if (type == 70) { //Breakable Block
					createDestroyable(memblock[count+1] * 20, memblock[count+2] * 20, 1);
				}
				else if (type == 71) { //Secret Trigger
					createSecretTrigger(memblock[count+1], memblock[count+2], memblock[count+3]);
				}
				else if (type == 73) { //Chests
					createChest(memblock[count + 1] * 20, memblock[count + 2] * 20, memblock[count + 3], memblock[count + 4]);
				}
				else if (type == 74) { //Save Points
					createSavePoint(memblock[count+1] * 20, memblock[count+2] * 20, memblock[count+3]);
				}
				else if (type == 75) { //door
					createDoor(memblock[count+1] * 20, memblock[count+2] * 20, memblock[count+3], memblock[count+4], memblock[count+5] * 20, memblock[count+6] * 20, memblock[count+7]);
				}
				else if (type == 76) { //Light Switch
					createSwitch(memblock[count+1] * 20, memblock[count+2] * 20, memblock[count+3]);
				}
				else if (type == 77) { //Floor Button
					createFloorPad(memblock[count+1]*20, memblock[count+2]*20, memblock[count+3]);
				}
				else if (type == 78) {
					roomDarkness = 1;				
				}
				else if (type == 79) { //Ladder Spawner
					createLadder(memblock[count+1] * 20, memblock[count+2] * 20, memblock[count+3]);
				}
				else if (type == 80) { //Generator
					createGenerator(memblock[count+1] * 20, memblock[count+2] * 20, memblock[count+3]);
				}
				else if (type == 81) { //Crown
					createCrown(memblock[count+1] * 20, memblock[count+2] * 20);
				}
			}
			count += 16;
		}
		
		free(memblock);
		fclose(file);
	}
	
}

void drawHud()
{
	//Repress certain screen altering variables
	int tempDark = roomDarkness;
	roomDarkness = 0;
	
	int tempQuake = quakeTimer;
	quakeTimer = 0;
	
	//Change HUD position
	int drawy = 8;
	{
		if (heroy <= 100) {
			drawy = 400;
		}
	}
	
	//Move scrolling health bar
	{
		if (drawhp > herohp) {
			drawhp -= 1;
		}
		if (drawhp < herohp) {
			drawhp += 1;
		}
	}
	
	//Main image
	{
		PHL_DrawSurfacePart(8, drawy, 0, 0, 368, 64, images[imgHud]);
	}
	
	//Health bar
	{
		PHL_RGB hpbarc = PHL_NewRGB(128, 255, 0);
		if (getHeroPoisoned() > 0) {
			hpbarc = PHL_NewRGB(255, 128, 255);
		}	
		
		PHL_DrawRect(76, drawy + 8, maxhp * 2, 6, PHL_NewRGB(255, 0, 0));
		PHL_DrawRect(76, drawy + 8, drawhp * 2, 6, hpbarc);
	}
	
	//Ammo counter
	{
		char c[10];
		sprintf(c, "%02d", heroAmmo);
		PHL_DrawTextBold(c, 74, drawy + 36, WHITE);
	}
	
	//Draw weapon icon
	{
		int wx = 32 * (heroWeapon + 1);
		if (hasWeapon[heroWeapon] == 0) {
			wx = 0;
		}
		PHL_DrawSurfacePart(24, drawy + 16, wx, 64, 32, 32, images[imgHud]);
	}
	
	//Restore screen altering variables
	{
		quakeTimer = tempQuake;
		roomDarkness = tempDark;
	}
}

void freeArrays()
{
	int i;
	for (i = 0; i < MAX_EFFECTS; i++) {
		effectDestroy(i);
	}
	
	for (i = 0; i < MAX_OBJECTS; i++) {
		objectDestroy(i);
	}
	
	for (i = 0; i < MAX_ENEMIES; i++) {
		enemyDestroy(i);
	}
	
	for (i = 0; i < MAX_WEAPONS; i++) {
		weaponDestroy(i);
	}
	
	for (i = 0; i < MAX_PLATFORMS; i++) {
		platformDestroy(i);
	}
}

void changeScreen(int dx, int dy)
{		
	roomSecret = 0;
	
	freeArrays();

	screenX += dx;
	screenY += dy;
	loadScreen();
	
	writeSave(savename);
}

int getTileType(int valx, int valy) {
	int result = 0;

	if (valy == 11 && valx == 8) {
		result = 3; //Ladder Top
	}else
	if (valy == 1 && valx == 1) {
		result = 5; //Lava
	}else
	if (valy > 7) {
		result = 1; //Solid
	}else
	//specifics
	if (valy == 0 && (valx == 3 || valx == 5)) {
		result = 2; //Ladders
	}else	
	if (valy == 1 && valx == 5) {
		result = 4; //Water
	}else
	if (valx == 0 && (valy == 1 || valy == 2)) {
		result = 6; //Spikes
	}else
	if (valy == 11 && (valx == 0 || valx == 1 || valx == 2)) {
		result = 1; //Breakable solid block
	}
	
	if (level == 4 && valy == 3 && (valx == 0 || valx == 1 || valx == 2)) {
		result = 6; //Spikes
	}

	return result;
}

//Save file load/save
int writeSave(char* fname)
{
	int result = 0;
	//mkdir("data");
	FILE* f;
	
	char fullPath[4096];
	strcpy(fullPath, "");
	#ifdef _3DS
		strcat(fullPath, "sdmc:/3ds/appdata/HydraCastleLabyrinth/");
	#elif _KOLIBRI
		strcat(fullPath, KOS_HCL_SAVES_PATH);
	#endif
	strcat(fullPath, fname);

	if ( (f = fopen(fullPath, "wb")) ) {
		int size = 4548;
		unsigned char* memblock = (unsigned char*)malloc(size);
		memset(memblock, 0, size);
		#if defined(__amigaos4__) || defined(__MORPHOS__)
		#define D 3
		#else
		#define D 0
		#endif
		memblock[0x0+D] = herohp;
		memblock[0x4+D] = maxhp;
		memblock[0x8+D] = heroAmmo;
		memblock[0x0C+D] = maxAmmo;
		
		if (heroWeapon == -1) {
			memblock[0x10] = 0;
		}else{
			memblock[0x10] = heroWeapon;
		}
		memblock[0x14] = 1; //Unknown, but always resets to 1
		
		int i;
		for (i = 0; i < 60; i++) {
			memblock[(0x3FC) + i] = flags[i];
		}
		
		for (i = 0; i < 5; i++) {
			memblock[(0x7E4) + i] = hasWeapon[i];
		}
		
		int itemorder[28] = { 0x7F6, 0x7FA, 0x7F9, 0x7F8, 0x7F1, 0x7F3, 0x7F2,
							0x7FB, 0x7ED, 0x7EF, 0x7EE, 0x7F0, 0x7EC, 0x7F4,
							0x7F7, 0x7F5, 0x7EA, 0x7EB, 0x7FF, 0x803, 0x804,
							0x7FE, 0x802, 0x805, 0x800, 0x7FD, 0x7FC, 0x801 };
							
		for (i = 0; i < 28; i++) {
			memblock[itemorder[i]] = hasItem[i];
		}
		
		for (i = 0; i < 8; i++) {
			memblock[(0x806) + i] = hasKey[i];
		}
		
		int writeHerox = herox;
		int writeHeroy = heroy;
		memcpy(&memblock[0x11B0], &writeHerox, 4);	
		memcpy(&memblock[0x11B4], &writeHeroy, 4);
		
		if (getHeroDirection() == 1) {
			memblock[0x11C0+D] = 0;
		}else{
			memblock[0x11C0+D] = 1;
		}
		
		memblock[0x11B8+D] = level;
		
		//Screen
		memblock[0x11BC+D] = (screenX) + (screenY * 12);
		
		//Time
		memcpy(&memblock[0x11AC], &playTime, 4);
		
		fwrite(memblock, 1, size, f);
		
		free(memblock);
		
		result = 1;
		fclose(f);
	}
	#ifdef EMSCRIPTEN
	EM_ASM(
		//persist changes
		FS.syncfs(false,function (err) {
						assert(!err);
		});
	);
	#endif

	return result;
}

void loadSave(char* fname)
{
	FILE* f;
	
	char fullPath[128];
	strcpy(fullPath, "");
	#ifdef _3DS
		strcat(fullPath, "sdmc:/3ds/appdata/HydraCastleLabyrinth/");
	#endif
	strcat(fullPath, fname);
	
	if ((f = fopen(fullPath, "rb"))) {
		//Reminder: read order matters
		unsigned long loadTemp = 0;
		int tmp;
		//Hero HP
		tmp = fread(&loadTemp, 4, 1, f);
		herohp = loadTemp;
		drawhp = herohp;
		
		//Max HP
		tmp = fread(&loadTemp, 4, 1, f);
		maxhp = loadTemp;
		
		//Ammo
		tmp = fread(&loadTemp, 4, 1, f);
		heroAmmo = loadTemp;
		
		//Max Ammo
		tmp = fread(&loadTemp, 4, 1, f);
		maxAmmo = loadTemp;
		
		int loadedWeapon = 0;
		tmp = fread(&loadedWeapon, 1, 1, f);
				
		//Read Flags
		fseek(f, 0x3FC, SEEK_SET);
		int i;
		for (i = 0; i < 60; i++) {
			tmp = fread(&flags[i], 1, 1, f);
		}
		
		//Read weapons
		fseek(f, 0x7E4, SEEK_SET);
		for (i = 0; i < 5; i++) {
			tmp = fread(&hasWeapon[i], 1, 1, f);
		}
		
		heroWeapon = -1;
		if (hasWeapon[loadedWeapon] == 1) {
			heroWeapon = loadedWeapon;
		}
		
		//Read items
		int itemorder[28] = { 16, 17, 12,  8, 10,  9, 11,
							   4,  6,  5, 13, 15,  0, 14, 
							   3,  2,  1,  7, 26, 25, 21,
							  18, 24, 27, 22, 19, 20, 23 };
		fseek(f, 0x7EA, SEEK_SET);
		for (i = 0; i < 28; i++) {
			tmp = fread(&hasItem[itemorder[i]], 1, 1, f);
		}
		
		//Read keys
		for (i = 0; i < 8; i++) {
			tmp = fread(&hasKey[i], 1, 1, f);
		}
		
		fseek(f, 0x11AC, SEEK_SET);
		tmp = fread(&playTime, 4, 1, f);
		
		//fseek(f, 4540, SEEK_SET);
		//Hero X and Y
		tmp = fread(&loadTemp, 4, 1, f);
		herox = loadTemp;		
		tmp = fread(&loadTemp, 4, 1, f);
		heroy = loadTemp;
		
		//Level
		tmp = fread(&loadTemp, 4, 1, f);
		level = loadTemp;
		
		//Screen coords
		tmp = fread(&loadTemp, 4, 1, f);
		screenX = (loadTemp) % 12;
		screenY = ((int)(loadTemp) / 12);
		
		//Direction
		tmp = fread(&loadTemp, 4, 1, f);		
		if (loadTemp == 0) {
			setHeroDirection(1);
		}else{
			setHeroDirection(-1);
		}		
		fclose(f);
		#undef D
	}
	
}

int fileExists(char* fpath)
{
	int result = 0;
	
	char fullPath[128];
	strcpy(fullPath, "");
	#ifdef _3DS
		strcat(fullPath, "sdmc:/3ds/appdata/HydraCastleLabyrinth/");
	#endif
	strcat(fullPath, fpath);
	
	FILE* f;
	if ( (f = fopen(fullPath, "rb")) ) {
		result = 1;
		fclose(f);
	}
	
	return result;
}

void playSecret()
{
	PHL_StopMusic();	
	secretTimer = 210;
}

void secretCountdown()
{
	if (secretTimer > 0) {
		secretTimer -= 1;
		if (secretTimer <= 0) {
			PHL_StopMusic();
			if (bossFlag == 0 && bossDefeatedFlag == 0) {
				PHL_PlayMusic(bgmMusic);
			}
		}else if (secretTimer == 180) {
			PHL_PlayMusic(bgmSecret);
		}
	}
}

int getDrawHP()
{
	return drawhp;
}

void setDrawHP(int val)
{
	drawhp = val;
}

int getLevel()
{
	return level;
}

void setBossRoom()
{	
	bossFlag = 1;
	PHL_StopMusic();
	secretTimer = 0;
	
	PHL_FreeMusic(bgmMusic);
	if (level != 8) {
		bgmMusic = PHL_LoadMusic("midi/boss", 1);
	}else{
		bgmMusic = PHL_LoadMusic("midi/lastboss", 1);
	}
	PHL_PlayMusic(bgmMusic);
}

void setAutoSave(char val)
{
	autoSave = val;
}

char getAutoSave()
{
	return autoSave;
}

void loadUncommonImages()
{
	//Seal Toungs
	if (level == 4) {
		images[imgMisc2040] = PHL_LoadQDA("chr20x40.BMP");
	}
	//Darkness
	if (level == 5) {
		images[imgDark] = PHL_LoadQDA("dark.bmp");
	}
	//Dragon Flame
	if (level == 7 || level == 8) {
		images[imgMisc6020] = PHL_LoadQDA("chr60x20.bmp");
	}
}
