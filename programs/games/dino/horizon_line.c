#include "horizon_line.h"

HorizonLine horizonLine;

void horizonLineInit() {
	// in BSS: xPos[0] starts at zero
	horizonLine.sourceXPos[0] = ATLAS_HORIZON_X;
	horizonLine.sourceXPos[1] = ATLAS_HORIZON_X + HORIZON_LINE_WIDTH;
	horizonLine.xPos[1] = HORIZON_LINE_WIDTH;
	horizonLineDraw();
}

void horizonLineDraw() {
	graphicsBlitAtlasImage(horizonLine.sourceXPos[0], ATLAS_HORIZON_Y, horizonLine.xPos[0], HORIZON_LINE_YPOS, HORIZON_LINE_WIDTH, HORIZON_LINE_HEIGHT);
	graphicsBlitAtlasImage(horizonLine.sourceXPos[1], ATLAS_HORIZON_Y, horizonLine.xPos[1], HORIZON_LINE_YPOS, HORIZON_LINE_WIDTH, HORIZON_LINE_HEIGHT);
}

int horizonLineGetRandomType() {
	// bump threshold 0.5
	return rand() > RAND_MAX / 2 ? HORIZON_LINE_WIDTH : 0;
}

void horizonLineUpdateXPos(int pos, int increment) {
	int line1 = pos;
	int line2 = pos == 0 ? 1 : 0;

	horizonLine.xPos[line1] -= increment;
	horizonLine.xPos[line2] = horizonLine.xPos[line1] + HORIZON_LINE_WIDTH;

	if (horizonLine.xPos[line1] <= -HORIZON_LINE_WIDTH) {
		horizonLine.xPos[line1] += HORIZON_LINE_WIDTH * 2;
		horizonLine.xPos[line2] = horizonLine.xPos[line1] - HORIZON_LINE_WIDTH;
		horizonLine.sourceXPos[line1] = horizonLineGetRandomType() + ATLAS_HORIZON_X;
	}
}

void horizonLineUpdate(int deltaTime, double speed) {
	// value is always positive, so truncation == floor
	int increment = speed * (FPS / 1000.0) * deltaTime;
	if (horizonLine.xPos[0] <= 0) {
		horizonLineUpdateXPos(0, increment);
	}
	else {
		horizonLineUpdateXPos(1, increment);
	}
	horizonLineDraw();
}

void horizonLineReset() {
	horizonLine.xPos[0] = 0;
	horizonLine.xPos[1] = HORIZON_LINE_WIDTH;
}
