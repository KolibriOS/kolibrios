#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL.h>

#define PHL_Surface SDL_Surface*

#define PHL_RGB SDL_Color

typedef struct {
	int tileX[16][12];
	int tileY[16][12];
} PHL_Background;

/*
typedef struct {
	unsigned int r, g, b;
} PHL_RGB;
*/
/*
typedef struct {
	OSL_IMAGE* pxdata;
	int width;
	int height;
	PHL_RGB colorKey;
} PHL_Surface;
*/
extern PHL_Surface screen;

extern int wantFullscreen;
extern int screenScale;
extern int desktopFS;

extern int deltaX;
extern int deltaY;

extern int screenW;
extern int screenH;

SDL_Color PHL_NewRGB(uint8_t r, uint8_t g, uint8_t b);
/*
{
    SDL_Color ret = {.r = r, .b = b, .g = g};
    return ret;
}
*/
void PHL_GraphicsInit();
void PHL_GraphicsExit();

void PHL_StartDrawing();
void PHL_EndDrawing();

void PHL_ForceScreenUpdate();

void PHL_SetDrawbuffer(PHL_Surface surf);
void PHL_ResetDrawbuffer();

//PHL_RGB PHL_NewRGB(int r, int g, int b);
void PHL_SetColorKey(PHL_Surface surf, int r, int g, int b);

PHL_Surface PHL_NewSurface(int w, int h);
void PHL_FreeSurface(PHL_Surface surf);

//PHL_Surface PHL_LoadBMP(char* fname);
PHL_Surface PHL_LoadBMP(int index);

void PHL_DrawRect(int x, int y, int w, int h, SDL_Color col);

void PHL_DrawSurface(double x, double y, PHL_Surface surface);
void PHL_DrawSurfacePart(double x, double y, int cropx, int cropy, int cropw, int croph, PHL_Surface surface);

void PHL_DrawBackground(PHL_Background back, PHL_Background fore);
void PHL_UpdateBackground(PHL_Background back, PHL_Background fore);

int getXBRZ();
void setXBRZ(int active);

#endif