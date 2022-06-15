#ifndef __RC_VARS_H__
#define __RC_VARS_H__

// DGen/SDL v1.17+
// RC-modified variables listed here

#include <stdint.h>
#include "pd-defs.h" // Keysyms are defined in here
#include "rc.h"

// main.cpp defines IS_MAIN_CPP, which means we actually define the variables.
// Otherwise, we just declare them as externs

struct rc_str {
	const char *val;
	char *alloc;
	struct rc_str *next;
};

#ifdef IS_MAIN_CPP
#define RCVAR(name, def) intptr_t name = def
#define RCSTR(name, def) struct rc_str name = { def, NULL, NULL }
#define RCCTL(name, defk, defj, defm) \
	intptr_t name[RCB_NUM] = { defk, defj, defm }
#else
#define RCVAR(name, def) extern intptr_t name
#define RCSTR(name, def) extern struct rc_str name
#define RCCTL(name, defk, defj, defm) \
	extern intptr_t name[RCB_NUM]
#endif

RCCTL(pad1_up, PDK_UP, JS_AXIS(0, 1, JS_AXIS_NEGATIVE), 0);
RCCTL(pad1_down, PDK_DOWN, JS_AXIS(0, 1, JS_AXIS_POSITIVE), 0);
RCCTL(pad1_left, PDK_LEFT, JS_AXIS(0, 0, JS_AXIS_NEGATIVE), 0);
RCCTL(pad1_right, PDK_RIGHT, JS_AXIS(0, 0, JS_AXIS_POSITIVE), 0);
RCCTL(pad1_a, 'a', JS_BUTTON(0, 0), 0);
RCCTL(pad1_b, 's', JS_BUTTON(0, 3), 0);
RCCTL(pad1_c, 'd', JS_BUTTON(0, 1), 0);
RCCTL(pad1_x, 'q', JS_BUTTON(0, 6), 0);
RCCTL(pad1_y, 'w', JS_BUTTON(0, 4), 0);
RCCTL(pad1_z, 'e', JS_BUTTON(0, 5), 0);
RCCTL(pad1_mode, PDK_BACKSPACE, JS_BUTTON(0, 9), 0);
RCCTL(pad1_start, PDK_RETURN, JS_BUTTON(0, 8), 0);

RCCTL(pad2_up, PDK_KP8, JS_AXIS(1, 1, JS_AXIS_NEGATIVE), 0);
RCCTL(pad2_down, PDK_KP2, JS_AXIS(1, 1, JS_AXIS_POSITIVE), 0);
RCCTL(pad2_left, PDK_KP4, JS_AXIS(1, 0, JS_AXIS_NEGATIVE), 0);
RCCTL(pad2_right, PDK_KP6, JS_AXIS(1, 0, JS_AXIS_POSITIVE), 0);
RCCTL(pad2_a, PDK_DELETE, JS_BUTTON(1, 0), 0);
RCCTL(pad2_b, PDK_END, JS_BUTTON(1, 3), 0);
RCCTL(pad2_c, PDK_PAGEDOWN, JS_BUTTON(1, 1), 0);
RCCTL(pad2_x, PDK_INSERT, JS_BUTTON(1, 6), 0);
RCCTL(pad2_y, PDK_HOME, JS_BUTTON(1, 4), 0);
RCCTL(pad2_z, PDK_PAGEUP, JS_BUTTON(1, 5), 0);
RCCTL(pad2_mode, PDK_KP_PLUS, JS_BUTTON(1, 9), 0);
RCCTL(pad2_start, PDK_KP_ENTER, JS_BUTTON(1, 8), 0);

RCCTL(pico_pen_up,
      PDK_UP,
      JS_AXIS(0, 1, JS_AXIS_NEGATIVE),
      MO_MOTION(0, 'u'));
RCCTL(pico_pen_down,
      PDK_DOWN,
      JS_AXIS(0, 1, JS_AXIS_POSITIVE),
      MO_MOTION(0, 'd'));
RCCTL(pico_pen_left,
      PDK_LEFT,
      JS_AXIS(0, 0, JS_AXIS_NEGATIVE),
      MO_MOTION(0, 'l'));
RCCTL(pico_pen_right,
      PDK_RIGHT,
      JS_AXIS(0, 0, JS_AXIS_POSITIVE),
      MO_MOTION(0, 'r'));
RCCTL(pico_pen_button,
      PDK_RETURN,
      JS_BUTTON(0, 0),
      MO_BUTTON(0, 1));
RCVAR(pico_pen_stride, 2);
RCVAR(pico_pen_delay, 2);

