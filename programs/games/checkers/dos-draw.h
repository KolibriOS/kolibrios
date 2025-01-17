#include "gr-draw.h"

#ifndef _DOS_GRAPHIC_DRAW_H
#define _DOS_GRAPHIC_DRAW_H

#include "dosmouse.h"
#include "keysym.h"
#include <graphics.h>
#include <conio.h>
#include <stdio.h>
#include <limits.h>

class TDosGraphDraw : public TBaseGraphDraw<TDosGraphDraw>
{
  typedef TBaseGraphDraw<TDosGraphDraw> TGraphDraw;
public:
  TDosGraphDraw(const char *s = 0);
  ~TDosGraphDraw() {}
public:
  static unsigned long GetKeySym(unsigned int key);
protected:
  unsigned long bgcolor;
  int quit;
  TCursorVisible cursor; 
public:
  unsigned long GetBlackColor() {return 0;}
  unsigned long GetWhiteColor() {return 15;}
  unsigned long CreateColor(unsigned short red,
                 unsigned short green, unsigned short blue);
  void FreeColor(unsigned long c) {}
  unsigned long GetBgColor() {return bgcolor;}
  void SetBgColor(unsigned long c) {bgcolor = c;}

  int GetStatus() {return (graphresult() == grOk) ? 1 : 0;}
  int Init() {return 0;}
  int Run(int evmask = 0, int w = INT_MIN, int h = INT_MIN);

  void GetSize(int &w, int &h);
  int OpenDraw();
  int IsDraw() {return graphresult() == grOk && cursor.IsHidden();}
  void CloseDraw() {cursor.Show();}

  int SetColor(unsigned long c);
  int DrawLine(int x0, int y0, int x1, int y1);
  int DrawText(int x0, int y0, char *text);
  int DrawClear();
  int GetTextH(const char *s) {return 8;}
  int GetTextW(const char *s) {return 8 * sizeof(s);}
  void Quit(int q = 1) {quit = (q > 0) ? q : 0;}
};

unsigned long TDosGraphDraw::GetKeySym(unsigned int key)
{
  switch(key)
  {
    case 331: return XK_Left;
    case 333: return XK_Right;
    case 328: return XK_Up;
    case 336: return XK_Down;
    case 13:  return XK_Return;
    case 32:  return XK_space;
    case 27:  return XK_Escape;
    case 44:  return XK_comma;
    case 46:  return XK_period;
    case 60:  return XK_comma;
    case 62:  return XK_period;
    case 45:  return XK_minus;
    case 61:  return XK_equal;
    case 95:  return XK_underscore;
    case 43:  return XK_plus;
    case 339: return XK_Delete;
    case 47:  return XK_slash;
    case 63:  return XK_question;
    case 315: return XK_F1;
    case 316: return XK_F2;
    case 317: return XK_F3;
    case 318: return XK_F4;
    case 319: return XK_F5;
    case 320: return XK_F6;
    case 321: return XK_F7;
    case 322: return XK_F8;
    case 323: return XK_F9;
    case 324: return XK_F10;
    case 389: return XK_F11;
    case 390: return XK_F12;
    case 97:  return XK_a;
    case 98:  return XK_b;
    case 99:  return XK_c;
    case 100: return XK_d;
    case 101: return XK_e;
    case 102: return XK_f;
    case 103: return XK_g;
    case 104: return XK_h;
    case 105: return XK_i;
    case 106: return XK_j;
    case 107: return XK_k;
    case 108: return XK_l;
    case 109: return XK_m;
    case 110: return XK_n;
    case 111: return XK_o;
    case 112: return XK_p;
    case 113: return XK_q;
    case 114: return XK_r;
    case 115: return XK_s;
    case 116: return XK_t;
    case 117: return XK_u;
    case 118: return XK_v;
    case 119: return XK_w;
    case 120: return XK_x;
    case 121: return XK_y;
    case 122: return XK_z;
    case 65:  return XK_A;
    case 66:  return XK_B;
    case 67:  return XK_C;
    case 68:  return XK_D;
    case 69:  return XK_E;
    case 70:  return XK_F;
    case 71:  return XK_G;
    case 72:  return XK_H;
    case 73:  return XK_I;
    case 74:  return XK_J;
    case 75:  return XK_K;
    case 76:  return XK_L;
    case 77:  return XK_M;
    case 78:  return XK_N;
    case 79:  return XK_O;
    case 80:  return XK_P;
    case 81:  return XK_Q;
    case 82:  return XK_R;
    case 83:  return XK_S;
    case 84:  return XK_T;
    case 85:  return XK_U;
    case 86:  return XK_V;
    case 87:  return XK_W;
    case 88:  return XK_X;
    case 89:  return XK_Y;
    case 90:  return XK_Z;
    default:  return XK_VoidSymbol;
  }
}

