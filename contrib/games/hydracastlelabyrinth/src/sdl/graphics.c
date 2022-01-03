#include <SDL.h>
#include "../PHL.h"
#include "../game.h"
#include "../qda.h"
#include "graphics.h"
#include "scale.h"
#include <stdlib.h>
#include <string.h>

#if defined(__amigaos4__) || defined(__MORPHOS__)
#include "../amigaos.h"
#endif

SDL_Surface* screen = NULL;
SDL_Surface* drawbuffer = NULL;
SDL_Surface* backbuffer = NULL;

int wantFullscreen = 0;
int screenScale = 2;
int desktopFS = 0;

int deltaX = 0;
int deltaY = 0;

int screenW = 640;
int screenH = 480;

int drawscreen = 0;

int xbrz = 0;

static uint32_t tframe;

extern void Input_InitJoystick();
extern void Input_CloseJoystick();

int getXBRZ()
{
	return xbrz;
}

void setXBRZ(int active)
{
#ifdef _KOLIBRI
	xbrz = 0; // Problems with "xBRZ". Temporarily not used.
#else
	if(active) active = 1;
	if(xbrz==active) return;
	xbrz = active;

	// try to reload everything, but boss ressource will not be reloaded
	freeImages();
	loadImages();
#endif
}


SDL_Color PHL_NewRGB(uint8_t r, uint8_t g, uint8_t b)
{
    SDL_Color ret = {.r = r, .b = b, .g = g};
    return ret;
}

void PHL_GraphicsInit()
{
	SDL_ShowCursor(SDL_DISABLE);

	Input_InitJoystick();
    	#ifdef __MORPHOS__
		uint32_t flags = SDL_SWSURFACE;
	#else
    		uint32_t flags = SDL_HWSURFACE|SDL_DOUBLEBUF;
	#endif
	if(wantFullscreen || desktopFS)
    	flags |= SDL_FULLSCREEN;
    screen = SDL_SetVideoMode((desktopFS)?0:screenW, (desktopFS)?0:screenH, 0, flags);
	if(desktopFS)
	{
		const SDL_VideoInfo* infos = SDL_GetVideoInfo();
		screenH = infos->current_h;
		screenW = infos->current_w;

		if(screenW/320 < screenH/240)
			screenScale = screenW/320;
		else
			screenScale = screenH/240; // automatic guess the scale
		deltaX = (screenW-320*screenScale)/2;
		deltaY = (screenH-240*screenScale)/2;
	}
	
	SDL_WM_SetCaption("Hydra Castle Labyrinth", NULL);
	
	drawbuffer = screen;
	drawscreen = 1;
	backbuffer = SDL_CreateRGBSurface(0, 320*screenScale, 240*screenScale, 32, 0, 0, 0, 0);
	tframe = SDL_GetTicks();
}

void PHL_GraphicsExit()
{
	Input_CloseJoystick();
    SDL_FreeSurface(backbuffer);
	SDL_Quit();    
}

void PHL_StartDrawing()
{
	PHL_ResetDrawbuffer();
}
void PHL_EndDrawing()
{
	//implement some crude frameskiping, limited to 2 frame skip
	static int skip = 0;
	uint32_t tnext = tframe + 1000/60;
	if (SDL_GetTicks()>tnext && skip<2) {
		tframe += 1000/60;
		skip++;
		return;
	}

	// handle black borders
	if(deltaX) {
		SDL_Rect rect = {0, 0, deltaX, screenH};
		SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0, 0, 0));
		rect.x = screenW - deltaX -1;
		SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0, 0, 0));
	}
	if(deltaY) {
		SDL_Rect rect = {0, 0, screenW, deltaY};
		SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0, 0, 0));
		rect.y = screenH - deltaY -1;
		SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0, 0, 0));
	}

    SDL_Flip(screen);
	while((tframe = SDL_GetTicks())<tnext)
        SDL_Delay(10);
}

