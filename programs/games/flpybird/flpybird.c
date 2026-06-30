// SPDX-License-Identifier: GPL-2.0-only
//
// Floppy Bird - Infinite flopping game
// Copyright (C) 2021-2026 KolibriOS team
//
// Contributor zorggish - Main code
// Contributor Burer - Rewrite to C

#include <sys/ksys.h>

#include "images.h"

/* ===== Global const strings ===== */

static const char HEADER_STRING[]   = "FLOPPY BIRD";
static const char CONTROL_STRING[]  = "SPACEBAR TO JUMP";
static const char GAMEOVER_STRING[] = "GAME OVER";
static const char SPA_KEY_STRING[]  = "PRESS SPACEBAR TO RESTART";
static const char ESC_KEY_STRING[]  = "PRESS ESCAPE TO MENU";
static const char FAST_STRING[]     = "1 - FAST";
static const char SLOW_STRING[]     = "2 - SLOW";

/* ===== Global window variables ===== */

#define WIN_W 400
#define WIN_H 376
#define WIN_B 5
#define WIN_S 10

#define BUF_W (WIN_W - WIN_S)
#define BUF_H (WIN_H - WIN_B)

static int WIN_X;
static int WIN_Y;
static int WIN_T;

/* ===== Global game variables ===== */

#define GAMESTATE_MENU     0
#define GAMESTATE_STARTED  1
#define GAMESTATE_GAMEOVER 2

static int  game_rng;
static int  game_speed = 1;
static char game_score[] = "SCORE: \0\0\0"; // digits set by start_game before first draw; \0 packs smaller than '0'
static int  game_state;

/* ===== Functions - Buffer ===== */

#define GPX (gbuf + 2)

static unsigned gbuf[2 + BUF_W * BUF_H];

static void buffer_init(void) {
    gbuf[0] = BUF_W;
    gbuf[1] = BUF_H;
}

static void buffer_fill(unsigned color) {
    unsigned* p = GPX;
    for (int i = BUF_W * BUF_H; i; --i)
        *p++ = color;
}

static void buffer_image(const char* img, int dx, int dy, int w, int h) {
    int sx0 = dx < 0 ? -dx : 0;
    int sx1 = dx + w > BUF_W ? BUF_W - dx : w;
    int sy0 = dy < 0 ? -dy : 0;
    int sy1 = dy + h > BUF_H ? BUF_H - dy : h;
    for (int sy = sy0; sy < sy1; ++sy) {
        unsigned* dst = GPX + (dy + sy) * BUF_W + dx;
        const char* src = img + sy * w;
        for (int sx = sx0; sx < sx1; ++sx)
            dst[sx] = GAME_PALETTE[(unsigned char)src[sx]];
    }
}

static void buffer_vrep(const char* row, int dx, int dy0, int dy1, int w) {
    int sx0 = dx < 0 ? -dx : 0;
    int sx1 = dx + w > BUF_W ? BUF_W - dx : w;
    if (dy0 < 0)     dy0 = 0;
    if (dy1 > BUF_H) dy1 = BUF_H;
    for (int y = dy0; y < dy1; ++y) {
        unsigned* dst = GPX + y * BUF_W + dx;
        for (int sx = sx0; sx < sx1; ++sx)
            dst[sx] = GAME_PALETTE[(unsigned char)row[sx]];
    }
}

static void buffer_text(const char* text, int x, int y, int len, unsigned color) {
    asm_inline(
        "int $0x40"
        :
        : "a"(4), "d"(text), "b"((x << 16) | y), "S"(len),
          "c"(color | 0x08000000u), "D"(gbuf)
        : "memory"
    );
}

static void buffer_blit(void) {
    ksys_draw_bitmap_palette(GPX, 0, 0, BUF_W, BUF_H, 32, 0, 0);
}

/* ===== Bird ===== */

#define BIRD_W 19
#define BIRD_H 20
#define BIRD_X 100

static int bird_pos;
static int bird_acc;

static void bird_initialize(void) {
    bird_pos = (WIN_H + WIN_T) / 2;
    bird_acc = 0;
}

static void bird_move(void) {
    bird_acc += (bird_acc <= 30) * 2; // 2 or 0
    bird_pos += bird_acc / 10;
}

static void bird_jump(void) {
    bird_acc = -50;
}

