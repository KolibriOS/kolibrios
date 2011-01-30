#ifndef _HEADER_BOARD_H
#define _HEADER_BOARD_H

#ifndef __MENUET__
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#endif
#include "position.h"
#include "history.h"
#include "gr-draw.h"

int ssf = 0;

struct TIntPoint
{
  TIntPoint() {}
  TIntPoint(int x, int y) : x(x), y(y) {}
  TIntPoint(const TIntPoint &p) : x(p.x), y(p.y) {}

  TIntPoint &operator=(const TIntPoint &p) {x = p.x; y = p.y; return *this;}

  int x, y;
};

struct TRealPoint
{
  TRealPoint() {}
  TRealPoint(double x, double y) : x(x), y(y) {}
  TRealPoint(const TRealPoint &p) : x(p.x), y(p.y) {}
  TRealPoint(const TIntPoint &p) : x(p.x), y(p.y) {}

  TRealPoint &operator=(const TRealPoint &p) {x = p.x; y = p.y; return *this;}
  TRealPoint &operator=(const TIntPoint &p) {x = p.x; y = p.y; return *this;}

  TIntPoint Round() {return TIntPoint((int)floor(x+0.5), (int)floor(y+0.5));}

  double x, y;
};

struct TProjectivePoint
{
  TProjectivePoint() {}
  TProjectivePoint(const TProjectivePoint &p) : x(p.x), y(p.y), z(p.z) {}
  TProjectivePoint(double x, double y, double z = 1) : x(x), y(y), z(z) {}
  TProjectivePoint(const TIntPoint &p, double z = 1) : x(p.x), y(p.y), z(z) {}
  TProjectivePoint(const TRealPoint &p, double z = 1) : x(p.x), y(p.y), z(z) {}

  int IsCorrect() const {return fabs(x) > Eps || fabs(y) > Eps || fabs(z) > Eps;}
  int IsFinite() const;
  TRealPoint rpnt() const;
  int rpnt(TRealPoint &p) const;
  TIntPoint pnt() const;
  int pnt(TIntPoint &p) const;

  TProjectivePoint &operator=(const TProjectivePoint &p);
  TProjectivePoint &operator=(const TIntPoint &p) {x = p.x; y = p.y; z = 1; return *this;}
  TProjectivePoint &operator=(const TRealPoint &p) {x = p.x; y = p.y; z = 1; return *this;}
  TProjectivePoint &operator*=(int n) {x *= n; y *= n; z *= n; return *this;}
  friend TProjectivePoint operator*(const TProjectivePoint &p, int n)
                {return TProjectivePoint(p.x * n, p.y * n, p.z * n);}
  friend TProjectivePoint operator*(int n, const TProjectivePoint &p)
                {return TProjectivePoint(n * p.x, n * p.y, n * p.z);}
  TProjectivePoint &operator*=(double n) {x *= n; y *= n; z *= n; return *this;}
  friend TProjectivePoint operator*(const TProjectivePoint &p, double n)
                {return TProjectivePoint(p.x * n, p.y * n, p.z * n);}
  friend TProjectivePoint operator*(double n, const TProjectivePoint &p)
                {return TProjectivePoint(n * p.x, n * p.y, n * p.z);}

  double px() const {return x/z;}
  double py() const {return y/z;}

  double x, y, z;

  static const double Eps, FR;
};

const double TProjectivePoint::Eps = 1e-12;
const double TProjectivePoint::FR = 1e+4;

inline int TProjectivePoint::IsFinite() const
{
  double fz = fabs(z);
  return fz > Eps && fabs(x) < fz * FR && fabs(y) < fz * FR;
}

TRealPoint TProjectivePoint::rpnt() const
{
  if (!IsFinite()) return TRealPoint(0, 0);
  else return TRealPoint(x/z, y/z);
}

inline int TProjectivePoint::rpnt(TRealPoint &p) const
{
  if (!IsFinite()) {p.x = 0; p.y = 0; return 0;}
  else {p.x = x/z; p.y = y/z; return 1;}
}

inline TIntPoint TProjectivePoint::pnt() const
{
  if (!IsFinite()) return TIntPoint(INT_MIN, INT_MIN);
  else return TIntPoint((int)floor(x/z + 0.5), (int)floor(y/z + 0.5));
}

inline int TProjectivePoint::pnt(TIntPoint &p) const
{
  if (!IsFinite()) {p.x = INT_MIN; p.y = INT_MIN; return 0;}
  else
  {
    p.x = (int)floor(x/z + 0.5);
    p.y = (int)floor(y/z + 0.5);
    return 1;
  }
}

TProjectivePoint &TProjectivePoint::operator=(const TProjectivePoint &p)
{
  x = p.x; y = p.y; z = p.z;
  return *this;
}


class TProjectiveMap
{
public:
  TProjectiveMap() {}
  TProjectiveMap(int n) {p[0].x = p[1].y = p[2].z = n;
                p[0].y = p[0].z = p[1].x = p[1].z = p[2].x = p[2].y = 0;}
  TProjectiveMap(const TProjectiveMap &map)
                {p[0] = map.p[0]; p[1] = map.p[1]; p[2] = map.p[2];}
  TProjectiveMap(const TProjectivePoint pnt[])
                {p[0] = pnt[0]; p[1] = pnt[1]; p[2] = pnt[2];}
  TProjectiveMap(const TProjectivePoint &p0, const TProjectivePoint &p1,
                 const TProjectivePoint &p2) {p[0] = p0; p[1] = p1; p[2] = p2;}

  TProjectiveMap &operator=(const TProjectiveMap &map)
                {p[0] = map.p[0]; p[1] = map.p[1]; p[2] = map.p[2]; return *this;}
  TProjectiveMap &operator=(const TProjectivePoint pnt[])
                {p[0] = pnt[0]; p[1] = pnt[1]; p[2] = pnt[2]; return *this;}
  TProjectiveMap &operator=(int n) {p[0].x = p[1].y = p[2].z = n;
                p[0].y = p[0].z = p[1].x = p[1].z = p[2].x = p[2].y = 0; return *this;}

  TProjectivePoint operator()(const TProjectivePoint &point) const;
  friend TProjectiveMap operator*(const TProjectiveMap &a, const TProjectiveMap &b);
  TProjectiveMap &operator*=(const TProjectiveMap &b);
  TProjectiveMap &operator*=(int n) {p[0] *= n; p[1] *= n; p[2] *= n; return *this;}
  friend TProjectiveMap operator*(const TProjectiveMap &a, int n)
                       {return TProjectiveMap(a.p[0] * n, a.p[1] * n, a.p[2] * n);}
  friend TProjectiveMap operator*(int n, const TProjectiveMap &b)
                       {return TProjectiveMap(n * b.p[0], n * b.p[1], n * b.p[2]);}
  TProjectiveMap Reversed() const;
  TProjectiveMap &Reverse() {return (*this) = Reversed();}
  friend TProjectiveMap operator/(const TProjectiveMap &a, const TProjectiveMap &b)
                                 {return a * b.Reversed();}
  TProjectiveMap &operator/=(const TProjectiveMap &b) {return (*this) *= b.Reversed();}
  TProjectiveMap &operator/=(int n) {if (!n) (*this) = 0; return *this;}
  TProjectivePoint Reversed(const TProjectivePoint &point) const;

