#include <sys/ksys.h>

#define B_SZ  10

static char* title = "BinClock";

static void draw_small_box(int x, int y, int is_on)
{
   _ksys_draw_bar(x, y, B_SZ, B_SZ, is_on ? 0xFF0000 : 0x103000);
}

static void draw_box_group(int x, int y, int num)
{
    int i, j;
    char buf[2];
    buf[0] = (num&(1+2+4+8))+'0';
    buf[1] = '\0';
    for(i = 0; i < 4; i++) {
        j = (B_SZ+2)*i;
        draw_small_box(x,y+((B_SZ+2)*i),num & (1<<(3-i)) ? 1 : 0);
    }
    _ksys_draw_bar(x, y+((B_SZ+2)*4),B_SZ,B_SZ, 0x800000);
    _ksys_draw_text(buf, x+2, y+((B_SZ+2)*4)+3, 1, 0xFFFFFF);
}

static void draw_bcd_num(int x,int y,int num)
{
    int v1, v2;
    v1 = (num>>4)&(1+2+4+8);
    v2 = num&(1+2+4+8);
    draw_box_group(x, y, v1);
    draw_box_group(x+B_SZ+2, y ,v2);
}

static void draw_hms(int x,int y)
{
    unsigned t;
    int h, m, s;
    __asm__ __volatile__("int $0x40" : "=a"(t) : "a"(3));
    s = (t & 0x00FF0000)>>16;
    m = (t & 0x0000FF00)>>8;
    h = (t & 0x000000FF);
    draw_bcd_num(x,y,h);
    x+=((B_SZ+2)<<1)+2;
    draw_bcd_num(x,y,m);
    x+=((B_SZ+2)<<1)+2;
    draw_bcd_num(x,y,s);
}

static void draw_h(void)
{
    draw_hms(22, 28);
}

static void paint(void)
{
    _ksys_start_draw();
    _ksys_create_window(100, 100, 40+((B_SZ+2)*6)+4,30+((B_SZ+2)*4)+16, title, 0x80, 0x13);
    _ksys_draw_bar(20, 26, ((B_SZ+2)*6)+4+2,4+((B_SZ+1)*4)+2, 0);
    draw_h();
    _ksys_end_draw();
}

int main(void)
{
    int i;
    paint();

    for(;;) {
        i = _ksys_wait_event_timeout(20);
        draw_h();
        switch(i) {
            case 1:
                paint();
                continue;
            case 2:
                _ksys_get_key();
                continue;
            case 3:
                if (_ksys_get_button() == 1) return 0;
                continue;
        }
    }
}

