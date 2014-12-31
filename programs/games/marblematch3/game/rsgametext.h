#ifndef RS_GAMETEXT_H
#define RS_GAMETEXT_H

#include "rsgame.h"

#define GAME_COLORS_COUNT   8



void game_font_init();
void game_font_term(); 

void game_textout(int x, int y, int font_index, char* s);
void game_textout_at_center(int x, int y, int font_index, char *s);
void game_textout_adv(rs_texture_t *dest, int x, int y, int font_index, int draw_mode, char* s);

#endif
