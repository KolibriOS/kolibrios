#ifndef _HEADER_POSITION_H
#define _HEADER_POSITION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NELEM(a) (sizeof(a) / sizeof((a)[0]))

const int NW_CELL = 8;

const int NUM_CELL = NW_CELL * NW_CELL / 2;
const int LEN_WPOS = (NUM_CELL + 2) / 3;

inline int PoleCpos(int i, int j)
{
  return (i + j) % 2 != 0;
}

inline int PoleToNum(int i, int j)
{
  return j * (NW_CELL / 2) + i/2;
}

inline void NumToPole(int k, int &i, int &j)
{
  j = k / (NW_CELL / 2);
  i = k % (NW_CELL / 2);
  i *= 2;
  if (j % 2 == 0) i++;
}

class Position
{
public:
  char SH[NUM_CELL];
  char wmove;

  Position() {Init();}
  Position(const Position &p);
  Position& operator=(const Position &p);

  void Init();
  int IsNull() const;
  void Add(int np, char sh) {SH[np] = sh;}
  void Del(int np) {SH[np] = 0;}
  void Move(int np0, int np1) {if (np0 != np1) {SH[np1] = SH[np0]; SH[np0] = 0;}}
  static int BecameD(int np, char ch);

  enum {AWrong = -1, AWColor = -2, AfCell = -3, AnfCell = -4,
        AMustEatMore = -5, AMustEat = -6, ANoMove = -7, AChBack = -8,
        ANotDm = -9, AOnlyDiag = -10, AEatYour = -11, AMoreOne = -12,
        ANotDmE = -13, AMustEatMoreD = -14, ATurnBack = -15};

  int ScanSide(int x, int y, int sx, int sy, int sh_k = -1) const;
  int CanEat(int k, int psx = 0, int psy = 0, int sh_k = -1) const;
  int CanMove(int k) const;
  int AChCell(int k);
  int AMove(const unsigned char MV[], int k = -1, int &mkmove = *(int*)0);
  int AllCanEat(int w = -1) const;
  int AllCanMove(int w = -1) const;

  char *Write(char WP[], int how = 0) const;
  Position &Read(const char WP[], int how = 0);
  static char *WriteMv(const unsigned char mv[], char WP[], int how = 0);
  int WriteMvEx(const unsigned char mv[], char WP[], int how = 0) const;
  static unsigned char *ReadMv(unsigned char mv[], const char WP[], int how = 0);
  static int GetLenMv(const unsigned char mv[], int how = 0);
  int GetLenMvEx(const unsigned char mv[], int how = 0) const;
  static int GetLenMwr(const char WP[], int how = 0);
  static void SetNullMv(unsigned char mv[]) {mv[0] = 0;}
  void Reverse();
};

Position::Position(const Position &p) : wmove(p.wmove)
{
  for(int i = 0; i < NUM_CELL; i++) SH[i] = p.SH[i];
}

Position& Position::operator=(const Position &p)
{
  wmove = p.wmove;
  for(int i = 0; i < NUM_CELL; i++) SH[i] = p.SH[i];
  return *this;
}

void Position::Init()
{
  wmove = 0;
  for (int i = 0; i < NUM_CELL; i++) SH[i] = 0;
}

int Position::IsNull() const
{
  for (int i = 0; i < NUM_CELL; i++) if (SH[i] != 0) return 0;
  return 1;
}

inline int Position::BecameD(int np, char ch)
{
  int x, y;
  NumToPole(np, x, y);
  return ch == 1 && y == NW_CELL - 1 || ch == 2 && y == 0;
}