TDosGraphDraw::TDosGraphDraw(const char *s) : TGraphDraw(s)
{
  bgcolor = GetWhiteColor();
}

unsigned long TDosGraphDraw::CreateColor(unsigned short red,
                  unsigned short green, unsigned short blue)
{
  const unsigned short PD = 12288U, PM = 36863U, PL = 65535U;
  const unsigned short COLOR[16][3] =
         {{0U, 0U, 0U}, {0U, 0U, PM}, {0U, PM, 0U}, {0U, PM, PM},
          {PM, 0U, 0U}, {PM, 0U, PM}, {PM, PM, 0U}, {PM, PM, PM},
          {PD, PD, PD}, {PD, PD, PL}, {PD, PL, PD}, {PD, PL, PL},
          {PL, PD, PD}, {PL, PD, PL}, {PL, PL, PD}, {PL, PL, PL}};
  int b[3];
  if (red > green)
  {
    if (green > blue) {b[0] = 4; b[1] = 2; b[2] = 1;}
    else if (red > blue) {b[0] = 4; b[1] = 1; b[2] = 2;}
    else {b[0] = 1; b[1] = 4; b[2] = 2;}
  }
  else
  {
    if (red > blue) {b[0] = 2; b[1] = 4; b[2] = 1;}
    else if (green > blue) {b[0] = 2; b[1] = 1; b[2] = 4;}
    else {b[0] = 1; b[1] = 2; b[2] = 4;}
  }
  int i, j, c, c0 = 0;
  long d, d0 = LONG_MAX;
  for (j = 0; j <= 8; j += 8) for (i = 0, c = j; i <= 3; c |= b[i++])
  {
    d = labs((long)red - COLOR[c][0]) + labs((long)green - COLOR[c][1]) +
	labs((long)blue - COLOR[c][2]);
    if (d0 >= d) {d0 = d; c0 = c;}
  }
  return c0;
}

void TDosGraphDraw::GetSize(int &w, int &h)
{
  if (graphresult() == grOk)
  {
    w = getmaxx() + 1; h = getmaxy() + 1;
  }
  else TGraphDraw::GetSize(w, h);
}

int TDosGraphDraw::OpenDraw()
{
  if (graphresult() == grOk)
  {
    cursor.Hide();
    return 1;
  }
  else return 0;
}

int TDosGraphDraw::SetColor(unsigned long c)
{
  if (!IsDraw()) return 0;
  else {setcolor((int)c); return 1;}
}

int TDosGraphDraw::DrawLine(int x0, int y0, int x1, int y1)
{
  if (!IsDraw()) return 0;
  else {line(x0, y0, x1, y1); return 1;}
}

int TDosGraphDraw::DrawText(int x0, int y0, char *text)
{
  if (!IsDraw()) return 0;
  else {outtextxy(x0, y0, text); return 1;}
}

int TDosGraphDraw::DrawClear()
{
  if (!IsDraw()) return 0;
  setbkcolor((int)bgcolor); setcolor((int)bgcolor);
  bar(0, 0, getmaxx(), getmaxy());
  setbkcolor(0);
  return 1;
}

