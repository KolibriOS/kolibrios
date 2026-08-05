#include "runner.h"

Runner runner;

void runnerInit() {
	runner.distanceRan = 0;
	runner.highestScore = 0;
	runner.time = 0;
	runner.currentSpeed = RUNNER_SPEED;
	runner.activated = false;
	runner.playing = false;
	runner.crashed = false;
	runner.timeAfterCrashedMs = 0;
	runner.paused = false;
	runner.playingIntro = false;
	runner.isRunning = false; // is running or game stopped
	runner.playCount = 0;
	runner.nextUpdateScheduled = false;
	runner.skipUpdateNow = false;
	// TODO sound
	graphicsFillBackground();

	horizonInit();
	distanceMeterInit();
	trexInit();

	runnerUpdate();
}

static bool isJumpKey(int key) {
	return key == RUNNER_KEYCODE_JUMP_1 || key == RUNNER_KEYCODE_JUMP_2;
}

void runnerOnKeyDown(int key) {
	if (!runner.crashed && isJumpKey(key)) {
		if (!runner.playing) {
			runner.playing = true;
			runnerUpdate();
			runner.nextUpdateScheduled = false;
			runner.skipUpdateNow = true;
		}
		// Jump on starting the game for the first time.
		if (!trex.jumping && !trex.ducking) {
			trexStartJump(runner.currentSpeed);
		}
	}
	if (runner.playing && !runner.crashed && key == RUNNER_KEYCODE_DUCK) {
		if (trex.jumping) {
			// Speed drop, activated only when jump key is not pressed.
			trexSetSpeedDrop();
		}
		else if (!trex.ducking) {
			// Duck
			trexSetDuck(true);
		}
	}
}

void runnerOnKeyUp(int key) {
	if (runner.isRunning && isJumpKey(key)) {
		trexEndJump();
	}
	else if (key == RUNNER_KEYCODE_DUCK) {
		trex.speedDrop = false;
		trexSetDuck(false);
	}
	else if (runner.crashed) {
		// Check that enough time has elapsed before allowing jump key to restart.
		if (key == RUNNER_KEYCODE_RESTART || (runner.timeAfterCrashedMs >= RUNNER_GAMEOVER_CLEAR_TIME && isJumpKey(key))) {
			runnerRestart();
		}
	}
	else if (runner.paused && isJumpKey(key)) {
		trexReset();
		runnerPlay();
	}
}

void runnerClearCanvas() {
	graphicsFillBackground();
}

void runnerUpdate() {
	int now = getTimeStamp();
	int deltaTime = now - runner.time;
	if (deltaTime < 0) {
		deltaTime = DELTA_MS_DEFAULT;
	}
	runner.time = now;
	if (runner.playing) {
		runnerClearCanvas();

		if (trex.jumping) {
			trexUpdateJump(deltaTime);
		}

		runner.runningTime += deltaTime;
		bool hasObstacles = runner.runningTime > RUNNER_CLEAR_TIME;

		// First jump triggers the intro.
		if (trex.jumpCount == 1 && !runner.playingIntro) {
			runnerPlayIntro();
		}

		// The horizon doesn't move until the intro is over.
		if (runner.playingIntro) {
			horizonUpdate(0, runner.currentSpeed, hasObstacles);
		}
		else {
			deltaTime = !runner.activated ? 0 : deltaTime;
			horizonUpdate(deltaTime, runner.currentSpeed, hasObstacles);
		}

		// Check for collisions.
		bool collision = hasObstacles && horizon.obstacleCount > 0 && runnerCheckForCollision(&horizon.obstacles[0]);

		if (!collision) {
			runner.distanceRan += runner.currentSpeed * deltaTime / RUNNER_MS_PER_FRAME;

			if (runner.currentSpeed < RUNNER_MAX_SPEED) {
				runner.currentSpeed += RUNNER_ACCELERATION;
			}
		}
		else {
			runnerGameOver();
		}

		// TODO sound: returns true when the achievement sound should play
		distanceMeterUpdate(deltaTime, (int)ceil(runner.distanceRan));
	}

	runner.nextUpdateScheduled = false;
	if (runner.playing || (!runner.activated && trex.blinkCount < RUNNER_MAX_BLINK_COUNT)) {
		trexUpdate(deltaTime, -1);
		runner.nextUpdateScheduled = true;
	}

	graphicsRender(); // blit all drawn to the screen
}

