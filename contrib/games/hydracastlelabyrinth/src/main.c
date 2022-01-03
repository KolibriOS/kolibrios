#include "PHL.h"
#include "game.h"
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef ODROID
#define _XTYPEDEF_MASK
#include <X11/Xlib.h>
#endif
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#ifdef __amigaos4__
static const char* __attribute__((used)) stackcookie = "$STACK: 1000000";
#endif
#ifdef __MORPHOS__
unsigned long __stack = 1000000;
#endif

#ifdef _KOLIBRI
extern char* dirname(char*);
extern void setcwd(char*);
#endif

void createSaveLocations()
{	
	//Force create save data folders	
	#ifdef _3DS
		//3DS builds
		mkdir("sdmc:/3ds", 0777);
		mkdir("sdmc:/3ds/appdata", 0777);
		mkdir("sdmc:/3ds/appdata/HydraCastleLabyrinth", 0777);
		mkdir("sdmc:/3ds/appdata/HydraCastleLabyrinth/data", 0777);
		mkdir("sdmc:/3ds/appdata/HydraCastleLabyrinth/map", 0777);
	#elif defined(_SDL)
		char buff[4096];
		#if defined(__amigaos4__) || defined(__MORPHOS__)
		strcpy(buff,"PROGDIR:.hydracastlelabyrinth");
		#elif defined(EMSCRIPTEN)
		strcpy(buff, "hcl_data");
		#elif defined (_KOLIBRI)
		mkdir(KOS_HCL_SAVES_PATH, 777);
		#else
		strcpy(buff, getenv("HOME"));
		strcat(buff, "/.hydracastlelabyrinth");
		#endif
		// if exist first?
		#ifndef _KOLIBRI
		struct stat sb;
		if(!(stat(buff, &sb)==0 && S_ISDIR(sb.st_mode)))
			mkdir(buff, 0777);
		#endif
	#else
		//psp, wii
		mkdir("/data", 0777);
		mkdir("/map", 0777);
	#endif
}

#ifdef EMSCRIPTEN
int fileSynched = 0;
#endif

int main(int argc, char **argv)
{	
	//Setup
	#ifdef _KOLIBRI
	setcwd(dirname(argv[0]));
	#endif

	#ifdef EMSCRIPTEN
	// that HEAP32 on &fileSynched looks like a hack, but I needed a way to be sure the DB is read before reading the ini files
	EM_ASM_INT({
		FS.mkdir('hcl_data'); 
		FS.mount(IDBFS,{},'hcl_data');
		Module.print("Will import permanent storage");
		FS.syncfs(true, function() {
			Module.print("Permanent storage imported");
			HEAP32[$0>>2] = 1;
		});
	}, &fileSynched);
	#endif
	#ifdef _3DS
		sdmcInit();
		osSetSpeedupEnable(false);
	#endif
	#ifdef _SDL
	if ( SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Delay(5000);
		exit(EXIT_FAILURE);
	}
	#if defined(PANDORA) || defined(PYRA) || defined(CHIP) || defined(ODROID)
	wantFullscreen = 1;
	#else
	wantFullscreen = 0;
	#endif
	#ifdef CHIP
	screenScale = 1;
	#elif defined(BITTBOY)
	screenScale = 1;
	#elif defined(PYRA)
	//screenScale = 3;
	desktopFS = 1;
	#elif defined(ODROID)
	desktopFS = 1;
	#else
	screenScale = 2;
	#endif
	useJoystick = 1;
	// get command line arguments
	for (int i=1; i<argc; i++)
	{
		if(!strcmp(argv[i], "-f"))
			wantFullscreen = 1;
		if(!strcmp(argv[i], "--fullscreen"))
			wantFullscreen = 1;
		if(!strcmp(argv[i], "-d"))
			desktopFS = 1;
		if(!strcmp(argv[i], "--desktop"))
			desktopFS = 1;
		if(!strcmp(argv[i], "-x1"))
			screenScale = 1;
		if(!strcmp(argv[i], "-x2"))
			screenScale = 2;
		if(!strcmp(argv[i], "-x3"))
			screenScale = 3;
		if(!strcmp(argv[i], "-x4"))
			screenScale = 4;
		if(!strcmp(argv[i], "-x5"))
			screenScale = 5;
		if(!strcmp(argv[i], "--xbrz"))
			setXBRZ(1);
		if(!strcmp(argv[i], "--no-xbrz"))
			setXBRZ(0);
		if(!strcmp(argv[i], "-j"))
			useJoystick = 0;
		if(!strcmp(argv[i], "--nojoy"))
			useJoystick = 0;
		if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			printf("Quick help\n-f|--fullscreen\tUse fullscreen\n-d|--desktop\tdesktop fullscreen\n-x1|-x2|-x3|-x4\tUse screenScale of *1..*4 (default *2 = 640x480)\n-j|-nojoy\tdo not use Joystick\n--xbrz\tUse xBRZ scaling\n--no-xbrz\tNo xBRZ scaling\n");
			exit(0);
		}
	}
	if(desktopFS)
	{
		#ifdef _SDL2
		SDL_DisplayMode infos;
		SDL_GetCurrentDisplayMode(0, &infos);
		screenW = infos.w;
		screenH = infos.h;
		#else
		const SDL_VideoInfo* infos = SDL_GetVideoInfo();
		screenH = infos->current_h;
		screenW = infos->current_w;
		#endif
	} else {
		screenW = 320 * screenScale;
		screenH = 240 * screenScale;
	}
	printf("Hydra Castle Labyrinth, %s %dx%d scale=x%d%s, using Joystick=%d\n", (wantFullscreen || desktopFS)?"Fullscreen":"Windowed", screenW, screenH, screenScale, getXBRZ()?" xBRZ":"", useJoystick);
	#endif

	srand(time(NULL));
	createSaveLocations();
	
	game();

	//System specific cleanup	
	#ifdef _PSP
		sceKernelExitGame();
	#endif
	
	#ifdef _3DS
		sdmcExit();
	#endif

	return 0;
}
