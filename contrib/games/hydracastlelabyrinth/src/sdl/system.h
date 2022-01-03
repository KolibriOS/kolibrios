#ifndef SYSTEM_H
#define SYSTEM_H

#include <SDL.h>
#include "../PHL.h"

extern char quitGame;

int PHL_MainLoop();
void PHL_ConsoleInit();
void PHL_GameQuit();

void PHL_ErrorScreen(char* message);

#endif