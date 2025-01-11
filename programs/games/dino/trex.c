#include "trex.h"

Trex trex;

CollisionBox trexDuckingCollisionBox = {.x = 1, .y = 18, .width = 55, .height = 25};
CollisionBox trexRunningCollisionBox[6] = 
{
	{.x = 22, .y = 0, .width = 17, .height = 16},
	{.x = 1, .y = 18, .width = 30, .height = 9},
	{.x = 10, .y = 35, .width = 14, .height = 8},
	{.x = 1, .y = 24, .width = 29, .height = 5},
	{.x = 5, .y = 30, .width = 21, .height = 4},
	{.x = 9, .y = 34, .width = 15, .height = 4}
};

TrexAnimFramesEntry trexAnimFrames[5] = {
	{.frameCount = 2, .frames = {44, 0}, .msPerFrame = 1000./3},
	{.frameCount = 2, .frames = {88, 132}, .msPerFrame = 1000./12},
	{.frameCount = 1, .frames = {220}, .msPerFrame = 1000./60},
	{.frameCount = 1, .frames = {0}, .msPerFrame = 1000./60},
	{.frameCount = 2, .frames = {264, 323}, .msPerFrame = 1000./8}
};

// T - rex player initaliser
// Sets the t - rex to blink at random intervals
void trexInit() {
	trex.xPos = 0;
	trex.currentFrame = 0;
	//this.currentAnimFrames = [];
	trex.blinkDelay = 0;
	trex.blinkCount = 0;
	trex.animStartTime = 0;
	trex.timer = 0;
	trex.msPerFrame = 1000. / FPS;
	trex.status = TREX_STATUS_WAITING;

	trex.jumping = false;
	trex.ducking = false;
	trex.jumpVelocity = 0;
	trex.reachedMinHeight = false;
	trex.speedDrop = false;
	trex.jumpCount = 0;
	trex.jumpspotX = 0;

	trex.groundYPos = RUNNER_DEFAULT_HEIGHT - TREX_HEIGHT - RUNNER_BOTTOM_PAD;
	trex.yPos = trex.groundYPos;
	trex.minJumpHeight = trex.groundYPos - TREX_MIN_JUMP_HEIGHT;
	trex.playingIntro = false;

	trexDraw(0, 0);
	trexUpdate(0, TREX_STATUS_WAITING);
}

// Set the animation status
void trexUpdate(int deltaTime, int opt_status) {
	//printf("trex.status = %d\n", trex.status);
	trex.timer += deltaTime;
	// Update the status
	if (opt_status != -1) {
		trex.status = opt_status;
		trex.currentFrame = 0;
		trex.msPerFrame = trexAnimFrames[opt_status].msPerFrame;
		trex.currentAnimFrames = trexAnimFrames[opt_status];
		if (opt_status == TREX_STATUS_WAITING) {
			trex.animStartTime = getTimeStamp();
			trexSetBlinkDelay();
		}
	}
	// Game intro animation, T-rex moves in from the left.
	if (trex.playingIntro) {
		if (trex.xPos < TREX_START_X_POS) {
			//printf("trex.xPos = %d\n", trex.xPos);
			trex.xPos += max((int)round(((double)TREX_START_X_POS / TREX_INTRO_DURATION) * deltaTime), 1);
		}
		else {
			runnerStartGame();
		}
	}

	if (trex.status == TREX_STATUS_WAITING) {
		trexBlink(getTimeStamp());
	}
	else {
		// printf("trex.status = %d\n", trex.status);
		trexDraw(trex.currentAnimFrames.frames[trex.currentFrame], 0);
	}

	// Update the frame position.
	if (trex.timer >= trex.msPerFrame) {
		trex.currentFrame = trex.currentFrame == trex.currentAnimFrames.frameCount - 1 ? 0 : trex.currentFrame + 1;
		trex.timer = 0;
	}

	// Speed drop becomes duck if the down key is still being pressed.
	if (trex.speedDrop && trex.yPos == trex.groundYPos) {
		trex.speedDrop = false;
		trexSetDuck(true);
	}
}