  TProjectivePoint operator()(double x, double y, double z = 1) const
                           {return (*this)(TProjectivePoint(x, y, z));}
  TProjectivePoint Reversed(double x, double y, double z = 1) const
                           {return Reversed(TProjectivePoint(x, y, z));}

  TProjectiveMap &Set4(const TProjectivePoint &a0, const TProjectivePoint &a1,
                       const TProjectivePoint &a2, const TProjectivePoint &a3);
  TProjectiveMap &Set4(const TProjectivePoint a[]) {return Set4(a[0], a[1], a[2], a[3]);}
  TProjectiveMap &Set4to4(const TProjectivePoint a[], const TProjectivePoint b[])
              {return Set4(b) /= TProjectiveMap().Set4(a);}
  TProjectiveMap &Set4to4(const TProjectivePoint &a0, const TProjectivePoint &a1,
                          const TProjectivePoint &a2, const TProjectivePoint &a3,
                          const TProjectivePoint b[])
              {return Set4(b) /= TProjectiveMap().Set4(a0, a1, a2, a3);}
  TProjectiveMap &Set4to4(const TProjectivePoint a[],
                          const TProjectivePoint &b0, const TProjectivePoint &b1,
                          const TProjectivePoint &b2, const TProjectivePoint &b3)
              {return Set4(b0, b1, b2, b2) /= TProjectiveMap().Set4(a);}
  TProjectiveMap &Set4to4(const TProjectivePoint &a0, const TProjectivePoint &a1,
                          const TProjectivePoint &a2, const TProjectivePoint &a3,
                          const TProjectivePoint &b0, const TProjectivePoint &b1,
                          const TProjectivePoint &b2, const TProjectivePoint &b3)
              {return Set4(b0, b1, b2, b2) /= TProjectiveMap().Set4(a0, a1, a2, a3);}

public:
  TProjectivePoint p[3];
};

TProjectivePoint TProjectiveMap::operator()(const TProjectivePoint &point) const
{
  return TProjectivePoint(p[0].x * point.x + p[1].x * point.y + p[2].x * point.z,
                          p[0].y * point.x + p[1].y * point.y + p[2].y * point.z,
                          p[0].z * point.x + p[1].z * point.y + p[2].z * point.z);
}

TProjectiveMap operator*(const TProjectiveMap &a, const TProjectiveMap &b)
{
  return TProjectiveMap(a(b.p[0]), a(b.p[1]), a(b.p[2]));
}

TProjectiveMap &TProjectiveMap::operator*=(const TProjectiveMap &b)
{
  TProjectivePoint p0 = (*this)(b.p[0]), p1 = (*this)(b.p[1]), p2 = (*this)(b.p[2]);
  p[0] = p0; p[1] = p1; p[2] = p2;
  return *this;
}

TProjectiveMap TProjectiveMap::Reversed() const
{
  return TProjectiveMap(TProjectivePoint(p[1].y * p[2].z - p[2].y * p[1].z,
           p[2].y * p[0].z - p[0].y * p[2].z, p[0].y * p[1].z - p[1].y * p[0].z),
                        TProjectivePoint(p[1].z * p[2].x - p[2].z * p[1].x,
           p[2].z * p[0].x - p[0].z * p[2].x, p[0].z * p[1].x - p[1].z * p[0].x),
                        TProjectivePoint(p[1].x * p[2].y - p[2].x * p[1].y,
           p[2].x * p[0].y - p[0].x * p[2].y, p[0].x * p[1].y - p[1].x * p[0].y));
}

TProjectivePoint TProjectiveMap::Reversed(const TProjectivePoint &point) const
{
  return TProjectivePoint((p[1].y * p[2].z - p[2].y * p[1].z) * point.x +
                          (p[1].z * p[2].x - p[2].z * p[1].x) * point.y +
                          (p[1].x * p[2].y - p[2].x * p[1].y) * point.z,
                 (p[2].y * p[0].z - p[0].y * p[2].z) * point.x +
                 (p[2].z * p[0].x - p[0].z * p[2].x) * point.y +
                 (p[2].x * p[0].y - p[0].x * p[2].y) * point.z,
        (p[0].y * p[1].z - p[1].y * p[0].z) * point.x +
        (p[0].z * p[1].x - p[1].z * p[0].x) * point.y +
        (p[0].x * p[1].y - p[1].x * p[0].y) * point.z);
}

TProjectiveMap &TProjectiveMap::Set4(const TProjectivePoint &a0,
   const TProjectivePoint &a1, const TProjectivePoint &a2, const TProjectivePoint &a3)
{
  p[0] = a0; p[1] = a1; p[2] = a2;
  TProjectivePoint K = Reversed(a3);
  p[0] *= K.x; p[1] *= K.y; p[2] *= K.z;
  return *this;
}


TRealPoint r_circle_pnt[13];
TRealPoint r_star_pnt[13];
const double r_star_large = 1.2, r_star_small = 0.8;
const double r_ch_lines[] = {0.8, 0.7, 0.6, 0.5};

void InitBoardData()
{
  int i, len;
  double a, r;
  len = NELEM(r_circle_pnt) - 1;
  for (i = 0; i < len; i++)
  {
    a = 2 * i * M_PI / len;
    r_circle_pnt[i].x = cos(a);
    r_circle_pnt[i].y = sin(a);
  }
  r_circle_pnt[len] = r_circle_pnt[0];
  len = sizeof(r_star_pnt) / sizeof(r_star_pnt[0]) - 1;
  for (i = 0; i < len; i++)
  {
    a = 2 * i * M_PI / len;
    r = (i % 2) ? r_star_small : r_star_large;
    r_star_pnt[i].x = cos(a) * r;
    r_star_pnt[i].y = sin(a) * r;
  }
  r_star_pnt[len] = r_star_pnt[0];
}

class TChBoard;

class TSomeDraw
{
public:
  TSomeDraw() {}

  virtual void Draw(TGraphDraw *drw, int w, int h) = 0;
  virtual void DrawB(TGraphDraw *drw, TChBoard &brd);
  virtual TIntPoint GetDSize(int w, int h) {return TIntPoint(INT_MIN, INT_MIN);}
};

class TChBoard
{
public:
  TChBoard(int id = 0);

  TProjectiveMap Get4PMap() const;
  int GetW() const {return width;}
  int GetH() const {return height;}
  void ClearWH() {width = -1; height = -1;}
  void Resize(int w, int h);
  void Resize(TIntPoint size) {Resize(size.x, size.y);}

  TIntPoint PlToWin(double x, double y) const;
  TIntPoint PlToWin(TRealPoint rp) const {return PlToWin(rp.x, rp.y);}
  TRealPoint WinToPl(int x, int y) const;
  TRealPoint WinToPl(TIntPoint p) const {return WinToPl(p.x, p.y);}

  void NewGame();
  int CanPlayerMove() const {return !IsPlayView && !game_end && !player[MainPos.wmove];}

