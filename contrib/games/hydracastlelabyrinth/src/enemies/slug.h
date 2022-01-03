#ifndef SLUG_H
#define SLUG_H

typedef struct {
	int id;
	int dir;
	double x, y, vsp;
	double imageIndex;
} Slug;

void createSlug(int x, int y, int dir);

#endif