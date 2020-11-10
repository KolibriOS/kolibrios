#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <kos32sys1.h>
#include <clayer/rasterworks.h>

int main()
{
    unsigned int event;
    int ln_str = countUTF8Z("Example", -1);
    void *buffi = malloc(768*256*3 * sizeof(char));
    *((int*)buffi) = 768;
    *((int*)buffi+1) = 256;
    memset((char*)buffi+8, (char)-1, 768*256*3);
    drawText(buffi, 0, 0, "Example", ln_str, 0xFF000000, 0x30C18);
    drawText(buffi, 0, 32, "Example", ln_str, 0xFF000000, 0x1030C18);
    drawText(buffi, 0, 64, "Example", ln_str, 0xFF000000, 0x2030C18);
    drawText(buffi, 0, 96, "Example", ln_str, 0xFF000000, 0x4030C18);
    drawText(buffi, 0, 128, "Example", ln_str, 0xFF000000, 0x8030C18);
    drawText(buffi, 0, 160, "Example", ln_str, 0xFF000000, 0x0F031428);
    for (;;){
            event = get_os_event();
            switch (event)
            {
                case 1:
                    begin_draw();
                    sys_create_window(50, 50, 800, 300, "rasterworks example" ,0x34f0f0f0, 0x14);
                    draw_bitmap(buffi+8, 5, 25, 768, 256);
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
