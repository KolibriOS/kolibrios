#include "rsgamemenu.h"

#include "rsgame.h"

#include "rskos.h"

PRSFUNC0 menu_actions[] = {
    /* a */ &menu_action_start,     
    /* b */ &menu_action_exit,
    /* c */ &menu_action_change_window_scale
};

char window_scale_str[] = "c< 2X >";

char* menu_main_titles[] = {
    "a5TART",
    "15ETTING5",
    "2ABOUT",
    "bQUIT",
    0
};

char* menu_settings_titles[] = {
    " WINDOW SCALE:",
    window_scale_str,
    " ",
    "0DONE",
    0
};

char* menu_about_titles[] = {
    " DEVELOPED BY",
    " ROMAN SHUVALOV",
    " ",
    "0DONE",
    0
};

char **menu_titles[] = {
    /* 0 */ menu_main_titles,
    /* 1 */ menu_settings_titles,
    /* 2 */ menu_about_titles,
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
    
    game.tx = GAME_WIDTH/2 - 50;
    game.ty = GAME_HEIGHT/2 - 10;
    
};

void menu_action_exit() {
    #ifndef RS_LINUX
        GameTerm();
    #endif
    rskos_exit();
};

void menu_action_change_window_scale() {
    game_change_window_scale(1);
};
