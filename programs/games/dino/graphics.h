#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdbool.h>

#include <sys/ksys.h>

#include "config.h"

#define ATLAS_CACTUS_LARGE_X 332
#define ATLAS_CACTUS_LARGE_Y 2
#define ATLAS_CACTUS_SMALL_X 228
#define ATLAS_CACTUS_SMALL_Y 2
#define ATLAS_CLOUD_X 86
#define ATLAS_CLOUD_Y 2
#define ATLAS_HORIZON_X 2
#define ATLAS_HORIZON_Y 54
#define ATLAS_PTERODACTYL_X 134
#define ATLAS_PTERODACTYL_Y 2
#define ATLAS_RESTART_X 2
#define ATLAS_RESTART_Y 2
#define ATLAS_TEXT_SPRITE_X 655
#define ATLAS_TEXT_SPRITE_Y 2
#define ATLAS_TREX_X 848
#define ATLAS_TREX_Y 2

#define BACKGROUND_COLOR 0xF7F7F7

void graphicsBlitAtlasImage(int atlasX, int atlasY, int destX, int destY, int w, int h);
void graphicsFillBackground(void);
void graphicsRender();
void graphicsDelay(int ms);

#endif