char *Position::Write(char WP[], int how) const
{
  if (how == 0)
  {
    int i = 0, j;
    for (j = 0; i < NUM_CELL; j++)
    {
      WP[j] = SH[i++];
      if (i < NUM_CELL) {WP[j] *= (char)5; WP[j] += SH[i++];}
      if (i < NUM_CELL) {WP[j] *= (char)5; WP[j] += SH[i++];}
      if (i >= NUM_CELL) {WP[j] *= (char)2; WP[j] += wmove;}
      WP[j]++;
    }
  }
  else if (how == 1)
  {
    int i;
    for (i = NUM_CELL - 1; i >= 0; i--)
    {
      if (SH[i] < 0 || SH[i] >= 5) return 0;
    }
    for (i = 0; i < NUM_CELL; i++)
    {
      const char SYMBOL[5] = {'0', 'R', 'B', 'X', 'Z'};
      WP[i] = SYMBOL[SH[NUM_CELL - 1 - i]];
    }
    WP[NUM_CELL] = ':';
    WP[NUM_CELL + 1] = (wmove == 0) ? 'r' : 'b';
    WP[NUM_CELL + 2] = 0;
  }
  return WP;
}

Position &Position::Read(const char WP[], int how)
{
  if (how == 0)
  {
    int i = 0, j, ii;
    for (j = 0; i < NUM_CELL; j++)
    {
      unsigned int cwp = WP[j] - 1;
      if (i >= NUM_CELL - 3)
      {
        wmove = char(cwp % 2);
        cwp /= 2;
        ii = NUM_CELL - 1;
      }
      else ii = i + 2;
      while(ii >= i) {SH[ii--] = char(cwp % 5); cwp /= 5;}
      i += 3;
    }
  }
  else if (how == 1)
  {
    int i;
    wmove = 0;
    for (i = 0; i < NUM_CELL; i++)
    {
      switch(WP[i])
      {
      case '0':
      case '-': case '.':
      case 'F': case 'f':
        SH[NUM_CELL - 1 - i] = 0;
        break;
      case '1':
      case 'A': case 'a':
      case 'R': case 'r':
        SH[NUM_CELL - 1 - i] = 1;
        break;
      case '2':
      case 'B': case 'b':
      case 'S': case 's':
        SH[NUM_CELL - 1 - i] = 2;
        break;
      case '3':
      case 'W': case 'w':
      case 'X': case 'x':
        SH[NUM_CELL - 1 - i] = 3;
        break;
      case '4':
      case 'Y': case 'y':
      case 'Z': case 'z':
        SH[NUM_CELL - 1 - i] = 4;
        break;
      default:
        Init();
        return *this;
      }
    }
    if (WP[NUM_CELL] == ':')
    {
      char c = WP[NUM_CELL + 1];
      if (c == 'B' || c == 'b' || c == 'S' || c == 's' ||
          c == 'Y' || c == 'y' || c == 'Z' || c == 'z')
      {
        wmove = 1;
      }
    }
  }
  return *this;
}

char *Position::WriteMv(const unsigned char mv[], char WP[], int how)
{
  int i, nmv = 0;
  if (mv) nmv = mv[0];
  if (how == 0)
  {
    WP[0] = char(nmv + 1);
    for (i = 1; i <= nmv; i++) WP[i] = char(mv[i] + 1);
  }
  else if (how == 1)
  {
    int j = 0;
    for (i = 1; i <= nmv; i++)
    {
      int x, y;
      NumToPole(mv[i], x, y);
      WP[j++] = char('a' + NW_CELL - 1 - x);
      int r = itoa(WP + j, 1 + y);
      if (r > 0) j += r;
      if (i != nmv) WP[j++] = '-';
    }
    WP[j] = 0;
  }
  return WP;
}

unsigned char *Position::ReadMv(unsigned char mv[], const char WP[], int how)
{
  int i;
  if (how == 0)
  {
    mv[0] = char(WP[0] - 1);
    for (i = 1; i <= mv[0]; i++) mv[i] = char(WP[i] - 1);
  }
  else if (how == 1)
  {
    int j = 0, x = -1, y = -1;
    mv[0] = 0;
    for (;;)
    {
      if (isdigit(WP[j]))
      {
        y = atoi(WP + j) - 1;
        while (isdigit(WP[j])) j++;
      }
      else if (islower(WP[j])) x = NW_CELL - 1 - (WP[j++] - 'a');
      else
      {
        if (x >= 0 && y >= 0 && x < NW_CELL && y < NW_CELL)
        {
          mv[++mv[0]] = (char)PoleToNum(x, y);
        }
        else if (y >= 0 && y < NUM_CELL) mv[++mv[0]] = (char)(NUM_CELL - 1 - y);
        x = -1; y = -1;
        if (WP[j] == '-' || WP[j] == '*' || WP[j] == ':') j++;
        else break;
      }
      if (x >= 0 && y >= 0 && x < NW_CELL && y < NW_CELL)
      {
        mv[++mv[0]] = (char)PoleToNum(x, y);
        x = -1; y = -1;
      }
    }
  }
  return mv;
}

