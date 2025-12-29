// SPDX-License-Identifier: GPL-2.0-only
//
// Floppy Bird - Infinite flopping game
// Copyright (C) 2021-2025 KolibriOS team
//
// Contributor zorggish - Main code
// Contributor Burer - Rewrite to C

#include <sys/ksys.h>

#include "images.h"

/* ===== Global const strings ===== */

static const char HEADER_STRING[]   = "Floppy Bird";
static const char CONTROL_STRING[]  = "SPACEBAR TO JUMP";
static const char GAMEOVER_STRING[] = "GAME OVER";
static const char SPA_KEY_STRING[]  = "PRESS SPACEBAR FOR RESTART";
static const char ESC_KEY_STRING[]  = "PRESS ESCAPE FOR MENU";
static const char FAST_STRING[]     = "1 - FAST";
static const char SLOW_STRING[]     = "2 - SLOW";

/* ===== Global window variables ===== */

#define WIN_W 400
#define WIN_H 376
#define WIN_B 5
#define WIN_S 10

static int WIN_X;
static int WIN_Y;
static int WIN_T;

/* ===== Global game variables ===== */

#define GAMESTATE_MENU     0
#define GAMESTATE_STARTED  1
#define GAMESTATE_GAMEOVER 2

static int  game_rng;
static int  game_speed = 1;
static int  game_pause = 0;
static char game_score[] = "Score: 000";
static int  game_state;

/* ===== Bird ===== */

#define BIRD_W 19
#define BIRD_H 20
#define BIRD_X 100

static int bird_pos;
static int bird_acc;

static void Bird_initialize(void) {
    bird_pos = (WIN_H + WIN_T) / 2;
    bird_acc = 0;
}

static void Bird_move(void) {
    bird_acc += (bird_acc <= 30) * 2; // 2 or 0
    bird_pos += bird_acc / 10;
}

static void Bird_jump(void) {
    bird_acc = -50;
}

/* ===== Tube ===== */

#define TUBE_WIDTH      50
#define TUBE_GAPHEIGHT  100
#define TUBE_HEADHEIGHT 18

#define tube_num        3

static int tube_pos[tube_num];
static int tube_gap[tube_num];

static void Tube_randomize(int t) {
    int x = game_rng; x ^= x << 13; x ^= x >> 17; x ^= x << 5; game_rng = x;
    int r = x & 255u;
    if (r >= WIN_H / 2)
        r -= 256 - WIN_H / 2;
    tube_pos[t] = WIN_W + 1;
    tube_gap[t] = r + (WIN_H / 2 - TUBE_GAPHEIGHT - WIN_B) / 2;
}

static void Tube_move(int t) {
    tube_pos[t] -= 2;
    if (tube_pos[t] < -TUBE_WIDTH - 2)
        Tube_randomize(t);
}

