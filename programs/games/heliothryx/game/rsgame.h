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

#include "rsgamelogic.h"


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
#define COLOR_DARK_RED  0xFF401000



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
void soundbuf_play(rs_soundbuf_t *snd, int mode);
void soundbuf_loop_check(rs_soundbuf_t *snd);
void soundbuf_stop(rs_soundbuf_t *snd);

// Game Objects

#define     GAME_OBJS_MAX_COUNT     1024

#define     OBJ_PLAYER      0
#define     OBJ_BULLET      1
#define     OBJ_EXPLOSION   2
#define     OBJ_ROCK        3
#define     OBJ_MINIROCK    4
#define     OBJ_TURRET      5
#define     OBJ_RED_BULLET  6
#define     OBJ_HUGE_EXPLOSION  7

typedef struct game_obj_t {
    int obj_type;
    int flags;
    int tag;
    int radius;
    
    float x;
    float y;
    int t;
    float f;

} game_obj_t;

#define OBJ_FLAG_DESTROYED      0x01
#define OBJ_FLAG_ENEMY          0x02
#define OBJ_FLAG_SIN            0x04
#define OBJ_FLAG_BOSS           0x08 // draw health-bar above

game_obj_t game_obj(int obj_type, int flags, int tag, int radius, float x, float y, int t, float f);

int game_obj_add(game_obj_t obj);
void game_obj_remove(int index);



// Game Registry

#define ROCKS_COUNT 8
#define MINIROCKS_COUNT ROCKS_COUNT // must equal
#define FONTS_COUNT 4
#define EXPLOSIONS_COUNT    8
#define EXPLOSION_RADIUS    16

#define HUGE_EXPLOSIONS_COUNT    24
#define HUGE_EXPLOSION_RADIUS    32


#define STATUS_MENU     0
#define STATUS_PLAYING  1
#define STATUS_PAUSED   2


#define RS_ARROW_LEFT_MASK	0x01
#define RS_ARROW_DOWN_MASK	0x02
#define RS_ARROW_UP_MASK	0x04
#define RS_ARROW_RIGHT_MASK	0x08
#define RS_ATTACK_KEY_MASK  0x10


#define GAME_SHOOT_PERIOD   3

#define GAME_FLAG_BOSS_DESTROYED    0x01
#define GAME_FLAG_INSTRUCTIONS_PASSED   0x02

#define SOUND_EXPLOSIONS_COUNT      8

typedef struct rs_game_t {
    rs_texture_t framebuffer; 
    unsigned char *scaled_framebuffer; // 24-bit BGRBGRBGR... for direct drawing
    
    rs_texture_t tex;
    
    rs_texture_t tex_clouds;
    rs_texture_t tex_ground;
    
    rs_texture_t tex_ship[4];
    rs_texture_t tex_rocks[ROCKS_COUNT];
    rs_texture_t tex_minirocks[MINIROCKS_COUNT];
    
    rs_texture_t tex_explosions[EXPLOSIONS_COUNT];
    rs_texture_t tex_huge_explosions[HUGE_EXPLOSIONS_COUNT];
    
    rs_texture_t tex_font[64*FONTS_COUNT];
    
    rs_texture_t tex_gui_line;
    
    int bg_color;
    
    rs_soundbuf_t sound_shoot;
    rs_soundbuf_t sound_turret_shoot;
    
    rs_soundbuf_t sound_test2;
    rs_soundbuf_t sound_test3;
    
    rs_soundbuf_t sound_explosions[SOUND_EXPLOSIONS_COUNT];
    rs_soundbuf_t sound_hit;
    rs_soundbuf_t sound_huge_explosion;
    
    rs_soundbuf_t sound_music;
//    rs_soundbuf_t sound_music2;
    
    int status;
    int flags;
    
    unsigned int keyboard_state;
    
    int menu_index;
    int menu_item_index;
    
    int window_scale;
    
    int tz;
    
    int player_x;
    int player_y;

    int shoot_delay;
    int shoot_keypressed;
    int shoot_restore_delay;
    
    int health;
    int ammo;
    int score;
    
//    int ammo_max;
    
    int stage; // (wave)
    int stage_timer;
    
    int stage_level; // (stage)
    
    game_obj_t *objs;
    int objs_count;
    
} rs_game_t;

#define GAME_HEALTH_MAX     8
#define GAME_AMMO_MAX       24

extern rs_game_t game;
void game_reg_init();









/*  __
   /cc\
  /aaaa\
 |kkkkkk|  <-- Easter Egg
  \eeee/
------------------------------- */









void game_ding(int i);

void GameInit();
void GameTerm();

void GameKeyDown(int key);
void GameKeyUp(int key);

void GameMouseDown(int x, int y);
void GameMouseUp(int x, int y);

void game_change_window_scale(int d);

int is_key_pressed(int mask);
unsigned short rs_rand();

#endif // RSGAME_H_INCLUDED
