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
#define HORIZON_BUMPY_THRESHOLD 0.3
#define HORIZON_CLOUD_FREQUENCY 0.5
#define HORIZON_HORIZON_HEIGHT 16
#define HORIZON_MAX_CLOUDS 6

typedef struct {
	int dim_width;
	double gapCoefficient;
	Ulist* obstacles;
	Ulist* obstacleHistory;
	// nightMode
	Ulist* clouds;
} Horizon;

extern Horizon horizon;

void horizonInit(int dim_width, double gapCoefficient);
void horizonUpdate(int deltaTime, double currentSpeed, bool updateObstacles, bool showNightMode);
void horizonUpdateClouds(int deltaTime, double speed);
void horizonUpdateObstacles(int deltaTime, double currentSpeed);
//void horizonRemoveFirstObstacle();
void horizonAddNewObstacle(double currentSpeed);
bool horizonDuplicateObstacleCheck(ObstacleType nextObstacleType);
void horizonReset();
//void horizonResize(int width, int height);
void horizonAddCloud();

#endif
