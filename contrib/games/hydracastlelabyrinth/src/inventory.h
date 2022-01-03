#ifndef INVENTORY_H
#define INVENTORY_H

#ifdef EMSCRIPTEN
void inventorySetup();
int inventoryEMStep();
#else
void inventory();
#endif
int inventoryStep();
void inventoryDraw();

#endif