void runnerGameOver() {
	// TODO sound
	runnerStop();
	runner.crashed = true;
	distanceMeter.achievement = false;
	trexUpdate(100, TREX_STATUS_CRASHED);

	// Game over panel
	gameOverPanelDraw();
	// Update the high score
	if (runner.distanceRan > runner.highestScore) {
		runner.highestScore = (int)ceil(runner.distanceRan);
		distanceMeterSetHighScore(runner.highestScore);
	}
	// Reset the time clock
	runner.time = getTimeStamp();
}

void runnerStop() {
	runner.playing = false;
	runner.paused = true;
	runner.isRunning = false;
}

void runnerPlay() {
	if (!runner.crashed) {
		runner.playing = true;
		runner.paused = false;
		trexUpdate(0, TREX_STATUS_RUNNING);
		runner.time = getTimeStamp();
		runnerUpdate();
	}
}

void runnerRestart() {
	if (!runner.isRunning) {
		runner.playCount++;
		runner.runningTime = 0;
		runner.playing = true;
		runner.crashed = false;
		runner.timeAfterCrashedMs = 0;
		runner.distanceRan = 0;
		runner.currentSpeed = RUNNER_SPEED;
		runner.time = getTimeStamp();
		runnerClearCanvas();
		distanceMeterReset();
		horizonReset();
		trexReset();
		runner.isRunning = true;
		runnerUpdate();
	}
}

void runnerPlayIntro() {
	if (!runner.activated && !runner.crashed) {
		runner.playingIntro = true;
		trex.playingIntro = true;
		runner.playing = true;
		runner.activated = true;
	}
	else if (runner.crashed) {
		runnerRestart();
	}
}

void runnerStartGame() {
	runner.runningTime = 0;
	runner.playingIntro = false;
	trex.playingIntro = false;
	runner.playCount++;
	runner.isRunning = true;
}

// Axis-Aligned Bounding Box method.
static bool boxesIntersect(int ax, int ay, int aw, int ah, int bx, int by, int bw, int bh) {
	return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

bool runnerCheckForCollision(const Obstacle* obstacle) {
	// Adjustments are made to the bounding box as there is a 1 pixel white
	// border around the t-rex and obstacles.
	int tx = trex.xPos + 1;
	int ty = trex.yPos + 1;
	int ox = obstacle->xPos + 1;
	int oy = obstacle->yPos + 1;
	int ow = obstacle->typeConfig.width * obstacle->size - 2;
	int oh = obstacle->typeConfig.height - 2;

	// Simple outer bounds check.
	if (boxesIntersect(tx, ty, TREX_WIDTH - 2, TREX_HEIGHT - 2, ox, oy, ow, oh)) {
		const CollisionBox* tRexCollisionBoxes = &trexDuckingCollisionBox;
		int tRexCollisionBoxesCount = 1;
		if (!trex.ducking) {
			tRexCollisionBoxes = trexRunningCollisionBox;
			tRexCollisionBoxesCount = 6;
		}

		// Detailed axis aligned box check with boxes adjusted to actual positions.
		for (int t = 0; t < tRexCollisionBoxesCount; t++) {
			const CollisionBox* tb = &tRexCollisionBoxes[t];
			for (int i = 0; i < obstacle->typeConfig.collisionBoxesCount; i++) {
				const CollisionBox* cb = &obstacle->typeConfig.collisionBoxes[i];
				if (boxesIntersect(tb->x + tx, tb->y + ty, tb->width, tb->height,
				                   cb->x + ox, cb->y + oy, cb->width, cb->height)) {
					return true;
				}
			}
		}
	}
	return false;
}
