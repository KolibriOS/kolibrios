#include "ini.h"
#include "game.h"
#include "options.h"
#include "PHL.h"
#include <stdio.h>
#include <string.h>
#include "text.h"
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

//char* getFileLocation();
char* trimString(char* orig);

void screenLoad(char* first, char* second);
void sizeLoad(char* first, char* second);
void blurLoad(char* first, char* second);
void xbrzLoad(char* first, char* second);
void languageLoad(char* first, char* second);
void autosaveLoad(char* first, char* second);
void musictypeLoad(char* first, char* second);
void musicvolumeLoad(char* first, char* second);

void iniInit()
{
	//Build filepath
	char fullPath[128];
	{
	#ifdef _SDL
		#if defined(__amigaos4__) || defined(__MORPHOS__)
		strcpy(fullPath, "PROGDIR:.hydracastlelabyrinth/");
		#elif defined(EMSCRIPTEN)
		strcpy(fullPath, "hcl_data/");
		#elif defined(_KOLIBRI)
		strcpy(fullPath, KOS_HCL_SAVES_PATH"/");
		#else
		strcpy(fullPath, getenv("HOME"));
		strcat(fullPath, "/.hydracastlelabyrinth/");
		#endif	
	#elif defined(_3DS)
		strcpy(fullPath, "sdmc:/3ds/appdata/HydraCastleLabyrinth/");
	#else
		strcpy(fullPath, "");
	#endif
		strcat(fullPath, "system.ini");
	}
	
	FILE* f;

	if ( (f = fopen(fullPath, "rt")) )
	{
		//File exists - read it
		fclose(f);
		loadSettings();
	}else{
		//File does not exists - create it (with default hardcoded settings)
		saveSettings();
	}
	
}

void saveSettings()
{	
	//Build filepath
	char fullPath[128];
	{
		#ifdef _SDL
		#if defined(__amigaos4__) || defined(__MORPHOS__)
		strcpy(fullPath, "PROGDIR:.hydracastlelabyrinth/");
		#elif defined(EMSCRIPTEN)
		strcpy(fullPath, "hcl_data/");
		#elif defined(_KOLIBRI)
		strcpy(fullPath, KOS_HCL_SAVES_PATH"/");
		#else
		strcpy(fullPath, getenv("HOME"));
		strcat(fullPath, "/.hydracastlelabyrinth/");
		#endif
		#elif defined(_3DS)
		strcpy(fullPath, "sdmc:/3ds/appdata/HydraCastleLabyrinth/");
		#else
		strcpy(fullPath, "");
		#endif
		strcat(fullPath, "system.ini");
	}

	FILE* f;
	
	if ( (f = fopen(fullPath, "wt")) )
	{
		fprintf(f, "[disp]");
		#ifdef _3DS
			//Screen
			fprintf(f, "\r\nscreen=");
			if (activeScreen->screen == GFX_BOTTOM) {
				fprintf(f, "bottom");
			}else{
				fprintf(f, "top");
			}
		#endif
		
		#ifdef _PSP
			//Screen Size
			fprintf(f, "\r\nsize=");
			if (getScreenSize() == 1) {
				fprintf(f, "1");
			}
			else if (getScreenSize() == 2) {
				fprintf(f, "2");
			}
			else {
				fprintf(f, "0");
			}
			
			//Screen Blur
			fprintf(f, "\r\nblur=");
			if (getBlur() == 1) {
				fprintf(f, "on");
			}else{
				fprintf(f, "off");
			}			
		#endif
		#ifdef _SDL
			//xBRZ Scaling
			fprintf(f, "\r\nxbrz=");
			if (getXBRZ() == 1) {
				fprintf(f, "on");
			}else{
				fprintf(f, "off");
			}			
		#endif

		fprintf(f, "\r\n[system]");
		
		//Language
		fprintf(f, "\r\nlanguage=");
		if (getLanguage() == 0) {
			fprintf(f, "jp");
		}
		if (getLanguage() == 1) {
			fprintf(f, "en");
		}
		
		//Autosave
		fprintf(f, "\r\nautosave=");
		if (getAutoSave() == 1) {
			fprintf(f, "on");
		}else{
			fprintf(f, "off");
		}

		#ifdef _SDL
		fprintf(f, "\r\n[audio]");
		fprintf(f, "\r\nmusic_type=%s", getMusicType()?"ogg":"midi");
		fprintf(f, "\r\nmusic=%d", music_volume);
		// Audio
		#endif
		fclose(f);
	}
	#ifdef EMSCRIPTEN
	EM_ASM(
		FS.syncfs(false,function () {
			Module.print("File sych'd")
		});
	);
	#endif
}

