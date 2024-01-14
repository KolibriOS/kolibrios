#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ksys.h>

#include "SDL.h"
#include "SDL_sysevents.h"
#include "SDL_sysvideo.h"
#include "SDL_events_c.h"
#include "SDL_kolibri_video.h"

extern void kos_SDL_RepaintWnd(void);

void kos_InitOSKeymap(_THIS)
{
    _ksys_set_key_input_mode(KSYS_KEY_INPUT_MODE_SCANC);
}

#define SHIFT (LSHIFT+RSHIFT)
#define CTRL (LCTRL+RCTRL)
#define ALT (LALT+RALT)

static SDLMod GetModState(void)
{
    unsigned controlstate = _ksys_get_control_key_state();
    SDLMod res = 0;
    if (controlstate & KSYS_CONTROL_LSHIFT)
        res |= KMOD_LSHIFT;
    if (controlstate & KSYS_CONTROL_RSHIFT)
        res |= KMOD_RSHIFT;
    if (controlstate & KSYS_CONTROL_LCTRL)
        res |= KMOD_LCTRL;
    if (controlstate & KSYS_CONTROL_RCTRL)
        res |= KMOD_RCTRL;
    if (controlstate & KSYS_CONTROL_LALT)
        res |= KMOD_LALT;
    if (controlstate &  KSYS_CONTROL_RALT)
        res |= KMOD_RALT;
    if (controlstate &  KSYS_CONTROL_CAPS)
        res |= KMOD_CAPS;
    if (controlstate &  KSYS_CONTROL_NUM_LOCK)
        res |= KMOD_NUM;
    return res;
}

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

extern void kos_CheckMouseMode(_THIS);

void kos_PumpEvents(_THIS)
{
    uint32_t kos_event;
    ksys_pos_t mouse_pos;
    ksys_pos_t center_pos;
    SDL_keysym key;
    static int ext_code = 0;
    static uint8_t old_mode = 0;
     
    while (1) {
        kos_event = _ksys_check_event();
        switch (kos_event) {
            case KSYS_EVENT_NONE:
                return;
            case KSYS_EVENT_REDRAW:
                kos_SDL_RepaintWnd();
                break;
            case KSYS_EVENT_KEY:
                key.scancode = _ksys_get_key().code;
                if (key.scancode == 0xE0 || key.scancode == 0xE1) {
                    ext_code = key.scancode;
                    break;
                }
                if (ext_code == 0xE1 && (key.scancode & 0x7F) == 0x1D) {
                    break;
                }
                if (ext_code == 0xE1 && key.scancode == 0xC5) {
                    ext_code=0;
                    break;
                }
                key.mod = GetModState();
    
                if (ext_code == 0xE1) key.mod &= ~KMOD_CTRL;
                if (!(key.scancode & 0x80))
                      old_mode = key.mod;
                SDL_SetModState(key.mod);
                int code = (key.scancode & 0x80) ? SDL_RELEASED : SDL_PRESSED;
                key.scancode &= 0x7F;

                if (ext_code == 0xE1 && key.scancode == 0x45)
                    key.sym = SDLK_PAUSE;
                else if (ext_code == 0xE0)
                    key.sym = sdlkeys_e0[key.scancode];
                else if (old_mode & KMOD_SHIFT)
                    key.sym = sdlkeys_shift[key.scancode];
                else
                    key.sym = sdlkeys[key.scancode];

                key.unicode=key.sym;
                ext_code = 0;
                if (!key.sym) break;

                SDL_PrivateKeyboard(code, &key);
                break;
            case KSYS_EVENT_BUTTON:
                if (_ksys_get_button() == 1) exit(0);
                break;
            case KSYS_EVENT_MOUSE: {
                static uint32_t old_mouse_but = 0;

                mouse_pos = _ksys_get_mouse_pos(KSYS_MOUSE_WINDOW_POS);
                if (mouse_pos.x >= 0 && mouse_pos.x < this->hidden->win_size_x &&
                    mouse_pos.y >= 0 && mouse_pos.y < this->hidden->win_size_y ||
                    this->input_grab != SDL_GRAB_OFF) {
     
                    if (this->input_grab != SDL_GRAB_OFF) {
                        center_pos.x = mouse_pos.x-this->hidden->win_size_x/2;
                        center_pos.y = mouse_pos.y-this->hidden->win_size_y/2;
                        if (center_pos.x || center_pos.y) {
                            SDL_PrivateMouseMotion(0, 1, center_pos.x, center_pos.y);
                            kos_CheckMouseMode(this);
                        }
                    } else {
                        SDL_PrivateMouseMotion(0, 0, mouse_pos.x, mouse_pos.y);
                    }

                    uint32_t mouse_but = _ksys_get_mouse_buttons();
                    if ((mouse_but ^ old_mouse_but) & KSYS_MOUSE_LBUTTON_PRESSED) {
                        if(mouse_but & KSYS_MOUSE_LBUTTON_PRESSED) {
                            SDL_PrivateMouseButton(SDL_PRESSED,SDL_BUTTON_LEFT,0,0);
                        } else {
                            SDL_PrivateMouseButton(SDL_RELEASED,SDL_BUTTON_LEFT,0,0);
                        }
                    }

                    if ((mouse_but ^ old_mouse_but) & KSYS_MOUSE_RBUTTON_PRESSED) {
                        if(mouse_but & KSYS_MOUSE_RBUTTON_PRESSED) {
                            SDL_PrivateMouseButton(SDL_PRESSED,SDL_BUTTON_RIGHT,0,0);
                        } else {
                            SDL_PrivateMouseButton(SDL_RELEASED,SDL_BUTTON_RIGHT,0,0);
                        }
                    }

                    if ((mouse_but ^ old_mouse_but) & KSYS_MOUSE_MBUTTON_PRESSED) {
                        if(mouse_but & KSYS_MOUSE_MBUTTON_PRESSED) {
                            SDL_PrivateMouseButton(SDL_PRESSED,SDL_BUTTON_MIDDLE,0,0);
                        } else {
                            SDL_PrivateMouseButton(SDL_RELEASED,SDL_BUTTON_MIDDLE,0,0);
                        }
                    }

                    old_mouse_but = mouse_but;
                }
            }
        }
    }
}