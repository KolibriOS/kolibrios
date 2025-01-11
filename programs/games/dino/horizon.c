#include "horizon.h"

Horizon horizon;

void horizonInit(int dim_width, double gapCoefficient) {
	horizon.dim_width = dim_width;
	horizon.gapCoefficient = gapCoefficient;
	horizon.obstacles = ulist_create();
	horizon.obstacleHistory = ulist_create();
	horizon.clouds = ulist_create();

	horizonAddCloud();

    horizonLineInit();
}

void horizonUpdate(int deltaTime, double currentSpeed, bool updateObstacles, bool showNightMode) {
	//horizon.runningTime += deltaTime;
	horizonLineUpdate(deltaTime, currentSpeed);
    // if (currentSpeed != currentSpeed) {
    //     currentSpeed = 6.0;
    // }
	// horizon.nightMode.update(showNightMode);
	horizonUpdateClouds(deltaTime, currentSpeed);
	if (updateObstacles) {
		horizonUpdateObstacles(deltaTime, currentSpeed);
	}
}

void horizonUpdateClouds(int deltaTime, double speed) {
    //printf("horizonUpdateClouds()\n");
    double cloudSpeed = HORIZON_BG_CLOUD_SPEED / 1000 * deltaTime * speed;
    int numClouds = ulist_size(horizon.clouds);
    //printf("horizonUpdateClouds() %d\n", numClouds);

    if (numClouds) {
        Node *cloudNode = horizon.clouds->tail;
        while (cloudNode != NULL) {
            cloudUpdate(cloudNode->data, cloudSpeed);
            cloudNode = cloudNode->prev;
        }
        Cloud *lastCloud = horizon.clouds->tail->data;
        // Check for adding a new cloud
        if (numClouds < HORIZON_MAX_CLOUDS && (horizon.dim_width - lastCloud->xPos) > lastCloud->cloudGap && HORIZON_CLOUD_FREQUENCY > (double)rand()/RAND_MAX) {
            horizonAddCloud();
        }
        // Remove expired clouds
        cloudNode = horizon.clouds->head;
        while (cloudNode != NULL) {
            Node* cloudNodeNext = cloudNode->next;
            Cloud* c = cloudNode->data;
            if (c->remove) {
                ulist_remove(horizon.clouds, cloudNode);
            }
            cloudNode = cloudNodeNext;
        }
    }
    else {
        horizonAddCloud();
    }
}

void horizonUpdateObstacles(int deltaTime, double currentSpeed) {
    //printf("horizonUpdateObstacles()\n");
    // Obstacles, move to Horizon layer
    Node* obNode = horizon.obstacles->head;
    while (obNode != NULL) {
        Node* obNodeNext = obNode->next;
        Obstacle* ob = obNode->data;
        obstacleUpdate(ob, deltaTime, currentSpeed);
        // Clean up existing obstacles
        if (ob->remove) {
            //ulist_remove(horizon.obstacles, obNode);
            //ulist_print(horizon.obstacles);
            ulist_remove_front(horizon.obstacles);
            //ulist_print(horizon.obstacles);
            //puts("");
        }
        obNode = obNodeNext;
    }

    if (ulist_size(horizon.obstacles) > 0) {
        Obstacle *lastObstacle = horizon.obstacles->tail->data;

        if (lastObstacle && !lastObstacle->followingObstacleCreated && obstacleIsVisible(lastObstacle) && (lastObstacle->xPos + lastObstacle->width + lastObstacle->gap) < horizon.dim_width) {
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
    int obstacleTypeIndex = getRandomNumber(0, sizeof(obstacleTypeConfigs)/sizeof(ObstacleTypeConfig) - 1);
    ObstacleTypeConfig *otc = &obstacleTypeConfigs[obstacleTypeIndex];

    // Check for multiples of the same type of obstacle.
    // Also check obstacle is available at current speed.
    if (horizonDuplicateObstacleCheck(otc->type) || currentSpeed < otc->minSpeed) {
        horizonAddNewObstacle(currentSpeed);
    }
    else {
        Obstacle* ob = malloc(sizeof(Obstacle));
        obstacleInit(ob, otc, horizon.dim_width, horizon.gapCoefficient, currentSpeed, otc->width);
        ulist_push_back(horizon.obstacles, ob);
        ulist_push_front(horizon.obstacleHistory, &(otc->type));
        if (ulist_size(horizon.obstacleHistory) > 1) {
            ulist_splice(horizon.obstacleHistory, RUNNER_MAX_OBSTACLE_DUPLICATION);
        }
    }
}

bool horizonDuplicateObstacleCheck(ObstacleType nextObstacleType) {
    //printf("horizonDuplicateObstacleCheck(%d)\n", nextObstacleType);
    int duplicateCount = 0;
    Node* ohNode = horizon.obstacleHistory->head;
    while (ohNode != NULL) {
        //printf("%d\n", *(int*)ohNode->data);
        duplicateCount = *(int*)ohNode->data == nextObstacleType ? duplicateCount + 1 : 0;
        ohNode = ohNode->next;
    }
    //printf("duplicateCount = %d\n\n", duplicateCount);
    return duplicateCount >= RUNNER_MAX_OBSTACLE_DUPLICATION;
}

void horizonReset() {
    // printf("horizonReset() !!\n");
    ulist_destroy(horizon.obstacles);
    horizon.obstacles = ulist_create();
    horizonLineReset();
}

//void horizonResize(int width, int height) {
//    
//}

void horizonAddCloud() {
    Cloud* c = malloc(sizeof(Cloud));
    cloudInit(c, horizon.dim_width);
    //printf("horizonAddCloud() %d -> ", ulist_size(horizon.obstacles));
    ulist_push_back(horizon.clouds, c);
    //printf("%d\n", ulist_size(horizon.obstacles));
}

