#ifndef _HEADER_PLAYER_H
#define _HEADER_PLAYER_H

#include "position.h"
#include "sysproc.h"
#ifndef __MENUET__
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#endif

template <class tvalue>
class TBaseCompPlayer : public TChPlayer
{
public:
  static const int PKind;
  static const tvalue win_val;
public:
  TBaseCompPlayer() : draw(0), data(0) {}

  virtual int PlayerID() {return PKind;}
  virtual int Move(PMv &pmv);

  struct PMvv : public PMv
  {
    tvalue val;
  };

  struct Z
  {
    Z(int marr = 400) : narr(0), marr(marr) {array = new PMvv[marr];}
    ~Z() {Clear();}

    void Clear() {if (array) {delete[] array; array = 0;} marr = 0; narr = 0;}
    void AddPos(int n);
    void AddPos() {AddPos(narr + 1);}
    int FindAllMoves(PMv pmv, int onlyeat = 0);
    void FindSideEats(PMv &pmv, int k, int sx, int sy);

    static int ComparePMv(const void *v1, const void *v2);
    void Sort(int n0, int n1);

    int marr, narr;
    PMvv *array;
  };

  static tvalue GetLossValue(const Position &pos);
  virtual tvalue GetValue(const Position &pos, int num = 0);
  tvalue GetFullValue(const Position &pos, int num = 0);
  tvalue FindBMove(Z &z, int num, PMvv *pmv, int zn = -1,
                tvalue a = -2 * win_val, tvalue b = 2 * win_val);
public:
  void (*draw)(void*, int = 0);
  void *data;
};

template <class tvalue>
const int TBaseCompPlayer<tvalue>::PKind = 0x2000;

template <class tvalue>
const tvalue TBaseCompPlayer<tvalue>::win_val = (tvalue)10000000L;

template <class tvalue>
void TBaseCompPlayer<tvalue>::Z::AddPos(int n)
{
  if (marr < n)
  {
     int m0 = marr;
     PMvv *arr0 = array;
     marr = 2*n + 10;
     array = new PMvv[marr];
     if (arr0)
     {
       int i;
       for (i = 0; i < m0; i++) array[i] = arr0[i];
       delete[] arr0;
     }
  }
}

template <class tvalue>
void TBaseCompPlayer<tvalue>::Z::FindSideEats(PMv &pmv, int k, int sx, int sy)
{
  int x, y;
  NumToPole(k, x, y);
  if (pmv.pos.SH[k] == pmv.pos.wmove + 1)
  {
    int xx = x + 2*sx, yy = y + 2*sy;
    if (xx >= 0 && xx < NW_CELL && yy >= 0 && yy < NW_CELL)
    {
      int kk = PoleToNum(xx, yy);
      if (pmv.pos.SH[kk] == 0)
      {
        int k1 = PoleToNum(x + sx, y + sy);
        char nk1 = pmv.pos.SH[k1];
        if (nk1 == 2 - pmv.pos.wmove || nk1 == 4 - pmv.pos.wmove)
        {
          char SH_k1 = pmv.pos.SH[k1];
          pmv.pos.Del(k1); pmv.pos.Move(k, kk);
          if (pmv.pos.wmove == 0 && yy == NW_CELL - 1 ||
              pmv.pos.wmove == 1 && yy == 0) pmv.pos.SH[kk] += (char)2;
          pmv.mv[++pmv.mv[0]] = (char)kk;
          int nold = narr;
          FindSideEats(pmv, kk, sx, sy);
          FindSideEats(pmv, kk, sy, -sx);
          FindSideEats(pmv, kk, -sy, sx);
          if (narr == nold)
          {
            AddPos();
            (PMv&)array[narr] = pmv;
            array[narr].pos.wmove = !pmv.pos.wmove;
            narr++;
          }
          pmv.mv[0]--;
          pmv.pos.SH[k1] = SH_k1; pmv.pos.Del(kk);
          pmv.pos.SH[k] = char(pmv.pos.wmove + 1);
        }
      }
    }
  }
  else if (pmv.pos.SH[k] == pmv.pos.wmove + 3)
  {
    int i, i0, i1;
    if (sx < 0) i0 = x;
    else i0 = NW_CELL - x - 1;
    if (sy < 0) i1 = y;
    else i1 = NW_CELL - y - 1;
    if (i0 > i1) i0 = i1;
    if (i0 >= 2)
    {
      pmv.pos.Del(k);
      pmv.mv[0]++;
      i1 = -1;
      int kk, kk1;
      char SH_kk1;
      int nold = narr;
      for (i = 1; i <= i0; i++)
      {
        kk = PoleToNum(x + i*sx, y + i*sy);
        char chh = pmv.pos.SH[kk];
        if (chh)
        {
          if (i1 >= 0 || (chh != 2 - pmv.pos.wmove && chh != 4 - pmv.pos.wmove)) break;
          else
          {
            i1 = i; kk1 = kk;
            SH_kk1 = chh;
            pmv.pos.Del(kk1);
          }
        }
        else if (i1 >= 0)
        {
          pmv.pos.SH[kk] = char(pmv.pos.wmove + 3);
          pmv.mv[pmv.mv[0]] = (char)kk;
          if (i == i1+1) FindSideEats(pmv, kk, sx, sy);
          FindSideEats(pmv, kk, sy, -sx);
          FindSideEats(pmv, kk, -sy, sx);
          pmv.pos.Del(kk);
        }
      }
      if (narr == nold && i1 >= 0)
      {
        while (--i > i1)
        {
          kk = PoleToNum(x + i*sx, y + i*sy);
          AddPos();
          (PMv&)array[narr] = pmv;
          array[narr].pos.SH[kk] = char(pmv.pos.wmove + 3);
          array[narr].mv[pmv.mv[0]] = (char)kk;
          array[narr].pos.wmove = !pmv.pos.wmove;
          narr++;
        }
      }
      pmv.mv[0]--;
      pmv.pos.SH[k] = char(pmv.pos.wmove + 3);
      if (i1 >= 0) pmv.pos.SH[kk1] = SH_kk1;
    }
  }
}

