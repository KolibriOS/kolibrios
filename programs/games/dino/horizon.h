#ifndef HORIZON_H
#define HORIZON_H

#include <math.h>
#include <stdbool.h>
#include "obstacle.h"
#include "cloud.h"
#include "horizon_line.h"
#include "runner.h"
#include "graphics.h"

#define HORIZON_BG_CLOUD_SPEED 0.2
#define HORIZON_CLOUD_FREQUENCY 0.5
#define HORIZON_MAX_CLOUDS 6

// Worst-case on-screen obstacles: gaps never drop below ~190px over a
// ~700px span, so 5 is the practical ceiling; 8 leaves margin.
#define HORIZON_MAX_OBSTACLES 8
#define HORIZON_MAX_OBSTACLE_DUPLICATION 2

typedef struct {
	int dim_width;
	double gapCoefficient;
	Obstacle obstacles[HORIZON_MAX_OBSTACLES];
	int obstacleCount;
	ObstacleType obstacleHistory[HORIZON_MAX_OBSTACLE_DUPLICATION];
	int obstacleHistoryCount;
	Cloud clouds[HORIZON_MAX_CLOUDS];
	int cloudCount;
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
