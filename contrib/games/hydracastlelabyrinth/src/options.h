#ifndef OPTIONS_H
#define OPTIONS_H

#ifdef EMSCRIPTEN
void optionsSetup(int only);
int optionsEMStep();
#else
int options(int only);
#endif

int optionsStep();
void optionsDraw();
#ifdef _SDL
int getMusicType();
void setMusicType(int t);
#endif

#endif