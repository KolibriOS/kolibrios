#include "obstacle.h"

ObstacleTypeConfig obstacleTypeConfigs[3] = {
		{
			.type = CACTUS_SMALL,
			.width = 17,
			.height = 35,
			.yPos = 105,
			.multipleSpeed = 4,
			.minGap = 120,
			.minSpeed = 0,
			.collisionBoxesCount = 3,
			.collisionBoxes = {
				{.x = 0, .y = 7, .width = 5, .height = 27},
				{.x = 4, .y = 0, .width = 6, .height = 34},
				{.x = 10, .y = 4, .width = 7, .height = 14}
			},
			.numFrames = 1,
			.speedOffset = 0
		},
		{
			.type = CACTUS_LARGE,
			.width = 25,
			.height = 50,
			.yPos = 90,
			.multipleSpeed = 7,
			.minGap = 120,
			.minSpeed = 0,
			.collisionBoxesCount = 3,
			.collisionBoxes = {
				{.x = 0, .y = 12, .width = 7, .height = 38},
				{.x = 8, .y = 0, .width = 7, .height = 49},
				{.x = 13, .y = 10, .width = 10, .height = 38}
			},
			.numFrames = 1,
			.speedOffset = 0
		},
		{
			.type = PTERODACTYL,
			.width = 46,
			.height = 40,
			.yPos = -1,
			.yPosArrSize = 3,
			.yPosArr = {100, 75, 50},
			.multipleSpeed = 999,
			.minGap = 150,
			.minSpeed = 8.5,
			.collisionBoxesCount = 5,
			.collisionBoxes = {
				{.x = 15, .y = 15, .width = 16, .height = 5},
				{.x = 18, .y = 21, .width = 24, .height = 6},
				{.x = 2, .y = 14, .width = 4, .height = 3},
				{.x = 6, .y = 10, .width = 4, .height = 7},
				{.x = 10, .y = 8, .width = 6, .height = 9}
			},
			.numFrames = 2,
			.frameRate = 1000 / 6,
			.speedOffset = 0.8
		}
};

int obstacleSpritePosX[3] = { ATLAS_CACTUS_SMALL_X, ATLAS_CACTUS_LARGE_X, ATLAS_PTERODACTYL_X};
int obstacleSpritePosY[3] = { ATLAS_CACTUS_SMALL_Y, ATLAS_CACTUS_LARGE_Y, ATLAS_PTERODACTYL_Y};


void obstacleInit(Obstacle* ob, const ObstacleTypeConfig *otc, int dim_width, double gapCoefficient, double speed, int opt_xOffset) {
	ob->typeConfig = *otc;
	ob->gapCoefficient = gapCoefficient;
	ob->size = getRandomNumber(1, OBSTACLE_MAX_OBSTACLE_LENGTH);
	ob->remove = false;
	ob->xPos = dim_width + opt_xOffset;
	ob->yPos = 0;

	// For animated obstacles
	ob->currentFrame = 0;
	ob->timer = 0;

	ob->followingObstacleCreated = false;

	if (ob->size > 1 && ob->typeConfig.multipleSpeed > speed) { // NOTE what it this?
		ob->size = 1;
	}
	ob->width = ob->typeConfig.width * ob->size;

	if (ob->typeConfig.yPos == -1) {
		ob->yPos = ob->typeConfig.yPosArr[getRandomNumber(0, ob->typeConfig.yPosArrSize)];
	}
	else {
		ob->yPos = ob->typeConfig.yPos;
	}

	obstacleDraw(ob);

	// Make collision box adjustments,
	// Central box is adjusted to the size as one box.
	//      ____        ______        ________
	//    _|   |-|    _|     |-|    _|       |-|
	//   | |<->| |   | |<--->| |   | |<----->| |
	//   | | 1 | |   | |  2  | |   | |   3   | |
	//   |_|___|_|   |_|_____|_|   |_|_______|_|
	//

	if (ob->size > 1) {
		ob->typeConfig.collisionBoxes[1].width = ob->width - ob->typeConfig.collisionBoxes[0].width - ob->typeConfig.collisionBoxes[2].width;
		ob->typeConfig.collisionBoxes[2].x = ob->width - ob->typeConfig.collisionBoxes[2].width;
	}

	// For obstacles that go at a different speed from the horizon
	if (ob->typeConfig.speedOffset) {
		ob->typeConfig.speedOffset = (double)rand() / RAND_MAX > 0.5 ? ob->typeConfig.speedOffset : -ob->typeConfig.speedOffset;
	}
	ob->gap = obstacleGetGap(ob, ob->gapCoefficient, speed);
}

void obstacleDraw(const Obstacle *ob) {
	int sourceWidth = ob->typeConfig.width;
	int sourceHeight = ob->typeConfig.height;
	int sourceX = (sourceWidth * ob->size) * (0.5 * ((double)ob->size - 1)) + obstacleSpritePosX[ob->typeConfig.type];
	if (ob->currentFrame > 0) {
		sourceX += sourceWidth*ob->currentFrame;
	}
	//dbg_printf("od ax=%u, ay=%u, dx=%u, dy=%u, w=%u, h=%u\n", sourceX, obstacleSpritePosY[ob->typeConfig.type], ob->xPos, ob->yPos, sourceWidth*ob->size, sourceHeight);
	graphicsBlitAtlasImage(sourceX, obstacleSpritePosY[ob->typeConfig.type], ob->xPos, ob->yPos, sourceWidth*ob->size, sourceHeight, false);
}

void obstacleUpdate(Obstacle *ob, int deltaTime, double speed) {
	if (!ob->remove) {
		double dx = floor(((speed + ob->typeConfig.speedOffset)*FPS/1000.)*deltaTime);
		//dbg_printf("sp = %lf, ots = %lf, dx = %d, xpos = %d\n", speed, ob->typeConfig.speedOffset, (int)dx, ob->xPos - dx);
		ob->xPos -= dx;//floor(((speed + ob->typeConfig.speedOffset)*FPS/1000.)*deltaTime);
	}
	// Update frames
	if (ob->typeConfig.numFrames > 1) {
		ob->timer += deltaTime;
		if (ob->timer >= ob->typeConfig.frameRate) {
			ob->currentFrame = ob->currentFrame == ob->typeConfig.numFrames - 1 ? 0 : ob->currentFrame + 1;
			ob->timer = 0;
		}
	}
	obstacleDraw(ob);
	if (!obstacleIsVisible(ob)) {
		ob->remove = true;
	}
}

int obstacleGetGap(const Obstacle *ob, double gapCoefficient, double speed) {
	int minGap = round(ob->width * speed + ob->typeConfig.minGap * gapCoefficient);
	int maxGap = round(minGap * OBSTACLE_MAX_GAP_COEFFICIENT);
	return getRandomNumber(minGap, maxGap);
}

bool obstacleIsVisible(const Obstacle* ob) {
	return ob->xPos + ob->width > 0;
}


