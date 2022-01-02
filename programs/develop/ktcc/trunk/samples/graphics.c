#include <kos32sys1.h>
#include <stdlib.h>
#include <string.h>
#include <clayer/gb.h>

/// ===========================================================

int main()
{
    GB_BMP b;
    unsigned event;

    b.w = 300;
    b.h = 200;
    b.bmp = malloc (300*200*3);

    gb_bar (&b, 4, 8, 4, 12, 0xff0000); // red
    gb_bar (&b, 10, 8, 4, 12, 0x00ff00); // green
    gb_bar (&b, 16, 8, 4, 12, 0x0000ff); // blue

    gb_line(&b, 4, 30, 50, 30, 0xffffff); // white line
    gb_line(&b, 55, 4, 120, 60, 0xf0f033); // another line

    gb_rect(&b, 65, 24, 100, 60, 0x2065ff); // rectangle

    gb_circle(&b, 55, 95, 40, 0x20ff20); // circle

    for (;;)
        {
        event = get_os_event();
        switch (event)
                {
                case 1:
                        begin_draw();
                        sys_create_window(50, 50, 310, 230, "testlibgb" ,0x34f0f0f0, 0x14);
                        draw_bitmap(b.bmp, 5, 25, 300, 200);
                        end_draw();
                        break;
                case 2:
                        get_key();
                        break;

                case 3:
                        if (1==get_os_button())
                        {
                         exit(0);
                        }
                        break;
                };
        }
    exit(0);
}
/// ===========================================================
