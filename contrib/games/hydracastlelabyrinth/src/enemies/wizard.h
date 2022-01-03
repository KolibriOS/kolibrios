#ifndef WIZARD_H
#define WIZARD_H

#include "../collision.h"

typedef struct {
	int id;
	double x, y;
	double imageIndex;
	int state, timer, visible;
	
	Mask mask;
} Wizard;

void createWizard(int x, int y);

void wizardStep(Wizard* w);
void wizardDraw(Wizard* w);

#endif