void PHL_ForceScreenUpdate()
{

}

void PHL_SetDrawbuffer(PHL_Surface surf)
{
	drawbuffer = surf;
	drawscreen = (surf==screen);
}
void PHL_ResetDrawbuffer()
{
	drawbuffer = screen;
	drawscreen = 1;
}

//PHL_RGB PHL_NewRGB(int r, int g, int b);
void PHL_SetColorKey(PHL_Surface surf, int r, int g, int b)
{
    SDL_SetColorKey(surf, SDL_SRCCOLORKEY|SDL_RLEACCEL, SDL_MapRGB(surf->format, r, g, b));
}

PHL_Surface PHL_NewSurface(int w, int h)
{
	if(getXBRZ())
		return SDL_CreateRGBSurface(0, w, h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	else
    	return SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0);
}
void PHL_FreeSurface(PHL_Surface surf)
{
    SDL_FreeSurface(surf);
}

//PHL_Surface PHL_LoadBMP(char* fname);
PHL_Surface PHL_LoadBMP(int index)
{
	SDL_Surface* surf;
	
	FILE* f;
	if ( (f = fopen("data/bmp.qda", "rb")) ) {
		uint8_t* QDAFile = (uint8_t*)malloc(headers[index].size);
		fseek(f, headers[index].offset, SEEK_SET);
		int tmp = fread(QDAFile, 1, headers[index].size, f);
		fclose(f);
		
		uint16_t w, h;
		
		//Read data from header
		memcpy(&w, &QDAFile[18], 2);
		memcpy(&h, &QDAFile[22], 2);
		#if defined(__amigaos4__) || defined(__MORPHOS__)
		BE16(&w); BE16(&h);
		#endif
		
        	surf = PHL_NewSurface(w * screenScale, h * screenScale);
		//surf = PHL_NewSurface(200, 200);

		//Load Palette
		int dx, dy;
		int count = 0;

		if(getXBRZ()) {
#ifndef _KOLIBRI
			Uint32 palette[20][18];

			for (dx = 0; dx < 20; dx++) {
				for (dy = 0; dy < 16; dy++) {
					palette[dx][dy] = 255<<24 | ((Uint32)QDAFile[54 + count])<<0 | ((Uint32)QDAFile[54 + count + 1])<<8 | ((Uint32)QDAFile[54 + count + 2])<<16;
					count += 4;
				}
			}
			Uint32* tmp = NULL;
			tmp = (Uint32*)malloc(w*h*screenScale*4);
			Uint32 transp;
			for (dx = 0; dx < w; dx++) {
				for (dy = 0; dy < h; dy++) {
				
					int pix = dx + w * dy;
					int px = QDAFile[1078 + pix] / 16;
					int py = QDAFile[1078 + pix] % 16;
					//Get transparency from first palette color
					if (dx == 0 && dy == 0) {					
						//Darkness special case
						if (index == 27)
							transp = 255<<24;
						else
							transp = palette[0][0];
					}
					
					Uint32 c = palette[px][py];
					if(c==transp)
						c=0;
					tmp[(h-1-dy)*w+dx] = c;
				}
			}

		    xbrz_scale((void*)tmp, (void*)surf->pixels, w, h, screenScale);
			free(tmp);
#endif
		} else {
			PHL_RGB palette[20][18];

			for (dx = 0; dx < 20; dx++) {
				for (dy = 0; dy < 16; dy++) {
					palette[dx][dy].b = QDAFile[54 + count];
					palette[dx][dy].g = QDAFile[54 + count + 1];
					palette[dx][dy].r = QDAFile[54 + count + 2];
					count += 4;
				}
			}
			for (dx = 0; dx < w; dx++) {
				for (dy = 0; dy < h; dy++) {
				
					int pix = dx + w * dy;
					int px = QDAFile[1078 + pix] / 16;
					int py = QDAFile[1078 + pix] % 16;
					//Get transparency from first palette color
					if (dx == 0 && dy == 0) {					
						//Darkness special case
						if (index == 27) {
							SDL_SetColorKey(surf, SDL_SRCCOLORKEY|SDL_RLEACCEL, SDL_MapRGB(surf->format, 0x00, 0x00, 0x00));
						}else{
							SDL_SetColorKey(surf, SDL_SRCCOLORKEY|SDL_RLEACCEL, SDL_MapRGB(surf->format, palette[0][0].r, palette[0][0].g, palette[0][0].b));
						}
					}
					
					PHL_RGB c = palette[px][py];
					//PHL_DrawRect(dx * 2, dy * 2, 2, 2, c);
					SDL_Rect rect = {dx * screenScale, (h-1-dy) * screenScale, screenScale, screenScale};	
					SDL_FillRect(surf, &rect, SDL_MapRGB(surf->format, c.r, c.g, c.b));
				}
			}
		}
		free(QDAFile);
	}
	
	return surf;
}

