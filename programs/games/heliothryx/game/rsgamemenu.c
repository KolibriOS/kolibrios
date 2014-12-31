#include "rsgamemenu.h"

#include "rsgame.h"

#include "rskos.h"

#include "strings.h"

PRSFUNC0 menu_actions[] = {
    /* a */ &menu_action_start,     
    /* b */ &menu_action_exit,
    /* c */ &menu_action_change_window_scale,
    /* d */ &menu_action_resume
};

char window_scale_str[] = "c< 2X >";
char level_passed_score_str[] = " 0000   ";

/*
    First char:
    - letter a...z means action (a = 0th, b = 1st, c = 2nd, see menu_actions[] above)
    - number 0...9 means goto menu #0, #1, #2... see menu_titles[] below
    - space ' ' means no action, menu item is unselectable
    - empty string "" is not allowed and can cause segfault
    String from second char is label of menu item

*/


char* menu_main_titles[] = {
    "a"L_START,
    "1"L_SETTINGS,
    "2"L_ABOUT,
    "b"L_QUIT,
    0
};

char* menu_settings_titles[] = {
    " "L_WINDOW_SCALE,
    window_scale_str,
    " ",
    "0"L_DONE,
    0
};

char* menu_about_titles[] = {
    " "L_DEVELOPED_BY,
    " "L_ROMAN_SHUVALOV,
    " ",
    "0"L_BACK,
    0
};

char* menu_game_over_titles[] = {
    " "L_GAME_OVER,
    " "L_YOUR_SCORE,
    level_passed_score_str,
    " ",
    "0"L_BACK,
    0
};


char* menu_pause_titles[] = {
    " "L_PAUSE,
    " ",
    "d"L_RESUME,
    "0"L_EXIT_TO_MAIN_MENU,
    0
};


char **menu_titles[] = {
    /* 0 */ menu_main_titles,
    /* 1 */ menu_settings_titles,
    /* 2 */ menu_about_titles,
    /* 3 */ menu_game_over_titles,
    /* 4 */ menu_pause_titles,
    0
};


void menu_cursor_down() {
    int new_index = game.menu_item_index+1;
    while ( (menu_titles[game.menu_index][new_index]) ) {
        if ((menu_titles[game.menu_index][new_index][0] != ' ')) {
            game.menu_item_index = new_index;
            game_ding(1);
            return;
        };
        new_index++;
    };
};

void menu_cursor_up() {
    int new_index = game.menu_item_index-1;
    while ( new_index+1 ) {
        if ((menu_titles[game.menu_index][new_index][0] != ' ')) {
            game.menu_item_index = new_index;
            game_ding(1);
            return;
        };
        new_index--;
    };
};

void menu_open(int i) {
    
    if ( ((game.menu_index == MENU_PAUSE) && (i != MENU_PAUSE)) || (i == MENU_GAME_OVER) ) {
        soundbuf_play( &game.sound_music, SND_MODE_LOOP );
    };

    game.menu_index = i;
    
    game.menu_item_index = -1;
    // (menu_cursor_down without sound)
    int new_index = game.menu_item_index+1;
    while ( (menu_titles[game.menu_index][new_index]) ) {
        if ((menu_titles[game.menu_index][new_index][0] != ' ')) {
            game.menu_item_index = new_index;
            return;
        };
        new_index++;
    };
    
};

void menu_cursor_click() {
    
    char c = menu_titles[game.menu_index][game.menu_item_index][0];
    
    game_ding(0);
    
    if (c > '9') {
        // action: call function
        menu_actions[c - 'a']();
    }
    else {
        // action: navigate to menu
        menu_open(c - '0');
    };
    
//    DEBUG10f("click: %c \n", c);
    
};

void menu_action_start() {
    game.status = STATUS_PLAYING;
    
    game.player_x = GAME_WIDTH/2 - 50;
    game.player_y = GAME_HEIGHT/2 - 10;
    
    game.stage = 0;
    game.stage_timer = 0;
    
    game.health = GAME_HEALTH_MAX;
    game.ammo = GAME_AMMO_MAX;
    
    game.shoot_delay = 0;
    game.shoot_keypressed = 0;
    game.shoot_restore_delay = 0;
    
    game.score = 0;
    game.flags = 0;
    
    game.stage_level = 0;
    
    game.objs_count = 0;
    
    game.bg_color = COLOR_BLACK;
    
    soundbuf_stop( &game.sound_music );
    
};

void menu_action_exit() {
    #ifdef RS_KOS
        GameTerm();
    #endif
    rskos_exit();
};

void menu_action_change_window_scale() {
    game_change_window_scale(1);
};

void menu_action_resume() {
    
    game.status = STATUS_PLAYING;
    
};
