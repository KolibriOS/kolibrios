
// simple sample by Ghost

#include <stdio.h>
#include <string.h>
#include <kolibrisys.h>

#define FONT0           0
#define FONT1           0x10000000

#define BT_NORMAL       0
#define BT_DEL          0x80000000
#define BT_HIDE         0x40000000
#define BT_NOFRAME      0x20000000

char header[]={" -= C demo programm. Compiled whith KTCC halyavin and andrew_programmer port =-   "};

void rotate_str(char *str){
        char tmp;
        int i;
        tmp = str[0];
        for(i = 1; str[i]; i++)str[i - 1] = str[i];
        str[i - 1] = tmp;
}

void draw_window(){
        static int offs = 0;
        static int fcolor = 0;
        static int col = 0;

        _ksys_window_redraw(1);
        _ksys_draw_window(100, 100, 300, 120, 0xaabbcc, 2, 0x5080d0, 0, 0x5080d0);
        _ksys_write_text(6 - offs, 8, fcolor | FONT0, header, strlen(header));
        _ksys_draw_bar(1, 6, 5, 13, 0x05080d0);
        _ksys_draw_bar(274, 6, 26, 13, 0x05080d0);
        _ksys_make_button(300 - 19, 5, 12, 12, 1 | BT_NORMAL, 0x6688dd);
        _ksys_window_redraw(2);

        offs = (offs + 1) % 6;
        if(!offs)rotate_str(header);

        fcolor += (col)?-0x80808:0x80808;
        if(fcolor > 0xf80000 || fcolor == 0)col = !col;
}

int main(int argc, char **argv){

        while(!0){
                switch(_ksys_wait_for_event(10)){
                case 2:return 0;

                case 3:
                        if(_ksys_get_button_id() == 1)return 0;
                        break;

                default:
                        draw_window();
                        break;
                }
        }
}
