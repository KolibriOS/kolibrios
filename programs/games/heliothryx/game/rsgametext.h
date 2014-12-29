#ifndef RS_GAMETEXT_H
#define RS_GAMETEXT_H

#include "rsgame.h"

#define GAME_COLORS_COUNT   8

#define GAME_COLOR_BLACK    0
#define GAME_COLOR_WHITE    1
#define GAME_COLOR_YELLOW   2
#define GAME_COLOR_RED      3
#define GAME_COLOR_BLUE     4
#define GAME_COLOR_ORANGE   5
#define GAME_COLOR_GRAY     6
#define GAME_COLOR_GREEN    7

#define GAME_FONT_DEFAULT   0
#define GAME_FONT_HEAVY     1
#define GAME_FONT_TITLE     2
#define GAME_FONT_LIGHT     3

#define GAME_ALIGN_LEFT     0
#define GAME_ALIGN_CENTER   1
#define GAME_ALIGN_RIGHT    2



void game_font_init();
void game_font_term(); 

void game_textout(int x, int y, int font_index, char* s);
void game_textout_at_center(int x, int y, int font_index, char *s);
void game_textout_adv(rs_texture_t *dest, int x, int y, int font_index, int draw_mode, char* s);
//void game_textout_init(int set_to_ortho, int font_index);

#endif
