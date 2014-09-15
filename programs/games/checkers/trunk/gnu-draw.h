#include "gr-draw.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/keysym.h>
#include <stdio.h>

#ifndef _GNU_GRAPHIC_DRAW_H
#define _GNU_GRAPHIC_DRAW_H

class TGnuGraphDraw : public TBaseGraphDraw<TGnuGraphDraw>
{
  typedef TBaseGraphDraw<TGnuGraphDraw> TGraphDraw;
public:
  TGnuGraphDraw(const char *s = 0);
  ~TGnuGraphDraw();

  static unsigned long GetKeySym(unsigned int key) {return key;}

  unsigned long GetBlackColor();
  unsigned long GetWhiteColor();
  unsigned long CreateColor(unsigned short red,
                            unsigned short green, unsigned short blue);
  void FreeColor(unsigned long c);
  unsigned int GetBgColor() const {return bgcolor;}
  void SetBgColor(unsigned long c) {bgcolor = c;}

  int GetStatus() {return disp ? (win ? 1 : 0) : (-1);}
  int Init() {return disp ? 0 : -1;}
  int Run(int evmask = 0, int w = INT_MIN, int h = INT_MIN);

  void GetSize(int &w, int &h);
  int OpenDraw();
  int IsDraw() {return disp && win && prGC;}
  void CloseDraw() {if (prGC) {XFreeGC(disp, prGC); prGC = 0;}}

  int SetColor(unsigned long c);
  int DrawLine(int x0, int y0, int x1, int y1);
  int DrawText(int x0, int y0, char *text);
  int DrawClear();
  int GetTextH(const char *s) {return 10;}
  int GetTextW(const char *s) {return 6 * strlen(s);}
  void Quit(int q = 1) {quit = (q > 0) ? q : 0;}
  void ResReinit(int w = INT_MIN, int h = INT_MIN);
protected:
  void InitDisplay();
  void InitWindow(int w = INT_MIN, int h = INT_MIN);
protected:
  int quit;
  unsigned long bgcolor;
  Display *disp;
  Window win;
  int ScrNum;
  GC prGC;
  Mask xmask;
};

TGnuGraphDraw::TGnuGraphDraw(const char *s /*= 0*/) : TGraphDraw(s), prGC(0)
{
  xmask = 0;
  InitDisplay();
}

TGnuGraphDraw::~TGnuGraphDraw()
{
  CloseDraw();
  if (disp) XCloseDisplay(disp);
}

void TGnuGraphDraw::InitDisplay()
{
  disp = XOpenDisplay(NULL);
  if (disp) ScrNum = DefaultScreen(disp);
  else
  {
    int L = 100;
    if (title) L += strlen(title);
    char *str = new char[L];
    if (str)
    {
      strcpy(str, "\n");
      if (title && title[0]) {strcat(str, title); strcat(str, ": ");}
      strcat(str, "Can't connect to X Server.\n");
      printf("%s", str);
    }
  }
  bgcolor = GetWhiteColor();
}

unsigned long TGnuGraphDraw::GetBlackColor()
{
  if (disp) return BlackPixel(disp, ScrNum);
  else return TGraphDraw::GetBlackColor();
}

unsigned long TGnuGraphDraw::GetWhiteColor()
{
  if (disp) return WhitePixel(disp, ScrNum);
  else return TGraphDraw::GetWhiteColor();
}

unsigned long TGnuGraphDraw::CreateColor(unsigned short red,
                            unsigned short green, unsigned short blue)
{
  if (disp)
  {
    XColor color;
    color.red = red; color.green = green; color.blue = blue;
    if (XAllocColor(disp, DefaultColormap(disp, ScrNum), &color))
    {
      return color.pixel;
    }
    else return BlackPixel(disp, ScrNum);
  }
  else return TGraphDraw::CreateColor(red, green, blue);
}

void TGnuGraphDraw::FreeColor(unsigned long c)
{
  if (disp && c != BlackPixel(disp, ScrNum))
  {
    XFreeColors(disp, DefaultColormap(disp, ScrNum), &c, 1, 0);
  }
}

void TGnuGraphDraw::GetSize(int &w, int &h)
{
  if (disp)
  {
    int x, y;
    Window w_ret;
    unsigned int br, dr, width = 0, height = 0;
    XGetGeometry(disp, win, &w_ret, &x, &y, &width, &height, &br, &dr);
    w = width; h = height;
  }
  else TGraphDraw::GetSize(w, h);
}

int TGnuGraphDraw::OpenDraw()
{
  if (!disp && !win) return 0;
  if (!prGC) prGC = XCreateGC(disp, win, 0, NULL);
  return 1;
}

