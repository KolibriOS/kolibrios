#include "runner.h"

int aaaaaaa[10000];
Runner runner;
int bbbbbb[10000];

void runnerInit() {
	runner.distanceRan = 0;
	runner.highestScore = 0;
	runner.time = 0;
	runner.msPerFrame = 1000.0 / FPS;
	runner.currentSpeed = RUNNER_SPEED;
	runner.activated = false;
	runner.playing = false;
	runner.crashed = false;
	runner.timeAfterCrashedMs = 0;
	runner.paused = false;
	runner.inverted = false;
	runner.playingIntro = false;
	runner.isRunning = false; // is running or game stopped
	runner.invertTimer = 0;
	runner.playCount = 0;
	runner.nextUpdateScheduled = false;
	runner.skipUpdateNow = false;
	// TODO sound
	// runnerLoadImages();
	runnerAdjustDimensions();
	// setSpeed
	graphicsFillBackground(0xF7, 0xF7, 0xF7);

	gameOverPanelInit(runner.width, runner.height);
	
	horizonInit(runner.width, RUNNER_GAP_COEFFICIENT);
	distanceMeterInit(runner.width);
	trexInit();

	// this.startListening();
	runnerUpdate();
	//window.addEventListener(Runner.events.RESIZE, this.debounceResize.bind(this));
}

void runnerAdjustDimensions() {
	runner.width = DEFAULT_WIDTH;
	runner.height = RUNNER_DEFAULT_HEIGHT;
	// distance meter ...
}

void runnerOnKeyDown(int key) {
	if (!runner.crashed && (key == RUNNER_KEYCODE_JUMP_1 || key == RUNNER_KEYCODE_JUMP_2)) {
		if (!runner.playing) {
			// this.loadSounds(); // TODO
			runner.playing = true;
			//printf("first jump! %u\n", getTimeStamp());
			runnerUpdate();
			runner.nextUpdateScheduled = false;
			runner.skipUpdateNow = true;
		}
		//  Play sound effect and jump on starting the game for the first time.
		if (!trex.jumping && !trex.ducking) {
			// this.playSound(this.soundFx.BUTTON_PRESS); // TODO
			trexStartJump(runner.currentSpeed);
		}
	}
	if (runner.playing && !runner.crashed && key == RUNNER_KEYCODE_DUCK) {
		if (trex.jumping) {
			// Speed drop, activated only when jump key is not pressed.
			trexSetSpeedDrop();
		}
		//else if (!trex.jumping &&!trex.ducking) {
		else if (!trex.ducking) {
			// Duck
			trexSetDuck(true);
		}
	}
}

void runnerOnKeyUp(int key) {
	if (runner.isRunning && (key == RUNNER_KEYCODE_JUMP_1 || key == RUNNER_KEYCODE_JUMP_2)) {
		trexEndJump();
	}
	else if (key == RUNNER_KEYCODE_DUCK) {
		trex.speedDrop = false;
		trexSetDuck(false);
	}
	else if (runner.crashed) {
		// Check that enough time has elapsed before allowing jump key to restart.

		int now = getTimeStamp();
		int deltaTime = now - runner.time;
		// if (deltaTime < 0) {
		// 	deltaTime = DELTA_MS_DEFAULT;
		// 	runner.time = 0;
		// }

		// dbg_printf(".now = %d .deltaTime = %d runner.time = %d\n", now, deltaTime, runner.time);
		if (key == RUNNER_KEYCODE_RESTART || (runner.timeAfterCrashedMs >= RUNNER_GAMEOVER_CLEAR_TIME && (key == RUNNER_KEYCODE_JUMP_1 || key == RUNNER_KEYCODE_JUMP_2))) {
			//dbg_printf("timeAfterCrashedMs = %d\n", runner.timeAfterCrashedMs);
			runnerRestart();
		}
	}
	else if (runner.paused && (key == RUNNER_KEYCODE_JUMP_1 || key == RUNNER_KEYCODE_JUMP_2)) {
		trexReset();
		runnerPlay();
	}
}

void runnerClearCanvas() {
	graphicsFillBackground(0xF7, 0xF7, 0xF7);
	//graphicsRender();
}

