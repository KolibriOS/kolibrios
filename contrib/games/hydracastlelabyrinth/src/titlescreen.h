#ifndef TITLESCREEN_H
#define TITLESCREEN_H

#ifdef EMSCRIPTEN
void titleScreenSetup();
int titleEMStep();
#else
int titleScreen();
#endif

#endif