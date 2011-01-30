#ifndef _HEADER_BUTTONS_H
#define _HEADER_BUTTONS_H

#ifndef __MENUET__
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#endif
#include "gr-draw.h"

struct TXButton
{
  TXButton(int n = -1, int w = 100, int h = 20) : num(n), w(w), h(h), drw(1) {}
  TXButton(const TXButton &b) : w(b.w), h(b.h) {}

  TXButton &operator=(const TXButton &b) {w = b.w; h = b.h; num = b.num; return *this;}

  virtual void Draw(TGraphDraw *drw, int x0, int y0,
      unsigned long dcolor, unsigned long lcolor, unsigned long tcolor, int ins = 0);
  virtual int ButtonPnt(int xp, int yp, int &n, int kind = 0);

  int num, w, h, drw;
};

void TXButton::Draw(TGraphDraw *drw, int x0, int y0,
         unsigned long dcolor, unsigned long /*lcolor*/,
         unsigned long /*tcolor*/, int /*ins*/)
{
  if (!drw || !drw->IsDraw()) return;
  drw->SetColor(dcolor);
  drw->DrawLine(x0, y0, x0 + w, y0);
  drw->DrawLine(x0, y0, x0, y0 + h);
  drw->DrawLine(x0 + w, y0, x0 + w, y0 + h);
  drw->DrawLine(x0, y0 + h, x0 + w, y0 + h);
}

int TXButton::ButtonPnt(int xp, int yp, int &n, int /*kind*/)
{
  n = num;
  return (xp >= 0 && yp >= 0 && xp <= w && yp <= h) ? 1 : INT_MIN;
}


struct TextButton : public TXButton
{
  enum {thick_def = 3};

  TextButton(int n = -1, int w = 100, int h = 20, char *text = 0, int thick = thick_def)
               : TXButton(n, w, h), text(text), thick(thick) {CheckDefW();}
  TextButton(const TXButton &b, char *text = 0, int thick = thick_def)
               : TXButton(b), text(text), thick(thick) {CheckDefW();}
  TextButton(const TextButton &b) : TXButton(b), text(b.text), thick(b.thick) {}

  TextButton &operator=(const TextButton &b);

  virtual void Draw(TGraphDraw *drw, int x0, int y0,
      unsigned long dcolor, unsigned long lcolor, unsigned long tcolor, int ins = 0);
  void CheckDefW();

  int thick;
  char *text;
};

inline TextButton &TextButton::operator=(const TextButton &b)
{
  text = b.text;
  TXButton::operator=(b);
  return *this;
}

void TextButton::Draw(TGraphDraw *drw, int x0, int y0,
           unsigned long dcolor, unsigned long lcolor, unsigned long tcolor, int ins)
{
  if (!drw || !drw->IsDraw()) return;
  int i;
  if (thick <= -1 && thick >= -5)
  {
    drw->SetColor(dcolor);
    for (i = 0; i < -thick; i++)
    {
      drw->DrawLine(x0 + i, y0 + i, x0 + w - i, y0 + i);
      drw->DrawLine(x0 + i, y0 + i, x0 + i, y0 + h - i);
      drw->DrawLine(x0 + w - i, y0 + i, x0 + w - i, y0 + h - i);
      drw->DrawLine(x0 + i, y0 + h - i, x0 + w - i, y0 + h - i);
    }
  }
  else if (thick > 0)
  {
    for (i = 0; i < thick; i++)
    {
      drw->SetColor(ins ? dcolor : lcolor);
      drw->DrawLine(x0 + i, y0 + i, x0 + w - i, y0 + i);
      drw->DrawLine(x0 + i, y0 + i, x0 + i, y0 + h - i);
      drw->SetColor(ins ? lcolor : dcolor);
      drw->DrawLine(x0 + w - i, y0 + i, x0 + w - i, y0 + h - i);
      drw->DrawLine(x0 + i, y0 + h - i, x0 + w - i, y0 + h - i);
    }
  }
  else TXButton::Draw(drw, x0, y0, dcolor, lcolor, tcolor, ins);
  if (text)
  {
    drw->SetColor(tcolor);
    drw->DrawText(x0 + 7, y0 + (h - drw->GetTextH(text)) / 2, text);
  }
}