int TDosGraphDraw::Run(int evmask, int /*w*/, int /*h*/)
{
  if (!evfunc) return -2;
  int wasgraph = graphresult();
  if (!wasgraph)
  {
    int gdriver = DETECT, gmode, errorcode;
    initgraph(&gdriver, &gmode, "");
    if ((errorcode = graphresult()) != grOk)
    {
      printf("Graphics error: %s\n", grapherrormsg(errorcode));
      return -1;
    }
  }
  event ev;
  int ch = INT_MIN, stpr = 0;
  int old_mx = -1000, old_my = -1000;
  clock_t old_mtime = clock();
  int show_cur = TCursorVisible::C.IsShowed();
  freopen("NUL", "wt", stdout);
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
    else if (stpr == 1)
    {
      ev.type = event::draw;
      ev.any.drw = this;
      evfunc(ev);
      CloseDraw();
      stpr = 2;
    }
    else if (ch >= 0 && (evmask & key_down_mask))
    {
      ev.type = event::key_up;
      ev.key.drw = this;
      ev.key.k = ch;
      evfunc(ev);
      CloseDraw();
      ch = INT_MIN;
    }
    else if (kbhit())
    {
      if (MouseStatus && TCursorVisible::C.IsShowed() &&
         (unsigned long)(clock() - old_mtime) > 5 * CLK_TCK)
      {
        TCursorVisible::C.Hide();
        old_mtime = clock();
      }
      int ch = (unsigned char)getch();
      if (!ch && kbhit()) ch = 0x100 | (unsigned char)getch();
      if (evmask & key_down_mask)
      {
        ev.type = event::key_down;
        ev.key.drw = this;
        ev.key.k = GetKeySym(ch);
        evfunc(ev);
        CloseDraw();
      }
    }
    else if (MouseStatus && (evmask & (button_down_mask |
             button_up_mask | mouse_move_mask | mouse_drag_mask)))
    {
      int k, x, y, z;
      for (k = 0; k < MouseStatus; k++)
      {
        z = GetButtonDown(k, x, y);
        if (z)
        {
          TCursorVisible::C.Show();
          old_mx = x; old_my = y;
          old_mtime = clock();
          if (evmask & button_down_mask)
          {
            ev.type = event::button_down;
            ev.button.drw = this;
            ev.button.x = x; ev.button.y = y;
            ev.button.n = k + 1;
            evfunc(ev);
            CloseDraw();
          }
          old_mtime = clock();
          k = -1; break;
        }
        z = GetButtonUp(k, x, y);
        if (z)
        {
          TCursorVisible::C.Show();
          old_mx = x; old_my = y;
          old_mtime = clock();
          if (evmask & button_up_mask)
          {
            ev.type = event::button_up;
            ev.button.drw = this;
            ev.button.x = x; ev.button.y = y;
            ev.button.n = k + 1;
            evfunc(ev);
            CloseDraw();
          }
          k = -1; break;
        }
      }
      if (k >= 0)
      {
        z = PosCursor(x, y);
        if (x != old_mx || y != old_my)
        {
          TCursorVisible::C.Show();
          old_mx = x; old_my = y;
          old_mtime = clock();
          if (evmask & (mouse_move_mask | mouse_drag_mask))
          {
            for (k = 0; k < MouseStatus; k++)
            {
              if (z & (1 << k)) break;
            }
            if (evmask & ((k == MouseStatus) ? mouse_move_mask : mouse_drag_mask))
            {
              ev.type = event::mouse_move;
              ev.button.drw = this;
              ev.button.x = x; ev.button.y = y;
              ev.button.n = (k >= MouseStatus) ? 0 : (k + 1);
              evfunc(ev);
              CloseDraw();
            }
          }
        }
        else if (TCursorVisible::C.IsShowed() &&
                (unsigned long)(clock() - old_mtime) > 30 * CLK_TCK)
        {
          TCursorVisible::C.Hide();
          old_mtime = clock();
        }
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
  TCursorVisible::C.Set(show_cur);
  if (!wasgraph) closegraph();
  freopen("CON", "wt", stdout);
  return quit;
}

#endif  //_DOS_GRAPHIC_DRAW_H