int Position::GetLenMv(const unsigned char mv[], int how)
{
  if (how == 0) return mv ? (1 + mv[0]) : 1;
  else if (how == 1)
  {
    int i, j = 0;
    if (!mv) return 1;
    for (i = 1; i <= mv[0]; i++)
    {
      int x, y;
      NumToPole(mv[i], x, y);
      j++; y++;
      while(y > 0) {j++; y /= 10;}
      if (i != mv[0]) j++;
    }
    return ++j;
  }
  else return 0;
}

int Position::GetLenMwr(const char WP[], int how)
{
  if (how == 0) return (unsigned char)WP[0];
  else if (how == 1)
  {
    int j;
    for (j = 0; WP[j] == '-' || WP[j] == '*' ||
                WP[j] == ':' || isdigit(j) || islower(j); j++);
    return j + 1;
  }
  else return 0;
}

inline int Position::GetLenMvEx(const unsigned char mv[], int how) const
{
  return WriteMvEx(mv, 0, how);
}

int Position::WriteMvEx(const unsigned char mv[], char WP[], int how) const
{
  if (how == 11)
  {
    Position pos = *this;
    int p, L = 0, was_d = 0;
    for (p = 1; p <= mv[0]; p++)
    {
      if (!was_d && pos.SH[mv[p]] > 2)
      {
        if (WP) WP[L] = '*';
        L++;
        was_d = 1;
      }
      int x0, y0, x1, y1;
      NumToPole(mv[p], x0, y0);
      if (WP)
      {
        WP[L++] = char('a' + NW_CELL - 1 - x0);
        int r = itoa(WP + L, 1 + y0);
        if (r > 0) L += r;
      }
      else
      {
        L++;
        int g = y0 + 1;
        while(g > 0) {L++; g /= 10;}
      }
      if (p >= mv[0]) break;
      NumToPole(mv[p+1], x1, y1);
      int mi = abs(x1 - x0), i, eat = -1;
      if (mi > 0 && mi == abs(y1 - y0))
      {
        int sx = (x1 > x0) ? 1 : -1;
        int sy = (y1 > y0) ? 1 : -1;
        for (i = 1; i < mi; i++)
        {
          int r = PoleToNum(x0 + i * sx, y0 + i * sy);
          if (pos.SH[r] != 0)
          {
            eat = r;
            pos.Del(r);
          }
        }
      }
      if (WP) WP[L] = (eat >= 0) ? ':' : '-';
      L++;
      if (pos.SH[mv[p]] == 1 && y1 == NW_CELL - 1) pos.SH[mv[p]] = 3;
      else if (pos.SH[mv[p]] == 2 && y1 == 0) pos.SH[mv[p]] = 4;
      pos.Move(mv[p], mv[p+1]);
    }
    if (WP) WP[L] = 0;
    L++;
    return L;
  }
  else
  {
    if (WP) WriteMv(mv, WP, how);
    return GetLenMv(mv, how);
  }
}

