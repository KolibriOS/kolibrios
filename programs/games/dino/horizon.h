#ifndef HORIZON_H
#define HORIZON_H

#include <stdbool.h>
#include "obstacle.h"
#include "cloud.h"
#include "horizon_line.h"
#include "runner.h"
#include "graphics.h"

#define HORIZON_BG_CLOUD_SPEED 0.2
#define HORIZON_MAX_CLOUDS 6

#define HORIZON_MAX_OBSTACLES 8 // ~5 fit on screen at the smallest gaps
#define HORIZON_MAX_OBSTACLE_DUPLICATION 2

typedef struct {
	Obstacle obstacles[HORIZON_MAX_OBSTACLES];
	int obstacleCount;
	ObstacleType obstacleHistory[HORIZON_MAX_OBSTACLE_DUPLICATION];
	int obstacleHistoryCount;
	Cloud clouds[HORIZON_MAX_CLOUDS];
	int cloudCount;
} Horizon;

extern Horizon horizon;

void horizonInit();
void horizonUpdate(int deltaTime, double currentSpeed, bool updateObstacles);
void horizonUpdateClouds(int deltaTime, double speed);
void horizonUpdateObstacles(int deltaTime, double currentSpeed);
void horizonAddNewObstacle(double currentSpeed);
bool horizonDuplicateObstacleCheck(ObstacleType nextObstacleType);
void horizonReset();
void horizonAddCloud();

#endif
