#ifndef __MGTK_TEXT_H
#define __MGTK_TEXT_H

#include<mgtk/widget.h>

#define BIG_FONT_XSZ		7
#define BIG_FONT_YSZ		9
#define SMALL_FONT_XSZ		5
#define SMALL_FONT_YSZ		7

class GStaticText: public GWidget
{
public:
    GStaticText(GRect *,char *,unsigned long);
    virtual ~GStaticText();
    virtual void DrawWidget();
private:
    unsigned long color;
    char * text;
    int tlen;
};

class GInputLine: public GWidget
{
public:
    GInputLine(GRect *,char *);
    virtual ~GInputLine();
    virtual void DrawWidget();
    virtual void HandleEvent(GEvent * ev);
    virtual void Idle();
    char * GetData();
private:
    int cur_pos,max_len;
    char * text_buffer;
    bool vis;
    int blink_cnt;
};

class GLine: public GWidget
{
public:
    GLine(GRect *,int is_vertical);
    virtual ~GLine();
    virtual void DrawWidget();
private:
    int vert;
};

#endif