void trexDraw(int x, int y) {
	//printf("trexDraw();\n");
	int sourceWidth = trex.ducking && trex.status != TREX_STATUS_CRASHED ? TREX_WIDTH_DUCK : TREX_WIDTH;
	int sourceHeight = TREX_HEIGHT;
	// Adjustments for sprite sheet position.
	int sourceX = x + ATLAS_TREX_X;
	int sourceY = y + ATLAS_TREX_Y;

	// Ducking.
	if (trex.ducking && trex.status != TREX_STATUS_CRASHED) {
		graphicsBlitAtlasImage(sourceX, sourceY, trex.xPos, trex.yPos, sourceWidth, sourceHeight, false);
	}
	else {
		// Crashed whilst ducking. Trex is standing up so needs adjustment.
		if (trex.ducking && trex.status == TREX_STATUS_CRASHED) {
			trex.xPos++;
		}
		// Standing / running
		graphicsBlitAtlasImage(sourceX, sourceY, trex.xPos, trex.yPos, sourceWidth, sourceHeight, false);
	}
}

void trexSetBlinkDelay() {
	trex.blinkDelay = (int)ceil(((double)rand()/RAND_MAX)*TREX_BLINK_TIMING);
}

void trexBlink(int time) {
	//printf("trexBlink(%d)\n", time);
	int deltaTime = time - trex.animStartTime;
	if (deltaTime < 0) {
		deltaTime = DELTA_MS_DEFAULT;
	}
	if (deltaTime >= trex.blinkDelay) {
		trexDraw(trex.currentAnimFrames.frames[trex.currentFrame], 0);
		if (trex.currentFrame == 1) {
			// Set new random delay to blink.
			trexSetBlinkDelay();
			trex.animStartTime = time;
			trex.blinkCount++;
		}
	}
}

// Initialise a jump
void trexStartJump(double speed) {
	if (!trex.jumping) {
		trexUpdate(0, TREX_STATUS_JUMPING);
		// Tweak the jump velocity based on the speed
		trex.jumpVelocity = TREX_INITIAL_JUMP_VELOCITY - (speed / 10);
		trex.jumping = true;
		trex.reachedMinHeight = false;
		trex.speedDrop = false;
	}
}

// Jump is complete, falling down
void trexEndJump() {
	if (trex.reachedMinHeight && trex.jumpVelocity < TREX_DROP_VELOCITY) {
		trex.jumpVelocity = TREX_DROP_VELOCITY;
	}
}

// Update frame for a jump
void trexUpdateJump(int deltaTime) {
	double msPerFrame = trexAnimFrames[trex.status].msPerFrame;
	double framesElapsed = deltaTime / msPerFrame;

	// Speed drop makes Trex fall faster.
	if (trex.speedDrop) {
		trex.yPos += (int)round(trex.jumpVelocity * TREX_SPEED_DROP_COEFFICIENT * framesElapsed);
	}
	else {
		trex.yPos += (int)round(trex.jumpVelocity * framesElapsed);
	}
	trex.jumpVelocity += TREX_GRAVITY * framesElapsed;
	// Minimum height has been reached.
	if (trex.yPos < trex.minJumpHeight || trex.speedDrop) {
		trex.reachedMinHeight = true;
	}
	// Reached max height
	if (trex.yPos < TREX_MAX_JUMP_HEIGHT || trex.speedDrop) {
		trexEndJump();
	}
	// Back down at ground level. Jump completed.
	if (trex.yPos > trex.groundYPos) {
		trexReset();
		trex.jumpCount++;
	}
	trexUpdate(deltaTime, -1);
}

// Set the speed drop.Immediately cancels the current jump
void trexSetSpeedDrop() {
	trex.speedDrop = true;
	trex.jumpVelocity = 1;
}

void trexSetDuck(bool isDucking) {
	if (isDucking && trex.status != TREX_STATUS_DUCKING) {
		trexUpdate(0, TREX_STATUS_DUCKING);
		trex.ducking = true;
	}
	else if (trex.status == TREX_STATUS_DUCKING) {
		trexUpdate(0, TREX_STATUS_RUNNING);
		trex.ducking = false;
	}
}

// Reset the t-rex to running at start of game
void trexReset() {
	trex.yPos = trex.groundYPos;
	trex.jumpVelocity = 0;
	trex.jumping = false;
	trex.ducking = false;
	trexUpdate(0, TREX_STATUS_RUNNING);
	//trex.midair = false; TODO: WTF is midair
	trex.speedDrop = false;
	trex.jumpCount = 0;
}