static void Tube_draw(int t) {
    /* cleanup */
    int pixels = WIN_W - WIN_S - TUBE_WIDTH - tube_pos[t];
    if (pixels > 0) {
        if (pixels > 2)
            pixels = 2;
        _ksys_draw_bar(tube_pos[t] + TUBE_WIDTH, tube_gap[t] - TUBE_HEADHEIGHT, pixels, TUBE_HEADHEIGHT, GAME_PALETTE[0]);
        _ksys_draw_bar(tube_pos[t] + TUBE_WIDTH, tube_gap[t] + TUBE_GAPHEIGHT , pixels, TUBE_HEADHEIGHT, GAME_PALETTE[0]);
    }
    if (tube_pos[t] < 0) {
        int w = -tube_pos[t];
        if (w > 2)
            w = 2;
        _ksys_draw_bar(0, tube_gap[t] - TUBE_HEADHEIGHT, w, TUBE_HEADHEIGHT, GAME_PALETTE[0]);
        _ksys_draw_bar(0, tube_gap[t] + TUBE_GAPHEIGHT , w, TUBE_HEADHEIGHT, GAME_PALETTE[0]);
    }

    int offset = tube_pos[t] >= 0 ? 0 : -tube_pos[t];
    int trim   = (tube_pos[t] + TUBE_WIDTH >= WIN_W - WIN_S)
               ? WIN_W - tube_pos[t] - TUBE_WIDTH - WIN_S : 0;

    if (offset >= TUBE_WIDTH + trim)
        return;

    /* top */
    for (int y = 0; y < tube_gap[t] - TUBE_HEADHEIGHT; ++y)
        ksys_draw_bitmap_palette(
            TUBE_BODY_IMAGE + offset,
            tube_pos[t] + offset, y, TUBE_WIDTH - offset + trim, 1, 8, GAME_PALETTE, 0
        );

    /* head top */
    for (int y = tube_gap[t] - TUBE_HEADHEIGHT; y < tube_gap[t]; ++y)
        ksys_draw_bitmap_palette(
            TUBE_HEAD_IMAGE + TUBE_WIDTH * (y - tube_gap[t] + TUBE_HEADHEIGHT) + offset,
            tube_pos[t] + offset, y, TUBE_WIDTH - offset + trim, 1, 8, GAME_PALETTE, 0
        );
    /* head down */
    for (int y = tube_gap[t] + TUBE_GAPHEIGHT; y < tube_gap[t] + TUBE_GAPHEIGHT + TUBE_HEADHEIGHT; ++y)
        ksys_draw_bitmap_palette(
            TUBE_HEAD_IMAGE + TUBE_WIDTH * (y - tube_gap[t] - TUBE_GAPHEIGHT) + offset,
            tube_pos[t] + offset, y, TUBE_WIDTH - offset + trim, 1, 8, GAME_PALETTE, 0
        );
    /* down */
    for (int y = tube_gap[t] + TUBE_GAPHEIGHT + TUBE_HEADHEIGHT; y < WIN_H - WIN_B; ++y)
        ksys_draw_bitmap_palette(
            TUBE_BODY_IMAGE + offset,
            tube_pos[t] + offset, y, TUBE_WIDTH - offset + trim, 1, 8, GAME_PALETTE, 0
        );
}

/* ===== Functions - Helpers ===== */

static void WriteBorderedText(int x, int y, const char* textPtr, int textLen, int fontType) {
    for (int bx = x + 2; bx >= x - 2; --bx) {
        for (int by = y + 2; by >= y - 2; --by) {
            _ksys_draw_text(textPtr, bx, by, textLen, fontType);
        }
    }
    _ksys_draw_text(textPtr, x, y, textLen, fontType | 0x00FFFFFF);
}

/* ===== Functions - Game Logic ===== */

static inline int checkCollision(int t) {
    return ((tube_pos[t] <= (BIRD_X + BIRD_W) && tube_pos[t] + TUBE_WIDTH >= BIRD_X)
        && (bird_pos <= tube_gap[t] || bird_pos + BIRD_H >= tube_gap[t] + TUBE_GAPHEIGHT)) ? 1 : 0;
}

static inline int checkAddScore(int t) {
    int r = tube_pos[t] + TUBE_WIDTH;
    return (r + 2 >= BIRD_X) & (r < BIRD_X);
}

static void updateScoreString(void) {
    for (int i = 9; i >= 6; --i) {
        if (++game_score[i] <= '9') return;
        game_score[i] = '0';
    }
}

/* ===== Functions - Game Drawing ===== */

static inline void createGameWindow(void) {
    _ksys_create_window(WIN_X, WIN_Y, WIN_W, WIN_H + WIN_T, HEADER_STRING, GAME_PALETTE[0], 0x34);
}

static void redrawGameWindow(void) {
    /* cleaning the score area */

    int move = bird_acc / 10;
    if (move < 0)
        move = -move;
    _ksys_draw_bar(BIRD_X, bird_pos - move, BIRD_W, move * 2 + BIRD_H, GAME_PALETTE[0]);

    ksys_draw_bitmap_palette(BIRD_IMAGE, BIRD_X, bird_pos, BIRD_W, BIRD_H, 8, GAME_PALETTE, 0);

    Tube_draw(0); Tube_draw(1); Tube_draw(2);

    WriteBorderedText(10, 10, game_score, 0, 0x81000000);
}

