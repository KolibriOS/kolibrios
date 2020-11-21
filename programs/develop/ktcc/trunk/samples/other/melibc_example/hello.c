#include <menuet/os.h>
 
char title[] = "Menuetlibc in TinyC";
char string[] = "Text";
 
void draw_window(void)
{
    // start redraw
    __menuet__window_redraw(1);
    // define&draw window
    __menuet__define_window(10,40,300,150,0x33FFFFFF,0,(__u32)title);
	// display string
	__menuet__write_text(30,10,0x80000000,string,0);
	// end redraw
	__menuet__window_redraw(2);
}

int main()
{
    draw_window();
    for (;;)
    {
        switch (__menuet__wait_for_event())
        {
			case 1:
				draw_window();
				break;
			case 3:
				return;
        }
    }
}
