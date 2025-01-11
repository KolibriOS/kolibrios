#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include "graphics.h"
#include "misc.h"
#include "config.h"
#include "collisionbox.h"

// Coefficient for calculating the maximum gap
#define OBSTACLE_MAX_GAP_COEFFICIENT 1.5

// Maximum obstacle grouping count
#define OBSTACLE_MAX_OBSTACLE_LENGTH 3

typedef enum {
    CACTUS_SMALL = 0,
    CACTUS_LARGE = 1,
    PTERODACTYL = 2
} ObstacleType;

extern int obstacleSpritePosX[3];
extern int obstacleSpritePosY[3];

typedef struct {
    ObstacleType type;
    int width;
    int height;
    int yPos;
    int yPosArrSize;
    int yPosArr[3]; // used if yPos is -1
    int multipleSpeed;
    int minGap;
    int minSpeed;
    int collisionBoxesCount;
    CollisionBox collisionBoxes[5];
    int numFrames;
    double frameRate;
    double speedOffset;
} ObstacleTypeConfig;

typedef struct {
    ObstacleTypeConfig typeConfig;
    double gapCoefficient;
    int size;
    bool remove;
    int xPos;
    int yPos;
    int width;
    int gap;
    // double speedOffset;
    int currentFrame;
    int timer;
    bool followingObstacleCreated;
} Obstacle;

extern ObstacleTypeConfig obstacleTypeConfigs[3];

void obstacleInit(Obstacle *ob, const ObstacleTypeConfig *otc, int dim_width, double gapCoefficient, double speed, int opt_xOffset);
void obstacleDraw(const Obstacle* ob);
void obstacleUpdate(Obstacle* ob, int deltaTime, double speed);
int obstacleGetGap(const Obstacle* ob, double gapCoefficient, double speed);
bool obstacleIsVisible(const Obstacle* ob);

#endif
