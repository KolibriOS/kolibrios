#include<menuet/os.h>
#include <stdlib.h>
#include <stdio.h>
#include "SDL.h"
#include "SDL_sysevents.h"
#include "SDL_sysvideo.h"
#include "SDL_events_c.h"
#include "SDL_menuetvideo.h"

extern void MenuetOS_SDL_RepaintWnd(void);

void MenuetOS_InitOSKeymap(_THIS)
{
	__asm__("int $0x40"::"a"(66),"b"(1),"c"(1));
}

#define LSHIFT 1
#define RSHIFT 2
#define LCTRL 4
#define RCTRL 8
#define LALT 0x10
#define RALT 0x20
#define CAPS 0x40
#define NUML 0x80
#define SCRL 0x100

#define SHIFT (LSHIFT+RSHIFT)
#define CTRL (LCTRL+RCTRL)
#define ALT (LALT+RALT)

static SDLMod GetModState(void)
{
	unsigned controlstate;
	__asm__("int $0x40":"=a"(controlstate):"a"(66),"b"(3));
	SDLMod res = 0;
	if (controlstate & LSHIFT)
		res |= KMOD_LSHIFT;
	if (controlstate & RSHIFT)
		res |= KMOD_RSHIFT;
	if (controlstate & LCTRL)
		res |= KMOD_LCTRL;
        if (controlstate & RCTRL)
        	res |= KMOD_RCTRL;
        if (controlstate & LALT)
        	res |= KMOD_LALT;
	if (controlstate & RALT)
		res |= KMOD_RALT;
	if (controlstate & CAPS)
		res |= KMOD_CAPS;
	if (controlstate & NUML)
		res |= KMOD_NUM;
	return res;
}

/*static __u8 scan2ascii(__u8 n,SDLMod mod)
{
	__u8 layout[128];
	int layouttype;
	int bControlLayout = 0;
	if (mod & KMOD_ALT)
		layouttype = 3;
	else if (mod & KMOD_SHIFT)
		layouttype = 2;
	else
	{
		if (mod & KMOD_CTRL)
			bControlLayout = 1;
		layouttype = 1;
	}
	__asm__("int $0x40" :: "a"(26),"b"(2),"c"(layouttype),"d"(layout));
	__u8 res = layout[n];
	if (bControlLayout)
		res -= 0x60;
	return res;
}*/
static SDLKey sdlkeys[0x80] =
{
	// 0x0*
	0, SDLK_ESCAPE, SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6,
	SDLK_7, SDLK_8, SDLK_9, SDLK_0, SDLK_MINUS, SDLK_EQUALS, SDLK_BACKSPACE, SDLK_TAB,
	// 0x1*
	SDLK_q, SDLK_w, SDLK_e, SDLK_r, SDLK_t, SDLK_y, SDLK_u, SDLK_i,
	SDLK_o, SDLK_p, SDLK_LEFTBRACKET, SDLK_RIGHTBRACKET, SDLK_RETURN, SDLK_LCTRL, SDLK_a, SDLK_s,
	// 0x2*
	SDLK_d, SDLK_f, SDLK_g, SDLK_h, SDLK_j, SDLK_k, SDLK_l, SDLK_SEMICOLON,
	SDLK_QUOTE, SDLK_BACKQUOTE, SDLK_LSHIFT, SDLK_BACKSLASH, SDLK_z, SDLK_x, SDLK_c, SDLK_v,
	// 0x3*
	SDLK_b, SDLK_n, SDLK_m, SDLK_COMMA, SDLK_PERIOD, SDLK_SLASH, SDLK_RSHIFT, SDLK_KP_MULTIPLY,
	SDLK_LALT, SDLK_SPACE, SDLK_CAPSLOCK, SDLK_F1, SDLK_F2, SDLK_F3, SDLK_F4, SDLK_F5,
	// 0x4*
	SDLK_F6, SDLK_F7, SDLK_F8, SDLK_F9, SDLK_F10, SDLK_NUMLOCK, SDLK_SCROLLOCK, SDLK_KP7,
	SDLK_KP8, SDLK_KP9, SDLK_KP_MINUS, SDLK_KP4, SDLK_KP5, SDLK_KP6, SDLK_KP_PLUS, SDLK_KP1,
	// 0x5*
	SDLK_KP2, SDLK_KP3, SDLK_KP0, SDLK_KP_PERIOD, 0, 0, 0, SDLK_F11,
	SDLK_F12, 0, 0, 0, 0, 0, 0, 0,
	// 0x6*
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	// 0x7*
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};
static SDLKey sdlkeys_shift[0x80] =
{
	// 0x0*
	0, SDLK_ESCAPE, SDLK_EXCLAIM, SDLK_AT, SDLK_HASH, SDLK_DOLLAR, '%', SDLK_CARET,
	SDLK_AMPERSAND, SDLK_ASTERISK, SDLK_LEFTPAREN, SDLK_RIGHTPAREN, SDLK_UNDERSCORE, SDLK_PLUS, SDLK_BACKSPACE, SDLK_TAB,
	// 0x1*
	SDLK_q, SDLK_w, SDLK_e, SDLK_r, SDLK_t, SDLK_y, SDLK_u, SDLK_i,
	SDLK_o, SDLK_p, '{', '}', SDLK_RETURN, SDLK_LCTRL, SDLK_a, SDLK_s,
	// 0x2*
	SDLK_d, SDLK_f, SDLK_g, SDLK_h, SDLK_j, SDLK_k, SDLK_l, SDLK_COLON,
	SDLK_QUOTEDBL, '~', SDLK_LSHIFT, '|', SDLK_z, SDLK_x, SDLK_c, SDLK_v,
	// 0x3*
	SDLK_b, SDLK_n, SDLK_m, SDLK_LESS, SDLK_GREATER, SDLK_QUESTION, SDLK_RSHIFT, SDLK_KP_MULTIPLY,
	SDLK_LALT, SDLK_SPACE, SDLK_CAPSLOCK, SDLK_F1, SDLK_F2, SDLK_F3, SDLK_F4, SDLK_F5,
	// 0x4*
	SDLK_F6, SDLK_F7, SDLK_F8, SDLK_F9, SDLK_F10, SDLK_NUMLOCK, SDLK_SCROLLOCK, SDLK_KP7,
	SDLK_KP8, SDLK_KP9, SDLK_KP_MINUS, SDLK_KP4, SDLK_KP5, SDLK_KP6, SDLK_KP_PLUS, SDLK_KP1,
	// 0x5*
	SDLK_KP2, SDLK_KP3, SDLK_KP0, SDLK_KP_PERIOD, 0, 0, 0, SDLK_F11,
	SDLK_F12, 0, 0, 0, 0, 0, 0, 0,
	// 0x6*
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	// 0x7*
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};
static SDLKey sdlkeys_e0[0x80] =
{
	// 0x0*
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	// 0x1*
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, SDLK_KP_ENTER, SDLK_RCTRL, 0, 0,
	// 0x2*
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	// 0x3*
	0, 0, 0, 0, 0, SDLK_KP_DIVIDE, 0, SDLK_PRINT,
	SDLK_RALT, 0, 0, 0, 0, 0, 0, 0,
	// 0x4*
	0, 0, 0, 0, 0, 0, 0, SDLK_HOME,
	SDLK_UP, SDLK_PAGEUP, 0, SDLK_LEFT, 0, SDLK_RIGHT, 0, SDLK_END,
	// 0x5*
	SDLK_DOWN, SDLK_PAGEDOWN, SDLK_INSERT, SDLK_DELETE, 0, 0, 0, 0,
	0, 0, 0, SDLK_LSUPER, SDLK_RSUPER, SDLK_MENU, 0, 0,
	// 0x6*
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	// 0x7*
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
};