/* ===== Tube ===== */

#define TUBE_WIDTH      50
#define TUBE_GAPHEIGHT  100
#define TUBE_HEADHEIGHT 18

#define tube_num        3

static int tube_pos[tube_num];
static int tube_gap[tube_num];

static void tube_randomize(int t) {
    int x = game_rng; x ^= x << 13; x ^= x >> 17; x ^= x << 5; game_rng = x;
    int r = x & 255u;
    if (r >= WIN_H / 2)
        r -= 256 - WIN_H / 2;
    tube_pos[t] = WIN_W + 1;
    tube_gap[t] = r + (WIN_H / 2 - TUBE_GAPHEIGHT - WIN_B) / 2;
}

static void tube_move(int t) {
    tube_pos[t] -= 2;
    if (tube_pos[t] < -TUBE_WIDTH - 2)
        tube_randomize(t);
}

static void tube_draw(int t) {
    int x     = tube_pos[t];
    int gap   = tube_gap[t];
    int top   = gap - TUBE_HEADHEIGHT;
    int bot   = gap + TUBE_GAPHEIGHT;

    /* top body */
    buffer_vrep(TUBE_BODY_IMAGE, x, 0, top, TUBE_WIDTH);
    /* top head */
    buffer_image(TUBE_HEAD_IMAGE, x, top, TUBE_WIDTH, TUBE_HEADHEIGHT);
    /* bottom head */
    buffer_image(TUBE_HEAD_IMAGE, x, bot, TUBE_WIDTH, TUBE_HEADHEIGHT);
    /* bottom body */
    buffer_vrep(TUBE_BODY_IMAGE, x, bot + TUBE_HEADHEIGHT, BUF_H, TUBE_WIDTH);
}

/* ===== Functions - Helpers ===== */

static void write_bordered_text(int x, int y, const char* text_ptr, int text_len, int font_type) {
    for (int bx = x + 2; bx >= x - 2; --bx)
        for (int by = y + 2; by >= y - 2; --by)
            buffer_text(text_ptr, bx, by, text_len, font_type);
    buffer_text(text_ptr, x, y, text_len, font_type | 0x00FFFFFF);
}

/* ===== Functions - Game Logic ===== */

static inline int check_collision(int t) {
    return (tube_pos[t] <= (BIRD_X + BIRD_W) && tube_pos[t] >= BIRD_X - TUBE_WIDTH)
        && (bird_pos <= tube_gap[t] || bird_pos >= tube_gap[t] + TUBE_GAPHEIGHT - BIRD_H);
}

static inline int check_add_score(int t) {
    int r = tube_pos[t] + TUBE_WIDTH;
    return (r >= BIRD_X - 2) & (r < BIRD_X);
}

static void update_score_string(void) {
    for (int i = 9; i >= 6; --i) {
        if (++game_score[i] <= '9') return;
        game_score[i] = '0';
    }
}

/* ===== Functions - Game Drawing ===== */

static void create_game_window(void) {
    _ksys_create_window(WIN_X, WIN_Y, WIN_W, WIN_H + WIN_T, HEADER_STRING, GAME_PALETTE[0], 0x34);
}

static void redraw_game_window(void) {
    buffer_fill(GAME_PALETTE[0]);

    tube_draw(0); tube_draw(1); tube_draw(2);

    buffer_image(BIRD_IMAGE, BIRD_X, bird_pos, BIRD_W, BIRD_H);

    write_bordered_text(10, 10, game_score, 0, 0x81000000);

    buffer_blit();
}

static void draw_game_over_window(void) {
    create_game_window();

    buffer_fill(GAME_PALETTE[0]);
    write_bordered_text(116, 100, GAMEOVER_STRING, 0, 0x82000000);
    write_bordered_text(136, 157,      game_score, 0, 0x81000000);
    write_bordered_text( 46, 207,  SPA_KEY_STRING, 0, 0x81000000);
    write_bordered_text( 76, 241,  ESC_KEY_STRING, 0, 0x81000000);

    buffer_blit();
}

