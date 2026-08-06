#ifndef CONFIG_H
#define CONFIG_H

#define DEFAULT_WIDTH 600
#define DEFAULT_HEIGHT 200

#define GAME_HEIGHT 150
#define GAME_Y_OFFSET ((DEFAULT_HEIGHT - GAME_HEIGHT) / 2)

#define FPS 60

#define DELTA_MS_DEFAULT 20

// The leading \3 tells the kernel the caption is UTF-8 (see gui/window.inc)
#ifdef LANG_RUS
#define WINDOW_TITLE "\3DINO [ПРЫЖОК - UP/SPACE | ПРИСЕД - DOWN | РЕСТАРТ - ПРЫЖОК/ENTER]"
#elif defined LANG_SPA
#define WINDOW_TITLE "\3DINO [SALTAR - ARRIBA/ESPACIO | AGACHARSE - ABAJO | REINICIAR - SALTAR/ENTER]"
#else
#define WINDOW_TITLE "\3DINO [JUMP - UP/SPACE | DUCK - DOWN | RESTART - JUMP/ENTER]"
#endif

// #define DBG

#ifdef DBG
#include <stdio.h>
#define dbg_printf(...) debug_printf(__VA_ARGS__)
#else
#define dbg_printf(...)
#endif

#endif
