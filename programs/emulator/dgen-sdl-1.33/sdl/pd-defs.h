#ifndef __SDL_PD_DEFS_H__
#define __SDL_PD_DEFS_H__

#include <SDL.h>
#include <SDL_audio.h>

// Platform-dependent definitions and inlines.
// In this file, you should define all the keysyms and audio formats, and
// if you want to inline any functions, put them in here too. :)

// Now for a grueling list of keysyms. *muahahahahahaha*
// Seriously, this is tedious, I realise, but don't be discouraged please. :)
// Just remember I had to do it also.
#define PDK_ESCAPE SDLK_ESCAPE
#define PDK_BACKSPACE SDLK_BACKSPACE
#define PDK_TAB SDLK_TAB
#define PDK_RETURN SDLK_RETURN
#define PDK_KP_MULTIPLY SDLK_KP_MULTIPLY
#define PDK_SPACE SDLK_SPACE
#define PDK_F1 SDLK_F1
#define PDK_F2 SDLK_F2
#define PDK_F3 SDLK_F3
#define PDK_F4 SDLK_F4
#define PDK_F5 SDLK_F5
#define PDK_F6 SDLK_F6
#define PDK_F7 SDLK_F7
#define PDK_F8 SDLK_F8
#define PDK_F9 SDLK_F9
#define PDK_F10 SDLK_F10
#define PDK_KP7 SDLK_KP7
#define PDK_KP8 SDLK_KP8
#define PDK_KP9 SDLK_KP9
#define PDK_KP_MINUS SDLK_KP_MINUS
#define PDK_KP4 SDLK_KP4
#define PDK_KP5 SDLK_KP5
#define PDK_KP6 SDLK_KP6
#define PDK_KP_PLUS SDLK_KP_PLUS
#define PDK_KP1 SDLK_KP1
#define PDK_KP2 SDLK_KP2
#define PDK_KP3 SDLK_KP3
#define PDK_KP0 SDLK_KP0
#define PDK_KP_PERIOD SDLK_KP_PERIOD
#define PDK_F11 SDLK_F11
#define PDK_F12 SDLK_F12
#define PDK_KP_ENTER SDLK_KP_ENTER
#define PDK_KP_DIVIDE SDLK_KP_DIVIDE
#define PDK_HOME SDLK_HOME
#define PDK_UP SDLK_UP
#define PDK_PAGEUP SDLK_PAGEUP
#define PDK_LEFT SDLK_LEFT
#define PDK_RIGHT SDLK_RIGHT
#define PDK_END SDLK_END
#define PDK_DOWN SDLK_DOWN
#define PDK_PAGEDOWN SDLK_PAGEDOWN
#define PDK_INSERT SDLK_INSERT
#define PDK_DELETE SDLK_DELETE
#define PDK_NUMLOCK SDLK_NUMLOCK
#define PDK_CAPSLOCK SDLK_CAPSLOCK
#define PDK_SCROLLOCK SDLK_SCROLLOCK
#define PDK_LSHIFT SDLK_LSHIFT
#define PDK_RSHIFT SDLK_RSHIFT
#define PDK_LCTRL SDLK_LCTRL
#define PDK_RCTRL SDLK_RCTRL
#define PDK_LALT SDLK_LALT
#define PDK_RALT SDLK_RALT
#define PDK_LMETA SDLK_LMETA
#define PDK_RMETA SDLK_RMETA

// There, that wasn't so hard, was it? :)
// If you want to inline any pd_ functions, put their bodies here.
// Otherwise, you're done with this file! :D

#endif // __SDL_PD_DEFS_H__