int Position::ScanSide(int x, int y, int sx, int sy, int sh_k) const
{
  if (sh_k < 0) sh_k = SH[PoleToNum(x, y)];
  if (sh_k < 1 || sh_k > 4) return -2;
  if (sh_k >= 2) sh_k -= 2;
  int i, i0, i1, f = 0, g = 0;
  if (sx < 0) i0 = x;
  else i0 = NW_CELL - x - 1;
  if (sy < 0) i1 = y;
  else i1 = NW_CELL - y - 1;
  if (i0 > i1) i0 = i1;
  for (i = 1; i <= i0; i++)
  {
    char nk = SH[PoleToNum(x + i*sx, y + i*sy)];
    if (nk)
    {
      if (f || (nk != 3 - sh_k && nk != 5 - sh_k)) return g;
      else f = 1;
    }
    else if (f) return (i == 2) ? 4 : (2 + g);
    else if (i == 1) g = 1;
  }
  return g;
}

int Position::CanEat(int k, int psx, int psy, int sh_k) const
{
  int x, y, sx, sy;
  if (sh_k < 0) sh_k = SH[k];
  if (sh_k < 1 || sh_k > 6) return 0;
  NumToPole(k, x, y);
  if (sh_k > 4)
  {
    int i, i0, i1, f = 0;
    if (-psx < 0) i0 = x;
    else i0 = NW_CELL - x - 1;
    if (-psy < 0) i1 = y;
    else i1 = NW_CELL - y - 1;
    if (i0 > i1) i0 = i1;
    for (i = 1; i <= i0; i++)
    {
      int nk = SH[PoleToNum(x - i*psx, y - i*psy)];
      if (nk)
      {
        if (f || (nk != 7 - sh_k && nk != 9 - sh_k)) break;
        else f = 1;
      }
      else
      {
        if (f) return 1;
        if (ScanSide(x - i*psx, y - i*psy, psy, -psx, sh_k-2) >= 2) return 1;
        if (ScanSide(x - i*psx, y - i*psy, -psy, psx, sh_k-2) >= 2) return 1;
      }
    }
  }
  else for (sx = -1; sx <= 1; sx += 2) if (x + 2*sx >= 0 && x + 2*sx < NW_CELL)
  {
    for (sy = -1; sy <= 1; sy += 2)
    {
      if ((sx != psx || sy != psy) && y + 2*sy >= 0 && y + 2*sy < NW_CELL)
      {
        if (sh_k <= 2)
        {
          if (SH[PoleToNum(x + 2*sx, y + 2*sy)] == 0)
          {
            int nk = SH[PoleToNum(x + sx, y + sy)];
            if (nk == 3 - sh_k || nk == 5 - sh_k) return 1;
          }
        }
        else if (ScanSide(x, y, sx, sy, sh_k) >= 2) return 1;
      }
    }
  }
  return 0;
}

int Position::CanMove(int k) const
{
  int x, y, xx, yy, y1, y2;
  NumToPole(k, x, y);
  if (SH[k] == 1) y1 = y2 = y + 1;
  else if (SH[k] == 2) y1 = y2 = y - 1;
  else if (SH[k] != 3 && SH[k] != 4) return 0;
  else {y1 = y - 1; y2 = y + 1;}
  for (yy = y1; yy <= y2; yy += 2) if (yy >= 0 && yy < NW_CELL)
  {
    for (xx = x - 1; xx <= x + 1; xx += 2) if (xx >= 0 && xx < NW_CELL)
    {
      if (SH[PoleToNum(xx, yy)] == 0) return 1;
    }
  }
  return 0;
}

int Position::AChCell(int k)
{
  if (k < 0 || k >= NUM_CELL) return AWrong;
  if (SH[k] == 0) return AfCell;
  if (SH[k] != 1 + wmove && SH[k] != 3 + wmove) return AWColor;
  if (CanEat(k)) return 1;
  if (AllCanEat()) return AMustEat;
  if (CanMove(k)) return 1;
  return ANoMove;
}