  void Draw(TGraphDraw *drw);
  int ResizeDraw(TGraphDraw *drw, int w, int h);
  int CheckResize(TGraphDraw *drw);
  int HaveCheckResize() const {return check_resize;}
  void SetCheckResize(int cr) {check_resize = cr;}
  int MouseClick(TGraphDraw *drw, int x, int y);
  void UnMove(TGraphDraw *drw);

  int CMove(TGraphDraw *drw, int x, int y);
  int AutoMove(TGraphDraw *drw, int nmove = 0, int draw_check = 1);

  enum PKey {PNull, PEnter, PLeft, PRight, PUp, PDown};
  PKey GetKeySide(PKey key);
  int PKeyEvent(TGraphDraw *drw, PKey key = PEnter);
protected:
  enum TextLineType {TLT_Main, TLT_Move, TLT_Wait, TLT_GameEnd, TLT_WrongMove,
                     TLT_PlDidntMove, TLT_PlWrongMove, TLT_WrongColor,
                     TLT_WfCell, TLT_WnfCell, TLT_WMustEat, TLT_WMustEatMore,
                     TLT_WNoMove, TLT_WChBack, TLT_WNotDm, TLT_WOnlyDiag,
                     TLT_WEatYour, TLT_WMoreOne, TLT_WNotDmE, TLT_WMustEatMoreD,
                     TLT_WTurnBack};
  void GetTextLine(char str[]) const;
  void DrawTextLine(TGraphDraw *drw) const;
  void LineB(TGraphDraw *drw, double x1, double y1, double x2, double y2) const;
  void DrawCh(TGraphDraw *drw, double x0, double y0, const TRealPoint pnt[], int npnt,
              int L = NELEM(r_ch_lines)) const;
  void DrawIL(TGraphDraw *drw, double x0, double y0, int L) const;
  void MoveErase();
  void CurPointClear(TGraphDraw *drw) const;
  void RenewMPos() {if (MainPlay.GetPos(MainPos, CurMoveN) < 0) MainPos.Init();}
  void PrintLMove();
  void SetNoMove(TGraphDraw *drw = 0, int force = 1);
  TextLineType GetTLTfromA(int s) const;
  void ChangeTLT(TGraphDraw *drw, TextLineType t);
  TextLineType GetSimpleTLT() const;
public:
  void ResetTextLine(TGraphDraw *drw);
  TChPlayer *GetPlayer(int k) const {return player[k];}
  void SetPlayer(int k, TChPlayer *pl) {player[k] = pl;}
  int GetBottomColor() const {return BottomColor;}
  void SetBottomColor(int c) {BottomColor = c & 1;}
public:
  enum {textlineh = 14, min_brd = 120, max_delt = 20, min_brdsize = 70};
  int GetTextLineY() const;
  TSomeDraw *GetSomeDraw() const {return some_draw;}
  void SetSomeDraw(TSomeDraw *drw) {some_draw = drw;}
  TIntPoint GetMinWSize() const {return min_wsize;}
  void SetMinWSize(TIntPoint mws) {min_wsize = mws; Resize(width, height);}
  void SetMinWSize(int w, int h) {SetMinWSize(TIntPoint(w, h));}
  void DrawTimer(TGraphDraw *drw, double t, int wh = 0) const;
  int GetNumMove() const {return MainPlay.GetN() - 1;}
  int GetCurMoveN() const {return CurMoveN;}
  int SetCurMoveN(int n, TGraphDraw *drw = 0);
  int SetPlay(const PlayWrite &play);
  PlayWrite GetPlay() const {return MainPlay;}
  int GetPViewStatus() const {return IsPlayView;}
  void SetPViewStatus(int pv) {IsPlayView = pv;}
  void GoToCurMove();
  int GetGameEnd() const {return game_end;}
  void EraseHistory() {hist_inited = 0;}
  int ReinitHistory();
protected:
  double dw_delt, dw_cell;
  int width, height;
  TProjectiveMap PoleMap;
  Position MainPos;
  int BottomColor;
  unsigned char TheMove[NUM_CELL];
  unsigned char Eaten[NUM_CELL];
  unsigned char BecameD;
  TIntPoint CurPoint;
  int game_end;
  int check_resize;
  TIntPoint delta_size;
  TextLineType text_line_type;
  PlayWrite MainPlay;
  TChPlayer *player[2];
  TSomeDraw *some_draw;
  TIntPoint min_wsize;
  int CurMoveN, IsPlayView;
  THistory history;
  int hist_inited;
};

inline void TSomeDraw::DrawB(TGraphDraw *drw, TChBoard &brd)
{
  Draw(drw, brd.GetW(), brd.GetH());
}

void TChBoard::MoveErase()
{
  TheMove[0] = 0;
  Eaten[0] = 0;
  BecameD = 0;
}

TChBoard::TChBoard(int id) : history(id)
{
  InitBoardData();
  player[0] = player[1] = 0;
  dw_delt = 0.3;
  dw_cell = 1;
  BottomColor = 0;
  check_resize = 0;
  delta_size.x = 0; delta_size.y = 0;
  some_draw = 0;
  min_wsize.x = 80; min_wsize.y = 80;
  ClearWH();
  Resize(400, 400);
  NewGame();
}

TProjectiveMap TChBoard::Get4PMap() const
{
  return TProjectiveMap().Set4(TRealPoint(-dw_delt, -dw_delt),
                  TRealPoint(NW_CELL*dw_cell + dw_delt, -dw_delt),
                  TRealPoint(NW_CELL*dw_cell + dw_delt, NW_CELL*dw_cell + dw_delt),
                  TRealPoint(-dw_delt, NW_CELL*dw_cell + dw_delt));
}

void TChBoard::Resize(int w, int h)
{
  width = w; height = h;
  if (width < min_wsize.x) width = min_wsize.x;
  if (height < min_wsize.y) height = min_wsize.y;
  for (;;)
  {
    if (some_draw)
    {
      TIntPoint dsz = some_draw->GetDSize(width, height);
      if (dsz.x >= 0 && dsz.y >= 0) delta_size = dsz;
    }
    int change = 0;
    if (width < delta_size.x + min_brdsize)
    {
      width = delta_size.x + min_brdsize;
      change++;
    }
    if (height < delta_size.y + min_brdsize + textlineh)
    {
      height = delta_size.y + min_brdsize + textlineh;
      change++;
    }
    if (!some_draw || !change) break;
  }
  double sx = max_delt, dx = width - 2*sx - delta_size.x;
  double sy = max_delt, dy = height - 2*sy - delta_size.y - textlineh;
  if (dy < min_brd)
  {
    double d = (min_brd - dy) / 2;
    if (d > sy) d = sy;
    sy -= d; dy += 2*d;
  }
  if (dx < min_brd)
  {
    double d = (min_brd - dx) / 2;
    if (d > sx) d = sx;
    sx -= d; dx += 2*d;
  }
  if (dy > dx) {sy += (dy - dx) / 2; dy = dx;}
  double tx = (dx - dy * dy / dx) / 3;
  PoleMap.Set4(TRealPoint(sx, sy + dy - 70*ssf), TRealPoint(sx + dx, sy + dy),
               TRealPoint(sx + dx - tx, sy), TRealPoint(sx + tx, sy + 40*ssf));
  PoleMap /= Get4PMap();
}

