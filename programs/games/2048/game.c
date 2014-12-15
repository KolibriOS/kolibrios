#include "game.h"

struct {
    rect    new_game_button;// new game button place
    rect    highscore_rect; // highscore place
    rect    score_rect;     // score place
    __u8    over;           // flag for game over
} game;

void game_draw_top()
{
    if (game.over)
    {
        __menuet__make_button(game.new_game_button.x,
                              game.new_game_button.y,
                              game.new_game_button.width,
                              game.new_game_button.height,
                              NEW_GAME_BUTTON,
                              BOARD_BG_COLOR);
        rect_draw_text(&game.new_game_button,"NEW GAME",8,GAME_BG_COLOR);
    }

    rect_draw(&game.highscore_rect,BOARD_BG_COLOR);
    rect_draw_value(&game.highscore_rect,board_highscore(),GAME_BG_COLOR);

    rect_draw(&game.score_rect,BOARD_BG_COLOR);
    rect_draw_value(&game.score_rect,board_score(),GAME_BG_COLOR);
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
                            GAME_BG_COLOR,
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

    game.new_game_button.x = av_area.x;
    game.new_game_button.y = (av_area.y - SCORE_HEIGHT) / 2;
    game.new_game_button.width = (av_area.width - BOARD_SPACING) / 3;
    game.new_game_button.height = SCORE_HEIGHT;

    game.highscore_rect.x = av_area.x + (av_area.width + BOARD_SPACING) / 3;
    game.highscore_rect.y = (av_area.y - SCORE_HEIGHT) / 2;
    game.highscore_rect.width = (av_area.width - BOARD_SPACING) / 3;
    game.highscore_rect.height = SCORE_HEIGHT;

    game.score_rect.x = av_area.x + (av_area.width + BOARD_SPACING) * 2 / 3;
    game.score_rect.y = (av_area.y - SCORE_HEIGHT) / 2;
    game.score_rect.width = (av_area.width - BOARD_SPACING) / 3;
    game.score_rect.height = SCORE_HEIGHT;

    game_draw_top();

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

    __menuet__define_window(0,              // __u16 x1     : ignored
                            0,              // __u16 y1     : ignored
                            0,              // __u16 xsize  : ignored
                            0,              // __u16 ysize  : ignored
                            GAME_BG_COLOR,  // __u32 body_color
                            0,              // __u32 grab_color
                            (__u32)header); // __u32 frame_color or header

    game_draw_top();
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
        game_draw_top();
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
        game_draw_top();
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
        game_draw_top();
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
        game_draw_top();
    }
}
