#ifndef CONFIG_H
#define CONFIG_H

#define DEFAULT_WIDTH 600
#define FPS 60

#define DELTA_MS_DEFAULT 20

#define WINDOW_TITLE "DINO. Jump: UP/SPACE  Duck: DOWN  Restart: jump or ENTER"

// #define DBG

#ifdef DBG
#define dbg_printf(...) debug_printf(__VA_ARGS__)
#else 
#define dbg_printf(...)
#endif

#endif