TIntPoint TChBoard::PlToWin(double x, double y) const
{
  if (BottomColor == 1) y = NW_CELL*dw_cell - y;
  else x = NW_CELL*dw_cell - x;
  /**/if (ssf) y += 0.6 * sin(x);/**/
  return PoleMap(x, y).pnt();
}

TRealPoint TChBoard::WinToPl(int x, int y) const
{
  TRealPoint rpnt;
  if (PoleMap.Reversed(x, y).rpnt(rpnt))
  {
    /**/if (ssf) rpnt.y -= 0.6 * sin(rpnt.x);/**/
    if (BottomColor == 1) rpnt.y = NW_CELL*dw_cell - rpnt.y;
    else rpnt.x = NW_CELL*dw_cell - rpnt.x;
    return rpnt;
  }
  else return TRealPoint(-dw_cell - dw_delt, -dw_cell - dw_delt);
}

void TChBoard::NewGame()
{
  int k;
  MoveErase();
  MainPos.Init();
  MainPlay.Clear();
  for (k = 0; k < NW_CELL * 3 / 2; k++) MainPos.SH[k] = 1;
  for (k = 0; k < NW_CELL * 3 / 2; k++) MainPos.SH[NUM_CELL - k - 1] = 2;
  MainPlay.Add(0, MainPos);
  CurMoveN = 0;
  text_line_type = TLT_Main;
  game_end = 0;
  CurPoint.x = -1; CurPoint.y = -1;
  IsPlayView = 0;
  printf("\nCheckers: New game.\n");
  EraseHistory();
}

int TChBoard::GetTextLineY() const
{
  int y = height - (max_delt + textlineh + delta_size.y);
  int i, j;
  for (i = 0; i <= 1; i++) for (j = 0; j <= 1; j++)
  {
    TRealPoint corner;
    corner.x = (2*i - 1) * dw_delt + i * NW_CELL * dw_cell;
    corner.y = (2*j - 1) * dw_delt + j * NW_CELL * dw_cell;
    TIntPoint wcr = PlToWin(corner);
    if (wcr.x != INT_MIN && wcr.y != INT_MIN)
    {
      if (y < wcr.y) y = wcr.y;
    }
  }
  y -= textlineh;
  if (y > height - delta_size.y - 2*textlineh)
  {
    y = height - delta_size.y - 2*textlineh;
  }
  return (y + height - delta_size.y) / 2;
}

#ifdef BUILD_RUS
#define aCheckersGame   "Игра в шашки."
#define aRedMoves       "Красные ходят."
#define aBlueMoves      "Синие ходят."
#define aWait           "Подождите."
#define aRedWins        "Красные выиграли."
#define aBlueWins       "Синие выиграли."
#define aDraw           "Ничья."
#define aRed            "Красные "
#define aBlue           "Синие "
#define aMadeWrongMove  "сделали неправильный ход."
#define aNoMove         "не смогли сделать ход."
#define aEndGame        "Конец игры."
#define aYouWin         " (Вы выиграли)"
#define aYouLose        " (Вы проиграли)"
#define aRedWin         " (Красные выиграли)"
#define aBlueWin        " (Синие выиграли)"
#define aWrongMove      "Неверный ход."
#define aNotYourChecker "Это не ваша шашка."
#define aFreeCell       "Эта клетка пустая."
#define aNotFreeCell    "Шашки могут ходить только на свободные клетки."
#define aMustEat        "Вы должны взять шашку."
#define aMustEatMore    "Вы должны взять ещё шашку."
#define aCheckerNoMove  "У этой шашки нет ходов."
#define aNoBack         "Шашки не ходят назад."
#define aNotDamka       "Это не дамка."
#define aOnlyDiag       "Шашки ходят только по диагонали."
#define aEatYour        "Шашки не могут есть своих."
#define aMoreOne        "Вы не можете взять более одной шашки за раз."
#define aNoTurnBack     "Шашки не могут разворачиваться назад при взятии."
#else
#define aCheckersGame   "The checkers game."
#define aRedMoves       "Red moves."
#define aBlueMoves      "Blue moves."
#define aWait           "Please wait."
#define aRedWins        "Red wins."
#define aBlueWins       "Blue wins."
#define aDraw           "Draw."
#define aRed            "Red "
#define aBlue           "Blue "
#define aMadeWrongMove  "make a wrong move."
#define aNoMove         "could not make a move."
#define aEndGame        "End of the game."
#define aYouWin         " (You win)"
#define aYouLose        " (You lose)"
#define aRedWin         " (Red win)"
#define aBlueWin        " (Blue win)"
#define aWrongMove      "Wrong move."
#define aNotYourChecker "It isn't your checker."
#define aFreeCell       "The cell is free."
#define aNotFreeCell    "Checkers may be moved only on a free cell."
#define aMustEat        "You must eat a checker."
#define aMustEatMore    "You must eat more checkers."
#define aCheckerNoMove  "The checker have no moves."
#define aNoBack         "Checkers may not be moved back."
#define aNotDamka       "It is not a damka."
#define aOnlyDiag       "Checkers may be moved only along diagonal."
#define aEatYour        "You may not eat your checkers."
#define aMoreOne        "You may not eat more than one checker at a time."
#define aNoTurnBack     "Checkers may not turn back when eating."
#endif

void TChBoard::GetTextLine(char str[]) const
{
  str[0] = 0;
  int g = (game_end - 1) % 2, h = 0;
  switch(text_line_type)
  {
  case TLT_Main:
    strcpy(str, aCheckersGame);
    break;
  case TLT_Move:
    if (MainPos.wmove == 0)
    {
      strcpy(str, aRedMoves);
    }
    else strcpy(str, aBlueMoves);
    break;
  case TLT_Wait:
    strcpy(str, aWait);
    break;
  case TLT_GameEnd:
  case TLT_PlDidntMove:
  case TLT_PlWrongMove:
    if (!game_end) break;
    if (text_line_type == TLT_GameEnd)
    {
      if (game_end == 1) strcpy(str, aRedWins);
      else if (game_end == 2) strcpy(str, aBlueWins);
      else if (game_end == 5) strcpy(str, aDraw);
    }
    if (!str[0])
    {
      if (game_end >= 1 && game_end <= 4)
      {
        if (g == 0) strcpy(str, aRed);
        else strcpy(str, aBlue);
        if (text_line_type == TLT_PlWrongMove)
        {
          strcat(str, aMadeWrongMove);
        }
        else strcat(str, aNoMove);
        h = 1;
      }
      else strcpy(str, aEndGame);
    }
    if (game_end >= 1 && game_end <= 4)
    {
      if (!IsPlayView && player[1-g] && !player[g])
      {
        strcat(str, aYouWin);
      }
      else if (!IsPlayView && player[g] && !player[1-g])
      {
        strcat(str, aYouLose);
      }
      else if (h && g == 0) strcat(str, aRedWin);
      else if (h && g == 1) strcat(str, aBlueWin);
    }
    break;
  case TLT_WrongMove:
    strcpy(str, aWrongMove);
    break;
  case TLT_WrongColor:
    strcpy(str, aNotYourChecker);
    break;
  case TLT_WfCell:
    strcpy(str, aFreeCell);
    break;
  case TLT_WnfCell:
    strcpy(str, aNotFreeCell);
    break;
  case TLT_WMustEat:
    strcpy(str, aMustEat);
    break;
  case TLT_WMustEatMore:
  case TLT_WMustEatMoreD:
    strcpy(str, aMustEatMore);
    break;
  case TLT_WNoMove:
    strcpy(str, aCheckerNoMove);
    break;
  case TLT_WChBack:
    strcpy(str, aNoBack);
    break;
  case TLT_WNotDm:
  case TLT_WNotDmE:
    strcpy(str, aNotDamka);
    break;
  case TLT_WOnlyDiag:
    strcpy(str, aOnlyDiag);
    break;
  case TLT_WEatYour:
    strcpy(str, aEatYour);
    break;
  case TLT_WMoreOne:
    strcpy(str, aMoreOne);
    break;
  case TLT_WTurnBack:
    strcpy(str, aNoTurnBack);
    break;
  }
}