static void draw_menu_window(void) {
    create_game_window();

    buffer_fill(GAME_PALETTE[0]);

    write_bordered_text( 88, 34, HEADER_STRING,     6, 0x04000000);
    write_bordered_text(188, 87, HEADER_STRING + 7, 4, 0x04000000);

    /* decorative tube, rotated 90 degrees (heads + body) */
    const char* pos = TUBE_HEAD_IMAGE;
    for (int x = 50 - 1; x >= 50 - TUBE_HEADHEIGHT; --x)
        for (int y = 156; y < 156 + TUBE_WIDTH; ++y) {
            unsigned c = GAME_PALETTE[(unsigned char)*pos++];
            GPX[y * BUF_W + x]          = c;
            GPX[(y + 82) * BUF_W + x]   = c;
        }

    for (int x = 50; x < BUF_W; ++x)
        for (int yy = 0; yy < TUBE_WIDTH; ++yy) {
            unsigned c = GAME_PALETTE[(unsigned char)TUBE_BODY_IMAGE[yy]];
            GPX[(156 + yy) * BUF_W + x] = c;
            GPX[(238 + yy) * BUF_W + x] = c;
        }

    write_bordered_text(139, 171,    FAST_STRING, 0, 0x82000000);
    write_bordered_text(139, 253,    SLOW_STRING, 0, 0x82000000);

    // Control hint
    write_bordered_text(100, 322, CONTROL_STRING, 0, 0x81000000);

    buffer_blit();
}

/* ===== Functions - Game Main ===== */

static void start_game(void) {
    create_game_window();

    game_score[7] = game_score[8] = game_score[9] = '0';

    bird_initialize();

    const int spacing = (WIN_W + TUBE_WIDTH) / 3;
    for (int i = tube_num - 1; i >= 0; --i) {
        tube_randomize(i);
        tube_pos[i] = WIN_W + i * spacing;
    }

    game_state = GAMESTATE_STARTED;
}

static void frame_game(void) {
    bird_move();

    /* Processing all tubes */
    for (int i = tube_num - 1; i >= 0; --i) {
        /* Adding score */
        if (check_add_score(i))
            update_score_string();

        /* Check collision with bird */
        if (check_collision(i))
            goto game_over;

        /* Move tube */
        tube_move(i);
    }

    /* Checking bird is too high or low */
    if (bird_pos > WIN_H - WIN_B - BIRD_H || bird_pos < 0)
        goto game_over;

    redraw_game_window();
    return;

    game_over:
    game_state = GAMESTATE_GAMEOVER;
    draw_game_over_window();
    return;
}

int main(void) {
    /* Setting RNG seed */
    ksys_time_t t = _ksys_get_time();
    game_rng = (int)t.val | 1u;

    /* Centering window */
    ksys_pos_t screen = _ksys_screen_size();
    WIN_X = (screen.x - WIN_W) / 2;
    WIN_Y = (screen.y - WIN_H) / 2;
    WIN_T = _ksys_get_skin_height();

    buffer_init();

    game_state = GAMESTATE_MENU;

    for (;;) {
        _ksys_delay(game_speed);

        switch (_ksys_check_event()) {
            case KSYS_EVENT_NONE:
                if (game_state == GAMESTATE_STARTED)
                    frame_game();
                break;

            case KSYS_EVENT_REDRAW: {
                switch (game_state) {
                    case GAMESTATE_MENU:     draw_menu_window();      break;
                    case GAMESTATE_STARTED:  redraw_game_window();    break;
                    case GAMESTATE_GAMEOVER: draw_game_over_window(); break;
                }
                break;
            }

            case KSYS_EVENT_KEY: {
                ksys_oskey_t k = _ksys_get_key();
                switch (game_state) {
                    case GAMESTATE_MENU: {
                        unsigned kc = k.code - 49; // 1 or 2
                        if (kc <= 1) {
                            game_speed = kc + 1;
                            start_game();
                        }
                        break;
                    }
                    case GAMESTATE_STARTED: {
                        if (k.code == 32) // SPACE
                            bird_jump();
                        break;
                    }
                    case GAMESTATE_GAMEOVER: {
                        if (k.code == 32) // SPACE
                            start_game();
                        if (k.code == 27) // ESCAPE
                        {
                            game_state = GAMESTATE_MENU;
                            draw_menu_window();
                        }
                        break;
                    }
                }
                break;
            }

            case KSYS_EVENT_BUTTON:
                _ksys_exit();
                break;
        }
    }
}
