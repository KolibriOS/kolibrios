#include "rsgamemenu.h"

#include "rsgame.h"

#include "rskos.h"

#include "strings.h"

//PRSFUNC0 menu_actions[] = {
//    /* a */ &menu_action_start,     
//    /* b */ &menu_action_exit,
//    /* c */ &menu_action_change_window_scale
//};

char window_scale_str[] = "c< 2X >";

/*
    First char:
    - letter a...z means action (a = 0th, b = 1st, c = 2nd, see menu_actions[] above)
    - number 0...9 means goto menu #0, #1, #2... see menu_titles[] below
    - space ' ' means no action, menu item is unselectable
    - empty string "" is now allowed and can cause segfault
    String from second char is label of menu item

*/


char* menu_main_titles[] = {
    "a"L_START,
//    "1"L_SETTINGS,
//    "2"L_ABOUT,
//    "b"L_QUIT,
    0
};

char* menu_settings_titles[] = {
//    " "L_WINDOW_SCALE,
//    window_scale_str,
//    " ",
//    "0"L_DONE,
    0
};

char* menu_about_titles[] = {
//    " "L_DEVELOPED_BY,
//    " "L_ROMAN_SHUVALOV,
//    " ",
//    "0"L_DONE,
    0
};

char **menu_titles[] = {
    /* 0 */ menu_main_titles,
    /* 1 */ menu_settings_titles,
    /* 2 */ menu_about_titles,
    0
};


//void menu_cursor_down() {
//    int new_index = game.menu_item_index+1;
//    while ( (menu_titles[game.menu_index][new_index]) ) {
//        if ((menu_titles[game.menu_index][new_index][0] != ' ')) {
//            game.menu_item_index = new_index;
//            game_ding(1);
//            return;
//        };
//        new_index++;
//    };
//};
//
//void menu_cursor_up() {
//    int new_index = game.menu_item_index-1;
//    while ( new_index+1 ) {
//        if ((menu_titles[game.menu_index][new_index][0] != ' ')) {
//            game.menu_item_index = new_index;
//            game_ding(1);
//            return;
//        };
//        new_index--;
//    };
//};

//void menu_open(int i) {
//    
//    game.menu_index = i;
//    
//    game.menu_item_index = -1;
//    // (menu_cursor_down without sound)
//    int new_index = game.menu_item_index+1;
//    while ( (menu_titles[game.menu_index][new_index]) ) {
//        if ((menu_titles[game.menu_index][new_index][0] != ' ')) {
//            game.menu_item_index = new_index;
//            return;
//        };
//        new_index++;
//    };
//    
//};

void menu_cursor_click() {
    
//
    
};

void menu_action_start() {
    game.status = STATUS_PLAYING;
    
//    game.tx = GAME_WIDTH/2 - 50;
//    game.ty = GAME_HEIGHT/2 - 10;
    
};

void menu_action_exit() {
    #ifdef RS_KOS
        GameTerm();
    #endif
    rskos_exit();
};