void TChBoard::DrawTextLine(TGraphDraw *drw) const
{
  if (!drw || !drw->IsDraw()) return;
  char str[100];
  GetTextLine(str);
  if (str[0]) drw->DrawText(10, GetTextLineY(), str);
}

void TChBoard::ResetTextLine(TGraphDraw *drw)
{
  if (!game_end && text_line_type != TLT_Move) ChangeTLT(drw, TLT_Move);
}

void TChBoard::LineB(TGraphDraw *drw, double x1, double y1, double x2, double y2) const
{
  if (!drw || !drw->IsDraw()) return;
  TIntPoint p1 = PlToWin(x1, y1), p2 = PlToWin(x2, y2);
  if (p1.x != INT_MIN && p1.y != INT_MIN && p2.x != INT_MIN && p2.y != INT_MIN)
  {
    drw->DrawLine(p1.x, p1.y, p2.x, p2.y);
  }
}

void TChBoard::DrawCh(TGraphDraw *drw, double x0, double y0, const TRealPoint pnt[], int npnt, int L) const
{
  if (!drw || !drw->IsDraw()) return;
  int i, j;
  for (j = 0; j < L; j++)
  {
    for (i = 0; i < npnt - 1; i++)
    {
      LineB(drw, x0 + pnt[i].x * dw_cell * r_ch_lines[j] / 2,
                 y0 + pnt[i].y * dw_cell * r_ch_lines[j] / 2,
                 x0 + pnt[i+1].x * dw_cell * r_ch_lines[j] / 2,
                 y0 + pnt[i+1].y * dw_cell * r_ch_lines[j] / 2);
    }
  }
}

void TChBoard::DrawIL(TGraphDraw *drw, double x0, double y0, int L) const
{
  if (!drw || !drw->IsDraw()) return;
  int i, j;
  if (L == 0 || L == 1)
  {
    const int numDST[2] = {2, 4};
    const double DST[2][4] = {{0.96, 0.88}, {0.8, 0.76, 0.72, 0.68}};
    const int MULT[4][4] = {{-1, -1, -1, 1}, {-1, 1, 1, 1},
                            {-1, -1, 1, -1}, {1, -1, 1, 1}};
    for (i = 0; i < numDST[L]; i++) for (j = 0; j < 4; j++)
    {
      LineB(drw, x0 + dw_cell * DST[L][i] * MULT[j][0] / 2,
                 y0 + dw_cell * DST[L][i] * MULT[j][1] / 2,
                 x0 + dw_cell * DST[L][i] * MULT[j][2] / 2,
                 y0 + dw_cell * DST[L][i] * MULT[j][3] / 2);
    }
  }
  else if (L == 2)
  {
    const double DP[] = {0.85, 0.90, 0.95};
    const int numDP = NELEM(DP);
    for (i = 0; i < numDP; i++) for (j = -1; j <= 1; j += 2)
    {
      LineB(drw, x0 - j * dw_cell * DP[i] / 2,
                 y0 - dw_cell * DP[numDP - i - 1] / 2,
                 x0 + j * dw_cell * DP[numDP - i - 1] / 2,
                 y0 + dw_cell * DP[i] / 2);
    }
  }
}

