/*
PHL stands for Portable Homebrew Library
*/
#ifndef PHL_H
#define PHL_H

#ifdef _3DS
	#include "3ds/system.h"
	#include "3ds/graphics.h"
	#include "3ds/input.h"
	#include "3ds/audio.h"
#endif

#ifdef _WII
	#include "wii/system.h"
	#include "wii/graphics.h"
	#include "wii/input.h"
	#include "wii/audio.h"
#endif

#ifdef _PSP
	#include "psp/system.h"
	#include "psp/graphics.h"
	#include "psp/input.h"
	#include "psp/audio.h"
#endif

#ifdef _SDL
	#ifdef _SDL2
		#include "sdl2/system.h"
		#include "sdl2/graphics.h"
		#include "sdl2/input.h"
		#include "sdl2/audio.h"
	#else
		#include "sdl/system.h"
		#include "sdl/graphics.h"
		#include "sdl/input.h"
		#include "sdl/audio.h"
	#endif
#endif


typedef struct {
	int x, y, w, h;
} PHL_Rect;

void PHL_Init();
void PHL_Deinit();

extern int WHITE, RED, YELLOW;

PHL_Surface PHL_LoadQDA(char* fname);
void PHL_DrawTextBold(char* txt, int dx, int dy, int col);
void PHL_DrawTextBoldCentered(char* txt, int dx, int dy, int col);

#endif
