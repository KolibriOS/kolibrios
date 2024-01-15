#include <sys/ksys.h>

/*
Calculate and draw FPS
x,y - window relative coordinates to draw FPS info

Returns the number of centiseconds (cs) spent in one drawing cycle
*/

int time1=0;
int time2=0;
int fps1=0;
int timerend=0;

int Fps (long x, long y)
{
    int tr;

    time1 = _ksys_get_tick_count(); // time in cs since boot

    if (timerend==0)
    {
       time2 = time1;
       timerend = time1;
    }

    tr = time1 - timerend;

    if ((time1 - time2) < 100)  // if passed less than one second
    {
        fps1++;
    }
    else
    {
        // draw FPS info
        _ksys_draw_bar(x, y, 23, 7, 0x00555555);
        _ksys_draw_number(fps1, x, y, 4, 0xfafafa);
        fps1 = 0;
        time2 = time1;
    }

    timerend = time1;

    return tr;
}
