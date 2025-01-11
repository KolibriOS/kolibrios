#ifndef CLOUD_H
#define CLOUD_H

#include <stdbool.h>
#include <math.h>
#include "graphics.h"
#include "misc.h"

#define CLOUD_WIDTH 46
#define CLOUD_HEIGHT 14
#define CLOUD_MAX_GAP 400
#define CLOUD_MAX_SKY_LEVEL 30
#define CLOUD_MIN_GAP 100
#define CLOUD_MIN_SKY_LEVEL 71

typedef struct {
	int width;
	int xPos;
	int yPos;
	bool remove;
	int cloudGap;
} Cloud;

void cloudInit(Cloud* cloud, int w);
void cloudDraw(const Cloud* cloud);
void cloudUpdate(Cloud* cloud, double speed);
bool cloudIsVisible(const Cloud* cloud);

#endif
