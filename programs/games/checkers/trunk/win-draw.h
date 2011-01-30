#ifndef _Windows
# error Can't include this file without windows
#endif

#include "gr-draw.h"

#ifndef _WIN_GRAPHIC_DRAW_H
#define _WIN_GRAPHIC_DRAW_H

#include "winabout.h"
#include "keysym.h"
#include <windows.h>
#include <owl\window.h>
#include <owl\framewin.h>
#include <owl\dialog.h>
#include <owl\checkbox.h>
#include <owl\edit.h>
#include <owl\applicat.h>

#define main	OwlMain

class TWinGraphDraw : public TGraphDraw
{
public:
  TWinGraphDraw(const char *s = 0);
  ~TWinGraphDraw();
public:
  static const int CM_ABOUT;
  static unsigned long GetKeySym(unsigned int key);
protected:
  class TDrwWindow : public TWindow
  {
  public:
    TDrwWindow(TWinGraphDraw *drw) : TWindow(), win_draw(drw)
    {
      Init((TWindow*)NULL, 0, 0);
    }

    TColor GetBkgndColor() const {return BkgndColor;}
  protected:
    void SetupWindow();
    bool CanClose();
    void Paint(TDC &dc, bool erase, TRect &rect);
    void EvSize(uint sizeType, TSize &size) {Invalidate();}
    void EvLButtonDblClk(uint modKeys, TPoint& point) {EvLButtonDown(modKeys, point);}
    void EvLButtonDown(uint modKeys, TPoint& point);
    void EvLButtonUp(uint modKeys, TPoint& point);
    void EvRButtonDblClk(uint modKeys, TPoint& point) {EvRButtonDown(modKeys, point);}
    void EvRButtonDown(uint modKeys, TPoint& point);
    void EvRButtonUp(uint modKeys, TPoint& point);
    void EvMButtonDblClk(uint modKeys, TPoint& point) {EvMButtonDown(modKeys, point);}
    void EvMButtonDown(uint modKeys, TPoint& point);
    void EvMButtonUp(uint modKeys, TPoint& point);
    void EvMouseMove(uint modKeys, TPoint& point);
    void EvKeyDown(uint key, uint repeatCount, uint flags);
    void EvKeyUp(uint key, uint repeatCount, uint flags);
  public:
    TWinGraphDraw *win_draw;

    DECLARE_RESPONSE_TABLE(TDrwWindow);
  };

  class TDrwFrameWindow : public TFrameWindow
  {
  public:
    TDrwFrameWindow(TWindow *parent, TDrwWindow *client);
  protected:
    virtual void SetupWindow();
    void EvSysCommand(uint cmdType, TPoint& point);
  public:
    TWinGraphDraw *win_draw;

    DECLARE_RESPONSE_TABLE(TDrwFrameWindow);
  };

  class TDrwApplication : public TApplication
  {
  public:
    TDrwApplication(TWinGraphDraw *drw, int w = 0, int h = 0)
                  : def_w(w), def_h(h), win_draw(drw) {}

    void InitMainWindow()
    {
      win_draw->win = new TDrwWindow(win_draw);
      SetMainWindow(new TDrwFrameWindow(0, win_draw->win));
      GetMainWindow()->SetIcon(this, "WinDrawIcon");
      GetMainWindow()->SetIconSm(this, "WinDrawIcon");
      if (def_w > 0 && def_h > 0)
      {
        GetMainWindow()->Attr.W = def_w + 2*GetSystemMetrics(SM_CXFRAME);
        GetMainWindow()->Attr.H = def_h +
           GetSystemMetrics(SM_CYCAPTION) + 2*GetSystemMetrics(SM_CYFRAME);
      }
    }
  public:
    int def_w, def_h;
    TWinGraphDraw *win_draw;
  };

  friend class TDrwWindow;
  friend class TDrwApplication;
protected:
  TDrwApplication app;
  TDrwWindow *win;
  TDC *dc;
  int kind_dc;
  TColor bgcolor;
  int evmask, quit;
public:
  unsigned long GetBgColor();
  void SetBgColor(unsigned long c);
  void SetTitle(const char *s);

  int GetStatus() {return win ? 1 : 0;}
  int Init() {return 0;}
  int Run(int evmask = 0, int w = INT_MIN, int h = INT_MIN);

  void GetSize(int &w, int &h);
  int OpenDraw();
  int IsDraw() {return win && dc;}
  void CloseDraw();

