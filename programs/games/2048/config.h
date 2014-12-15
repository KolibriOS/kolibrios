#ifndef CONFIG_H
#define CONFIG_H

#include "defines.h"

// Get saved highscore
__u32 config_load_highscore();

// Save current highscore
void config_save_highscore(__u32 score);

#endif // CONFIG_H
