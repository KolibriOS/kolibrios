#include<menuet/os.h>

#define B_SZ	10

static char * Title="BinClock";

void draw_small_box(int x,int y,int is_on)
{
 __menuet__bar(x,y,B_SZ,B_SZ,is_on ? 0xFF0000 : 0x103000);
}

void draw_box_group(int x,int y,int num)
{
 int i,j;
 char buf[2];
 buf[0]=(num&(1+2+4+8))+'0';
 buf[1]='\0';
 for(i=0;i<4;i++)
 { 
  j=(B_SZ+2)*i;
  draw_small_box(x,y+((B_SZ+2)*i),num & (1<<(3-i)) ? 1 : 0);
 }
 __menuet__bar(x,y+((B_SZ+2)*4),B_SZ,B_SZ,0x800000);
 __menuet__write_text(x+2,y+((B_SZ+2)*4)+3,0xFFFFFF,buf,1);
}

void draw_bcd_num(int x,int y,int num)
{
 int v1,v2;
 v1=(num>>4)&(1+2+4+8);
 v2=num & (1+2+4+8);
 draw_box_group(x,y,v1);
 draw_box_group(x+B_SZ+2,y,v2);
}

void draw_hms(int x,int y)
{
 __u32 t;
 int h,m,s;
 t=__menuet__getsystemclock();
 s=(t & 0x00FF0000)>>16;
 m=(t & 0x0000FF00)>>8;
 h=(t & 0x000000FF);
 draw_bcd_num(x,y,h);
 x+=((B_SZ+2)<<1)+2;
 draw_bcd_num(x,y,m);
 x+=((B_SZ+2)<<1)+2;
 draw_bcd_num(x,y,s);
}

void draw_h(void)
{
 draw_hms(22,28);
}

void paint(void)
{
 __menuet__window_redraw(1);
 __menuet__define_window(100,100,40+((B_SZ+2)*6)+4,30+((B_SZ+2)*4)+16,0x03000080,0x800000FF,0x000080);
 __menuet__write_text(3,3,0xFFFFFF,Title,strlen(Title));
 __menuet__bar(20,26,((B_SZ+2)*6)+4+2,4+((B_SZ+1)*4)+2,0);
 draw_h();
 __menuet__window_redraw(2);
}

void main(void)
{
 int i;
 paint();
 for(;;)
 {
  __menuet__delay100(20);
  i=__menuet__check_for_event();
  draw_h();
  switch(i)
  {
   case 1:
    paint();
    continue;
   case 2:
    __menuet__getkey();
    continue;
   case 3:
    if(__menuet__get_button_id()==1) __menuet__sys_exit();
    continue;
  }
 }
}
  