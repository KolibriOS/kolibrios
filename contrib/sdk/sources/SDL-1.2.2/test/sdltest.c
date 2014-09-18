#include"SDL.h"
#include<menuet/os.h>

SDL_Surface * screen;
static int done=0;

void app_main(void)
{
 SDL_Event event;
 if(SDL_Init(SDL_INIT_VIDEO)<0) __menuet__sys_exit();
 atexit(SDL_Quit);
 screen=SDL_SetVideoMode(320,200,8,SDL_SWSURFACE);
 while(!done)
 {
  while(SDL_PollEvent(&event))
  {
   switch(event.type)
   {
    case SDL_KEYDOWN:
    case SDL_QUIT:
     done=1;
     break;
    default:
     break;
   }
  }
 }
}
