#ifndef HORIZON_H
#define HORIZON_H

#include <math.h>
#include <stdbool.h>
#include "obstacle.h"
#include "cloud.h"
#include "horizon_line.h"
#include "runner.h"
#include "graphics.h"
#include "ulist.h"

#define HORIZON_BG_CLOUD_SPEED 0.2
#define HORIZON_CLOUD_FREQUENCY 0.5
#define HORIZON_MAX_CLOUDS 6

typedef struct {
	int dim_width;
	double gapCoefficient;
	Ulist* obstacles;
	Ulist* obstacleHistory;
	Ulist* clouds;
} Horizon;

extern Horizon horizon;

void horizonInit(int dim_width, double gapCoefficient);
void horizonUpdate(int deltaTime, double currentSpeed, bool updateObstacles);
void horizonUpdateClouds(int deltaTime, double speed);
void horizonUpdateObstacles(int deltaTime, double currentSpeed);
void horizonAddNewObstacle(double currentSpeed);
bool horizonDuplicateObstacleCheck(ObstacleType nextObstacleType);
void horizonReset();
void horizonAddCloud();

#endif
