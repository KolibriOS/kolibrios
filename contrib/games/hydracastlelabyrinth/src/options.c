#include <stdio.h>
#include "options.h"
#include "PHL.h"
#include "game.h"
#include "ini.h"
#include "text.h"

char page = 0;
int optCursor = 0;
int lastOption = -1;
#ifdef _SDL
int musicType = 1;

int getMusicType()
{
	return musicType;
}
void setMusicType(int t)
{
#ifndef _KOLIBRI
	if(t!=musicType)
	{
		musicType = t;
		// restart music...
	}
#else
	musicType = 1;  /* MIDI NOT WORK! */
#endif
}
#endif

#ifdef EMSCRIPTEN
static char tempDark;
static int emOnly;
void optionsSetup(int only)
{
	tempDark = roomDarkness;
	roomDarkness = 0;
	emOnly = only;
	page = only?1:0;
}

int optionsEMStep()
{
	int loop = 1;
	PHL_MainLoop();

	PHL_StartDrawing();
	
	optionsDraw();
	
	PHL_ScanInput();	
	int result = optionsStep();
	
	PHL_EndDrawing();
	
	if (page == 0 && result != -1 && result != 2) {
		loop = 0;
	}
	if (emOnly && page==0) {
		loop = 0;
	}

	if(!loop) roomDarkness = tempDark;
	
	return (!loop)?result:-1;
}
#else

int options(int only)
{
	char tempDark = roomDarkness;
	roomDarkness = 0;
	
	int result = -1;
	char loop = 1;

	if(only) page=1;
	
	while (PHL_MainLoop() && loop == 1)
	{
		PHL_StartDrawing();
		
		optionsDraw();
		
		PHL_ScanInput();	
		result = optionsStep();
		
		PHL_EndDrawing();
		
		if (page == 0 && result != -1 && result != 2) {
			loop = 0;
		}
		if (only && page==0) {
			loop = 0;
		}
	}
	
	roomDarkness = tempDark;
	
	return result;
}
#endif
int optionsStep()
{		
	int result = -1;
	
	secretCountdown();
	
	//input
	if (btnDown.pressed == 1) {
		optCursor += 1;
		PHL_PlaySound(sounds[sndPi01], CHN_SOUND);
		
	}else if (btnUp.pressed == 1) {
		optCursor -= 1;
		PHL_PlaySound(sounds[sndPi01], CHN_SOUND);
	}
	
	//First screen (continue/reset/exit)
	if (page == 0) {
		//Limit cursor
		if (optCursor >= 4) {
			optCursor = 0;
		}
		
		if (optCursor < 0) {
			optCursor = 3;
		}
		
		if (btnAccept.pressed == 1 || btnStart.pressed == 1) {
			result = optCursor;
			if (result == 1) {
				PHL_StopMusic();
			}
			if (result == 2) {
				page = 1;
				optCursor = 0;
			}
			
			//No blip on game exit
			if (optCursor != 3) {
				PHL_PlaySound(sounds[sndOk], CHN_SOUND);
			}
		}
		
		if (btnSelect.pressed == 1) {
			result = 0;
			PHL_PlaySound(sounds[sndOk], CHN_SOUND);
		}
	}
	//Actual options screen
	else if (page == 1) {
		//Limit cursor
		if (optCursor > lastOption) {
			optCursor = 0;
		}
		
		if (optCursor < 0) {
			optCursor = lastOption;
		}
		
		if (btnAccept.pressed == 1 || btnStart.pressed == 1) {
			//Universal settings
			//Language
			if (optCursor == 0) {
				if (getLanguage() == JAPANESE) {
					setLanguage(ENGLISH);
				}else{
					setLanguage(JAPANESE);
				}
			}
			
			//Autosave
			if (optCursor == 1) {
				if (getAutoSave() == 0) {
					setAutoSave(1);
				}else{
					setAutoSave(0);
				}
			}
			
			#ifdef _3DS
				if (optCursor == 2) {
					if (activeScreen->screen == GFX_TOP) {
						swapScreen(GFX_BOTTOM, GFX_LEFT);
					}else{
						swapScreen(GFX_TOP, GFX_LEFT);
					}
				}				
			#endif
			
			#ifdef _PSP
				if (optCursor == 2) {
					if (getScreenSize() == 0) {
						setScreenSize(1);
					}else if (getScreenSize() == 1) {
						setScreenSize(2);
					}else{
						setScreenSize(0); 
					}
				}
				
				//Blur
				if (optCursor == 3) {
					if (getBlur() == 0) {
						setBlur(1);
					}else{
						setBlur(0); 
					}
				}
			#endif

			#ifdef _SDL
				// Music type
				if(optCursor == 2) {
					if(getMusicType()  == 0)
						setMusicType(1);
					else
						setMusicType(0);
				}
				// Music volume
				if(optCursor == 3) {
					music_volume = (music_volume+1)%5;
					PHL_MusicVolume(0.25f * music_volume);
				}
				// xBRZ
				if (optCursor == 4) {
					if (getXBRZ() == 0) {
						setXBRZ(1);
					}else{
						setXBRZ(0); 
					}
				}
			#endif
			
			//Back
			if (optCursor == lastOption) {
				page = 0;
			}
		}		
		else if (btnDecline.pressed == 1) {
			page = 0;			
		}
		
		if (page == 0) {
			saveSettings();
		}
	}
	
	return result;
}

