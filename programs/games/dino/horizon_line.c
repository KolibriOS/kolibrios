#include "horizon_line.h"

HorizonLine horizonLine;

void horizonLineInit() {
	horizonLine.width = HORIZON_LINE_WIDTH;
	horizonLine.height = HORIZON_LINE_HEIGHT;
	horizonLine.sourceXPos[0] = ATLAS_HORIZON_X;
	horizonLine.sourceXPos[1] = ATLAS_HORIZON_X + horizonLine.width;
	horizonLine.bumpThreshold = 0.5;
	horizonLine.xPos[0] = 0;
	horizonLine.xPos[1] = horizonLine.width;
	horizonLine.yPos = HORIZON_LINE_YPOS;
	horizonLineDraw();
}

void horizonLineDraw() {
	//printf("horizonLineDraw(); xPos[0] = %d, xPos[1] = %d, yPos = %d, width = %d, height = %d\n", horizonLine.xPos[0], horizonLine.xPos[1], horizonLine.yPos, horizonLine.width, horizonLine.height);
	graphicsBlitAtlasImage(horizonLine.sourceXPos[0], ATLAS_HORIZON_Y, horizonLine.xPos[0], horizonLine.yPos, horizonLine.width, horizonLine.height, false);
	graphicsBlitAtlasImage(horizonLine.sourceXPos[1], ATLAS_HORIZON_Y, horizonLine.xPos[1], horizonLine.yPos, horizonLine.width, horizonLine.height, false);
}

int horizonLineGetRandomType() {
	return (double)rand() / RAND_MAX > horizonLine.bumpThreshold ? horizonLine.width : 0;
}

void horizonLineUpdateXPos(int pos, int increment) {
	int line1 = pos;
	int line2 = pos == 0 ? 1 : 0;

	horizonLine.xPos[line1] -= increment;
	horizonLine.xPos[line2] = horizonLine.xPos[line1] + horizonLine.width;

	if (horizonLine.xPos[line1] <= -horizonLine.width) {
		horizonLine.xPos[line1] += horizonLine.width * 2;
		horizonLine.xPos[line2] = horizonLine.xPos[line1] - horizonLine.width;
		horizonLine.sourceXPos[line1] = horizonLineGetRandomType() + ATLAS_HORIZON_X;
	}
}

void horizonLineUpdate(int deltaTime, double speed) {
	int increment = floor(speed * (FPS / 1000.0) * deltaTime);
	if (horizonLine.xPos[0] <= 0) {
		horizonLineUpdateXPos(0, increment);
	}
	else {
		horizonLineUpdateXPos(1, increment);
	}
	// asm_inline("int $3");
	horizonLineDraw();
}

void horizonLineReset() {
	horizonLine.xPos[0] = 0;
	horizonLine.xPos[1] = horizonLine.width;
}
