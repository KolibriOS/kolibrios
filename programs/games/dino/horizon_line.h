#ifndef HORIZON_LINE_H
#define HORIZON_LINE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "config.h"
#include "graphics.h"

#define HORIZON_LINE_WIDTH 600
#define HORIZON_LINE_HEIGHT 12
#define HORIZON_LINE_YPOS 127

typedef struct {
	int width;
	int height;
	int sourceXPos[2];
	int xPos[2];
	int yPos;
	double bumpThreshold;
} HorizonLine;

extern HorizonLine horizonLine;

void horizonLineInit();
void horizonLineDraw();
int horizonLineGetRandomType();
void horizonLineUpdateXPos(int pos, int increment);
void horizonLineUpdate(int deltaTime, double speed);
void horizonLineReset();

#endif