void optionsDraw()
{	
	PHL_DrawRect(0, 0, 640, 480, PHL_NewRGB(0, 0, 0));
	
	if (page == 0)
	{
		PHL_DrawTextBold("CONTINUE", 254, 144, YELLOW);
		PHL_DrawTextBold("RESET", 278, 176, YELLOW);
		PHL_DrawTextBold("OPTIONS", 262, 208, YELLOW);
		PHL_DrawTextBold("EXIT", 286, 240, YELLOW);
		
		PHL_DrawTextBold("<", 232, 144 + (32 * optCursor), RED);
		PHL_DrawTextBold(">", 390, 144 + (32 * optCursor), RED);
	}
	else if (page == 1)
	{
		int xleft = 216;
		int xright = xleft + 160;
		
		int ydrawstart = 144;
		int ydraw = ydrawstart;
		int ystep = 32;
		int optioncount = 0;
		
		//Language
		PHL_DrawTextBold("LANGUAGE", xleft, ydraw, YELLOW);
		if (getLanguage() == 1) {
			PHL_DrawTextBold("EN", xright, ydraw, YELLOW);
		}
		if (getLanguage() == 0) {
			PHL_DrawTextBold("JP", xright, ydraw, YELLOW);
		}
		
		ydraw += ystep;
		optioncount++;
		
		//AutoSave
		PHL_DrawTextBold("SAFE SAVE", xleft, ydraw, YELLOW);
		if (getAutoSave() == 1) {
			PHL_DrawTextBold("ON", xright, ydraw, YELLOW);
		}else{
			PHL_DrawTextBold("OFF", xright, ydraw, YELLOW);
		}		
		
		ydraw += ystep;
		optioncount++;
		
		#ifdef _3DS
			//Screen
			PHL_DrawTextBold("SCREEN", xleft, ydraw, YELLOW);
			if (activeScreen->screen == GFX_TOP) {
				PHL_DrawTextBold("TOP", xright, ydraw, YELLOW);
			}else{
				PHL_DrawTextBold("BOTTOM", xright, ydraw, YELLOW);
			}

			ydraw += ystep;
			optioncount++;			
		#endif
		
		#ifdef _PSP			
			//Screen Size
			PHL_DrawTextBold("SCREEN", xleft, ydraw, YELLOW);
			if (getScreenSize() == 0) {
				PHL_DrawTextBold("1:1", xright, ydraw, YELLOW);
			}
			else if (getScreenSize() == 1) {
				PHL_DrawTextBold("FILL", xright, ydraw, YELLOW);
			}
			else if (getScreenSize() == 2) {
				PHL_DrawTextBold("FULL", xright, ydraw, YELLOW);
			}
			
			ydraw += ystep;
			optioncount++;
			
			//Blur
			PHL_DrawTextBold("BLUR", xleft, ydraw, YELLOW);
			if (getBlur() == 1) {
				PHL_DrawTextBold("ON", xright, ydraw, YELLOW);
			}
			else{
				PHL_DrawTextBold("OFF", xright, ydraw, YELLOW);
			}
			
			ydraw += ystep;
			optioncount++;
		#endif

		#ifdef _SDL
			// Music type
			PHL_DrawTextBold("MUSIC", xleft, ydraw, YELLOW);
			if (getMusicType() == 1) {
				PHL_DrawTextBold("OGG", xright, ydraw, YELLOW);
			}
			else{
				PHL_DrawTextBold("MIDI", xright, ydraw, YELLOW);
			}
			ydraw += ystep;
			optioncount++;
			// Music volume
			PHL_DrawTextBold("MUSIC", xleft, ydraw, YELLOW);
			char buff[50];
			sprintf(buff, "%d%%", music_volume*25);
			PHL_DrawTextBold(buff, xright, ydraw, YELLOW);
			ydraw += ystep;
			optioncount++;
			// xBRZ scaling
			PHL_DrawTextBold("XBRZ", xleft, ydraw, YELLOW);
			if (getXBRZ() == 1) {
				PHL_DrawTextBold("ON", xright, ydraw, YELLOW);
			}
			else{
				PHL_DrawTextBold("OFF", xright, ydraw, YELLOW);
			}
			ydraw += ystep;
			optioncount++;
		#endif
		
		PHL_DrawTextBold("BACK", xleft, ydraw, YELLOW);
		
		if (lastOption == -1) {
			lastOption = optioncount;
		}
		
		PHL_DrawTextBold(">", xleft - 32, ydrawstart + (ystep * optCursor), RED);
	}
}
