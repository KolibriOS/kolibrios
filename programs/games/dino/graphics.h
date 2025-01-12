#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <sys/ksys.h>
#include <clayer/libimg.h>

#include "config.h"

#define ATLAS_CACTUS_LARGE_X 332
#define ATLAS_CACTUS_LARGE_Y 2
#define ATLAS_CACTUS_SMALL_X 228
#define ATLAS_CACTUS_SMALL_Y 2
#define ATLAS_CLOUD_X 86
#define ATLAS_CLOUD_Y 2
#define ATLAS_HORIZON_X 2
#define ATLAS_HORIZON_Y 54
#define ATLAS_MOON_X 484
#define ATLAS_MOON_Y 2
#define ATLAS_PTERODACTYL_X 134
#define ATLAS_PTERODACTYL_Y 2
#define ATLAS_RESTART_X 2
#define ATLAS_RESTART_Y 2
#define ATLAS_TEXT_SPRITE_X 655
#define ATLAS_TEXT_SPRITE_Y 2
#define ATLAS_TREX_X 848
#define ATLAS_TREX_Y 2
#define ATLAS_STAR_X 645
#define ATLAS_STAR_Y 2

extern Image* screenImage;

void graphicsInit();
void graphicsBlitAtlasImage(int atlasX, int atlasY, int destX, int destY, int w, int h, bool center);
void graphicsFillBackground(unsigned r, unsigned g, unsigned b);
void graphicsRender();
void graphicsDelay(int ms);
void graphicsDestroy();

#endif