void loadSettings()
{
	//Build filepath
	char fullPath[128];
	{
		#ifdef _SDL
		#if defined(__amigaos4__) || defined(__MORPHOS__)
		strcpy(fullPath, "PROGDIR:.hydracastlelabyrinth/");
		#elif defined(EMSCRIPTEN)
		strcpy(fullPath, "hcl_data/");
		#elif defined(_KOLIBRI)
		strcat(fullPath, KOS_HCL_SAVES_PATH"/");
		#else
		strcpy(fullPath, getenv("HOME"));
		strcat(fullPath, "/.hydracastlelabyrinth/");
		#endif
		#elif defined(_3DS)
		strcpy(fullPath, "sdmc:/3ds/appdata/HydraCastleLabyrinth/");
		#else
		strcpy(fullPath, "");
		#endif
		strcat(fullPath, "system.ini");
	}
	
	FILE* f;
	
	if ( (f = fopen(fullPath, "rt")) )
	{		
		char line[80];
		
		while ( (fgets(line, 80, f) != NULL) )
		{
			char* lineptr = line;
			lineptr = trimString(lineptr);
			
			if (lineptr != NULL) {				
				//Ignore category lines
				if (lineptr[0] != '[')
				{
					//Check if it has a = delimiter first
					int i;
					for (i = 0; i < 80; i++) {
						if (line[i] == '=')
						{							
							//Begin line splitting
							char* half;
							if ( (half = strsep(&lineptr, "=")) != NULL)
							{
								//first half
								char* fhalf = half;
								
								if ( (half = strsep(&lineptr, "=")) != NULL) {
									//Second half
									char* shalf = half;
									
									//Load options	
									screenLoad(fhalf, shalf);
									sizeLoad(fhalf, shalf);									
									blurLoad(fhalf, shalf);
									xbrzLoad(fhalf, shalf);
									languageLoad(fhalf, shalf);
									autosaveLoad(fhalf, shalf);
									musictypeLoad(fhalf, shalf);
									musicvolumeLoad(fhalf, shalf);
								}
							}
							
							//End 
							i = 81;
						}
					}					
				}
			}
		}
		fclose(f);
	}
	
}

//Build file path
/*
char* getFileLocation()
{	
	char fullPath[128];
	strcpy(fullPath, "");
	#ifdef _CIA
		strcat(fullPath, "sdmc:/3ds/HydraCastleLabyrinth/");
	#endif
	strcat(fullPath, "system.ini");
	
	return fullPath;
}
*/

char* trimString(char* orig)
{
	char* output = orig;
	
	int i, r = 0;
	for (i = 0; i < strlen(orig); i++) {
		if (orig[i] != ' ' && orig[i] != '\n' && orig[i] != '\r') {
			output[r] = orig[i];
			r++;
		}
	}		
	
	orig[r] = 0;
	
	return output;
}

void screenLoad(char* first, char* second)
{
	#ifdef _3DS
	if (strcmp(first, "screen") == 0) {
		if (strcmp(second, "top") == 0) {
			swapScreen(GFX_TOP, GFX_LEFT);
		}
		if (strcmp(second, "bottom") == 0) {
			swapScreen(GFX_BOTTOM, GFX_LEFT);
		}
	}
	#endif
}

void sizeLoad(char* first, char* second)
{
	#ifdef _PSP
	if (strcmp(first, "size") == 0) {
		if (second[0] == '0') {
			//fprintf(debug, "\nsize is 0");
			setScreenSize(0);
		}
		if (second[0] == '1') {
			//fprintf(debug, "\nsize is 1");
			setScreenSize(1);
		}
		if (second[0] == '2') {
			//fprintf(debug, "\nsize is 2");
			setScreenSize(2);
		}
	}
	#endif
}

void blurLoad(char* first, char* second)
{
	#ifdef _PSP
	if (strcmp(first, "blur") == 0) {
		if (strcmp(second, "on") == 0) {
			//fprintf(debug, "\nblur is on");
			//oslSetBilinearFilter(1);
			setBlur(1);
		}
		if (strcmp(second, "off") == 0) {
			//fprintf(debug, "\nblur is off");
			//oslSetBilinearFilter(0);
			setBlur(0);
		}
	}
	#endif
}

void xbrzLoad(char* first, char* second)
{
	#ifdef _SDL
	if (strcmp(first, "xbrz") == 0) {
		if (strcmp(second, "on") == 0) {
			setXBRZ(1);
		}
		if (strcmp(second, "off") == 0) {
			setXBRZ(0);
		}
	}
	#endif
}

void languageLoad(char* first, char* second)
{
	if (strcmp(first, "language") == 0) {
		if (strcmp(second, "en") == 0) {
			setLanguage(ENGLISH);
		}
		if (strcmp(second, "jp") == 0) {
			setLanguage(JAPANESE);
		}
	}
}

void autosaveLoad(char* first, char* second)
{
	if (strcmp(first, "autosave") == 0) {
		if (strcmp(second, "on") == 0) {
			//fprintf(debug, "\nautosave is on");
			setAutoSave(1);
		}
		if (strcmp(second, "off") == 0) {
			//fprintf(debug, "\nautosave is off");
			setAutoSave(0);
		}
	}
}

void musicvolumeLoad(char* first, char* second)
{
	#ifdef _SDL
	if (strcmp(first, "music") == 0) {
		if (second[0] >= '0' && second[0] <= '4') {
			music_volume = second[0]-'0';
			PHL_MusicVolume(0.25f * music_volume);
		}
	}
	#endif
}

void musictypeLoad(char* first, char* second)
{
	#ifdef _SDL
	if (strcmp(first, "music_type") == 0) {
		if (strcmp(second, "ogg") == 0) {
			setMusicType(1);
		}
		if (strcmp(second, "midi") == 0) {
			setMusicType(0);
		}
	}
	#endif
}
