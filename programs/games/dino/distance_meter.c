#include "distance_meter.h"

DistanceMeter distanceMeter;

void distanceMeterInit(int w) {
	distanceMeter.x = 0;
	distanceMeter.y = 5;
	distanceMeter.currentDistance = 0;
	distanceMeter.maxScore = 0;
	distanceMeter.achievement = false;
	distanceMeter.flashTimer = 0;
	distanceMeter.flashIterations = 0;
	distanceMeter.invertTrigger = false;
	distanceMeter.maxScoreUnits = DM_MAX_DISTANCE_UNITS;
	distanceMeterCalcXPos(w);
	for (int i = 0; i < distanceMeter.maxScoreUnits; i++) {
		distanceMeterDraw(i, 0, false);
	}
	distanceMeter.maxScore = (int)pow(10, distanceMeter.maxScoreUnits) - 1;
	distanceMeter.digits[0] = '\0';
	distanceMeter.highScore[0] = '\0';
}

void distanceMeterCalcXPos(int w) {
	distanceMeter.x = w - (DM_DEST_WIDTH * (distanceMeter.maxScoreUnits + 1));
}

void distanceMeterDraw(int digitPos, int value, bool opt_highscore) {

	int dx, dy;
	if (opt_highscore) {
		dx = distanceMeter.x - (distanceMeter.maxScoreUnits * 2) * DM_WIDTH;
		dy = distanceMeter.y;
	}
	else {
		dx = distanceMeter.x;
		dy = distanceMeter.y;
	}
	//printf("%d %d %d  %d %d    %d\n", digitPos, value, opt_highscore, dx, dy, DM_WIDTH * value + ATLAS_TEXT_SPRITE_X);
	graphicsBlitAtlasImage(
		DM_WIDTH * value + ATLAS_TEXT_SPRITE_X,
		0 + ATLAS_TEXT_SPRITE_Y,
		digitPos * DM_DEST_WIDTH + dx,
		distanceMeter.y + dy,
		DM_WIDTH,
		DM_HEIGHT,
		false
	);
}

void distanceMeterDrawHighScore() {
	// TODO canvasCtx.globalAlpha = .8;
	for (int i = (int)strlen(distanceMeter.highScore) - 1; i >= 0; i--) {
		distanceMeterDraw(i, distanceMeter.highScore[i] > 12 ? distanceMeter.highScore[i] - '0' : distanceMeter.highScore[i], true);
	}
}

void distanceMeterSetHighScore(int _distance) {
	int distance = distanceMeterGetActualDistance(_distance);
	distanceMeter.highScore[0] = 10;
	distanceMeter.highScore[1] = 11;
	distanceMeter.highScore[2] = 12;
	intToStr(distance, distanceMeter.maxScoreUnits, distanceMeter.highScore + 3);
}

void distanceMeterReset() {
	distanceMeterUpdate(0, 0);
	distanceMeter.achievement = false;
}

int distanceMeterGetActualDistance(int distance) {
	return distance ? (int)round(distance * DM_COEFFICIENT) : 0;
}

bool distanceMeterUpdate(int deltaTime, int _distance) {
	bool paint = true;
	bool playSound = false;
	int distance = _distance;
	if (!distanceMeter.achievement) {
		distance = distanceMeterGetActualDistance(distance);
		// check if score has gone beyond the initial digit count.
		if (distance > distanceMeter.maxScore && distanceMeter.maxScoreUnits == DM_MAX_DISTANCE_UNITS) {
			distanceMeter.maxScoreUnits++;
			distanceMeter.maxScore = distanceMeter.maxScore * 10 + 9;
		}
		// else {
			// NOTE this.distance was in original but i didsnt see any usage of this field
		// }

		if (distance > 0) {
			// Acheivement unlocked
			if (distance % DM_ACHIEVEMENT_DISTANCE == 0) {
				// Flash score and play sound
				distanceMeter.achievement = true;
				distanceMeter.flashTimer = 0;
				playSound = true;
			}
			// Create a string representation of the distance with leading 0.
			intToStr(distance, distanceMeter.maxScoreUnits, distanceMeter.digits);
		}
		else {
			intToStr(0, distanceMeter.maxScoreUnits, distanceMeter.digits);
		}
	}
	else {
		// Control flashing of the score on reaching acheivement.
		if (distanceMeter.flashIterations <= DM_FLASH_ITERATIONS) {
			distanceMeter.flashTimer += deltaTime;

			if (distanceMeter.flashTimer < DM_FLASH_DURATION) {
				paint = false;
			}
			else if (distanceMeter.flashTimer > DM_FLASH_DURATION * 2) {
				distanceMeter.flashTimer = 0;
				distanceMeter.flashIterations++;
			}
		}
		else {
			distanceMeter.achievement = false;
			distanceMeter.flashIterations = 0;
			distanceMeter.flashTimer = 0;
		}
	}

	// Draw the digits if not flashing
	if (paint) {
		for (int i = distanceMeter.maxScoreUnits - 1; i >= 0; i--) {
			distanceMeterDraw(i, (int)distanceMeter.digits[i] - '0', false);
		}
	}

	distanceMeterDrawHighScore();
	return playSound;
}