void runnerUpdate() {
	//dbg_printf("runnerUpdate() runner.playing = %d\n", runner.playing);
	//runner.updatePending = false;
	int now = getTimeStamp();
	//printf("now = %d\n", now);
	int deltaTime = now - (runner.time ? runner.time : 0);
	if (deltaTime < 0) {
		deltaTime = DELTA_MS_DEFAULT;
	}
	// dbg_printf("runnerUpdate() deltaTime = %d\n", deltaTime);
	runner.time = now;
	if (runner.playing) {
		//printf("runnerUpdate() %d\n", getTimeStamp());
		runnerClearCanvas();

		if (trex.jumping) {
			trexUpdateJump(deltaTime);
		}

		runner.runningTime += deltaTime;
		bool hasObstacles = runner.runningTime > RUNNER_CLEAR_TIME;

		// First jump triggers the intro.
		if (trex.jumpCount == 1 && !runner.playingIntro) {
			//printf("trex.jumpCount = %d\n", trex.jumpCount);
			runnerPlayIntro();
		}

		// The horizon doesn't move until the intro is over.
		if (runner.playingIntro) {
			horizonUpdate(0, runner.currentSpeed, hasObstacles, false);
		}
		else {
			deltaTime = !runner.activated ? 0 : deltaTime;
			horizonUpdate(deltaTime, runner.currentSpeed, hasObstacles, runner.inverted);
		}

		// Check for collisions.
		bool collision = hasObstacles && runnerCheckForCollision(horizon.obstacles->head->data);

		if (!collision) {
			runner.distanceRan += runner.currentSpeed * deltaTime / runner.msPerFrame;

			if (runner.currentSpeed < RUNNER_MAX_SPEED) {
				runner.currentSpeed += RUNNER_ACCELERATION;
			}
		}
		else {
			runnerGameOver();
		}

		bool playAchievementSound = distanceMeterUpdate(deltaTime, (int)ceil(runner.distanceRan));

		if (playAchievementSound) {
			//this.playSound(this.soundFx.SCORE); // TODO
		}

		/*// Night mode.
		if (this.invertTimer > this.config.INVERT_FADE_DURATION) {
			this.invertTimer = 0;
			this.invertTrigger = false;
			this.invert();
		}
		else if (this.invertTimer) {
			this.invertTimer += deltaTime;
		}
		else {
			var actualDistance =
				this.distanceMeter.getActualDistance(Math.ceil(this.distanceRan));

			if (actualDistance > 0) {
				this.invertTrigger = !(actualDistance %
					this.config.INVERT_DISTANCE);

				if (this.invertTrigger&& this.invertTimer == = 0) {
					this.invertTimer += deltaTime;
					this.invert();
				}
			}
		}*/
	}

	runner.nextUpdateScheduled = false;//
	if (runner.playing || (!runner.activated && trex.blinkCount < RUNNER_MAX_BLINK_COUNT)) {
		trexUpdate(deltaTime, -1);
		runner.nextUpdateScheduled = true;
	}
	
	graphicsRender(); // blit all drawn to the screen
	//printf("runner update end\n\n");
}

void runnerGameOver() {
	// this.playSound(this.soundFx.HIT); // TODO
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
		distanceMeterReset(runner.highestScore);
		horizonReset();
		trexReset();
		//this.playSound(this.soundFx.BUTTON_PRESS);
		//this.invert(true);
		runner.isRunning = true;
		runnerUpdate();
	}
}

void runnerPlayIntro() {
	//printf("runnerPlayIntro()\n");
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

CollisionBox createAdjustedCollisionBox(CollisionBox box, CollisionBox adjustment) {
	return (CollisionBox){ .x = box.x + adjustment.x, .y = box.y + adjustment.y, .width = box.width, .height = box.height };
}

// Returns whether boxes intersected
bool boxCompare(CollisionBox tRexBox, CollisionBox obstacleBox) {
	// Axis-Aligned Bounding Box method.
	return (tRexBox.x < obstacleBox.x + obstacleBox.width &&
		tRexBox.x + tRexBox.width > obstacleBox.x &&
		tRexBox.y < obstacleBox.y + obstacleBox.height &&
		tRexBox.height + tRexBox.y > obstacleBox.y);
}

bool runnerCheckForCollision(const Obstacle* obstacle) {
	// Adjustments are made to the bounding box as there is a 1 pixel white
		// border around the t-rex and obstacles.
	CollisionBox tRexBox = {
		.x = trex.xPos + 1,
		.y = trex.yPos + 1,
		.width = TREX_WIDTH - 2,
		.height = TREX_HEIGHT - 2 };

	CollisionBox obstacleBox = {
		.x = obstacle->xPos + 1,
		.y = obstacle->yPos + 1,
		.width = obstacle->typeConfig.width * obstacle->size - 2,
		.height = obstacle->typeConfig.height - 2 };

	// Simple outer bounds check.
	if (boxCompare(tRexBox, obstacleBox)) {
		CollisionBox* tRexCollisionBoxes = &trexDuckingCollisionBox;
		int tRexCollisionBoxesCount = 1;
		if (!trex.ducking) {
			tRexCollisionBoxes = trexRunningCollisionBox;
			tRexCollisionBoxesCount = 6;
		}

		// Detailed axis aligned box check.
		for (int t = 0; t < tRexCollisionBoxesCount; t++) {
			for (int i = 0; i < obstacle->typeConfig.collisionBoxesCount; i++) {
				// Adjust the box to actual positions.
				CollisionBox adjTrexBox = createAdjustedCollisionBox(tRexCollisionBoxes[t], tRexBox);
				CollisionBox adjObstacleBox = createAdjustedCollisionBox(obstacle->typeConfig.collisionBoxes[i], obstacleBox);
				
				if (boxCompare(adjTrexBox, adjObstacleBox)) {
					return true;// [adjTrexBox, adjObstacleBox] ;
				}
			}
		}
	}
	return false;
}