int TGnuGraphDraw::SetColor(unsigned long c)
{
  if (!disp || !prGC) return 0;
  XSetForeground(disp, prGC, c);
  return 1;
}

int TGnuGraphDraw::DrawLine(int x0, int y0, int x1, int y1)
{
  if (!disp || !prGC) return 0;
  XDrawLine(disp, win, prGC, x0, y0, x1, y1);
  return 1;
}

int TGnuGraphDraw::DrawText(int x0, int y0, char *text)
{
  if (!disp || !prGC) return 0;
  XDrawString(disp, win, prGC, x0, y0 + GetTextH(text), text, strlen(text));
  return 1;
}

int TGnuGraphDraw::DrawClear()
{
  if (!disp || !prGC) return 0;
  XClearWindow(disp, win);
  return 1;
}

void TGnuGraphDraw::InitWindow(int w, int h)
{
  if (w <= 0 || h <= 0) TGraphDraw::GetSize(w, h);
  win = XCreateSimpleWindow(disp, RootWindow(disp, ScrNum),
            100, 100, w, h, 2, BlackPixel(disp, ScrNum), GetBgColor());
  XSelectInput(disp, win, xmask);
  XMapWindow(disp, win);
}

void TGnuGraphDraw::ResReinit(int w, int h)
{
  if (disp) {free(disp); disp = 0;}
  InitDisplay();
  if (!disp) exit(1);
  else if (win) InitWindow(w, h);
}

int TGnuGraphDraw::Run(int evmask, int w, int h)
{
  if (!disp) return -1;
  if (!evfunc) return -2;
  xmask = ExposureMask;
  if (evmask & button_down_mask) xmask |= ButtonPressMask;
  if (evmask & button_up_mask) xmask |= ButtonReleaseMask;
  if (evmask & key_down_mask) xmask |= KeyPressMask;
  if (evmask & key_up_mask) xmask |= KeyReleaseMask;
  if (evmask & mouse_move_mask) xmask |= PointerMotionMask;
  if (evmask & mouse_drag_mask)
  {
    xmask |= ButtonMotionMask | Button1MotionMask | Button2MotionMask |
             Button3MotionMask | Button4MotionMask | Button5MotionMask;
  }
  InitWindow(w, h);
  XEvent xevent;
  KeySym sym;
  char str[50];
  event ev;
  int stpr = 0;
  quit = 0;
  while (quit == 0)
  {
    if (stpr == 0)
    {
      ev.type = event::start;
      ev.any.drw = this;
      evfunc(ev);
      CloseDraw();
      stpr = 1;
    }
    else
    {
      XNextEvent(disp, &xevent);
      switch(xevent.type)
      {
      case Expose:
        if (xevent.xexpose.count != 0) break;
        ev.type = event::draw;
        ev.any.drw = this;
        evfunc(ev);
        CloseDraw();
        break;
      case ButtonPress:
        ev.type = event::button_down;
        ev.button.drw = this;
        ev.button.x = xevent.xbutton.x;
        ev.button.y = xevent.xbutton.y;
        ev.button.n = xevent.xbutton.button;
        evfunc(ev);
        CloseDraw();
        break;
      case ButtonRelease:
        ev.type = event::button_up;
        ev.button.drw = this;
        ev.button.x = xevent.xbutton.x;
        ev.button.y = xevent.xbutton.y;
        ev.button.n = xevent.xbutton.button;
        evfunc(ev);
        CloseDraw();
        break;
      case MotionNotify:
        ev.type = event::mouse_move;
        ev.button.drw = this;
        ev.button.x = xevent.xbutton.x;
        ev.button.y = xevent.xbutton.y;
        ev.button.n = xevent.xbutton.button;
        evfunc(ev);
        CloseDraw();
        break;
      case KeyPress:
        memset(str, 0, 20);
        XLookupString(&xevent.xkey, str, 20, &sym, 0);
        ev.type = event::key_down;
        ev.key.drw = this;
        ev.key.k = (unsigned long)sym;
        evfunc(ev);
        CloseDraw();
        break;
      case KeyRelease:
        memset(str, 0, 20);
        XLookupString(&xevent.xkey, str, 20, &sym, 0);
        ev.type = event::key_up;
        ev.key.drw = this;
        ev.key.k = (unsigned long)sym;
        evfunc(ev);
        CloseDraw();
        break;
      }
    }
    if (quit == 1)
    {
      ev.type = event::close;
      ev.any.drw = this;
      Quit(evfunc(ev));
      CloseDraw();
    }
  }
  CloseDraw();
  XDestroyWindow(disp, win);
  win = 0;
  return quit;
}

#endif  //_GNU_GRAPHIC_DRAW_H
