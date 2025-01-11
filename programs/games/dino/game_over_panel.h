#ifndef GAME_OVER_PANEL_H
#define GAME_OVER_PANEL_H

#include <math.h>
#include "graphics.h"

#define GOP_TEXT_X 0
#define GOP_TEXT_Y 13
#define GOP_TEXT_WIDTH 191
#define GOP_TEXT_HEIGHT 11
#define GOP_RESTART_WIDTH 36
#define GOP_RESTART_HEIGHT 32

typedef struct {
	int width;
	int height;
} GameOverPanel;

extern GameOverPanel gameOverPanel;

void gameOverPanelInit(int width, int height);
void gameOverPanelDraw();

#endif
