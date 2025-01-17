#ifndef _GRAPHIC_DRAW_H
#define _GRAPHIC_DRAW_H

#include <limits.h>
#include <string.h>

#ifdef _Windows
# include <windows.h>
#endif

template<class TRealGraphDraw>
class TBaseGraphDraw
{
public:
  union event
  {
    enum evtype {noevent = 0, draw, button_down, button_up,
                 mouse_move, key_down, key_up, start, close} type;

    struct evany
    {
      evtype type;
      TRealGraphDraw *drw;
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
  TBaseGraphDraw(const char *s = 0) : title(0), about_info(0),
                evfunc(0), id(0), data(0) {CopyTitle(s);}
  ~TBaseGraphDraw() {FreeTitle();}

  unsigned long GetBlackColor() {return 0;}
  unsigned long GetWhiteColor() {return 0xFFFFFFL;}
  unsigned long CreateColor(unsigned short red,
                         unsigned short green, unsigned short blue);
  void FreeColor(unsigned long c) {}
  unsigned long GetBgColor() {return GetWhiteColor();}
  void SetBgColor(unsigned long c) {}

  void SetTitle(const char *s) {CopyTitle(s);}
  const char *GetTitle() const {return title;}

  int GetStatus() {return 0;}  //1 - can draw, 0 - can't draw, <0 - error
  int Init() {return 0;}
  void UnInit() {}
  int Run(int evmask = 0, int w = INT_MIN, int h = INT_MIN) {return -100;}

  void GetSize(int &w, int &h) {w = 200; h = 200;}
  int OpenDraw() {return 0;}
  int IsDraw() {return 0;}
  void CloseDraw() {}

  int SetColor(unsigned long c) {return 0;}
  int DrawLine(int x0, int y0, int x1, int y1) {return 0;}
  int DrawText(int x0, int y0, char *text) {return 0;}
  int DrawClear() {return 0;}
  int GetTextH(const char *s) {return 16;}
  int GetTextW(const char *s) {return 8 * strlen(s);}
  void Quit(int q = 1) {}
  void ResReinit(int w = INT_MIN, int h = INT_MIN) {}
  int GetAboutInfo() {return about_info;}
  void SetAboutInfo(int inf) {about_info = inf;}
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

template<class TRealGraphDraw>
unsigned long TBaseGraphDraw<TRealGraphDraw>::CreateColor(unsigned short red,
                          unsigned short green, unsigned short blue)
{
  return (unsigned long)(red >> 8) + ((unsigned long)(green >> 8) << 8) +
         ((unsigned long)(blue >> 8) << 16);
}

template<class TRealGraphDraw>
void TBaseGraphDraw<TRealGraphDraw>::CopyTitle(const char *s)
{
  FreeTitle();
  if (s) {title = new char[strlen(s) + 1]; strcpy(title, s);}
}

#if defined _KOLIBRI
# include "kolibri-draw.h"
  typedef TKlbrGraphDraw TMainGraphDraw;
#elif defined __GNUC__
# include "gnu-draw.h"
  typedef TGnuGraphDraw TMainGraphDraw;
#elif defined _Windows
# include "win-draw.h"
  typedef TWinGraphDraw TMainGraphDraw;
#elif defined __MSDOS__
# include "dos-draw.h"
  typedef TDosGraphDraw TMainGraphDraw;
#else
#error "Unknown platform"
#endif
typedef TMainGraphDraw TGraphDraw;

#endif  //_GRAPHIC_DRAW_H