int Position::AMove(const unsigned char MV[], int k, int &mkmove)
{
  if (k >= NUM_CELL) return AWrong;
  if (MV[0] <= 0)
  {
    if (k < 0) return NUM_CELL;
    int s = AChCell(k);
    if (s < 0) return s;
    else return NUM_CELL;
  }
  if (MV[0] == 1 && k < 0)
  {
    int s = AChCell(MV[1]);
    if (s < 0) return s;
    else return NUM_CELL;
  }
  if (SH[MV[1]] == 0) return AfCell;
  if (SH[MV[1]] != 1 + wmove && SH[MV[1]] != 3 + wmove) return AWColor;
  int i, mi, p, MV_L, MV_N = MV[0], eat = -1, r;
  int psx = 0, psy = 0;
  if (k >= 0) MV_N++;
  Position pos = *this;
  for (p = 1; p < MV_N; p++)
  {
    int x0, y0, x1, y1, i_eat;
    if (p < MV[0]) MV_L = MV[p+1];
    else if (k < 0) break;
    else MV_L = k;
    if (pos.SH[MV_L] != 0) return AnfCell;
    NumToPole(MV[p], x0, y0);
    NumToPole(MV_L, x1, y1);
    mi = abs(x1 - x0);
    if (mi <= 0 || mi != abs(y1 - y0)) return AOnlyDiag;
    int sx = (x1 > x0) ? 1 : -1;
    int sy = (y1 > y0) ? 1 : -1;
    if (sx == psx && sy == psy) return ATurnBack;
    psx = -sx; psy = -sy;
    eat = -1; i_eat = -1;
    for (i = 1; i < mi; i++)
    {
      r = PoleToNum(x0 + i * sx, y0 + i * sy);
      if (pos.SH[r] != 0)
      {
        if (eat >= 0) return AMoreOne;
        if (pos.SH[r] != 2 - wmove && pos.SH[r] != 4 - wmove) return AEatYour;
        eat = r; i_eat = i;
        pos.Del(r);
      }
    }
    if (eat >= 0)
    {
      if (pos.SH[MV[p]] <= 2 && mi != 2) return ANotDmE;
    }
    else
    {
      if (MV_N > 2) return AMustEatMore;
      if (pos.SH[MV[p]] <= 2)
      {
        if (mi != 1) return ANotDm;
        if (wmove == 0 && y1 < y0 || wmove == 1 && y1 > y0) return AChBack;
      }
      if (AllCanEat()) return AMustEat;
    }
    if (i_eat >= 0 && pos.SH[MV[p]] > 2)
    {
      if (!pos.CanEat(MV_L, psx, psy, pos.SH[MV[p]]))
      {
        if (pos.CanEat(PoleToNum(x0 + i_eat*sx, y0 + i_eat*sy),
                                 psx, psy, pos.SH[MV[p]] + 2))
        {
          return AMustEatMoreD;
        }
      }
    }
    if (wmove == 0 && y1 == NW_CELL - 1) pos.SH[MV[p]] = 3;
    else if (wmove == 1 && y1 == 0) pos.SH[MV[p]] = 4;
    pos.Move(MV[p], MV_L);
  }
  if (&mkmove)
  {
    int end = MV_N > 1 && (eat < 0 || !pos.CanEat(MV_L, psx, psy));
    if (mkmove == 1 && end)
    {
      *this = pos;
      wmove = !wmove;
    }
    if (end) mkmove = 0;
    else
    {
      if (MV_N > 1 && eat >= 0) mkmove = AMustEatMore;
      else mkmove = AWrong;
    }
  }
  if (k < 0 || eat < 0) eat = NUM_CELL;
  return eat;
}

int Position::AllCanEat(int w) const
{
  int k;
  if (w < 0) w = wmove;
  for (k = 0; k < NUM_CELL; k++)
  {
    if ((SH[k] == w+1 || SH[k] == w+3) && CanEat(k)) return 1;
  }
  return 0;
}

int Position::AllCanMove(int w) const
{
  int k;
  if (w < 0) w = wmove;
  for (k = 0; k < NUM_CELL; k++)
  {
    if ((SH[k] == w+1 || SH[k] == w+3) && CanMove(k)) return 1;
  }
  return 0;
}

