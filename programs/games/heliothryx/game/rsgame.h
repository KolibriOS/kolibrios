#ifndef RSGAME_H_INCLUDED
#define RSGAME_H_INCLUDED

/*

    Heliothryx
    Game by Roman Shuvalov

*/


#include "rskos.h"
#include "rs/rsplatform.h"
//#include "rs/rstexture.h"
//#include "rs/rsshader.h"
//#include "rs/rsgl.h"
#include "rs/rsdebug.h"
#include "rs/rsbits.h"

//#include "rs/rskeyboard.h"

//#include "rs/rsaudio.h"

//#include "rs/rsfile.h"

//#include "rs/rsvbo.h"
//#include "rs/rsfbo.h"

//#include "rs/rsthread.h"

#include "rs/rsmx.h"




#define GAME_LANG_EN    0
#define GAME_LANG_RU    1

#define GAME_WIDTH  320
#define GAME_HEIGHT 180


typedef struct {
    unsigned int status; 
    int w;
    int h;
    unsigned char *data; // BGRA BGRA
} rs_texture_t;


// for little-endian
typedef union color_t {
    int d;                 // 0x44332211 (ARGB)
    struct {
        unsigned char b; // 0x11
        unsigned char g; // 0x22
        unsigned char r; // 0x33
        unsigned char a; // 0x44
    };
} color_t;

// for little-endian (ARGB)
#define COLOR_BLACK     0xFF000000
#define COLOR_TRANSPARENT   0x00000000
#define COLOR_DARK_RED  0xFF660000



void texture_init(rs_texture_t *tex, int w, int h);
void texture_free(rs_texture_t *tex);
void texture_clear(rs_texture_t *tex, unsigned int color); 
void texture_draw(rs_texture_t *dest, rs_texture_t *src, int x, int y, int mode);
void texture_draw_vline(rs_texture_t *tex, int x, int y, int l, unsigned int color);
void texture_draw_hline(rs_texture_t *tex, int x, int y, int l, unsigned int color);
void texture_set_pixel(rs_texture_t *tex, int x, int y, unsigned int color);

unsigned char clamp_byte(int value);

#define DRAW_MODE_REPLACE   0
#define DRAW_MODE_ADDITIVE  1
#define DRAW_MODE_ALPHA     2

#define DRAW_MODE_MASK      0x0000FFFF
#define DRAW_TILED_FLAG     0x00010000



typedef struct {
    unsigned int status;
    int length_samples;
    SNDBUF hbuf;
    signed short *data;
} rs_soundbuf_t;

void soundbuf_init(rs_soundbuf_t *snd, int length);
void soundbuf_free(rs_soundbuf_t *snd);
void soundbuf_fill(rs_soundbuf_t *snd, int amp, int freq_div);
void soundbuf_sin(rs_soundbuf_t *snd, float freq);
void soundbuf_sin_fade(rs_soundbuf_t *snd, float freq);
void soundbuf_play(rs_soundbuf_t *snd);
void soundbuf_stop(rs_soundbuf_t *snd);

// Game Registry

#define ROCKS_COUNT 3
#define FONTS_COUNT 4

#define STATUS_MENU     0
#define STATUS_PLAYING  1
#define STATUS_PAUSED   2


#define RS_ARROW_LEFT_MASK	0x01
#define RS_ARROW_DOWN_MASK	0x02
#define RS_ARROW_UP_MASK	0x04
#define RS_ARROW_RIGHT_MASK	0x08
#define RS_ATTACK_KEY_MASK  0x10

#define BULLETS_COUNT   8

#define GAME_SHOOT_PERIOD   3

typedef struct rs_game_t {
    rs_texture_t framebuffer; 
    unsigned char *scaled_framebuffer; // 24-bit BGRBGRBGR... for direct drawing
    
    rs_texture_t tex;
    
    rs_texture_t tex_clouds;
    rs_texture_t tex_ground;
    
    rs_texture_t tex_ship[4];
    rs_texture_t tex_rocks[ROCKS_COUNT];
    
    rs_texture_t tex_font[64*FONTS_COUNT];
    
    rs_texture_t tex_gui_line;
    
    
    rs_soundbuf_t sound_test1;
    rs_soundbuf_t sound_test2;
    rs_soundbuf_t sound_test3;
    
    int status;
    
    unsigned int keyboard_state;
    
    int menu_index;
    int menu_item_index;
    
    int window_scale;
    
    int tx;
    int ty;
    int tz;
    
    int bullet_x[BULLETS_COUNT];
    int bullet_y[BULLETS_COUNT];
    int bullet_index;
    int shoot_delay;
    int shoot_keypressed;
    
} rs_game_t;

extern rs_game_t game;
void game_reg_init();

/*  __
   /cc\
  /aaaa\
 |kkkkkk|  <-- Easter Egg
  \eeee/
------------------------------- */

void GameProcess();

void game_ding(int i);

void GameInit();
void GameTerm();

void GameKeyDown(int key, int first);
void GameKeyUp(int key);

void game_change_window_scale(int d);

#endif // RSGAME_H_INCLUDED
