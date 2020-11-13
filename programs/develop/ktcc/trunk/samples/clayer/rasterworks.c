#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <kos32sys1.h>
#include <clayer/rasterworks.h>

// Sizes
int x_size = 768, y_size = 256;

// Out example string
char* string = "Пример/Example";

int main()
{
    // Count length
    int ln_str = countUTF8Z(string, -1);
    
    // Create image buffer
    void *buffi = malloc(x_size * y_size * 3 * sizeof(char) + 8);
    
    // Set sizes
    *((int*)buffi) = x_size;
    *((int*)buffi+1) = y_size;
    
    // Fill color
    memset((char*)buffi + 8, 0xFF, x_size * y_size * 3);
    
    // Draw text on buffer
    drawText(buffi, 5, 0, string, ln_str, 0xFF000000, 0x30C18);
    drawText(buffi, 5, 32, string, ln_str, 0xFF000000, 0x1030C18);
    drawText(buffi, 5, 64, string, ln_str, 0xFF000000, 0x2030C18);
    drawText(buffi, 5, 96, string, ln_str, 0xFF000000, 0x4030C18);
    drawText(buffi, 5, 128, string, ln_str, 0xFF000000, 0x8030C18);
    drawText(buffi, 5, 160, string, ln_str, 0xFF000000, 0x0F031428);
    
    while (1) 
    {
		switch (get_os_event())
		{
			case 1:
				BeginDraw();
				DrawWindow(50, 50, 800, 300, "Rasterworks Example", 0x999999, 0x34);
				DrawBitmap(buffi + 8, 10, 10, 768, 256);
				EndDraw();
				break;
			case 2:
				get_key();
				break;
			case 3:
				if (get_os_button() == 1) exit(0);
				break;
		};
     }
     return 0;
}
