#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "SDL.h"
#include "SDL_ttf.h"

int app_main(int argc, char *argv[])
{ 
 SDL_Surface *screen, *txt;
 static SDL_Color kolor={0xC0,0xC0,0,0};
 static SDL_Color kolor1={0x00,0xC0,0xC0,0};
 SDL_Event event;
 SDL_Rect xtmp={x:30,y:30};
 TTF_Font * fnt, * fnt1;
 static int done=0;
 int i;
 if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) 
 {
  SDL_printf("Couldn't initialize SDL: %s\n",SDL_GetError());
  return(255);
 }
 screen = SDL_SetVideoMode(512,348,24,SDL_SWSURFACE);
 if ( screen == NULL ) 
 {
  SDL_printf("Couldn't set %dx%dx%d video mode: %s\n",
   	     640,480,24, SDL_GetError());
  exit(-1);
 }
 if(TTF_Init()!=0)
 {
  SDL_printf("Couldn't initialize TTF library\n");
  exit(-1);
 }
 fnt=TTF_OpenFont("/SYS/INDIGO.TTF",50);
 fnt1=TTF_OpenFont("/SYS/HYDROGEN.TTF",35);
 TTF_SetFontStyle(fnt,TTF_STYLE_ITALIC|TTF_STYLE_BOLD|TTF_STYLE_UNDERLINE);
 txt=TTF_RenderText_Solid(fnt,"MenuetOS",kolor);
 if(!txt)
 {
  SDL_printf("Unable to create rendering surface\n");
  exit(-1);
 }
 xtmp.w=txt->w;
 xtmp.h=txt->h;
 SDL_BlitSurface(txt,NULL,screen,&xtmp); 
 SDL_FreeSurface(txt);
 txt=TTF_RenderText_Solid(fnt1,"supports TTF",kolor1);
 xtmp.w=txt->w;
 xtmp.h=txt->h;
 xtmp.x=40;
 xtmp.y=100;
 SDL_BlitSurface(txt,NULL,screen,&xtmp); 
 SDL_FreeSurface(txt);
 while ( ! done ) 
 {
  if ( SDL_PollEvent(&event) ) 
  {
   switch (event.type) 
   {
    case SDL_QUIT:
     argv[i+1] = NULL;
     done = 1;
     break;
    default:
     break;
   }
  } else {
   SDL_Delay(3);
  }
 }
 SDL_FreeSurface(txt);
 SDL_Quit();
 return(0);
}