  int SetColor(unsigned long c);
  int DrawLine(int x0, int y0, int x1, int y1);
  int DrawText(int x0, int y0, char *text);
  int DrawClear();
  int GetTextH(const char *s);
  int GetTextW(const char *s);
  void Quit(int q = 1);
};

const int TWinGraphDraw::CM_ABOUT = 7128;

unsigned long TWinGraphDraw::GetKeySym(uint key)
{
  switch(key)
  {
    case 37:  return XK_Left;
    case 39:  return XK_Right;
    case 38:  return XK_Up;
    case 40:  return XK_Down;
    case 13:  return XK_Return;
    case 32:  return XK_space;
    case 27:  return XK_Escape;
    case 188: return XK_comma;
    case 190: return XK_period;
    case 189: return XK_minus;
    case 187: return XK_equal;
    case 46:  return XK_Delete;
    case 191: return XK_slash;
    case 112: return XK_F1;
    case 113: return XK_F2;
    case 114: return XK_F3;
    case 115: return XK_F4;
    case 116: return XK_F5;
    case 117: return XK_F6;
    case 118: return XK_F7;
    case 119: return XK_F8;
    case 120: return XK_F9;
    case 121: return XK_F10;
    case 122: return XK_F11;
    case 123: return XK_F12;
    case 65:  return XK_a;
    case 66:  return XK_b;
    case 67:  return XK_c;
    case 68:  return XK_d;
    case 69:  return XK_e;
    case 70:  return XK_f;
    case 71:  return XK_g;
    case 72:  return XK_h;
    case 73:  return XK_i;
    case 74:  return XK_j;
    case 75:  return XK_k;
    case 76:  return XK_l;
    case 77:  return XK_m;
    case 78:  return XK_n;
    case 79:  return XK_o;
    case 80:  return XK_p;
    case 81:  return XK_q;
    case 82:  return XK_r;
    case 83:  return XK_s;
    case 84:  return XK_t;
    case 85:  return XK_u;
    case 86:  return XK_v;
    case 87:  return XK_w;
    case 88:  return XK_x;
    case 89:  return XK_y;
    case 90:  return XK_z;
    default:  return XK_VoidSymbol;
  }
}

TWinGraphDraw::TWinGraphDraw(const char *s)
             : TGraphDraw(s), app(this), dc(0), kind_dc(0)
{
  bgcolor = GetWhiteColor();
}

TWinGraphDraw::~TWinGraphDraw()
{
  CloseDraw();
  dc = 0; kind_dc = 0; win = 0;
}

unsigned long TWinGraphDraw::GetBgColor()
{
  if (win) bgcolor = win->GetBkgndColor();
  return bgcolor.GetValue();
}

void TWinGraphDraw::SetBgColor(unsigned long c)
{
  bgcolor.SetValue(c);
  if (win) win->SetBkgndColor(bgcolor);
}

void TWinGraphDraw::SetTitle(const char *s)
{
  TGraphDraw::SetTitle(s);
  if (win) win->SetCaption(s);
}

int TWinGraphDraw::Run(int evmask, int w, int h)
{
  if (!evfunc) return -2;
  this->evmask = evmask;
  app.def_w = w; app.def_h = h;
  quit = 0;
  app.Run();
  return quit;
}

void TWinGraphDraw::GetSize(int &w, int &h)
{
  if (win)
  {
    TRect rect;
    win->GetWindowRect(rect);
    w = rect.Width(); h = rect.Height();
  }
  else TGraphDraw::GetSize(w, h);
}

int TWinGraphDraw::OpenDraw()
{
  if (!win) return 0;
  if (!dc) {dc = new TWindowDC(*win); kind_dc = 1;}
  return 1;
}

void TWinGraphDraw::CloseDraw()
{
  if (dc && kind_dc == 1) delete dc;
  if (kind_dc <= 2) {dc = 0; kind_dc = 0;}
}

int TWinGraphDraw::SetColor(unsigned long c)
{
  if (!win || !dc) return 0;
  TColor color(c);
  dc->SelectObject((TPen)color);
  dc->SetTextColor(color);
  return 1;
}

int TWinGraphDraw::DrawLine(int x0, int y0, int x1, int y1)
{
  if (!win || !dc) return 0;
  dc->MoveTo(x0, y0);
  dc->LineTo(x1, y1);
  return 1;
}

int TWinGraphDraw::DrawText(int x0, int y0, char *text)
{
  if (!win || !dc) return 0;
  dc->TextOut(x0, y0, text);
  return 1;
}

int TWinGraphDraw::DrawClear()
{
  if (!win || !dc) return 0;
  dc->FillRect(TRect(TPoint(0, 0), win->GetWindowRect().Size()), bgcolor);
  return 1;
}

