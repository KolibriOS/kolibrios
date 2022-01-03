#ifndef HYDRA_H
#define HYDRA_H

typedef struct {
	int id;
	int hp, blink;
	double x, y;
	double hsp, vsp;
	double imageIndex;
	int state, timer;
	int patternCounter;
	char onground;
	char noheads;
	int headid[4];
} Hydra;

void createHydra(int x);

typedef struct {
	int id;
	int hp, blink;
	int dir;
	int position; //0 = lower 1 = higher
	double imageIndex;
	double neckRot;
	int state, timer, counter;
	int bodyid;
	double bodyposX[7];
	double bodyposY[7];
} Hydrahead;

int createHydrahead(int dir, int position, int bodyid);

typedef struct {
	int id;
	double x, y;
	double hsp, vsp;
	char inwall;
	char bounce;
	double imageIndex;
} Hydragoop;

void createHydragoop(int x, int y, int hsp, int vsp);

typedef struct {
	int id;
	double x, y;
	double vsp;
	char bounce;
	double imageIndex;
} Hydrarock;

void createHydrarock();

typedef struct {
	int id;
	int timer;
	double x, y;
	double angle;
	double imageIndex;
} Hydrashock;

void createHydrashock(int x, int y);

#endif