#ifndef RUNNER_H
#define RUNNER_H

#include <stdbool.h>
#include <math.h>
#include "config.h"
#include "graphics.h"
#include "horizon.h"
#include "distance_meter.h"
#include "game_over_panel.h"
#include "trex.h"

#define RUNNER_DEFAULT_HEIGHT 150

#define RUNNER_ACCELERATION 0.001
#define RUNNER_BOTTOM_PAD 10
#define RUNNER_CLEAR_TIME 3000
#define RUNNER_GAMEOVER_CLEAR_TIME 750
#define RUNNER_MAX_BLINK_COUNT 3
#define RUNNER_MS_PER_FRAME (1000.0 / FPS)
#define RUNNER_MAX_SPEED 13.0
#define RUNNER_SPEED 6.0

#define RUNNER_KEYCODE_JUMP_1 82
#define RUNNER_KEYCODE_JUMP_2 32
#define RUNNER_KEYCODE_DUCK 81
#define RUNNER_KEYCODE_RESTART 13

typedef struct {
	double distanceRan;
	int highestScore;
	int time;
	int runningTime;
	double currentSpeed;
	bool activated;
	bool playing;
	bool crashed;
	int timeAfterCrashedMs;
	bool paused;
	bool playingIntro;
	bool isRunning;
	int playCount;
	bool nextUpdateScheduled;
	bool skipUpdateNow;
} Runner;

extern Runner runner;

void runnerInit();

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
