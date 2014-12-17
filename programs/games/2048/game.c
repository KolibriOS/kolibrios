#include "game.h"

struct {
    rect    reset_button;   // new game button place
    rect    highscore_rect; // highscore place
    rect    score_rect;     // score place
    rect    over_rect;      // game over window
    __u8    over;           // flag for game over
} game;

void game_draw_ui()
{
    __menuet__make_button(game.reset_button.x,
                          game.reset_button.y,
                          game.reset_button.width,
                          game.reset_button.height,
                          NEW_GAME_BUTTON,
                          BOARD_BG_COLOR);
    rect_draw_text(&game.reset_button,"Restart",7,GAME_BG_COLOR,0);

    rect_draw(&game.highscore_rect,BOARD_BG_COLOR);
    rect_draw_value(&game.highscore_rect,board_highscore(),GAME_BG_COLOR,0);

    rect_draw(&game.score_rect,BOARD_BG_COLOR);
    rect_draw_value(&game.score_rect,board_score(),GAME_BG_COLOR,0);

    if (game.over)
    {
        __u16 line_step = FONT_HEIGHT * 2;
        rect_draw(&game.over_rect,BOARD_BG_COLOR);

        rect line_rect = {
            .x = game.over_rect.x,
            .y = game.over_rect.y + line_step,
            .width = game.over_rect.width,
            .height = line_step
        };

        rect_draw_text(&line_rect,"It looks like there is",22,0xFFFFFF,0);

        line_rect.y += line_step;
        rect_draw_text(&line_rect,"no more moves",13,0xFFFFFF,0);

        line_rect.y += line_step;
        rect_draw_text(&line_rect,"available",9,0xFFFFFF,0);
    }
}

void game_init()
{
    game.over = false;
    // place window at the center of screen
    __u16 screen_w = 0;
    __u16 screen_h = 0;
    __menuet__get_screen_max(&screen_w,&screen_h);

    __menuet__window_redraw(1);

    __menuet__define_window((screen_w - WND_WIDTH) / 2,
                            (screen_h - WND_HEIGHT) / 2,
                            WND_WIDTH,
                            WND_HEIGHT,
                            0x34 << 24 | GAME_BG_COLOR,
                            0,
                            (__u32)header);

    // find info about window client area
    __menuet__get_process_table(&proc_info,PID_WHOAMI);

    // calc board
    rect av_area = {0};
    av_area.x = GAME_BORDER;
    av_area.y = (SCORE_HEIGHT > GAME_BORDER) ? SCORE_HEIGHT : GAME_BORDER;
    av_area.width = proc_info.client_width - av_area.x * 2;
    av_area.height = proc_info.client_height - av_area.y - GAME_BORDER;
    // minimal square
    if (av_area.width < av_area.height)
    {
        av_area.y += (av_area.height - av_area.width) / 2;
        av_area.height = av_area.width;
    }
    else // if (av_area.height < av_area.width)
    {
        av_area.x += (av_area.width - av_area.height) / 2;
        av_area.width = av_area.height;
    }

    board_init(&av_area);

    rect top_base = {
        .x = av_area.x,
        .y = (av_area.y - SCORE_HEIGHT) / 2,
        .width = (av_area.width - BOARD_SPACING * 2) / 3,
        .height = SCORE_HEIGHT
    };

    game.reset_button = top_base;

    top_base.x += top_base.width + BOARD_SPACING;
    game.highscore_rect = top_base;

    top_base.x += top_base.width + BOARD_SPACING;
    game.score_rect = top_base;

    av_area.x += av_area.width / 4;
    av_area.y += av_area.height / 4;
    av_area.width /= 2;
    av_area.height /= 2;

    game.over_rect = av_area;

    game_draw_ui();

    __menuet__window_redraw(2);
}

void game_exit()
{
    board_delete();
}

void game_redraw()
{
    __menuet__get_process_table(&proc_info,PID_WHOAMI);

    // start redraw
    __menuet__window_redraw(1);

    __menuet__define_window(0,                          // __u16 x1     : ignored
                            0,                          // __u16 y1     : ignored
                            0,                          // __u16 xsize  : ignored
                            0,                          // __u16 ysize  : ignored
                            0x34 << 24 | GAME_BG_COLOR, // __u32 body_color
                            0,                          // __u32 grab_color
                            (__u32)header);             // __u32 frame_color or header

    game_draw_ui();
    board_redraw();

    // end redraw
    __menuet__window_redraw(2);
}

void game_move_up()
{
    if (board_up())
    {
        board_redraw();
        __u8 added = board_add_random_tile();
        board_redraw();

        game.over = !added || !board_has_moves();
        game_draw_ui();
    }
}

void game_move_down()
{
    if (board_down())
    {
        board_redraw();
        __u8 added = board_add_random_tile();
        board_redraw();

        game.over = !added || !board_has_moves();
        game_draw_ui();
    }
}

void game_move_left()
{
    if (board_left())
    {
        board_redraw();
        __u8 added = board_add_random_tile();
        board_redraw();

        game.over = !added || !board_has_moves();
        game_draw_ui();
    }
}

void game_move_right()
{
    if (board_right())
    {
        board_redraw();
        __u8 added = board_add_random_tile();
        board_redraw();

        game.over = !added || !board_has_moves();
        game_draw_ui();
    }
}