template <class tvalue>
int TBaseCompPlayer<tvalue>::Z::FindAllMoves(PMv pmv, int onlyeat)
{
  int k, nold = narr, was_eat = 1;
  pmv.mv[0] = 1;
  for (k = 0; k < NUM_CELL; k++)
  {
    if (pmv.pos.SH[k] == pmv.pos.wmove + 1 || pmv.pos.SH[k] == pmv.pos.wmove + 3)
    {
      pmv.mv[1] = (char)k;
      FindSideEats(pmv, k, 1, 1);
      FindSideEats(pmv, k, 1, -1);
      FindSideEats(pmv, k, -1, 1);
      FindSideEats(pmv, k, -1, -1);
    }
  }
  if (narr == nold)
  {
    was_eat = 0;
    if (!onlyeat)
    {
      pmv.mv[0] = 2;
      for (k = 0; k < NUM_CELL; k++)
      {
        if (pmv.pos.SH[k] == pmv.pos.wmove + 1)
        {
          pmv.mv[1] = (char)k;
          int x, x0, x1, y;
          NumToPole(k, x0, y);
          if (pmv.pos.wmove == 1) y--; else y++;
          if (y >= 0 && y < NW_CELL)
          {
            int kk;
            x1 = (x0--) + 1;
            for (x = x0; x <= x1; x += 2) if (x >= 0 && x < NW_CELL)
            {
              kk = PoleToNum(x, y);
              if (pmv.pos.SH[kk] == 0)
              {
                AddPos();
                (PMv&)array[narr] = pmv;
                array[narr].pos.Del(k);
                if (pmv.pos.wmove == 0 && y == NW_CELL - 1 ||
                    pmv.pos.wmove == 1 && y == 0)
                {
                  array[narr].pos.Add(kk, char(pmv.pos.wmove + 3));
                }
                else array[narr].pos.Add(kk, char(pmv.pos.wmove + 1));
                array[narr].mv[2] = (char)kk;
                array[narr].pos.wmove = !pmv.pos.wmove;
                narr++;
              }
            }
          }
        }
        else if (pmv.pos.SH[k] == pmv.pos.wmove + 3)
        {
          pmv.mv[1] = (char)k;
          int x, y, sx, sy;
          NumToPole(k, x, y);
          for (sx = -1; sx <= 1; sx += 2) if (x + sx >= 0 && x + sx < NW_CELL)
          {
            for (sy = -1; sy <= 1; sy += 2) if (y + sy >= 0 && y + sy < NW_CELL)
            {
              int i, i0, i1;
              if (sx < 0) i0 = x;
              else i0 = NW_CELL - x - 1;
              if (sy < 0) i1 = y;
              else i1 = NW_CELL - y - 1;
              if (i0 > i1) i0 = i1;
              for (i = 1; i <= i0; i++)
              {
                int kk = PoleToNum(x + i*sx, y + i*sy);
                if (pmv.pos.SH[kk]) break;
                AddPos();
                (PMv&)array[narr] = pmv;
                array[narr].pos.Move(k, kk);
                array[narr].mv[2] = (char)kk;
                array[narr].pos.wmove = !pmv.pos.wmove;
                narr++;
              }
            }
          }
        }
      }
    }
  }
  pmv.mv[0] = 0;
  return was_eat;
}

