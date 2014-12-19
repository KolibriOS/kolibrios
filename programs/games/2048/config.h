#ifndef CONFIG_H
#define CONFIG_H

#include "defines.h"

typedef struct {
    __u32 score;
    __u32 highscore;
    __u32 value_map[BOARD_COUNT * BOARD_COUNT];
} config_state;

// Get saved highscore
__u8 config_load(config_state* st);

// Save current highscore
__u8 config_save(config_state* st);

#endif // CONFIG_H