void TChBoard::Draw(TGraphDraw *drw)
{
  if (!drw || !drw->IsDraw()) return;
  if (CheckResize(drw)) drw->DrawClear();
  int i, j, k, kn;
  unsigned long black = drw->GetBlackColor();
  unsigned long red = drw->CreateColor(65535u, 0, 0);
  unsigned long green = drw->CreateColor(0, 49151u, 0);
  unsigned long blue = drw->CreateColor(0, 0, 65535u);
  drw->SetColor(black);
  for (i = -1; i <= NW_CELL + 1; i++) for (j = -1; j <= NW_CELL; j++)
  {
    if (i < 0 || i > NW_CELL || j >= 0 && j < NW_CELL)
    {
      double mval = dw_cell * NW_CELL + dw_delt;
      double x, y0, y1;
      if (i < 0) x = -dw_delt;
      else if (i > NW_CELL) x = mval;
      else x = i * dw_cell;
      if (j < 0) y0 = -dw_delt;
      else if (j > NW_CELL) y0 = mval;
      else y0 = j * dw_cell;
      if ((j+1) < 0) y1 = -dw_delt;
      else if ((j+1) > NW_CELL) y1 = mval;
      else y1 = (j+1) * dw_cell;
      LineB(drw, x, y0, x, y1);
    }
  }
  for (i = -1; i <= NW_CELL; i++) for (j = -1; j <= NW_CELL + 1; j++)
  {
    if (j < 0 || j > NW_CELL || i >= 0 && i < NW_CELL)
    {
      double mval = dw_cell * NW_CELL + dw_delt;
      double x0, x1, y;
      if (i < 0) x0 = -dw_delt;
      else if (i > NW_CELL) x0 = mval;
      else x0 = i * dw_cell;
      if ((i+1) < 0) x1 = -dw_delt;
      else if ((i+1) > NW_CELL) x1 = mval;
      else x1 = (i+1) * dw_cell;
      if (j < 0) y = -dw_delt;
      else if (j > NW_CELL) y = mval;
      else y = j * dw_cell;
      LineB(drw, x0, y, x1, y);
    }
  }
  for (i = -1; i <= 1; i += 2)
  {
    drw->SetColor((i < 0) ? red : blue);
    for (j = -1; j <= 1; j += 2)
    {
      double c = dw_cell * NW_CELL / 2;
      double d = (dw_cell/2 > dw_delt) ? dw_delt : dw_cell/2;
      LineB(drw, c + j * (c + 0.5*d), c + i * (c - 0.5*d),
                  c + j * (c + 0.5*d), c + i * (c - 3.5*d));
      LineB(drw, c + j * (c + 0.5*d), c + i * (c - 3.5*d),
                  c + j * (c + 0.2*d), c + i * (c - 2.5*d));
      LineB(drw, c + j * (c + 0.5*d), c + i * (c - 3.5*d),
                  c + j * (c + 0.8*d), c + i * (c - 2.5*d));
      LineB(drw, c + j * (c + 0.2*d), c + i * (c - 2.5*d),
                  c + j * (c + 0.8*d), c + i * (c - 2.5*d));
    }
  }
  for (i = 0; i < NW_CELL; i++) for (j = 0; j < NW_CELL; j++)
  {
    if (!PoleCpos(i, j))
    {
      drw->SetColor(black);
      LineB(drw, (i+0.33) * dw_cell, j * dw_cell, (i+0.33) * dw_cell, (j+1) * dw_cell);
      LineB(drw, (i+0.67) * dw_cell, j * dw_cell, (i+0.67) * dw_cell, (j+1) * dw_cell);
      LineB(drw, i * dw_cell, (j+0.33) * dw_cell, (i+1) * dw_cell, (j+0.33) * dw_cell);
      LineB(drw, i * dw_cell, (j+0.67) * dw_cell, (i+1) * dw_cell, (j+0.67) * dw_cell);
      LineB(drw, i * dw_cell, j * dw_cell, (i+1) * dw_cell, (j+1) * dw_cell);
      LineB(drw, (i+1) * dw_cell, j * dw_cell, i * dw_cell, (j+1) * dw_cell);
    }
    else
    {
      k = PoleToNum(i, j);
      kn = MainPos.SH[k];
      if (TheMove[0] > 0 && k == TheMove[1])
      {
        if (kn == 1 || kn == 2) kn = 5;
        else if (kn == 3 || kn == 4) kn = 6;
      }
      if (TheMove[0] > 0 && k == TheMove[TheMove[0]])
      {
        kn = MainPos.SH[TheMove[1]];
        if (kn <= 2 && BecameD) kn += 2;
      }
      if (kn == 1)
      {
        drw->SetColor(red);
        DrawCh(drw, (i+0.5)*dw_cell, (j+0.5)*dw_cell,
                     r_circle_pnt, NELEM(r_circle_pnt));
      }
      else if (kn == 2)
      {
        drw->SetColor(blue);
        DrawCh(drw, (i+0.5)*dw_cell, (j+0.5)*dw_cell,
                     r_circle_pnt, NELEM(r_circle_pnt));
      }
      else if (kn == 3)
      {
        drw->SetColor(red);
        DrawCh(drw, (i+0.5)*dw_cell, (j+0.5)*dw_cell,
                     r_star_pnt, NELEM(r_star_pnt));
      }
      else if (kn == 4)
      {
        drw->SetColor(blue);
        DrawCh(drw, (i+0.5)*dw_cell, (j+0.5)*dw_cell,
                     r_star_pnt, NELEM(r_star_pnt));
      }
      else if (kn == 5)
      {
        drw->SetColor(green);
        DrawCh(drw, (i+0.5)*dw_cell, (j+0.5)*dw_cell,
                  r_circle_pnt, NELEM(r_circle_pnt), 2);
      }
      else if (kn == 6)
      {
        drw->SetColor(green);
        DrawCh(drw, (i+0.5)*dw_cell, (j+0.5)*dw_cell,
                  r_circle_pnt, NELEM(r_circle_pnt), 2);
      }
    }
  }
  for (k = 1; k <= Eaten[0]; k++)
  {
    if (TheMove[0] <= 0 || Eaten[k] != TheMove[TheMove[0]])
    {
      NumToPole(Eaten[k], i, j);
      kn = MainPos.SH[Eaten[k]];
      if (kn)
      {
        if (kn == 1 || kn == 3) drw->SetColor(blue);
        else if (kn == 2 || kn == 4) drw->SetColor(red);
        DrawIL(drw, (i+0.5)*dw_cell, (j+0.5)*dw_cell, 2);
      }
    }
  }
  if (TheMove[0] > 0)
  {
    NumToPole(TheMove[TheMove[0]], i, j);
    drw->SetColor(green);
    DrawIL(drw, (i+0.5)*dw_cell, (j+0.5)*dw_cell, 1);
  }
  if (CurPoint.x >= 0 && CurPoint.y >= 0)
  {
    drw->SetColor(green);
    DrawIL(drw, (CurPoint.x+0.5)*dw_cell, (CurPoint.y+0.5)*dw_cell, 0);
  }
  drw->SetColor(black);
  DrawTextLine(drw);
  if (some_draw) some_draw->DrawB(drw, *this);
  drw->FreeColor(red);
  drw->FreeColor(green);
  drw->FreeColor(blue);
}

int TChBoard::ResizeDraw(TGraphDraw *drw, int w, int h)
{
  int w0 = width, h0 = height;
  Resize(w, h);
  if (drw && drw->IsDraw() && (width != w0 || height != h0))
  {
    drw->DrawClear();
    Draw(drw);
    return 1;
  }
  else return 0;
}

int TChBoard::CheckResize(TGraphDraw *drw)
{
  if (!drw || !check_resize) return 0;
  int w, h;
  drw->GetSize(w, h);
  if (w < 0 || h < 0) return 0;
  int w0 = width, h0 = height;
  Resize(w, h);
  return width != w0 || height != h0;
}

void TChBoard::CurPointClear(TGraphDraw *drw) const
{
  if (!drw || !drw->IsDraw()) return;
  drw->SetColor(drw->GetWhiteColor());
  DrawIL(drw, (CurPoint.x+0.5)*dw_cell, (CurPoint.y+0.5)*dw_cell, 0);
}

int TChBoard::MouseClick(TGraphDraw *drw, int x, int y)
{
  TRealPoint rpnt = WinToPl(x, y);
  if (rpnt.x < -dw_delt || rpnt.y < -dw_delt ||
      rpnt.x > NW_CELL*dw_cell + dw_delt ||
      rpnt.y > NW_CELL*dw_cell + dw_delt) return -1;
  if (CurPoint.x >= 0 && CurPoint.y >= 0)
  {
    CurPointClear(drw);
    CurPoint.x = -1; CurPoint.y = -1;
    Draw(drw);
  }
  ResetTextLine(drw);
  if (AutoMove(drw)) return 3;
  if (!CanPlayerMove()) return 0;
  int i = (int)floor(rpnt.x / dw_cell), j = (int)floor(rpnt.y / dw_cell);
  if (i < 0 || j < 0 || i >= NW_CELL || j >= NW_CELL) return 0;
  if (!PoleCpos(i, j)) return 1;
  return 2 + CMove(drw, i, j);
}

