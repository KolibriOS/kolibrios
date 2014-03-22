#ifndef __MGTK_TERMINAL_H
#define __MGTK_TERMINAL_H

#include<mgtk/widget.h>
#include<mgtk/pen.h>

class GTerminal: public GWidget
{
public:
    GTerminal(GRect * r,int cursor_vis,unsigned long TextColor=0x00FF00,
	unsigned long BackgrColor=0);
    virtual ~GTerminal();
    virtual void DrawWidget();
    virtual void Putch(char);
    virtual void ClrScr();
    virtual void Idle();
    virtual void Scroll();
    inline void Puts(char * s)
    {
     for(;*s;s++) Putch(*s);
    }
private:
    int cx,cy;
    char * screen_buf;
    int xchars,ychars;
    unsigned long _tcolor,_bcolor;
    void LocalUpdate();
};

#endif