void PHL_DrawRect(int x, int y, int w, int h, SDL_Color col)
{
	SDL_Rect rect = {x*screenScale/2 + (drawscreen?deltaX:0), y*screenScale/2 + (drawscreen?deltaY:0), w*screenScale/2, h*screenScale/2};
	
	SDL_FillRect(drawbuffer, &rect, SDL_MapRGB(drawbuffer->format, col.r, col.g, col.b));
}

void PHL_DrawSurface(double x, double y, PHL_Surface surface)
{
	if (quakeTimer > 0) {
		int val = quakeTimer % 4;
		if (val == 0) {
			y -= 1;
		} else if (val == 2) {
			y += 1;
		}
	}
	
	SDL_Rect offset;
	offset.x = x*screenScale/2 + (drawscreen?deltaX:0);
	offset.y = y*screenScale/2 + (drawscreen?deltaY:0);
	
	SDL_BlitSurface(surface, NULL, drawbuffer, &offset);
}
void PHL_DrawSurfacePart(double x, double y, int cropx, int cropy, int cropw, int croph, PHL_Surface surface)
{
	if (quakeTimer > 0) {
		int val = quakeTimer % 4;
		if (val == 0) {
			y -= (screenScale==1)?2:1;
		}else if (val == 2) {
			y += (screenScale==1)?2:1;
		}
	}
	
	SDL_Rect crop, offset;
	crop.x = cropx*screenScale/2;
	crop.y = cropy*screenScale/2;
	crop.w = cropw*screenScale/2;
	crop.h = croph*screenScale/2;
	
	offset.x = x*screenScale/2 + (drawscreen?deltaX:0);
	offset.y = y*screenScale/2 + (drawscreen?deltaY:0);
	
	SDL_BlitSurface(surface, &crop, drawbuffer, &offset);
}

void PHL_DrawBackground(PHL_Background back, PHL_Background fore)
{
	PHL_DrawSurface(0, 0, backbuffer);
}
void PHL_UpdateBackground(PHL_Background back, PHL_Background fore)
{
	PHL_SetDrawbuffer(backbuffer);
	
	int xx, yy;

	for (yy = 0; yy < 12; yy++)
	{
		for (xx = 0; xx < 16; xx++)
		{
			//Draw Background tiles
			PHL_DrawSurfacePart(xx * 40, yy * 40, back.tileX[xx][yy] * 40, back.tileY[xx][yy] * 40, 40, 40, images[imgTiles]);
			
			//Only draw foreground tile if not a blank tile
			if (fore.tileX[xx][yy] != 0 || fore.tileY[xx][yy] != 0) {
				PHL_DrawSurfacePart(xx * 40, yy * 40, fore.tileX[xx][yy] * 40, fore.tileY[xx][yy] * 40, 40, 40, images[imgTiles]);
			}
		}
	}
	
	PHL_ResetDrawbuffer();
}