inline void TextButton::CheckDefW()
{
  if (w < 0 && text) w = 15 + strlen(text) * 8;
}

class TXButtonArray
{
public:
  TXButtonArray(int d = 10) : button(0), nbutton(0), mbutton(0),
                              delt(d), ins(-1), k_ins(0) {}
  ~TXButtonArray() {Clear();}

  void Clear();
  void Add(TXButton *bt);
  void Add(int n = -1, int w = 100, int h = 20, char *str = 0,
           int thick = TextButton::thick_def);
  TXButton *operator[](int i) {return button[i];}
  int GetNButton() const {return nbutton;}
  int GetInsert() const {return ins;}
  TXButton *GetButton(int n);

  int GetDelt() const {return delt;}
  void SetDelt(int d) {delt = d;}

  int GetParam(int w, int p) const;
  int GetNumStr(int w) const {return GetParam(w, 0);}
  int GetHeight(int w) const {return GetParam(w, 1);}
  int GetFWidth() const;

  void Draw(TGraphDraw *drw, int x0, int y0, int w);
  void Draw(TGraphDraw *drw, int x0, int y0, int w,
     unsigned long dcolor, unsigned long lcolor, unsigned long tcolor, int insd = 1);
  int ButtonPnt(int xp, int yp, int w, int &n, int kind = 0);
protected:
  void Extend(int n = -1);

  TXButton **button;
  int nbutton, mbutton;
  int delt;
  int ins, k_ins;
};

void TXButtonArray::Clear()
{
  int i;
  if (button)
  {
    for (i = 0; i < nbutton; i++) if (button[i]) delete button[i];
    delete[] button;
  }
  button = 0; nbutton = 0; mbutton = 0;
}

void TXButtonArray::Extend(int n)
{
  if (n < 0) n = nbutton + 1;
  if (mbutton < n)
  {
    typedef TXButton *TXButtonPnt;
    TXButton **b_old = button;
    int m_old = mbutton;
    mbutton = n * 2;
    button = new TXButtonPnt[mbutton];
    for (int i = 0; i < mbutton; i++) button[i] = 0;
    if (b_old)
    {
      for (int i = 0; i < m_old; i++) button[i] = b_old[i];
      delete[] b_old;
    }
  }
}

void TXButtonArray::Add(TXButton *bt)
{
  Extend();
  if (button[nbutton]) delete[] button[nbutton];
  button[nbutton] = bt;
  nbutton++;
}

void TXButtonArray::Add(int n, int w, int h, char *str, int thick)
{
  if (thick < 0 && !str) Add(new TXButton(n, w, h));
  else Add(new TextButton(n, w, h, str, thick));
}

TXButton *TXButtonArray::GetButton(int n)
{
  int i;
  for (i = 0; i < nbutton; i++)
  {
    if (button[i] && button[i]->num == n) return button[i];
  }
  return 0;
}

int TXButtonArray::GetFWidth() const
{
  int i, x = 0;
  for (i = 0; i < nbutton; i++)
  {
    if (button[i] && button[i]->drw > 0) x += button[i]->w + delt;
  }
  if (x != 0) x -= delt;
  return x;
}

int TXButtonArray::GetParam(int w, int p) const
{
  int i, k = 0;
  int x = 0, y = 0, ym = 0;
  for (i = 0; i < nbutton; i++) if (button[i] && button[i]->drw > 0)
  {
    int xx = x + button[i]->w + delt;
    if (x != 0 && xx > w + delt)
    {
      k++; y += ym + delt; ym = 0;
      x = 0; xx = x + button[i]->w + delt;
    }
    x = xx;
    if (ym < button[i]->h) ym = button[i]->h;
  }
  if (x != 0) {k++; y += ym;}
  else if (y != 0) y -= delt;
  if (p == 0) return k;
  else if (p == 1) return y;
  else return -1;
}