int TWinGraphDraw::GetTextH(const char *s)
{
  if (win)
  {
    TFont fnt = win->GetWindowFont();
    return fnt.GetHeight();
  }
  else return TGraphDraw::GetTextH(s);
}

int TWinGraphDraw::GetTextW(const char *s)
{
  if (win)
  {
    TFont fnt = win->GetWindowFont();
    return fnt.GetMaxWidth() * strlen(s);
  }
  else return TGraphDraw::GetTextW(s);
}

void TWinGraphDraw::Quit(int q)
{
  quit = q;
  if (quit <= 0) quit = 0;
  else if (win) win->PostMessage(WM_CLOSE);
}

DEFINE_RESPONSE_TABLE1(TWinGraphDraw::TDrwFrameWindow, TFrameWindow)
  EV_WM_SYSCOMMAND,
END_RESPONSE_TABLE;

TWinGraphDraw::TDrwFrameWindow::TDrwFrameWindow(TWindow *parent, TDrwWindow *client)
             : TFrameWindow(parent, client->win_draw->GetTitle(), client)
{
  win_draw = client->win_draw;
}

void TWinGraphDraw::TDrwFrameWindow::SetupWindow()
{
  TFrameWindow::SetupWindow();
  if (win_draw->GetAboutInfo() > 0)
  {
    TMenu menu = GetSystemMenu(false);
    menu.AppendMenu(MF_SEPARATOR);
    menu.AppendMenu(MF_STRING, CM_ABOUT, "&About");
  }
}

void TWinGraphDraw::TDrwFrameWindow::EvSysCommand(uint cmdType, TPoint& point)
{
  if (cmdType == CM_ABOUT)
  {
    char dia_name[50] = "IDD_ABOUT";
    int dia_num = win_draw->GetAboutInfo();
    if (dia_num > 0)
    {
      itoa(dia_num, dia_name + strlen(dia_name), 10);
      TAboutDialog(this, dia_name).Execute();
    }
  }
  else TFrameWindow::EvSysCommand(cmdType, point);
}

DEFINE_RESPONSE_TABLE1(TWinGraphDraw::TDrwWindow, TWindow)
  EV_WM_MOUSEMOVE,
  EV_WM_LBUTTONDBLCLK,
  EV_WM_LBUTTONDOWN,
  EV_WM_LBUTTONUP,
  EV_WM_RBUTTONDBLCLK,
  EV_WM_RBUTTONDOWN,
  EV_WM_RBUTTONUP,
  EV_WM_MBUTTONDBLCLK,
  EV_WM_MBUTTONDOWN,
  EV_WM_MBUTTONUP,
  EV_WM_SIZE,
  EV_WM_KEYDOWN,
  EV_WM_KEYUP,
END_RESPONSE_TABLE;

void TWinGraphDraw::TDrwWindow::SetupWindow()
{
  TWindow::SetupWindow();
  if (!win_draw || !win_draw->evfunc || win_draw->quit > 0) return;
  event ev;
  ev.type = event::start;
  ev.any.drw = win_draw;
  win_draw->evfunc(ev);
  win_draw->CloseDraw();
}

bool TWinGraphDraw::TDrwWindow::CanClose()
{
  if (!win_draw || !win_draw->evfunc || win_draw->quit > 1) return true;
  event ev;
  ev.type = event::close;
  ev.any.drw = win_draw;
  win_draw->quit = win_draw->evfunc(ev);
  if (win_draw->quit <= 0) win_draw->quit = 0;
  win_draw->CloseDraw();
  return win_draw->quit != 0;
}

void TWinGraphDraw::TDrwWindow::Paint(TDC &dc, bool, TRect&)
{
  if (!win_draw || !win_draw->evfunc || win_draw->quit > 0) return;
  win_draw->CloseDraw();
  win_draw->dc = &dc; win_draw->kind_dc = 2;
  event ev;
  ev.type = event::draw;
  ev.any.drw = win_draw;
  win_draw->evfunc(ev);
  win_draw->CloseDraw();
}

void TWinGraphDraw::TDrwWindow::EvLButtonDown(uint /*modKeys*/, TPoint& point)
{
  if (!win_draw || !win_draw->evfunc || win_draw->quit > 0 ||
      !(win_draw->evmask & button_down_mask)) return;
  event ev;
  ev.type = event::button_down;
  ev.button.drw = win_draw;
  ev.button.n = 1;
  ev.button.x = point.x;
  ev.button.y = point.y;
  int r = win_draw->evfunc(ev);
  if (r >= 0 && (r & ret_setcapture)) SetCapture();
  win_draw->CloseDraw();
}

