#include "game.h"

#define KEY_RELEASED 0x80

void main()
{
    enable_scancode();
    game_init();
    for(;;)
    {
        int ev = __menuet__wait_for_event();
        switch (ev)
        {
        case 1 :    // EVENT_REDRAW:
            game_redraw();
            break;
        case 2 :    // EVENT_KEY:
        {
            ev = __menuet__getkey() & 0xFF;
            switch (ev)
            {
            case ((K_Up     & 0xFF) | KEY_RELEASED) :   // key Up released
                game_move_up();
                clear_key_buffer();
                break;
            case ((K_Down   & 0xFF) | KEY_RELEASED) :   // key Down released
                game_move_down();
                clear_key_buffer();
                break;
            case ((K_Left   & 0xFF) | KEY_RELEASED) :   // key Left released
                game_move_left();
                clear_key_buffer();
                break;
            case ((K_Right  & 0xFF) | KEY_RELEASED) :   // key Right released
                game_move_right();
                clear_key_buffer();
                break;
            }

            break;
        }
        case 3 :    // EVENT_BUTTON
            ev = __menuet__get_button_id();
            switch (ev)
            {
            case NEW_GAME_BUTTON :
                game_init();
                break;
            default : // close
                game_exit();
                return;
            }
        }
    }
}