template <class tvalue>
int TBaseCompPlayer<tvalue>::Z::ComparePMv(const void *v1, const void *v2)
{
  PMvv *pmv1 = (PMvv*)v1, *pmv2 = (PMvv*)v2;
  if (pmv1->val < pmv2->val) return -1;
  else if (pmv1->val > pmv2->val) return 1;
  else return 0;
}

template <class tvalue>
void TBaseCompPlayer<tvalue>::Z::Sort(int n0, int n1)
{
  qsort(array + n0, n1 - n0, sizeof(PMvv), ComparePMv);
}


template <class tvalue>
tvalue TBaseCompPlayer<tvalue>::GetLossValue(const Position &pos)
{
  tvalue val = -win_val - 1000021L;
  for (int i = 0; i < NUM_CELL; i++)
  {
    if (pos.SH[i] == 1 + pos.wmove) val -= 10000L;
    else if (pos.SH[i] == 2 - pos.wmove) val -= 100L;
    else if (pos.SH[i] == 3 + pos.wmove) val -= 80000L;
    else if (pos.SH[i] == 4 - pos.wmove) val -= 100L;
  }
  return val;
}

template <class tvalue>
tvalue TBaseCompPlayer<tvalue>::GetValue(const Position &pos, int num)
{
  tvalue val = 0;
  if (num == 0)
  {
    int NumDM0 = 0, NumDM1 = 0;
    for (int i = 0; i < NUM_CELL; i++)
    {
      short PreimSHPos[32] = {243, 243, 243, 245,
                              240, 240, 240, 240,
                              244, 244, 244, 244,
                              245, 248, 248, 245,
                              249, 250, 250, 248,
                              256, 260, 260, 256,
                              280, 280, 280, 260,
                              280, 280, 280, 280};
      if (pos.SH[i] == 1 + pos.wmove)
      {
        val += PreimSHPos[pos.wmove ? (NUM_CELL - 1 - i) : i];
      }
      else if (pos.SH[i] == 2 - pos.wmove)
      {
        val -= PreimSHPos[pos.wmove ? i : (NUM_CELL - 1 - i)];
      }
      else if (pos.SH[i] == 3 + pos.wmove) NumDM1++;
      else if (pos.SH[i] == 4 - pos.wmove) NumDM0++;
    }
    if (NumDM1 > 0)
    {
      val += 560; NumDM1--;
      if (NumDM1 > 0)
      {
        val += 432; NumDM1--;
        val += NumDM1 * 384;
      }
    }
    if (NumDM0 > 0)
    {
      val -= 560; NumDM0--;
      if (NumDM0 > 0)
      {
        val -= 432; NumDM0--;
        val -= NumDM0 * 384;
      }
    }
  }
  if (num == 1)
  {
    char NSH1 = 0, NSH0 = 0, NDM1 = 0, NDM0 = 0;
    int i;
    for (i = 0; i < 32; i++)
    {
      if (pos.SH[i] == 1 + pos.wmove) NSH1++;
      else if (pos.SH[i] == 2 - pos.wmove) NSH0++;
      else if (pos.SH[i] == 3 + pos.wmove) NDM1++;
      else if (pos.SH[i] == 4 - pos.wmove) NDM0++;
    }
    if (NDM1 > 0 && NDM0 > 0 && NSH1 + NSH0 < 3)
    {
      unsigned char HwoBD = 0;
      char Sh0BD = 1, Sh1BD = 1;
      for (i = 0; i < 8; i++)
      {
        char ShBD = pos.SH[PoleToNum(i, 7 - i)];
        if (ShBD == 1 + pos.wmove) Sh1BD++;
        else if (ShBD == 2 - pos.wmove) Sh0BD++;
        else if (ShBD == 3 + pos.wmove) HwoBD |= 2;
        else if (ShBD == 4 - pos.wmove) HwoBD |= 1;
      }
      if (HwoBD == 2) val += 128 / Sh0BD;
      if (HwoBD == 1) val -= 128 / Sh1BD;
      if (NDM1 >= 3 && NDM0 == 1 && NSH1 == 0 && NSH0 == 0 && HwoBD == 1)
      {
        char Best4P[4][2] = {{0,9}, {4,13}, {31,22}, {27,18}};
        char Add = 0;
        for (i = 0; i < 4; i++)
        {
          Add |= char((pos.SH[Best4P[i][0]] == 3 + pos.wmove) * 3 +
                      (pos.SH[Best4P[i][1]] == 3 + pos.wmove));
        }
        if (Add >= 4) val += 32;
        else if (Add == 3) val += 24;
        else if (Add >= 1) val += 16;
      }
      else if (NDM0 >= 3 && NDM1 == 1 && NSH0 == 0 && NSH1 == 0 && HwoBD == 2)
      {
        char Best4P[4][2] = {{0,9}, {4,13}, {31,22}, {27,18}};
        char Add = 0;
        for (i = 0; i < 4; i++)
        {
          Add |= char((pos.SH[Best4P[i][0]] == 4 - pos.wmove) * 3 +
                      (pos.SH[Best4P[i][1]] == 4 - pos.wmove));
        }
        if (Add >= 4) val -= 32;
        else if (Add == 3) val -= 24;
        else if (Add >= 1) val -= 16;
      }
    }
    else
    {
      for (i = 0; i < NUM_CELL; i++)
      {
        char Color = char(pos.SH[i] - 1);
        if (Color == 0 || Color == 1)
        {
          char qi = Color ? char(NUM_CELL - 1 - i) : char(i);
          char Zn = Color ? char(-1) : char(1);
          char PreZ = (Color == pos.wmove) ? char(1) : char(-1);
          if (pos.SH[i + Zn * 8] != 2 - Color)
          {
            if (qi / 4 == 2)
            {
              char IsFree = 0;
              if (pos.SH[i - Zn * 4] == 2 - Color) IsFree += (char)2;
              else if (qi != 8)
              {
                if (pos.SH[i - Zn    ] == 2 - Color ||
                    pos.SH[i - Zn * 9] == 2 - Color) IsFree += (char)2;
                else if (Color != pos.wmove)
                      if (pos.SH[i - Zn * 5] == 2 - Color) IsFree++;
              }
              if (qi == 11) IsFree += (char)2;
              else if (pos.SH[i + Zn    ] == 2 - Color ||
                        pos.SH[i - Zn * 3] == 2 - Color ||
                        pos.SH[i - Zn * 7] == 2 - Color) IsFree += (char)2;
              else if (Color != pos.wmove && qi != 10)
              {
                if (pos.SH[i - Zn * 2] == 2 - Color) IsFree++;
              }
              if (IsFree < 3) val += PreZ * 176 / (1 + NDM0 + NDM1);
              else if (qi == 9 || qi == 10)
              {
                val += PreZ * 128 / (1 + NDM0 + NDM1);
              }
            }
            else if (qi / 4 == 3)
            {
              char IsFree = 0;
              if (pos.SH[i - Zn * 12] == 2 - Color)
              {
                if (Color == pos.wmove) IsFree += (char)11;
                else IsFree += (char)12;
              }
              else if (pos.SH[i - Zn * 4] == 2 - Color) IsFree += (char)11;
              else if (qi == 15) IsFree += (char)5;
              else if (pos.SH[i - Zn * 7] == 2 - Color) IsFree += (char)9;
              else if (pos.SH[i + Zn] == 2 - Color) IsFree += (char)8;
              else if (pos.SH[i - Zn * 11] == 2 - Color)
              {
                if (Color == pos.wmove) IsFree += (char)5;
                else IsFree += (char)7;
              }
              else if (pos.SH[i - Zn * 3] == 2 - Color) IsFree += (char)5;
              else if (qi != 14)
              {
                if (pos.SH[i - Zn * 6] == 2 - Color) IsFree += (char)3;
                else if (Color != pos.wmove)
                {
                  if (pos.SH[i - Zn * 10] == 2 - Color) IsFree++;
                }
              }
              if (qi == 12) IsFree += (char)7;
              else if (pos.SH[i - Zn * 13] == 2 - Color)
              {
                if (Color == pos.wmove) IsFree += (char)11;
                else IsFree += (char)12;
              }
              else if (pos.SH[i - Zn * 5] == 2 - Color) IsFree += (char)11;
              else if (pos.SH[i - Zn * 9] == 2 - Color) IsFree += (char)9;
              else if (pos.SH[i - Zn] == 2 - Color) IsFree += (char)8;
              else if (qi != 13)
              {
                if (pos.SH[i - Zn * 14] == 2 - Color)
                {
                  if (Color == pos.wmove) IsFree += (char)5;
                  else IsFree += (char)7;
                }
                else if (pos.SH[i - Zn * 6] == 2 - Color) IsFree += (char)5;
                else if (pos.SH[i - Zn * 10] == 2 - Color) IsFree += (char)3;
                else if (Color != pos.wmove && qi != 14)
                {
                  if (pos.SH[i - Zn * 15] == 2 - Color) IsFree++;
                }
              }
              if (IsFree < ((Color == pos.wmove) ? 14 : 12))
              {
                val += PreZ * 160 / (1 + NDM0 + NDM1);
              }
            }
          }
        }
      }
    }
  }
  if (num == 2)
  {
    char NSH1 = 0, NSH0 = 0, NDM1 = 0, NDM0 = 0;
    for (int i = 0; i < NUM_CELL; i++)
    {
      if (pos.SH[i] == 1 + pos.wmove) NSH1++;
      else if (pos.SH[i] == 2 - pos.wmove) NSH0++;
      else if (pos.SH[i] == 3 + pos.wmove) NDM1++;
      else if (pos.SH[i] == 4 - pos.wmove) NDM0++;
    }
    if (NDM1 > 0 && NDM0 > 0 && NSH1 == 0 && NSH0 == 0)
    {
      short PrP = 0;
      char Cpos3 = -1;
      if (NDM1 == 3 && NDM0 == 1 && NSH1 == 0 && NSH0 == 0) Cpos3 = 1;
      else if (NDM1 == 1 && NDM0 == 3 && NSH1 == 0 && NSH0 == 0) Cpos3 = 0;
      if (Cpos3 >= 0)
      {
        for (int Osm = 0; Osm <= 1; Osm++) for (int Csm = 0; Csm <= 1; Csm++)
        {
          char PosSH[7][3] = {{13, 17, 18}, {6, 17, 18}, {9, 21, 22},
                              {17, 18, 19}, {9, 10, 15}, {11, 14, 18}, {2, 14, 18}};
          for (char PosNi = 0; PosNi < 7; PosNi++)
          {
            bool IsPosR = 1;
            for (char ShNi = 0; ShNi < 3; ShNi++)
            {
              char DNomSh = (Csm == 1) ? char(31 - PosSH[PosNi][ShNi])
                                       : char(PosSH[PosNi][ShNi]);
              if (Osm == 1)
              {
                int x, y;
                NumToPole(DNomSh, x, y);
                DNomSh = (char)PoleToNum(y, x);
              }
              if (pos.SH[DNomSh] != 3 + (Cpos3 == pos.wmove)) IsPosR = 0;
            }
            if (IsPosR)
            {
              if (PosNi == 3)
              {
                if (Cpos3 == 1)
                {
                  if (pos.SH[(Csm == 1) ? 29 : 2] !=
                      4 - (Cpos3 == pos.wmove) && pos.SH[(Csm == 1) ? 11 : 20] !=
                      4 - (Cpos3 == pos.wmove)) PrP = 216;
                }
                else
                {
                  bool PrPZ = 1;
                  for (int i = 0; i < 6; i++)
                      if (pos.SH[PoleToNum((Csm == 1) ? (i + 2) : i,
                        (Csm == 1) ? (7 - i) : (5 - i))] ==
                          4 - (Cpos3 == pos.wmove)) PrPZ = 0;
                  if (PrPZ) PrP = -216;
                }
              }
              else if (PosNi == 4)
              {
                if (Cpos3 == 1)
                {
                  if (pos.SH[ 0] != 4 - (Cpos3 == pos.wmove) &&
                      pos.SH[ 4] != 4 - (Cpos3 == pos.wmove) &&
                      pos.SH[27] != 4 - (Cpos3 == pos.wmove) &&
                      pos.SH[31] != 4 - (Cpos3 == pos.wmove))
                  {
                    PrP = 216;
                  }
                }
                else
                {
                  if (pos.SH[(Csm == Osm) ?  4 :  0] != 4 - (Cpos3 == pos.wmove) &&
                      pos.SH[(Csm == Osm) ?  8 :  5] != 4 - (Cpos3 == pos.wmove) &&
                      pos.SH[(Csm == Osm) ? 26 : 23] != 4 - (Cpos3 == pos.wmove) &&
                      pos.SH[(Csm == Osm) ? 31 : 27] != 4 - (Cpos3 == pos.wmove))
                  {
                    PrP = -216;
                  }
                }
              }
              else if (PosNi == 5)
              {
                char DNomSh = (Cpos3 == 1) ? ((Osm == 1) ? (char)16 : (char)6)
                                           : ((Osm == 1) ? (char)20 : (char)2);
                if (Csm == 1) DNomSh = char(31 - DNomSh);
                if (pos.SH[DNomSh] == 4 - (Cpos3 == pos.wmove))
                {
                  PrP = (Cpos3 == 1) ? short(160) : short(-160);
                }
              }
              else if (PosNi == 6)
              {
                if (Cpos3 == 1)
                {
                  if (pos.SH[ 1] == 4 - (Cpos3 == pos.wmove) ||
                      pos.SH[12] == 4 - (Cpos3 == pos.wmove) ||
                      pos.SH[19] == 4 - (Cpos3 == pos.wmove) ||
                      pos.SH[30] == 4 - (Cpos3 == pos.wmove))
                  {
                    PrP = 168;
                  }
                }
                else
                {
                  if (pos.SH[(Csm == 1) ? 15 :  6] == 4 - (Cpos3 == pos.wmove) ||
                      pos.SH[(Csm == 1) ? 25 : 16] == 4 - (Cpos3 == pos.wmove))
                  {
                    PrP = -168;
                  }
                }
              }
              else PrP = short(((Cpos3 == 1) ? 1 : -1) * ((PosNi == 0) ? 200 : 208));
            }
          }
        }
      }
      if (PrP == 0)
      {
        unsigned char HwoBD = 0;
        char NShSBD = 0;
        for (int i = 0; i < 8; i++)
        {
          char ShBD = pos.SH[PoleToNum(i, 7 - i)];
          if (ShBD == 3 + pos.wmove) {HwoBD |= 2; NShSBD++;}
          else if (ShBD == 4 - pos.wmove) {HwoBD |= 1; NShSBD++;}
        }
        if (NDM1 >= 3 && NDM0 == 1 && NSH1 == 0 && NSH0 == 0 && HwoBD == 2)
        {
          if (NShSBD >= 1) val -= NShSBD - 1;
          if (pos.SH[ 3] == 3 + pos.wmove) val--;
          if (pos.SH[28] == 3 + pos.wmove) val--;
          char Drg1 = 0, DrgPS = 0;
          bool Drg1p = 0; 
          for (int i = 0; i < 7; i++)
          {
            char Sh7D = pos.SH[PoleToNum(i, i + 1)];
            if (Sh7D == 3 + pos.wmove) {Drg1++; DrgPS |= 1;}
            else if (Sh7D == 4 - pos.wmove) Drg1p = 1;
            Sh7D = pos.SH[PoleToNum(i + 1, i)];
            if (Sh7D == 3 + pos.wmove) {Drg1++; DrgPS |= 2;}
            else if (Sh7D == 4 - pos.wmove) Drg1p = 1;
          }
          if (pos.SH[0] == 3 + pos.wmove || pos.SH[4] == 3 + pos.wmove ||
              pos.SH[27] == 3 + pos.wmove || pos.SH[31] == 3 + pos.wmove)
                  {if (Drg1p) val += 4; else val -= 1;}
          if ((pos.SH[14] == 3 + pos.wmove) == (pos.SH[17] == 3 + pos.wmove))
              {if (Drg1 == 1) val += 2;}
          else
          {
            if (Drg1 >= 2)
            {
              if (Drg1 > 2) val -= 1;
              if (DrgPS == 3) val += 4;
              if (Drg1p) val += 4; else val += 16;
              if (!Drg1p && DrgPS)
              {
                Drg1 = 0; Drg1p = 0; DrgPS = 0;
                for (int i = 0; i < 6; i++)
                {
                  char Sh7D = pos.SH[PoleToNum(i, 5 - i)];
                  if (Sh7D == 3 + pos.wmove) {Drg1++; DrgPS |= 1;}
                  else if (Sh7D == 4 - pos.wmove) Drg1p = 1;
                  Sh7D = pos.SH[PoleToNum(i + 2, 7 - i)];
                  if (Sh7D == 3 + pos.wmove) {Drg1++; DrgPS |= 2;}
                  else if (Sh7D == 4 - pos.wmove) Drg1p = 1;
                }
                if (pos.SH[2] == 3 + pos.wmove || pos.SH[11] == 3 + pos.wmove ||
                    pos.SH[20] == 3 + pos.wmove || pos.SH[29] == 3 + pos.wmove)
                        val += 4;
                if ((pos.SH[14] == 3 + pos.wmove)
                    ? (pos.SH[13] == 3 + pos.wmove || pos.SH[22] == 3 + pos.wmove)
                    : (pos.SH[ 9] == 3 + pos.wmove || pos.SH[18] == 3 + pos.wmove))
                {
                  if (Drg1 >= 2)
                  {
                    if (DrgPS == 3) val += 4;
                    if (Drg1p) val += 4; else val += 16;
                  }
                  else if (Drg1 == 1) val += 1;
                }
                else if (Drg1 == 1) val += 2;
              }
            }
          }
        }
        else if (NDM0 >= 3 && NDM1 == 1 && NSH0 == 0 && NSH1 == 0 && HwoBD == 1)
        {
          if (NShSBD >= 1) val += NShSBD - 1;
          if (pos.SH[ 3] == 4 - pos.wmove) val++;
          if (pos.SH[28] == 4 - pos.wmove) val++;
          char Drg1 = 0, DrgPS = 0;
          bool Drg1p = 0;
          for (int i = 0; i < 7; i++)
          {
            char Sh7D = pos.SH[PoleToNum(i, i + 1)];
            if (Sh7D == 4 - pos.wmove) {Drg1++; DrgPS |= 1;}
            else if (Sh7D == 3 + pos.wmove) Drg1p = 1;
            Sh7D = pos.SH[PoleToNum(i + 1, i)];
            if (Sh7D == 4 - pos.wmove) {Drg1++; DrgPS |= 2;}
            else if (Sh7D == 3 + pos.wmove) Drg1p = 1;
          }
          if (pos.SH[0] == 4 - pos.wmove || pos.SH[4] == 4 - pos.wmove ||
              pos.SH[27] == 4 - pos.wmove || pos.SH[31] == 4 - pos.wmove)
                  {if (Drg1p) val -= 4; else val += 1;}
          if ((pos.SH[14] == 4 - pos.wmove) == (pos.SH[17] == 4 - pos.wmove))
              {if (Drg1 == 1) val -= 2;}
          else
          {
            if (Drg1 >= 2)
            {
              if (Drg1 > 2) val += 1;
              if (DrgPS == 3) val -= 4;
              if (Drg1p) val -= 4; else val -= 16;
              if (!Drg1p && DrgPS)
              {
                Drg1 = 0; Drg1p = 0; DrgPS = 0;
                for (int i = 0; i < 6; i++)
                {
                  char Sh7D = pos.SH[PoleToNum(i, 5 - i)];
                  if (Sh7D == 4 - pos.wmove) {Drg1++; DrgPS |= 1;}
                  else if (Sh7D == 3 + pos.wmove) Drg1p = 1;
                  Sh7D = pos.SH[PoleToNum(i + 2, 7 - i)];
                  if (Sh7D == 4 - pos.wmove) {Drg1++; DrgPS |= 2;}
                  else if (Sh7D == 3 + pos.wmove) Drg1p = 1;
                }
                if (pos.SH[2] == 4 - pos.wmove || pos.SH[11] == 4 - pos.wmove ||
                    pos.SH[20] == 4 - pos.wmove || pos.SH[29] == 4 - pos.wmove)
                {
                  val -= 4;
                }
                if ((pos.SH[14] == 4 - pos.wmove)
                    ? (pos.SH[13] == 4 - pos.wmove || pos.SH[22] == 4 - pos.wmove)
                    : (pos.SH[ 9] == 4 - pos.wmove || pos.SH[18] == 4 - pos.wmove))
                {
                  if (Drg1 >= 2)
                  {
                    if (DrgPS == 3) val -= 4;
                    if (Drg1p) val -= 4; else val -= 16;
                  }
                  else if (Drg1 == 1) val -= 1;
                }
                else if (Drg1 == 1) val -= 2;
              }
            }
          }
        }
        else if (NDM1 >= 3 && NDM0 == 1 && NSH1 == 0 && NSH0 == 0 && HwoBD == 1)
        {
          char Best4P[4][2] = {{0,9}, {4,13}, {31,22}, {27,18}};
          char Add = 0;
          for (int i = 0; i < 4; i++)
          {
            Add |= char((pos.SH[Best4P[i][0]] == 3 + pos.wmove) * 3 +
                        (pos.SH[Best4P[i][1]] == 3 + pos.wmove));
          }
          if (Add >= 4) val += 3;
          else if (Add == 3) val += 2;
          else if (Add >= 1) val += 1;
        }
        else if (NDM0 >= 3 && NDM1 == 1 && NSH0 == 0 && NSH1 == 0 && HwoBD == 2)
        {
          char Best4P[4][2] = {{0,9}, {4,13}, {31,22}, {27,18}};
          char Add = 0;
          for (int i = 0; i < 4; i++)
          {
            Add |= char((pos.SH[Best4P[i][0]] == 4 - pos.wmove) * 3 +
                        (pos.SH[Best4P[i][1]] == 4 - pos.wmove));
          }
          if (Add >= 4) val -= 3;
          else if (Add == 3) val -= 2;
          else if (Add >= 1) val -= 1;
        }
      }
      else val += PrP;
    }
  }
  return val;
}