void Position::Reverse()
{
  int i;
  for (i = 0; i <= (NUM_CELL-1) / 2; i++)
  {
    int sh1 = SH[i], sh2 = SH[NUM_CELL - 1 - i];
    if (sh1 == 1) sh1 = 2;
    else if (sh1 == 2) sh1 = 1;
    else if (sh1 == 3) sh1 = 4;
    else if (sh1 == 4) sh1 = 3;
    if (sh2 == 1) sh2 = 2;
    else if (sh2 == 2) sh2 = 1;
    else if (sh2 == 3) sh2 = 4;
    else if (sh2 == 4) sh2 = 3;
    SH[i] = (char)sh2; SH[NUM_CELL - 1 - i] = (char)sh1;
  }
  wmove = !wmove;
}


class PlayWrite
{
public:
  PlayWrite() : play(0), mplay(0), nplay(0), start(0), mstart(0), nstart(0) {}
  PlayWrite(const PlayWrite &pl) : play(0), mplay(0), nplay(0),
               start(0), mstart(0), nstart(0) {(*this) = pl;}
  ~PlayWrite() {Clear();}

  void Clear();
  PlayWrite &operator=(const PlayWrite &pl);
  int GetN() const {return nstart - 1;}
  int GetLen() const {return nplay - sizeof(int);}

  struct PMv
  {
    Position pos;
    unsigned char mv[NUM_CELL];
  };

  void Add(const unsigned char move[], const Position &pos);
  void Add(const PMv &pmv) {Add(pmv.mv, pmv.pos);}
  int Add(const unsigned char move[]);
  int GetMove(unsigned char move[], int k) const;
  int GetPos(Position &pos, int k) const;
  int GetPMv(PMv &pmv, int k) const;
  int GetMoveL(unsigned char move[], int k = 0) const
              {return GetMove(move, nstart - 2 - k);}
  int GetPosL(Position &pos, int k = 0) const {return GetPos(pos, nstart - 2 - k);}
  int GetPMvL(PMv &pmv, int k = 0) const {return GetPMv(pmv, nstart - 2 - k);}
  int ClearFrom(int k = 0);
  int IsDraw(int nmove = -1);
protected:
  void IncPlay(int k);
  void IncStart(int k);
  void IncStart() {IncStart(nstart + 1);}
  void AddStart() {IncStart(); start[nstart++] = nplay;}
  void Split();
  void SplitClear();
protected:
  char *play;
  int *start;
  int mplay, nplay, mstart, nstart;
};

void PlayWrite::Clear()
{
  if (play)
  {
    if ((*(int*)play) > 0) (*(int*)play)--;
    else delete[] play;
  }
  play = 0; mplay = 0; nplay = 0;
  if (start)
  {
    if (start[0] > 0) start[0]--;
    else delete[] start;
  }
  start = 0; mstart = 0; nstart = 0;
}

void PlayWrite::Split()
{
  if (play && (*(int*)play) > 0)
  {
    (*(int*)play)--;
    char *play0 = play;
    mplay = nplay;
    play = new char[mplay];
    memcpy(play, play0, nplay * sizeof(play[0]));
    (*(int*)play) = 0;
  }
  if (start && start[0] > 0)
  {
    start[0]--;
    int *start0 = start;
    mstart = nstart;
    start = new int[mstart];
    memcpy(start, start0, nstart * sizeof(start[0]));
    start[0] = 0;
  }
}

void PlayWrite::SplitClear()
{
  if (play && (*(int*)play) > 0)
  {
    (*(int*)play)--;
    play = 0;
    nplay = 0; mplay = 0;
  }
  if (start && start[0] > 0)
  {
    start[0]--;
    start = 0;
    nstart = 0; mstart = 0;
  }
}

PlayWrite &PlayWrite::operator=(const PlayWrite &pl)
{
  if (&pl != this)
  {
    play = pl.play;
    (*(int*)play)++;
    nplay = pl.nplay; mplay = pl.mplay;
    start = pl.start;
    start[0]++;
    nstart = pl.nstart; mstart = pl.mstart;
  }
  return *this;
}

void PlayWrite::IncPlay(int k)
{
  if (mplay < k)
  {
    int m0 = mplay;
    char *play0 = play;
    mplay = 2*k + 10;
    play = new char[mplay];
    memcpy(play, play0, m0 * sizeof(play[0]));
    (*(int*)play) = 0;
    if (play0)
    {
      if ((*(int*)play0) > 0) (*(int*)play0)--;
      else delete[] play0;
    }
  }
}

