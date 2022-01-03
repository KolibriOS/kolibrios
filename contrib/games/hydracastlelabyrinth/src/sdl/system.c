#include <SDL.h>
#include "system.h"


char quitGame = 0;

void Input_KeyEvent(SDL_Event* evt);
void Input_JoyEvent(SDL_Event* evt);
void Input_JoyAxisEvent(SDL_Event* evt);
void Input_JoyHatEvent(SDL_Event* evt);

int PHL_MainLoop()
{
    SDL_Event evt;
    while(SDL_PollEvent(&evt))
    {
        switch(evt.type)
        {
            case SDL_QUIT:
                quitGame = 1;
                return 0;
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                Input_KeyEvent(&evt);
                break;
            case SDL_JOYAXISMOTION:
                Input_JoyAxisEvent(&evt);
                break;
            case SDL_JOYHATMOTION:
                Input_JoyHatEvent(&evt);
                break;
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYBUTTONUP:
                Input_JoyEvent(&evt);
                break;
        }
    }
    if (quitGame == 1) 
    {
		return 0;
	}
	return 1;
}
void PHL_ConsoleInit()
{

}
void PHL_GameQuit()
{
    quitGame = 1;
}

void PHL_ErrorScreen(char* message)
{
    fprintf(stderr, "%s\n", message);
}