TChBoard::PKey TChBoard::GetKeySide(PKey key)
{
  if (key != PLeft && key != PRight && key != PUp && key != PDown) return key;
  TIntPoint a0 = PlToWin(4*dw_cell, 0);
  TIntPoint a1 = PlToWin(4*dw_cell, 8*dw_cell);
  TIntPoint b0 = PlToWin(0, 4*dw_cell);
  TIntPoint b1 = PlToWin(8*dw_cell, 4*dw_cell);
  double ax = a0.x - a1.x, ay = a0.y - a1.y;
  double bx = b0.x - b1.x, by = b0.y - b1.y;
  double t;
  if (a0.x == INT_MIN || a0.y == INT_MIN || a1.x == INT_MIN ||
      a1.y == INT_MIN || fabs(ax) < 0.5 && fabs(ay) < 0.5)
  {
    ax = 0, ay = 1;
  }
  if (b0.x == INT_MIN || b0.y == INT_MIN || b1.x == INT_MIN ||
      b1.y == INT_MIN || fabs(bx) < 0.5 && fabs(by) < 0.5)
  {
    bx = 1, by = 0;
  }
  t = fabs(ax) + fabs(ay); ax /= t; ay /= t;
  t = fabs(bx) + fabs(by); bx /= t; by /= t;
  if (fabs(ax) <= fabs(bx))
  {
    if (key == PLeft) return (bx > 0) ? PRight : PLeft;
    if (key == PRight) return (bx > 0) ? PLeft : PRight;
    if (key == PUp) return (ay > 0) ? PDown : PUp;
    if (key == PDown) return (ay > 0) ? PUp : PDown;
  }
  else
  {
    if (key == PLeft) return (ax > 0) ? PDown : PUp;
    if (key == PRight) return (ax > 0) ? PUp : PDown;
    if (key == PUp) return (by > 0) ? PRight : PLeft;
    if (key == PDown) return (by > 0) ? PLeft : PRight;
  }
  return PNull;
}

int TChBoard::PKeyEvent(TGraphDraw *drw, PKey key)
{
  ResetTextLine(drw);
  if (AutoMove(drw)) return 3;
  if (!CanPlayerMove()) return 0;
  key = GetKeySide(key);
  if (CurPoint.x < 0 || CurPoint.y < 0 ||
      CurPoint.x >= NW_CELL || CurPoint.y >= NW_CELL)
  {
    CurPoint.x = NW_CELL / 2;
    CurPoint.y = NW_CELL / 2;
    Draw(drw);
    return 1;
  }
  if (key == PEnter) return 2 + CMove(drw, CurPoint.x, CurPoint.y);
  if (drw && drw->IsDraw() && CheckResize(drw))
  {
    drw->DrawClear();
    Draw(drw);
  }
  if (key == PLeft)
  {
    if (CurPoint.x == 0) return 0;
    CurPointClear(drw);
    CurPoint.x--;
    Draw(drw);
    return 1;
  }
  else if (key == PRight)
  {
    if (CurPoint.x == NW_CELL - 1) return 0;
    CurPointClear(drw);
    CurPoint.x++;
    Draw(drw);
    return 1;
  }
  else if (key == PUp)
  {
    if (CurPoint.y == 0) return 0;
    CurPointClear(drw);
    CurPoint.y--;
    Draw(drw);
    return 1;
  }
  else if (key == PDown)
  {
    if (CurPoint.y == NW_CELL - 1) return 0;
    CurPointClear(drw);
    CurPoint.y++;
    Draw(drw);
    return 1;
  }
  else return 0;
}

void TChBoard::UnMove(TGraphDraw *drw)
{
  MoveErase();
  if (drw && drw->IsDraw())
  {
    drw->DrawClear();
    Draw(drw);
  }
}

TChBoard::TextLineType TChBoard::GetTLTfromA(int s) const
{
  if (s >= 0) return TLT_Main;
  switch(s)
  {
    case Position::AWColor: return TLT_WrongColor;
    case Position::AfCell: return TLT_WfCell;
    case Position::AnfCell: return TLT_WnfCell;
    case Position::AMustEat: return TLT_WMustEat;
    case Position::AMustEatMore: return TLT_WMustEatMore;
    case Position::AMustEatMoreD: return TLT_WMustEatMoreD;
    case Position::ANoMove: return TLT_WNoMove;
    case Position::AChBack: return TLT_WChBack;
    case Position::ANotDm: return TLT_WNotDm;
    case Position::ANotDmE: return TLT_WNotDmE;
    case Position::AOnlyDiag: return TLT_WOnlyDiag;
    case Position::AEatYour: return TLT_WEatYour;
    case Position::AMoreOne: return TLT_WMoreOne;
    case Position::ATurnBack: return TLT_WTurnBack;
    default: return TLT_WrongMove;
  }
}

void TChBoard::ChangeTLT(TGraphDraw *drw, TextLineType t)
{
  if (text_line_type == t) return;
  if (drw && drw->IsDraw())
  {
    drw->SetColor(drw->GetWhiteColor());
    DrawTextLine(drw);
  }
  text_line_type = t;
  if (drw && drw->IsDraw())
  {
    drw->SetColor(drw->GetBlackColor());
    DrawTextLine(drw);
  }
}

TChBoard::TextLineType TChBoard::GetSimpleTLT() const
{
  if (game_end && CurMoveN >= MainPlay.GetN() - 1) return TLT_GameEnd;
  else return TLT_Move;
}

inline int TChBoard::ReinitHistory()
{
  if (history.Play(MainPlay)) {hist_inited = 1; return 1;}
  else {hist_inited = 0; return 0;}
}

void TChBoard::PrintLMove()
{
  PlayWrite::PMv pmv;
  if (MainPlay.GetMoveL(pmv.mv) >= 0)
  {
    MainPlay.GetPosL(pmv.pos, 1);
    char *s = new char[pmv.pos.GetLenMvEx(pmv.mv, 11)];
    if (s)
    {
      pmv.pos.WriteMvEx(pmv.mv, s, 11);
      printf("Checkers: %s%s\n", (pmv.pos.wmove == 1) ? "..." : "", s);
      delete[] s;
    }
    if (!hist_inited) ReinitHistory();
    else history.Move(pmv.pos, pmv.mv, MainPlay.GetN() - 1);
  }
}

int TChBoard::CMove(TGraphDraw *drw, int x, int y)
{
  if (AutoMove(drw)) return 1;
  if (!CanPlayerMove()) return 0;
  if (!game_end && text_line_type != TLT_Move) ChangeTLT(drw, TLT_Move);
  int k = PoleToNum(x, y), s;
  if (TheMove[0] > 0)
  {
    if (!PoleCpos(x, y) || k == TheMove[TheMove[0]]) {UnMove(drw); return 1;}
    int e = 1;
    s = MainPos.AMove(TheMove, k, e);
    if (s < 0)
    {
      ChangeTLT(drw, GetTLTfromA(s));
      return 0;
    }
    if (s < NUM_CELL) Eaten[++Eaten[0]] = (unsigned char)s;
    TheMove[++TheMove[0]] = (unsigned char)k;
    BecameD = BecameD || MainPos.BecameD(k, MainPos.SH[TheMove[1]]);
    if (e == 0)
    {
      if (MainPlay.Add(TheMove) != 0)
      {
        ChangeTLT(drw, TLT_WrongMove);
        return 0;
      }
      CurMoveN = MainPlay.GetN() - 1;
      MoveErase();
      RenewMPos();
      PrintLMove();
      if (AutoMove(drw)) return 1;
    }
    if (drw && drw->IsDraw())
    {
      drw->DrawClear();
      Draw(drw);
    }
    return 1;
  }
  else
  {
    if (!PoleCpos(x, y)) return 0;
    s = MainPos.AChCell(k);
    if (s != 1)
    {
      ChangeTLT(drw, GetTLTfromA(s));
      return 0;
    }
    TheMove[0] = 1;
    TheMove[1] = (unsigned char)k;
    Draw(drw);
    return 1;
  }
}

