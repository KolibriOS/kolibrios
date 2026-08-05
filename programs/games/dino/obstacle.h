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

// Coefficient for calculating the minimum gap
#define OBSTACLE_GAP_COEFFICIENT 0.6

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
    float minSpeed; // 8.5 for the pterodactyl: exact in float, must not truncate
    int collisionBoxesCount;
    CollisionBox collisionBoxes[5];
    int numFrames;
    float frameRate;
    double speedOffset;
} ObstacleTypeConfig;

typedef struct {
    ObstacleTypeConfig typeConfig;
    int size;
    bool remove;
    int xPos;
    int yPos;
    int width;
    int gap;
    int currentFrame;
    int timer;
    bool followingObstacleCreated;
} Obstacle;

extern ObstacleTypeConfig obstacleTypeConfigs[3];

void obstacleInit(Obstacle *ob, const ObstacleTypeConfig *otc, double speed, int opt_xOffset);
void obstacleDraw(const Obstacle* ob);
void obstacleUpdate(Obstacle* ob, int deltaTime, double speed);
int obstacleGetGap(const Obstacle* ob, double speed);
bool obstacleIsVisible(const Obstacle* ob);

#endif
