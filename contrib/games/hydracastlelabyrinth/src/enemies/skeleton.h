#ifndef SKELETON_H
#define SKELETON_H

#include "../collision.h"

typedef struct {
	int id;
	double x, y;
	double hsp;
	double imageIndex;
	int dir;
	int hp;
	int state, timer, invincible;
	
	Mask mask;
} Skeleton;

void createSkeleton(int x, int y, int dir);

typedef struct {
	int id;
	double x, y;
	double hsp, vsp, grav;
	double imageIndex;
	
	Mask mask;
} Bone;

void createBone(int x, int y, int dir);

#endif