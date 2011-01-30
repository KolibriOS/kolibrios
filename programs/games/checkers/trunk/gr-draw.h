#ifndef _GRAPHIC_DRAW_H
#define _GRAPHIC_DRAW_H

#ifndef __MENUET__
#include <limits.h>
#include <string.h>

#ifdef _Windows
# include <windows.h>
#endif
#else
#define LONG_MIN      (-2147483647L-1)  /* minimum signed   long value */
#define INT_MIN LONG_MIN
#endif

class TGraphDraw
{
public:
  union event
  {
    enum evtype {noevent = 0, draw, button_down, button_up,
                 mouse_move, key_down, key_up, start, close} type;

    struct evany
    {
      evtype type;
      TGraphDraw *drw;
    } any;

    struct evbutton : public evany
    {
      int x, y, n;
    } button;

    struct evkey : public evany
    {
      unsigned long k;
    } key;
  };

  enum {button_down_mask = 0x1, button_up_mask = 0x2, key_down_mask = 0x4,
        key_up_mask = 0x8, mouse_move_mask = 0x10, mouse_drag_mask = 0x20};

  enum {ret_setcapture = 0x10};
public:
  TGraphDraw(const char *s = 0) : title(0), about_info(0),
                evfunc(0), id(0), data(0) {CopyTitle(s);}
  ~TGraphDraw() {FreeTitle();}

  virtual unsigned long GetBlackColor() {return 0;}
  virtual unsigned long GetWhiteColor() {return 0xFFFFFFL;}
  virtual unsigned long CreateColor(unsigned short red,
                         unsigned short green, unsigned short blue);
  virtual void FreeColor(unsigned long c) {}
  virtual unsigned long GetBgColor() {return GetWhiteColor();}
  virtual void SetBgColor(unsigned long c) {}

  virtual void SetTitle(const char *s) {CopyTitle(s);}
  const char *GetTitle() const {return title;}

  virtual int GetStatus() {return 0;}  //1 - can draw, 0 - can't draw, <0 - error
  virtual int Init() {return 0;}
  virtual void UnInit() {}
  virtual int Run(int evmask = 0, int w = INT_MIN, int h = INT_MIN) {return -100;}

  virtual void GetSize(int &w, int &h) {w = 200; h = 200;}
  virtual int OpenDraw() {return 0;}
  virtual int IsDraw() {return 0;}
  virtual void CloseDraw() {}

  virtual int SetColor(unsigned long c) {return 0;}
  virtual int DrawLine(int x0, int y0, int x1, int y1) {return 0;}
  virtual int DrawText(int x0, int y0, char *text) {return 0;}
  virtual int DrawClear() {return 0;}
  virtual int GetTextH(const char *s) {return 16;}
  virtual int GetTextW(const char *s) {return 8 * strlen(s);}
  virtual void Quit(int q = 1) {}
  virtual void ResReinit(int w = INT_MIN, int h = INT_MIN) {}
  virtual int GetAboutInfo() {return about_info;}
  virtual void SetAboutInfo(int inf) {about_info = inf;}
protected:
  void FreeTitle() {if (title) {delete[] title; title = 0;}}
  void CopyTitle(const char *s);
  char *title;
  int about_info;
public:
  int (*evfunc)(const event &ev);
  int id;
  void *data;
};

unsigned long TGraphDraw::CreateColor(unsigned short red,
                          unsigned short green, unsigned short blue)
{
  return (unsigned long)(red >> 8) + ((unsigned long)(green >> 8) << 8) +
         ((unsigned long)(blue >> 8) << 16);
}

void TGraphDraw::CopyTitle(const char *s)
{
  FreeTitle();
  if (s) {title = new char[strlen(s) + 1]; strcpy(title, s);}
}

#if defined __GNUC__
# include "gnu-draw.h"
  typedef TGnuGraphDraw TMainGraphDraw;
#elif defined __MENUET__
# include "mt-draw.h"
  typedef TKlbrGraphDraw TMainGraphDraw;
#elif defined _Windows
# include "win-draw.h"
  typedef TWinGraphDraw TMainGraphDraw;
#elif defined __MSDOS__
# include "dos-draw.h"
  typedef TDosGraphDraw TMainGraphDraw;
#else
  typedef TGraphDraw TMainGraphDraw;
#endif

#endif  //_GRAPHIC_DRAW_H
