#include <clayer/rasterworks.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ksys.h>

// Sizes
int x_size = 768, y_size = 256;

// Out example string
char* string = "Пример/Example";

int main()
{
    // Count length
    int ln_str = countUTF8Z(string, -1);

    // Create image buffer
    void* buffi = malloc(x_size * y_size * 3 * sizeof(char) + 8);

    // Set sizes
    *((int*)buffi) = x_size;
    *((int*)buffi + 1) = y_size;

    // Fill color
    memset((char*)buffi + 8, 0xFF, x_size * y_size * 3);

    // Draw text on buffer
    drawText(buffi, 5, 0, string, ln_str, 0xFF000000, 0x30C18);
    drawText(buffi, 5, 32, string, ln_str, 0xFF000000, 0x1030C18);
    drawText(buffi, 5, 64, string, ln_str, 0xFF000000, 0x2030C18);
    drawText(buffi, 5, 96, string, ln_str, 0xFF000000, 0x4030C18);
    drawText(buffi, 5, 128, string, ln_str, 0xFF000000, 0x8030C18);
    drawText(buffi, 5, 160, string, ln_str, 0xFF000000, 0x0F031428);

    while (1) {
        switch (_ksys_get_event()) {
        case KSYS_EVENT_REDRAW:
            _ksys_start_draw();
            _ksys_create_window(50, 50, 800, 300, "Rasterworks Example", 0x999999, 0x34);
            _ksys_draw_bitmap(buffi + 8, 10, 10, 768, 256);
            _ksys_end_draw();
            break;

        case KSYS_EVENT_BUTTON:
            if (_ksys_get_button() == 1)
                exit(0);
            break;
        };
    }
    return 0;
}
