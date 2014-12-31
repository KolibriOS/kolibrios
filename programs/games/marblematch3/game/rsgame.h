#ifndef RSGAME_H_INCLUDED
#define RSGAME_H_INCLUDED

/*

    Heliothryx
    Game by Roman Shuvalov

*/

#ifndef RS_LINUX
    #ifndef RS_WIN32
        #ifndef RS_KOS
            #error Please specify platform
        #endif
    #endif
#endif


#include "rskos.h"
#include "rs/rsplatform.h"

#include "rs/rsdebug.h"
#include "rs/rsbits.h"


#include "rs/rsmx.h"




#define GAME_WIDTH  512
#define GAME_HEIGHT 512


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
#define COLOR_DARK_GRAY 0xFF333344
#define COLOR_SILVER    0xFFCCCCDD
#define COLOR_SEMI_TRANSPARENT      0x80808080

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
#define DRAW_MODE_MULT      3

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

#define FONTS_COUNT 4
#define CRYSTALS_COUNT  7

#define STATUS_LOADING  0
#define STATUS_MENU     1
#define STATUS_PLAYING  2
#define STATUS_PAUSED   3


#define RS_ARROW_LEFT_MASK	0x01
#define RS_ARROW_DOWN_MASK	0x02
#define RS_ARROW_UP_MASK	0x04
#define RS_ARROW_RIGHT_MASK	0x08
#define RS_ATTACK_KEY_MASK  0x10

#define BULLETS_COUNT   8

#define GAME_SHOOT_PERIOD   3

#define FIELD_WIDTH     11
#define FIELD_HEIGHT    8
#define FIELD_LENGTH    (FIELD_WIDTH * FIELD_HEIGHT)
#define CRYSTAL_SIZE    40
#define FIELD_X0     36
#define FIELD_Y0     128
#define FIELD_ITEM(x,y)     (game.field[(y)*FIELD_WIDTH+(x)])

#define CRYSTAL_INDEX_MASK      0x0F
#define CRYSTAL_VISIBLE_BIT     0x10
#define CRYSTAL_EXPLODED_BIT    0x20
#define CRYSTAL_MOVING_BIT      0x40

#define EXPLOSION_FRAMES_COUNT      10
#define EXPLOSION_SIZE      64

#define EXPLOSIONS_MAX_COUNT        16
//#define EXPLOSION_PACK(x,y,frame)   ( (x) | ( (y)<<8 ) |  (frame)<<16 )

#define ANIMATION_PROCESS_TIMER_LIMIT   3

#define SOUND_EXPLOSION_COUNT   8

//#define GAME_MODE_MATCH3    0
//#define GAME_MODE_RAMPAGE   1

#ifdef RS_KOS
    #define HISCORE_FILENAME    "/sys/games/marble-hiscore.dat"
#else
    #define HISCORE_FILENAME    "marble-hiscore.dat"
#endif

typedef struct rs_game_t {
    rs_texture_t framebuffer; 
    unsigned char *bgr_framebuffer; // 24-bit BGRBGRBGR... for direct drawing
    
    int loader_counter;
    
    rs_texture_t tex_bg;
    rs_texture_t tex_bg_gameplay;
    rs_texture_t tex_field;
    
    rs_texture_t tex_logo;
    rs_texture_t tex_clouds;
    
    rs_texture_t tex_crystals[CRYSTALS_COUNT];
    rs_texture_t tex_cursor;
    rs_texture_t tex_explosion[EXPLOSION_FRAMES_COUNT];
    
    rs_texture_t tex_font[64*FONTS_COUNT];
    
    int sound_index;
    rs_soundbuf_t sound_explosion[SOUND_EXPLOSION_COUNT];
    rs_soundbuf_t sound_tick;
//    rs_soundbuf_t sound_tack;
    rs_soundbuf_t sound_click;
    rs_soundbuf_t sound_bang;
    
    int status;
//    int game_mode;
    int menu_replay_timeout;
    
    unsigned int keyboard_state;
    
//    int menu_index;
//    int menu_item_index;
    
    int process_timer;
    
    int hiscore;
    
//    int tx;
//    int ty;
//    int tz;
    
    unsigned char *field;
    
    int selected;
    unsigned char selected_x;
    unsigned char selected_y;
    
    unsigned int explosions_count;
    unsigned int explosions[EXPLOSIONS_MAX_COUNT]; //0x00TTYYXX, TT = frame, YY = fieldY, XX = fieldX
    
    int need_redraw;
    
    int score;
    int time;
    
    
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

void GameMouseDown(int x, int y);
void GameMouseUp(int x, int y);


#endif // RSGAME_H_INCLUDED