void TXButtonArray::Draw(TGraphDraw *drw, int x0, int y0, int w)
{
  const unsigned short power_gray = 49152U;
  unsigned long black = drw->GetBlackColor();
  unsigned long grey = drw->CreateColor(power_gray, power_gray, power_gray);
  Draw(drw, x0, y0, w, black, grey, black, 1);
  drw->FreeColor(grey);
}

void TXButtonArray::Draw(TGraphDraw *drw, int x0, int y0, int w,
        unsigned long dcolor, unsigned long lcolor, unsigned long tcolor, int insd)
{
  int i;
  int x = 0, y = 0, ym = 0;
  if (!insd && ins >= 0) k_ins = 2;
  for (i = 0; i < nbutton; i++) if (button[i] && button[i]->drw > 0)
  {
    int xx = x + button[i]->w + delt;
    if (x != 0 && xx > w + delt)
    {
      y += ym + delt; ym = 0;
      x = 0; xx = x + button[i]->w + delt;
    }
    button[i]->Draw(drw, x0 + x, y0 + y,
                    dcolor, lcolor, tcolor, k_ins == 1 && ins == i);
    x = xx;
    if (ym < button[i]->h) ym = button[i]->h;
  }
}

int TXButtonArray::ButtonPnt(int xp, int yp, int w, int &n, int kind)
{
  int i;
  int x = 0, y = 0, ym = 0;
  if (ins < 0 && (kind == 2 || kind == 3)) return INT_MIN;
  for (i = 0; i < nbutton; i++) if (button[i] && button[i]->drw > 0)
  {
    int xx = x + button[i]->w + delt;
    if (x != 0 && xx > w + delt)
    {
      y += ym + delt; ym = 0;
      x = 0; xx = x + button[i]->w + delt;
    }
    if (ins < 0 || i == ins)
    {
      int wh = button[i]->ButtonPnt(xp - x, yp - y, n, kind);
      if (n == -1) n = i;
      if (i == ins)
      {
        if (kind == 1) return wh;
        else if (kind == 2)
        {
          if (wh == INT_MIN || n < 0)
          {
            if (k_ins != 2) {k_ins = 2; n = -10; return 1000;}
          }
          else if (k_ins != 1) {k_ins = 1; return 1000;}
          return wh;
        }
        else if (kind == 3)
        {
          if (wh == INT_MIN)
          {
            n = -10;
            if (k_ins == 1) wh = 1000;
          }
          k_ins = 0; ins = -1;
          return wh;
        }
        else return wh;
      }
      else if (wh != INT_MIN)
      {
        if (kind == 1) {ins = i; k_ins = 1; return wh;}
        else if (kind == 2 || kind == 3) return INT_MIN;
        else return wh;
      }
    }
    x = xx;
    if (ym < button[i]->h) ym = button[i]->h;
  }
  n = -10;
  return INT_MIN;
}

struct TMultiButton : public TXButton
{
  TMultiButton(int n = -1, int w = 100, int h = 20, int d = 2)
                         : TXButton(n, w, h), a(d) {}

  virtual void Draw(TGraphDraw *drw, int x0, int y0, unsigned long dcolor,
                    unsigned long lcolor, unsigned long tcolor, int ins = 0);
  virtual int ButtonPnt(int xp, int yp, int &n, int kind = 0);

  void SetDefW();

  TXButtonArray a;
};

void TMultiButton::Draw(TGraphDraw *drw, int x0, int y0, unsigned long dcolor,
                        unsigned long lcolor, unsigned long tcolor, int ins)
{
  a.Draw(drw, x0, y0, w, dcolor, lcolor, tcolor, ins);
}

int TMultiButton::ButtonPnt(int xp, int yp, int &n, int kind)
{
  if (a.GetInsert() < 0 && (xp < 0 || yp < 0 || xp > w || yp > h)) return INT_MIN;
  return a.ButtonPnt(xp, yp, w, n, kind);
}

void TMultiButton::SetDefW()
{
  w = a.GetFWidth();
  if (w < 0) w = 0;
  h = a.GetHeight(w);
}

#endif  //_HEADER_BUTTONS_H

