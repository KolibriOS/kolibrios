#ifndef DISTANCE_METER_H
#define DISTANCE_METER_H

#include <stdbool.h>
#include <string.h>
#include "graphics.h"
#include "misc.h"

#define DM_MAX_DISTANCE_UNITS 5
#define DM_ACHIEVEMENT_DISTANCE 100
#define DM_FLASH_DURATION 1000/4
#define DM_FLASH_ITERATIONS 3
#define DM_WIDTH 10
#define DM_HEIGHT 13
#define DM_DEST_WIDTH 11
#define DM_X (DEFAULT_WIDTH - DM_DEST_WIDTH * (DM_MAX_DISTANCE_UNITS + 1))
#define DM_Y 5

typedef struct {
	int maxScore;
	char digits[16];
	char highScore[16];
	bool achievement;
	int flashTimer;
	int flashIterations;
	int maxScoreUnits;
} DistanceMeter;

extern DistanceMeter distanceMeter;

void distanceMeterInit();
void distanceMeterDraw(int digitPos, int value, bool opt_highscore);
int distanceMeterGetActualDistance(int distance);
bool distanceMeterUpdate(int deltaTime, int _distance);
void distanceMeterDrawHighScore();
void distanceMeterSetHighScore(int _distance);
void distanceMeterReset();

#endif