void PlayWrite::IncStart(int k)
{
  if (mstart < k)
  {
    int m0 = mstart;
    int *start0 = start;
    mstart = 2*k + 10;
    start = new int[mstart];
    memcpy(start, start0, m0 * sizeof(start[0]));
    start[0] = 0;
    if (start0)
    {
      if (start0[0] > 0) start0[0]--;
      else delete[] start0;
    }
  }
}

void PlayWrite::Add(const unsigned char move[], const Position &pos)
{
  Split();
  int k = Position::GetLenMv(move);
  if (nstart < 1) nstart = 1;
  if (nplay < sizeof(int)) nplay = sizeof(int);
  AddStart();
  IncPlay(nplay + k + LEN_WPOS);
  Position::WriteMv(move, play + nplay, 0);
  nplay += k;
  pos.Write(play + nplay, 0);
  nplay += LEN_WPOS;
}

int PlayWrite::Add(const unsigned char move[])
{
  if (nstart <= 1) return 1;
  Position pos;
  GetPosL(pos);
  int mkmove = 1;
  int res = pos.AMove(move, -1, mkmove);
  if (res < 0) return res;
  else if (mkmove != 0) return mkmove;
  Add(move, pos);
  return 0;
}

int PlayWrite::GetMove(unsigned char move[], int k) const
{
  if (!play || !start) return -1;
  k++;
  if (k <= 0 || k >= nstart) return -1;
  Position::ReadMv(move, play + start[k], 0);
  return Position::GetLenMv(move);
}

int PlayWrite::GetPos(Position &pos, int k) const
{
  if (!play || !start) return -1;
  k++;
  if (k <= 0 || k >= nstart) return -1;
  int mlen = Position::GetLenMwr(play + start[k], 0);
  pos.Read(play + start[k] + mlen, 0);
  return LEN_WPOS;
}

int PlayWrite::GetPMv(PMv &pmv, int k) const
{
  if (!play || !start) return -1;
  k++;
  if (k <= 0 || k >= nstart) return -1;
  Position::ReadMv(pmv.mv, play + start[k], 0);
  int mlen = Position::GetLenMv(pmv.mv);
  pmv.pos.Read(play + start[k] + mlen, 0);
  return mlen + LEN_WPOS;
}

int PlayWrite::ClearFrom(int k)
{
  if (!play || !start) return 0;
  k++;
  if (k >= nstart) return 0;
  if (k <= 1) {Clear(); return 2;}
  nplay = start[k];
  nstart = k;
  return 1;
}

int PlayWrite::IsDraw(int nmove)
{
  nmove++;
  if (nmove <= 0 || nmove > nstart) nmove = nstart;
  if (!start || nmove <= 3) return 0;
  int i, j, k, draw = 0;
  for (i = 1; i < nmove; i++)
  {
    k = 1;
    char *p1 = play + start[i] + Position::GetLenMwr(play + start[i], 0);
    for (j = 1; j < i; j++)
    {
      char *p2 = play + start[j] + Position::GetLenMwr(play + start[j], 0);
      if (memcmp(p1, p2, LEN_WPOS) == 0) k++;
    }
    if (k >= 3) {draw = 1; break;}
  }
  return draw;
}


class TChPlayer
{
public:
  TChPlayer() {}

  typedef struct PlayWrite::PMv PMv;

  virtual int PlayerID() {return 0;}
  virtual int Move(PMv &pmv) = 0;

  int Move(Position &pos, char mv[]);
};

int TChPlayer::Move(Position &pos, char mv[])
{
  PMv pmv;
  pmv.pos = pos; memcpy(pmv.mv, mv, sizeof(pmv.mv));
  int res = Move(pmv);
  pos = pmv.pos; memcpy(mv, pmv.mv, sizeof(pmv.mv));
  return res;
}

#endif  //_HEADER_POSITION_H