RCCTL(dgen_fix_checksum, PDK_F1, 0, 0);
RCCTL(dgen_quit, PDK_ESCAPE, 0, 0);
RCCTL(dgen_craptv_toggle, PDK_F5, 0, 0);
RCCTL(dgen_scaling_toggle, PDK_F6, 0, 0);
RCCTL(dgen_screenshot, PDK_F12, 0, 0);
RCCTL(dgen_reset, PDK_TAB, 0, 0);
RCCTL(dgen_z80_toggle, PDK_F10, 0, 0);
RCCTL(dgen_cpu_toggle, PDK_F11, 0, 0);
RCCTL(dgen_stop, 'z', 0, 0);
RCCTL(dgen_prompt, ':', 0, 0);
RCCTL(dgen_game_genie, PDK_F9, 0, 0);
RCCTL(dgen_fullscreen_toggle, (KEYSYM_MOD_ALT | PDK_RETURN), 0, 0);
RCCTL(dgen_debug_enter, '`', 0, 0);
RCCTL(dgen_volume_inc, '=', 0, 0);
RCCTL(dgen_volume_dec, '-', 0, 0);

RCCTL(dgen_slot_0, '0', 0, 0);
RCCTL(dgen_slot_1, '1', 0, 0);
RCCTL(dgen_slot_2, '2', 0, 0);
RCCTL(dgen_slot_3, '3', 0, 0);
RCCTL(dgen_slot_4, '4', 0, 0);
RCCTL(dgen_slot_5, '5', 0, 0);
RCCTL(dgen_slot_6, '6', 0, 0);
RCCTL(dgen_slot_7, '7', 0, 0);
RCCTL(dgen_slot_8, '8', 0, 0);
RCCTL(dgen_slot_9, '9', 0, 0);
RCCTL(dgen_slot_next, PDK_F8, 0, 0);
RCCTL(dgen_slot_prev, PDK_F7, 0, 0);
RCCTL(dgen_save, PDK_F2, 0, 0);
RCCTL(dgen_load, PDK_F3, 0, 0);

RCVAR(dgen_autoload, 0);
RCVAR(dgen_autosave, 0);
RCVAR(dgen_autoconf, 1);
RCVAR(dgen_frameskip, 1);
RCVAR(dgen_show_carthead, 0);
RCSTR(dgen_rom_path, "roms"); /* synchronize with romload.c */

RCVAR(dgen_sound, 1);
RCVAR(dgen_soundrate, 44100);
RCVAR(dgen_soundsegs, 8);
RCVAR(dgen_soundsamples, 0);
RCVAR(dgen_volume, 100);
RCVAR(dgen_mjazz, 0);

RCVAR(dgen_hz, 60);
RCVAR(dgen_pal, 0);
RCVAR(dgen_region, 1);
RCSTR(dgen_region_order, "JUEX");

RCVAR(dgen_raw_screenshots, 0);
RCVAR(dgen_craptv, 0);
RCVAR(dgen_scaling, 0);
RCVAR(dgen_nice, 0);
#ifdef WITH_JOYSTICK
RCVAR(dgen_joystick, 1);
#else
RCVAR(dgen_joystick, 0);
#endif
RCVAR(dgen_mouse_delay, 50);

RCVAR(dgen_fps, 1);
RCVAR(dgen_buttons, 0);
RCVAR(dgen_fullscreen, 0);
RCVAR(dgen_info_height, -1);
RCVAR(dgen_width, -1);
RCVAR(dgen_height, -1);
RCVAR(dgen_scale, -1);
RCVAR(dgen_x_scale, -1);
RCVAR(dgen_y_scale, -1);
RCVAR(dgen_aspect, 1);
RCVAR(dgen_depth, 24);
RCVAR(dgen_swab, 0);
RCVAR(dgen_opengl, 1);
RCVAR(dgen_opengl_stretch, 1);
RCVAR(dgen_opengl_linear, 1);
RCVAR(dgen_opengl_32bit, 1);
RCVAR(dgen_opengl_square, 0);
RCVAR(dgen_doublebuffer, 1);
RCVAR(dgen_screen_thread, 0);
RCVAR(dgen_vdp_hide_plane_a, 0);
RCVAR(dgen_vdp_hide_plane_b, 0);
RCVAR(dgen_vdp_hide_plane_w, 0);
RCVAR(dgen_vdp_hide_sprites, 0);
RCVAR(dgen_vdp_sprites_boxing, 0);
RCVAR(dgen_vdp_sprites_boxing_fg, 0xffff00); // yellow
RCVAR(dgen_vdp_sprites_boxing_bg, 0x00ff00); // green

// Keep values in sync with rc.cpp and enums in md.h

#if defined(WITH_DRZ80)
RCVAR(dgen_emu_z80, 3);
#elif defined(WITH_CZ80)
RCVAR(dgen_emu_z80, 2);
#elif defined(WITH_MZ80)
RCVAR(dgen_emu_z80, 1);
#else
RCVAR(dgen_emu_z80, 0);
#endif

#if defined(WITH_CYCLONE)
RCVAR(dgen_emu_m68k, 3);
#elif defined(WITH_MUSA)
RCVAR(dgen_emu_m68k, 2);
#elif defined(WITH_STAR)
RCVAR(dgen_emu_m68k, 1);
#else
RCVAR(dgen_emu_m68k, 0);
#endif

#endif // __RC_VARS_H__
