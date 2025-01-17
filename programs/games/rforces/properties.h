/* Rocket Forces
 * Filename: properties.h
 * Version 0.1
 * Copyright (c) Serial 2007
 */


// Color defines
#define BLACK			0x00000000
#define BLUE			0x000000FF
#define GREEN			0x0000FF00
#define RED				0x00FF0000
#define LIGHTGRAY		0x00C0C0C0
#define DARKGRAY		0x00808080
#define YELLOW			0x00FFFF00
#define WHITE			0x00FFFFFF

// Visual properties
#define GAME_NAME		"Rocket Forces"
#define BG_COLOR		BLACK
#define TEXT_COLOR      WHITE
#define CUR_COLOR       GREEN
#define CROSS_COLOR     RED
#define G_COLOR			WHITE
#define R_COLOR			GREEN
#define B_COLOR			BLUE
#define EXP_COLOR		YELLOW
#define H_COLOR			LIGHTGRAY
#define SMOKE_COLOR     LIGHTGRAY

// Game properties
//   R == Rocket
//   B == Bomb
//   EXP == Explode
int R_COUNT			= 6;
int R_SPEED			= 9;
int B_COUNT			= 5;
int B_SPEED			= 4;
int B_POSSIBILITY	= 200;
int EXP_RAD 		= 12;
int FRAME_TIME		= 5;
int HARDWARE_CURSOR	= 0;
int DEBUG			= 0;

#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 480
