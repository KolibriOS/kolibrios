#ifndef EFFECTS_H
#define EFFECTS_H

typedef struct {
	int id, type;
	double x, y,
		   vsp, hsp, grav,
		   imageIndex, imageSpeed;
		   
	int cropx, cropy;
	int width, height;
	int image, timer;
	
	int visible;
	int val1;
	int loop, frames;
	int depth;
} Effect;

void createEffect(int type, int x, int y);
void createEffectExtra(int t, int x, int y, double hsp, double vsp, int val);

void effectStep(Effect* e);
void effectDraw(Effect* e);
void effectDestroy(int id);

void createRockSmash(int x, int y);
void createSplash(int x, int y);
void createLavaSplash(int x, int y);

#endif