void TChBoard::SetNoMove(TGraphDraw *drw, int force)
{
  if (!force && CurPoint.x < 0 && CurPoint.y < 0 && TheMove[0] == 0 && Eaten[0] == 0)
  {
    return;
  }
  CurPoint.x = -1; CurPoint.y = -1;
  MoveErase();
  if (drw && drw->IsDraw())
  {
    drw->DrawClear();
    Draw(drw);
  }
}

int TChBoard::AutoMove(TGraphDraw *drw, int nmove, int draw_check)
{
  if (game_end || IsPlayView) {SetNoMove(drw, 0); return 0;}
  if (CurMoveN < MainPlay.GetN() - 1)
  {
    CurMoveN = MainPlay.GetN() - 1;
    RenewMPos();
    SetNoMove(drw);
    return 2;
  }
  if (!MainPos.AllCanEat() && !MainPos.AllCanMove())
  {
    game_end = 2 - MainPos.wmove;
    ChangeTLT(drw, TLT_GameEnd);
    if (!player[game_end - 1]) printf("Checkers: You win.\n");
    else printf("Checkers: You lose.\n");
    SetNoMove(drw);
    return 3;
  }
  else if (draw_check > 0 && MainPlay.IsDraw())
  {
    game_end = 5;
    ChangeTLT(drw, TLT_GameEnd);
    printf("Checkers: Draw.\n");
    SetNoMove(drw);
    return 3;
  }
  if (!player[MainPos.wmove]) return 0;
  TIntPoint CurP0 = CurPoint;
  if (CurPoint.x >= 0 && CurPoint.y >= 0)
  {
    if (drw) CurPointClear(drw);
    CurPoint.x = -1; CurPoint.y = -1;
  }
  MoveErase();
  int k;
  for (k = 0; player[MainPos.wmove] && (k < nmove || nmove == 0); k++)
  {
    TChPlayer::PMv pmv;
    pmv.pos = MainPos;
    Position::SetNullMv(pmv.mv);
    MainPlay.GetMoveL(pmv.mv);
    text_line_type = TLT_Move;
    if (!player[MainPos.wmove]->Move(pmv))
    {
      text_line_type = TLT_PlDidntMove;
      game_end = 4 - MainPos.wmove;
      break;
    }
    if (MainPlay.Add(pmv.mv) != 0)
    {
      text_line_type = TLT_PlWrongMove;
      game_end = 4 - MainPos.wmove;
      break;
    }
    CurMoveN = MainPlay.GetN() - 1;
    MoveErase();
    RenewMPos();
    PrintLMove();
    if (!MainPos.AllCanEat() && !MainPos.AllCanMove())
    {
      game_end = 2 - MainPos.wmove;
      text_line_type = TLT_GameEnd;
      if (!player[game_end - 1]) printf("Checkers: You win.\n");
      else printf("Checkers: You lose.\n");
      break;
    }
    else if (draw_check >= 0 && MainPlay.IsDraw())
    {
      game_end = 5;
      text_line_type = TLT_GameEnd;
      printf("Checkers: Draw.\n");
      break;
    }
  }
  if (!game_end)
  {
    text_line_type = TLT_Move;
    CurPoint = CurP0;
  }
  if (drw && drw->IsDraw())
  {
    drw->DrawClear();
    Draw(drw);
  }
  return 1;
}

void TChBoard::DrawTimer(TGraphDraw *drw, double t, int wh) const
{
  if (!drw || !drw->IsDraw()) return;
  if (wh) drw->SetColor(drw->GetWhiteColor());
  else drw->SetColor(drw->GetBlackColor());
  double r1 = dw_delt * 0.4, r2 = dw_delt * 0.45;
  double x = t * dw_cell * NW_CELL, y = -dw_delt / 2;
  if (MainPos.wmove == 1)
  {
    x = dw_cell * NW_CELL - x;
    y = dw_cell * NW_CELL - y;
  }
  LineB(drw, x - r1, y - r2, x + r2, y + r1);
  LineB(drw, x - r2, y - r1, x + r1, y + r2);
  LineB(drw, x - r1, y + r2, x + r2, y - r1);
  LineB(drw, x - r2, y + r1, x + r1, y - r2);
  if (wh)
  {
    int i, j, jj;
    drw->SetColor(drw->GetBlackColor());
    for (i = -1; i <= NW_CELL; i++)
    {
      double mval = dw_cell * NW_CELL + dw_delt;
      double x0, x1, y;
      if (i < 0) x0 = -dw_delt;
      else if (i > NW_CELL) x0 = mval;
      else x0 = i * dw_cell;
      if ((i+1) < 0) x1 = -dw_delt;
      else if ((i+1) > NW_CELL) x1 = mval;
      else x1 = (i+1) * dw_cell;
      if (fabs(x0 - x) < dw_delt || fabs(x1 - x) < dw_delt)
      {
        for (jj = 0; jj <= 1; jj++)
        {
          if (MainPos.wmove == 1) j = NW_CELL + jj;
          else j = -jj;
          if (j < 0 || j > NW_CELL || i >= 0 && i < NW_CELL)
          {
            if (j < 0) y = -dw_delt;
            else if (j > NW_CELL) y = mval;
            else y = j * dw_cell;
            LineB(drw, x0, y, x1, y);
          }
        }
      }
    }
  }
}

int TChBoard::SetCurMoveN(int n, TGraphDraw *drw)
{
  int nmove = MainPlay.GetN() - 1;
  if (n > nmove) n = nmove;
  if (n < 0) n = 0;
  if (CurMoveN != n)
  {
    CurMoveN = n;
    RenewMPos();
    if (n < nmove)
    {
      MoveErase();
      CurPoint.x = -1; CurPoint.y = -1;
    }
    text_line_type = GetSimpleTLT();
    if (drw && drw->IsDraw())
    {
      drw->DrawClear();
      Draw(drw);
    }
    return 1;
  }
  else return 0;
}

int TChBoard::SetPlay(const PlayWrite &play)
{
  if (play.GetN() <= 0) return 0;
  MainPlay = play;
  MoveErase();
  CurPoint.x = -1; CurPoint.y = -1;
  CurMoveN = INT_MIN;
  SetCurMoveN(play.GetN());
  if (!MainPos.AllCanEat() && !MainPos.AllCanMove()) game_end = 2 - MainPos.wmove;
  else if (MainPlay.IsDraw()) game_end = 5;
  else game_end = 0;
  text_line_type = GetSimpleTLT();
  IsPlayView = 1;
  EraseHistory();
  return 1;
}

void TChBoard::GoToCurMove()
{
  if (!MainPos.AllCanEat() && !MainPos.AllCanMove() ||
       MainPlay.IsDraw(CurMoveN + 1)) return;
  MainPlay.ClearFrom(CurMoveN + 1);
  MoveErase();
  game_end = 0;
  text_line_type = GetSimpleTLT();
  IsPlayView = 0;
}

#endif  //_HEADER_BOARD_H