extern void KolibriOS_CheckMouseMode(_THIS);
void MenuetOS_PumpEvents(_THIS)
{
 int i;
 SDL_keysym key;
 static int ext_code=0;
 static __u8 old_mode=0;
 for (;;) {
  i=__menuet__check_for_event();
  switch(i)
  {
   case 0:
    return;
   case 1:
    MenuetOS_SDL_RepaintWnd();
    break;
   case 2:
    key.scancode = __menuet__getkey();
    if (key.scancode == 0xE0 || key.scancode == 0xE1)
    {ext_code=key.scancode;break;}
    if (ext_code == 0xE1 && (key.scancode & 0x7F) == 0x1D) break;
    if (ext_code == 0xE1 && key.scancode == 0xC5) {ext_code=0;break;}
    key.mod = GetModState();
    if (ext_code == 0xE1) key.mod &= ~KMOD_CTRL;
    if (!(key.scancode&0x80))
      old_mode = key.mod;
    SDL_SetModState(key.mod);
    int code = (key.scancode & 0x80) ? SDL_RELEASED : SDL_PRESSED;
    key.scancode &= 0x7F;
//    key.sym = scan2ascii(key.scancode,key.mod);
    if (ext_code == 0xE1 && key.scancode == 0x45)
	key.sym = SDLK_PAUSE;
    else if (ext_code == 0xE0)
	key.sym = sdlkeys_e0[key.scancode];
    else if (old_mode & KMOD_SHIFT)
	key.sym = sdlkeys_shift[key.scancode];
    else
	key.sym = sdlkeys[key.scancode];
    ext_code = 0;
    if (!key.sym) break;	
    SDL_PrivateKeyboard(code,&key);
    break;
   case 3:
    if(__menuet__get_button_id()==1) exit(0);
    break;
   case 6: {
    int __tmp,mx,my;
    static int oldmousestate = 0;
    __asm__("int $0x40":"=a"(__tmp):"a"(37),"b"(1));
    mx=(__tmp>>16);
    my=(__tmp&0xffff);
    if(mx>=0 && mx<this->hidden->win_size_x &&
       my>=0 && my<this->hidden->win_size_y || this->input_grab != SDL_GRAB_OFF)
    {
     if (this->input_grab != SDL_GRAB_OFF)
     {
      int dx=mx-this->hidden->win_size_x/2;
      int dy=my-this->hidden->win_size_y/2;
      if (dx||dy)
      {
       SDL_PrivateMouseMotion(0,1,dx,dy);
       KolibriOS_CheckMouseMode(this);
      }
     }
     else
      SDL_PrivateMouseMotion(0,0,mx,my);
     __asm__("int $0x40":"=a"(__tmp):"a"(37),"b"(2));
     if ((__tmp^oldmousestate)&1) {
     if(__tmp&1)
     {
      SDL_PrivateMouseButton(SDL_PRESSED,SDL_BUTTON_LMASK,0,0);
     } else {
     SDL_PrivateMouseButton(SDL_RELEASED,SDL_BUTTON_LMASK,0,0);
     } }
     if ((__tmp^oldmousestate)&2) {
     if(__tmp&2)
     {
      SDL_PrivateMouseButton(SDL_PRESSED,SDL_BUTTON_RMASK,0,0);
     } else {
     SDL_PrivateMouseButton(SDL_RELEASED,SDL_BUTTON_RMASK,0,0);
     } }
     oldmousestate = __tmp;
    }
   }
  }
 }
}
