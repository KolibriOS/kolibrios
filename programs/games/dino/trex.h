#ifndef TREX_H
#define TREX_H

#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include "collisionbox.h"
#include "runner.h"
#include "graphics.h"
#include "misc.h"

// Blinking coefficient
#define TREX_BLINK_TIMING 7000

#define TREX_DROP_VELOCITY -5
#define TREX_GRAVITY 0.6
#define TREX_HEIGHT 47
#define TREX_HEIGHT_DUCK 25
#define TREX_INITIAL_JUMP_VELOCITY -10
#define TREX_INTRO_DURATION 750
#define TREX_MAX_JUMP_HEIGHT 30
#define TREX_MIN_JUMP_HEIGHT 30
#define TREX_SPEED_DROP_COEFFICIENT 3
#define TREX_SPRITE_WIDTH 262
#define TREX_START_X_POS 25
#define TREX_WIDTH 44
#define TREX_WIDTH_DUCK 59

// Animation states
typedef enum {
	TREX_STATUS_WAITING = 0,
	TREX_STATUS_RUNNING = 1,
	TREX_STATUS_CRASHED = 2,
	TREX_STATUS_JUMPING = 3,
	TREX_STATUS_DUCKING = 4,
} TrexStatus;

typedef struct {
	int frameCount;
	int frames[2];
	double msPerFrame;
} TrexAnimFramesEntry;

typedef struct {
	int xPos;
	int yPos;
	int groundYPos;
	int currentFrame;
	TrexAnimFramesEntry currentAnimFrames;
	int blinkDelay;
	int blinkCount;
	int animStartTime;
	int timer;
	double msPerFrame;
	TrexStatus status;
	bool jumping;
	bool ducking;
	double jumpVelocity;
	bool reachedMinHeight;
	bool speedDrop;
	int jumpCount;
	int jumpspotX;
	int minJumpHeight;
	bool playingIntro;
} Trex;

extern CollisionBox trexDuckingCollisionBox;
extern CollisionBox trexRunningCollisionBox[6];
extern Trex trex;

void trexInit();
void trexUpdate(int deltaTime, int opt_status);
void trexDraw(int x, int y);
void trexSetBlinkDelay();
void trexBlink(int time);
void trexStartJump(double speed);
void trexEndJump();
void trexUpdateJump(int deltaTime);
void trexSetSpeedDrop();
void trexSetDuck(bool isDucking);
void trexReset();

#endif
