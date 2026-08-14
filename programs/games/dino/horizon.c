#include "horizon.h"

Horizon horizon;

void horizonInit() {
	// in BSS: the counts start at zero
	horizonAddCloud();

    horizonLineInit();
}

void horizonUpdate(int deltaTime, double currentSpeed, bool updateObstacles) {
	horizonLineUpdate(deltaTime, currentSpeed);
	horizonUpdateClouds(deltaTime, currentSpeed);
	if (updateObstacles) {
		horizonUpdateObstacles(deltaTime, currentSpeed);
	}
}

void horizonUpdateClouds(int deltaTime, double speed) {
    double cloudSpeed = HORIZON_BG_CLOUD_SPEED / 1000 * deltaTime * speed;

    if (horizon.cloudCount) {
        // Update (and draw) the newest cloud first
        for (int i = horizon.cloudCount - 1; i >= 0; i--) {
            cloudUpdate(&horizon.clouds[i], cloudSpeed);
        }
        Cloud *lastCloud = &horizon.clouds[horizon.cloudCount - 1];
        // Check for adding a new cloud (frequency 0.5)
        if (horizon.cloudCount < HORIZON_MAX_CLOUDS && (DEFAULT_WIDTH - lastCloud->xPos) > lastCloud->cloudGap && rand() <= RAND_MAX / 2) {
            horizonAddCloud();
        }
        // Remove expired clouds
        int kept = 0;
        for (int i = 0; i < horizon.cloudCount; i++) {
            if (!horizon.clouds[i].remove) {
                if (kept != i) {
                    horizon.clouds[kept] = horizon.clouds[i];
                }
                kept++;
            }
        }
        horizon.cloudCount = kept;
    }
    else {
        horizonAddCloud();
    }
}

void horizonUpdateObstacles(int deltaTime, double currentSpeed) {
    for (int i = 0; i < horizon.obstacleCount; i++) {
        obstacleUpdate(&horizon.obstacles[i], deltaTime, currentSpeed);
    }
    // Clean up removed obstacles
    int kept = 0;
    for (int i = 0; i < horizon.obstacleCount; i++) {
        if (!horizon.obstacles[i].remove) {
            if (kept != i) {
                horizon.obstacles[kept] = horizon.obstacles[i];
            }
            kept++;
        }
    }
    horizon.obstacleCount = kept;

    if (horizon.obstacleCount > 0) {
        Obstacle *lastObstacle = &horizon.obstacles[horizon.obstacleCount - 1];

        if (!lastObstacle->followingObstacleCreated && obstacleIsVisible(lastObstacle) && (lastObstacle->xPos + lastObstacle->width + lastObstacle->gap) < DEFAULT_WIDTH) {
            horizonAddNewObstacle(currentSpeed);
            lastObstacle->followingObstacleCreated = true;
        }
    }
    else {
        // Create new obstacles.
        horizonAddNewObstacle(currentSpeed);
    }
}

void horizonAddNewObstacle(double currentSpeed) {
    if (horizon.obstacleCount >= HORIZON_MAX_OBSTACLES) {
        return; // no room; retried on a later frame
    }
    int obstacleTypeIndex = getRandomNumber(0, sizeof(obstacleTypeConfigs)/sizeof(ObstacleTypeConfig) - 1);
    ObstacleTypeConfig *otc = &obstacleTypeConfigs[obstacleTypeIndex];

    // Check for multiples of the same type of obstacle.
    // Also check obstacle is available at current speed.
    if (horizonDuplicateObstacleCheck(otc->type) || currentSpeed < otc->minSpeed) {
        horizonAddNewObstacle(currentSpeed);
    }
    else {
        obstacleInit(&horizon.obstacles[horizon.obstacleCount], otc, currentSpeed, otc->width);
        horizon.obstacleCount++;
        // Record the type, newest first, keeping the last few entries
        for (int i = HORIZON_MAX_OBSTACLE_DUPLICATION - 1; i > 0; i--) {
            horizon.obstacleHistory[i] = horizon.obstacleHistory[i - 1];
        }
        horizon.obstacleHistory[0] = otc->type;
        if (horizon.obstacleHistoryCount < HORIZON_MAX_OBSTACLE_DUPLICATION) {
            horizon.obstacleHistoryCount++;
        }
    }
}

bool horizonDuplicateObstacleCheck(ObstacleType nextObstacleType) {
    int duplicateCount = 0;
    for (int i = 0; i < horizon.obstacleHistoryCount; i++) {
        duplicateCount = horizon.obstacleHistory[i] == nextObstacleType ? duplicateCount + 1 : 0;
    }
    return duplicateCount >= HORIZON_MAX_OBSTACLE_DUPLICATION;
}

void horizonReset() {
    horizon.obstacleCount = 0;
    horizonLineReset();
}

void horizonAddCloud() {
    if (horizon.cloudCount >= HORIZON_MAX_CLOUDS) {
        return;
    }
    cloudInit(&horizon.clouds[horizon.cloudCount]);
    horizon.cloudCount++;
}