template <class tvalue>
tvalue TBaseCompPlayer<tvalue>::GetFullValue(const Position &pos, int num)
{
  if (!pos.AllCanMove() && !pos.AllCanEat()) return GetLossValue(pos);
  else return GetValue(pos, num);
}

template <class tvalue>
tvalue TBaseCompPlayer<tvalue>::FindBMove(Z &z, int num,
                           PMvv *pmv, int zn, tvalue a, tvalue b)
{
  assert(b > a);
  assert(num >= 0);
  if (num >= 3 && draw) draw(data);
  int nlast = z.narr;
  if (zn < 0) {z.AddPos(); z.array[zn = z.narr++] = *pmv;}
  if (pmv) pmv->mv[0] = 0;
  int n0 = z.narr;
  int was_eat = z.FindAllMoves(z.array[zn], num <= 0);
  int n1 = z.narr;
  tvalue val;
  if (n1 == n0)
  {
    assert(!z.array[zn].pos.AllCanEat());
    assert(num == 0 || !z.array[zn].pos.AllCanMove());
    if (num > 0 || !z.array[zn].pos.AllCanMove()) val = GetLossValue(z.array[zn].pos);
    else val = GetValue(z.array[zn].pos, 0);
  }
  else if (pmv && n1 == n0 + 1 && nlast < n0)
  {
    *pmv = z.array[n0];
    if (!z.array[n0].pos.AllCanMove() && !z.array[n0].pos.AllCanMove())
    {
      val = -1 - GetLossValue(z.array[n0].pos);
    }
    else
    {
      val = -GetValue(z.array[n0].pos, 0);
      n1 = -1;
    }
  }
  else
  {
    int k, opt;
    if (num >= 2)
    {
      if (pmv && n1 > n0 + 1)
      {
        for (k = 0; k < 2*(n1 - n0); k++)
        {
          int i0 = n0 + random(n1 - n0), i1 = n0 + random(n1 - n0 - 1);
          if (i1 >= i0) i1++;
          PMvv t_pmv = z.array[i0];
          z.array[i0] = z.array[i1];
          z.array[i1] = t_pmv;
        }
      }
      for (k = n0; k < n1; k++) z.array[k].val = GetFullValue(z.array[k].pos);
      z.Sort(n0, n1);
    }
    tvalue cc = 2 * win_val;
    tvalue dval = was_eat ? 0 : GetValue(z.array[zn].pos, num);
    tvalue aa = -b, bb = -a;
    if (aa < -win_val) aa--;
    else if (aa > win_val) aa++;
    else aa += dval;
    if (bb < -win_val) bb--;
    else if (bb > win_val) bb++;
    else bb += dval;
    for (k = n0; k < n1 && bb > aa; k++)
    {
      tvalue vk;
      vk = FindBMove(z, num-1+was_eat, 0, k, aa, bb);
      if (vk < cc)
      {
        opt = k; cc = vk;
        if (bb > cc) bb = cc;
      }
    }
    if (cc < -win_val) cc++;
    else if (cc > win_val) cc--;
    else cc -= dval;
    val = -cc;
    assert(opt >= n0 && opt < n1);
    if (pmv) *pmv = z.array[opt];
  }
  z.array[zn].val = val;
  z.narr = nlast;
  if (pmv)
  {
    if (n1 >= 0) printf("Checkers: value = %ld\n", val);
    else printf("Checkers: value = ?\n");
  }
  return val;
}

template <class tvalue>
int TBaseCompPlayer<tvalue>::Move(PMv &pmv)
{
  Z z;
  PMvv zpmv;
  (PMv&)zpmv = pmv;
  if (draw) draw(data, 1);
  FindBMove(z, 6, &zpmv);
  if (draw) draw(data, -1);
  if (zpmv.mv[0] == 0) return 0;
  pmv = zpmv;
  return 1;
}

typedef TBaseCompPlayer<long> TComputerPlayer;

#endif  //_HEADER_PLAYER_H
