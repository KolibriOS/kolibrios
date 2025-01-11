#ifndef RUNNER_H
#define RUNNER_H

#include <stdbool.h>
#include "config.h"
#include "ulist.h"
#include "graphics.h"
#include "horizon.h"
#include "distance_meter.h"
#include "game_over_panel.h"
#include "trex.h"

#define RUNNER_DEFAULT_HEIGHT 150

#define RUNNER_ACCELERATION 0.001
#define RUNNER_BG_CLOUD_SPEED 0.2
#define RUNNER_BOTTOM_PAD 10
#define RUNNER_CLEAR_TIME 3000
#define RUNNER_CLOUD_FREQUENCY 0.5
#define RUNNER_GAMEOVER_CLEAR_TIME 750
#define RUNNER_GAP_COEFFICIENT 0.6
#define RUNNER_GRAVITY 0.6
#define RUNNER_INITIAL_JUMP_VELOCITY 12
#define RUNNER_INVERT_FADE_DURATION 12000
#define RUNNER_INVERT_DISTANCE 700
#define RUNNER_MAX_BLINK_COUNT 3
#define RUNNER_MAX_CLOUDS 6
#define RUNNER_MAX_OBSTACLE_LENGTH 3
#define RUNNER_MAX_OBSTACLE_DUPLICATION 2
#define RUNNER_MAX_SPEED 13.0
#define RUNNER_MIN_JUMP_HEIGHT 35
#define RUNNER_MOBILE_SPEED_COEFFICIENT 1.2
#define RUNNER_SPEED 6.0
#define RUNNER_SPEED_DROP_COEFFICIENT 3

#define RUNNER_KEYCODE_JUMP_1 82
#define RUNNER_KEYCODE_JUMP_2 32
#define RUNNER_KEYCODE_DUCK 81
#define RUNNER_KEYCODE_RESTART 13

typedef struct {
	int width;
	int height;
	double distanceRan;
	int highestScore;
	int time;
	int runningTime;
	double msPerFrame;
	double azazzaza; //
	double currentSpeed;
	// Ulist* obstacles;
	bool activated;
	bool playing;
	bool crashed;
	int timeAfterCrashedMs;
	bool paused;
	bool inverted;
	bool invertTimer;
	bool playingIntro;
	bool isRunning;
	// resizeTimerId_
	int playCount;
	// soundFx
	// audioContext
	// images
	// imagesLoaded
	bool nextUpdateScheduled;
	bool skipUpdateNow;
} Runner;

extern Runner runner;

void runnerInit();
void runnerAdjustDimensions();
//void runnerLoadImages();

void runnerClearCanvas();

void runnerPlayIntro();
void runnerStartGame();
void runnerUpdate();
void runnerOnKeyDown(int key);
void runnerOnKeyUp(int key);
void runnerGameOver();
void runnerStop();
void runnerPlay();
void runnerRestart();

bool runnerCheckForCollision(const Obstacle *obstacle);

#endif