void TWinGraphDraw::TDrwWindow::EvLButtonUp(uint /*modKeys*/, TPoint& point)
{
  ReleaseCapture();
  if (!win_draw || !win_draw->evfunc || win_draw->quit > 0 ||
      !(win_draw->evmask & button_up_mask)) return;
  event ev;
  ev.type = event::button_up;
  ev.button.drw = win_draw;
  ev.button.n = 1;
  ev.button.x = point.x;
  ev.button.y = point.y;
  win_draw->evfunc(ev);
  win_draw->CloseDraw();
}

void TWinGraphDraw::TDrwWindow::EvRButtonDown(uint /*modKeys*/, TPoint& point)
{
  if (!win_draw || !win_draw->evfunc || win_draw->quit > 0 ||
      !(win_draw->evmask & button_down_mask)) return;
  event ev;
  ev.type = event::button_down;
  ev.button.drw = win_draw;
  ev.button.n = 2;
  ev.button.x = point.x;
  ev.button.y = point.y;
  int r = win_draw->evfunc(ev);
  if (r >= 0 && (r & ret_setcapture)) SetCapture();
  win_draw->CloseDraw();
}

void TWinGraphDraw::TDrwWindow::EvRButtonUp(uint /*modKeys*/, TPoint& point)
{
  ReleaseCapture();
  if (!win_draw || !win_draw->evfunc || win_draw->quit > 0 ||
      !(win_draw->evmask & button_up_mask)) return;
  event ev;
  ev.type = event::button_up;
  ev.button.drw = win_draw;
  ev.button.n = 2;
  ev.button.x = point.x;
  ev.button.y = point.y;
  win_draw->evfunc(ev);
  win_draw->CloseDraw();
}

void TWinGraphDraw::TDrwWindow::EvMButtonDown(uint /*modKeys*/, TPoint& point)
{
  if (!win_draw || !win_draw->evfunc || win_draw->quit > 0 ||
      !(win_draw->evmask & button_down_mask)) return;
  event ev;
  ev.type = event::button_down;
  ev.button.drw = win_draw;
  ev.button.n = 3;
  ev.button.x = point.x;
  ev.button.y = point.y;
  int r = win_draw->evfunc(ev);
  if (r >= 0 && (r & ret_setcapture)) SetCapture();
  win_draw->CloseDraw();
}

void TWinGraphDraw::TDrwWindow::EvMButtonUp(uint /*modKeys*/, TPoint& point)
{
  ReleaseCapture();
  if (!win_draw || !win_draw->evfunc || win_draw->quit > 0 ||
      !(win_draw->evmask & button_up_mask)) return;
  event ev;
  ev.type = event::button_up;
  ev.button.drw = win_draw;
  ev.button.n = 3;
  ev.button.x = point.x;
  ev.button.y = point.y;
  win_draw->evfunc(ev);
  win_draw->CloseDraw();
}

void TWinGraphDraw::TDrwWindow::EvMouseMove(uint /*modKeys*/, TPoint& point)
{
  if (!win_draw || !win_draw->evfunc || win_draw->quit > 0 ||
      !(win_draw->evmask & (mouse_move_mask | mouse_drag_mask))) return;
  event ev;
  ev.type = event::mouse_move;
  ev.button.drw = win_draw;
  ev.button.n = -1;
  ev.button.x = point.x;
  ev.button.y = point.y;
  win_draw->evfunc(ev);
  win_draw->CloseDraw();
}

void TWinGraphDraw::TDrwWindow::EvKeyDown(uint key, uint /*repeatCount*/, uint /*flags*/)
{
  if (!win_draw || !win_draw->evfunc || win_draw->quit > 0 ||
      !(win_draw->evmask & key_down_mask)) return;
  event ev;
  ev.type = event::key_down;
  ev.key.drw = win_draw;
  ev.key.k = GetKeySym(key);
  win_draw->evfunc(ev);
  win_draw->CloseDraw();
}

void TWinGraphDraw::TDrwWindow::EvKeyUp(uint key, uint /*repeatCount*/, uint /*flags*/)
{
  if (!win_draw || !win_draw->evfunc || win_draw->quit > 0 ||
      !(win_draw->evmask & key_up_mask)) return;
  event ev;
  ev.type = event::key_up;
  ev.key.drw = win_draw;
  ev.key.k = GetKeySym(key);
  win_draw->evfunc(ev);
  win_draw->CloseDraw();
}

#endif  //_WIN_GRAPHIC_DRAW_H
