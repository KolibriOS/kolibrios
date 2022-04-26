#ifndef _SDL_kolibri_video_h
#define _SDL_kolibri_video_h

#include "SDL_mouse.h"
#include "SDL_sysvideo.h"

#define _THIS	SDL_VideoDevice *this

struct SDL_PrivateVideoData {
 unsigned char * __video_buffer;
 char 	       * __title;
 int		 win_size_x,win_size_y;
 int		 vx_ofs,vy_ofs;
 unsigned char** __lines;
};

void kos_InitOSKeymap(_THIS);
void kos_PumpEvents(_THIS);

#endif