#include<menuet/os.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<jpeglib.h>
#include"libmgfx.h"

static char * Title="MenuetMultiview";

mgfx_image_t * img;

void paint(void)
{
 __menuet__window_redraw(1);
 __menuet__define_window(100,100,400,300,0x03000080,0x800000FF,0x000080);
 __menuet__write_text(3,3,0xFFFFFF,Title,strlen(Title));
 if(img)
  paint_image(10,30,img);
 __menuet__window_redraw(2);
}

int event_loop(void)
{
 int i;
 i=__menuet__wait_for_event();
 switch(i)
 {
  case 1:
   paint(); return 0;
  case 2:
   return __menuet__getkey();
  case 3:
   if(__menuet__get_button_id()==1) exit(0); return 0;
 }
}

void main(void)
{
 img=NULL;
 init_mgfx_library();
 paint();
 load_image("/rd/1/test.jpg",&img);
 paint_image(10,30,img);
 for(;;) event_loop(); 
}