static void drawGameOverWindow(void) {
    createGameWindow();

    WriteBorderedText(116, 100, GAMEOVER_STRING, 0, 0x82000000);
    WriteBorderedText(136, 157,      game_score, 0, 0x81000000);
    WriteBorderedText( 40, 207,  SPA_KEY_STRING, 0, 0x81000000);
    WriteBorderedText( 70, 241,  ESC_KEY_STRING, 0, 0x81000000);
}

static void drawMenuWindow(void) {
    createGameWindow();

    WriteBorderedText( 88, 34, HEADER_STRING,     6, 0x04000000);
    WriteBorderedText(188, 87, HEADER_STRING + 7, 4, 0x04000000);

    char* pos = &TUBE_HEAD_IMAGE[0];
    for (int x = 50 - 1; x >= 50 - TUBE_HEADHEIGHT; --x) {
        for (int y = 156; y < 156 + TUBE_WIDTH; ++y) {
            ksys_draw_bitmap_palette(pos, x, y     , 1, 1, 8, GAME_PALETTE, 0);
            ksys_draw_bitmap_palette(pos, x, y + 82, 1, 1, 8, GAME_PALETTE, 0);
            ++pos;
        }
    }

    for (int x = 50; x < WIN_W - WIN_S; ++x)
    {
        ksys_draw_bitmap_palette(TUBE_BODY_IMAGE, x, 156, 1, TUBE_WIDTH, 8, GAME_PALETTE, 0);
        ksys_draw_bitmap_palette(TUBE_BODY_IMAGE, x, 238, 1, TUBE_WIDTH, 8, GAME_PALETTE, 0);
    }

    WriteBorderedText(139, 171,    FAST_STRING, 0, 0x82000000);
    WriteBorderedText(139, 253,    SLOW_STRING, 0, 0x82000000);

    // Control hint
    WriteBorderedText(100, 322, CONTROL_STRING, 0, 0x81000000);
}

/* ===== Functions - Game Main ===== */

static void startGame(void) {
    createGameWindow();

    game_score[7] = game_score[8] = game_score[9] = '0';

    Bird_initialize();

    const int spacing = (WIN_W + TUBE_WIDTH) / 3;
    for (int i = tube_num - 1; i >= 0; --i) {
        Tube_randomize(i);
        tube_pos[i] = WIN_W + i * spacing;
    }

    game_state = GAMESTATE_STARTED;
}

static void frameGame(void) {
    Bird_move();

    /* Processing all tubes */
    for (int i = tube_num - 1; i >= 0; --i) {
        /* Adding score */
        if (checkAddScore(i)) {
           updateScoreString();
           _ksys_draw_bar(92, 8, 38, 18, GAME_PALETTE[0]);
        }

        /* Check collision with bird */
        if (checkCollision(i))
            goto game_over;

        /* Move tube */
        Tube_move(i);
    }

    /* Checking bird is too high or low */
    if (bird_pos + BIRD_H > WIN_H - WIN_B || bird_pos < 0)
        goto game_over;

    redrawGameWindow();
    return;

    game_over:
    game_state = GAMESTATE_GAMEOVER;
    drawGameOverWindow();
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

    game_state = GAMESTATE_MENU;

    for (;;) {
        _ksys_delay(game_speed);

        switch (_ksys_check_event()) {
            case KSYS_EVENT_NONE:
                if (game_state == GAMESTATE_STARTED)
                    frameGame();
                break;

            case KSYS_EVENT_REDRAW: {
                switch (game_state) {
                    case GAMESTATE_MENU:     drawMenuWindow();     break;
                    case GAMESTATE_STARTED:  redrawGameWindow();   break;
                    case GAMESTATE_GAMEOVER: drawGameOverWindow(); break;
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
                            startGame();
                        }
                        break;
                    }
                    case GAMESTATE_STARTED: {
                        if (k.code == 32) // SPACE
                            Bird_jump();
                        break;
                    }
                    case GAMESTATE_GAMEOVER: {
                        if (k.code == 32) // SPACE
                            startGame();
                        if (k.code == 27) // ESCAPE
                        {
                            game_state = GAMESTATE_MENU;
                            drawMenuWindow();
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

    return 0;
}
