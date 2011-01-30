#ifndef _HEADER_TMP_PLAYER_H
#define _HEADER_TMP_PLAYER_H

#include "player.h"

class CompPosition : public Position
{
public:
  bool Move;

  CompPosition() : Position() {Move = 1;}
  CompPosition(CompPosition &p, bool IsChange = 0) : Position(p)
                                     {Move = p.Move + IsChange;}
  CompPosition& operator=(CompPosition &p)
         {for(int i = 0; i < 32; i++) SH[i] = p.SH[i]; Move = p.Move; return *this;}
  CompPosition& Init(Position &p, bool MoveColor = 1)
       {for(int i = 0; i < 32; i++) SH[i] = p.SH[i]; Move = MoveColor; return *this;}
};

namespace ComputerMove
{
#define BCM_START   -256
#define BCM_ISM1    -257

  class BestMove;
  bool MoveEat(BestMove* BM);
  bool MovePr(BestMove* BM);

  class BestMove
  {
  public:
     BestMove (CompPosition &Pos1, BestMove *par = 0)
                        {Pos = Pos1; Parent = par; NMax = 0;}

     void StartMove()
     {
       if (Parent->NomP == BCM_ISM1) return;
       if (Parent->NomP == BCM_START) NomP = 258; else NomP = Parent->NomP;
       if (!MoveEat(this))
       {
         bool IsNP256 = 0, IsNP258 = 0;
         if (NomP == 258) IsNP258 = 1;
         if (NomP == 256)
         {
           IsNP256 = 1; int NSH1 = 0; int NSH0 = 0;
           for (int i = 0; i < 32; i++)
           {
             if (Pos.SH[i] == 1 + Pos.Move) NSH1 += 1;
             else if (Pos.SH[i] == 2 - Pos.Move) NSH0 += 1;
             else if (Pos.SH[i] == 3 + Pos.Move) NSH1 += 6;
             else if (Pos.SH[i] == 4 - Pos.Move) NSH0 += 6;
           }
           int NSH = NSH0 * NSH1;
           if (NSH > 100) NomP = 0; else if (NSH > 30) NomP = 1;
                     else if (NSH > 5) NomP = 2; else NomP = 3;
         }
         else if (NomP == 0) NomP = BCM_ISM1; else NomP--;
         if (!MovePr(this))
         {
           NextPreim = -16777472;
           for (int i = 0; i < 32; i++)
           {
             if (Pos.SH[i] == 1 + Pos.Move) NextPreim -= 512;
             else if (Pos.SH[i] == 2 - Pos.Move) NextPreim -= 32;
             else if (Pos.SH[i] == 3 + Pos.Move) NextPreim -= 4096;
             else if (Pos.SH[i] == 4 - Pos.Move) NextPreim -= 32;
           }
         }
         else if (NomP == BCM_ISM1)
         {
           char NomDM0 = 0, NomDM1 = 0;
           NextPreim = 0;
           for (int i = 0; i < 32; i++)
           {
             short PreimSHPos[32] = {243, 243, 243, 245,
                                     240, 240, 240, 240,
                                     244, 244, 244, 244,
                                     245, 248, 248, 245,
                                     249, 250, 250, 248,
                                     256, 260, 260, 256,
                                     280, 280, 280, 260,
                                     280, 280, 280, 280};
             if (Pos.SH[i] == 1 + Pos.Move)
               NextPreim += PreimSHPos[Pos.Move ? (31 - i) : i];
             else if (Pos.SH[i] == 2 - Pos.Move)
               NextPreim -= PreimSHPos[Pos.Move ? i : (31 - i)];
             else if (Pos.SH[i] == 3 + Pos.Move) NomDM1++;
             else if (Pos.SH[i] == 4 - Pos.Move) NomDM0++;
           }
           if (NomDM1 > 0)
           {
             NextPreim += 560; NomDM1--;
             if (NomDM1 > 0)
             {
               NextPreim += 432; NomDM1--;
               NextPreim += long(NomDM1) * 384;
             }
           }
           if (NomDM0 > 0)
           {
             NextPreim -= 560; NomDM0--;
             if (NomDM0 > 0)
             {
               NextPreim -= 432; NomDM0--;
               NextPreim -= long(NomDM0) * 384;
             }
           }
         }
         else if (IsNP256 && NextPreim < 16777216 && NextPreim > -16777216)
         {
           char NSH1 = 0, NSH0 = 0, NDM1 = 0, NDM0 = 0;
           for (int i = 0; i < 32; i++)
           {
             if (Pos.SH[i] == 1 + Pos.Move) NSH1++;
             else if (Pos.SH[i] == 2 - Pos.Move) NSH0++;
             else if (Pos.SH[i] == 3 + Pos.Move) NDM1++;
             else if (Pos.SH[i] == 4 - Pos.Move) NDM0++;
           }
           if (NDM1 > 0 && NDM0 > 0 && NSH1 + NSH0 < 3)
           {
             unsigned char HwoBD = 0; char Sh0BD = 1, Sh1BD = 1;
             for (int i = 0; i < 8; i++)
             {
               char ShBD = Pos.SH[PoleToNum(i, 7 - i)];
               if (ShBD == 1 + Pos.Move) Sh1BD++;
               else if (ShBD == 2 - Pos.Move) Sh0BD++;
               else if (ShBD == 3 + Pos.Move) HwoBD |= 2;
               else if (ShBD == 4 - Pos.Move) HwoBD |= 1;
             }
             if (HwoBD == 2) NextPreim += 128 / Sh0BD;
             if (HwoBD == 1) NextPreim -= 128 / Sh1BD;
             if (NDM1 >= 3 && NDM0 == 1 && NSH1 == 0 && NSH0 == 0 && HwoBD == 1)
             {
               char Best4P[4][2] = {{0,9}, {4,13}, {31,22}, {27,18}};
               char Add = 0;
               for (int i = 0; i < 4; i++) Add |=
                   (char(Pos.SH[Best4P[i][0]] == 3 + Pos.Move) * 3 +
                    char(Pos.SH[Best4P[i][1]] == 3 + Pos.Move));
               if (Add >= 4) NextPreim += 32; else if (Add == 3) NextPreim += 24;
                   else if (Add >= 1) NextPreim += 16;
             }
             else if (NDM0 >= 3 && NDM1 == 1 && NSH0 == 0 && NSH1 == 0 && HwoBD == 2)
             {
               char Best4P[4][2] = {{0,9}, {4,13}, {31,22}, {27,18}};
               char Add = 0;
               for (int i = 0; i < 4; i++) Add |=
                   (char(Pos.SH[Best4P[i][0]] == 4 - Pos.Move) * 3 +
                    char(Pos.SH[Best4P[i][1]] == 4 - Pos.Move));
               if (Add >= 4) NextPreim -= 32; else if (Add == 3) NextPreim -= 24;
                   else if (Add >= 1) NextPreim -= 16;
             }
           }
           else
           {
             for (int i = 0; i < 32; i++)
             {
               char Color = Pos.SH[i] - 1;
               if (Color == 0 || Color == 1)
               {
                 char qi = Color ? (31 - i) : i;
                 char Zn = Color ? -1 : 1;
                 char PreZ = (Color == Pos.Move) ? 1 : -1;
                 if (Pos.SH[i + Zn * 8] != 2 - Color)
                 {
                   if (qi / 4 == 2)
                   {
                     char IsFree = 0;
                     if (Pos.SH[i - Zn * 4] == 2 - Color) IsFree += 2;
                     else if (qi != 8)
                     {
                       if (Pos.SH[i - Zn    ] == 2 - Color ||
                           Pos.SH[i - Zn * 9] == 2 - Color) IsFree += 2;
                       else if (Color != Pos.Move)
                            if (Pos.SH[i - Zn * 5] == 2 - Color) IsFree++;
                     }
                     if (qi == 11) IsFree += 2;
                     else if (Pos.SH[i + Zn    ] == 2 - Color ||
                              Pos.SH[i - Zn * 3] == 2 - Color ||
                              Pos.SH[i - Zn * 7] == 2 - Color) IsFree += 2;
                     else if (Color != Pos.Move && qi != 10)
                          if (Pos.SH[i - Zn * 2] == 2 - Color) IsFree++;
                     if (IsFree < 3) NextPreim += PreZ * 176 / (1 + NDM0 + NDM1);
                     else if (qi == 9 || qi == 10) NextPreim +=
                              PreZ * 128 / (1 + NDM0 + NDM1);
                   }
                   else if (qi / 4 == 3)
                   {
                     char IsFree = 0;
                     if (Pos.SH[i - Zn * 12] == 2 - Color)
                       {if (Color == Pos.Move) IsFree += 11; else IsFree += 12;}
                     else if (Pos.SH[i - Zn * 4] == 2 - Color) IsFree += 11;
                     else if (qi == 15) IsFree += 5;
                     else if (Pos.SH[i - Zn * 7] == 2 - Color) IsFree += 9;
                     else if (Pos.SH[i + Zn] == 2 - Color) IsFree += 8;
                     else if (Pos.SH[i - Zn * 11] == 2 - Color)
                       {if (Color == Pos.Move) IsFree += 5; else IsFree += 7;}
                     else if (Pos.SH[i - Zn * 3] == 2 - Color) IsFree += 5;
                     else if (qi != 14)
                     {
                       if (Pos.SH[i - Zn * 6] == 2 - Color) IsFree += 3;
                       else if (Color != Pos.Move)
                            if (Pos.SH[i - Zn * 10] == 2 - Color) IsFree++;
                     }
                     if (qi == 12) IsFree += 7;
                     else if (Pos.SH[i - Zn * 13] == 2 - Color)
                       {if (Color == Pos.Move) IsFree += 11; else IsFree += 12;}
                     else if (Pos.SH[i - Zn * 5] == 2 - Color) IsFree += 11;
                     else if (Pos.SH[i - Zn * 9] == 2 - Color) IsFree += 9;
                     else if (Pos.SH[i - Zn] == 2 - Color) IsFree += 8;
                     else if (qi != 13)
                     {
                       if (Pos.SH[i - Zn * 14] == 2 - Color)
                         {if (Color == Pos.Move) IsFree += 5; else IsFree += 7;}
                       else if (Pos.SH[i - Zn * 6] == 2 - Color) IsFree += 5;
                       else if (Pos.SH[i - Zn * 10] == 2 - Color) IsFree += 3;
                       else if (Color != Pos.Move && qi != 14)
                            if (Pos.SH[i - Zn * 15] == 2 - Color) IsFree++;
                     }
                     if (IsFree < ((Color == Pos.Move) ? 14 : 12))
                         NextPreim += PreZ * 160 / (1 + NDM0 + NDM1);
                   }
                 }
               }
             }
           }
         }
         else if (IsNP258 && NextPreim < 16777216 && NextPreim > -16777216)
         {
           char NSH1 = 0, NSH0 = 0, NDM1 = 0, NDM0 = 0;
           for (int i = 0; i < 32; i++)
           {
             if (Pos.SH[i] == 1 + Pos.Move) NSH1++;
             else if (Pos.SH[i] == 2 - Pos.Move) NSH0++;
             else if (Pos.SH[i] == 3 + Pos.Move) NDM1++;
             else if (Pos.SH[i] == 4 - Pos.Move) NDM0++;
           }
           if (NDM1 > 0 && NDM0 > 0 && NSH1 == 0 && NSH0 == 0)
           {
             short PrP = 0; char Cpos3 = -1;
             if (NDM1 == 3 && NDM0 == 1 && NSH1 == 0 && NSH0 == 0) Cpos3 = 1;
             else if (NDM1 == 1 && NDM0 == 3 && NSH1 == 0 && NSH0 == 0) Cpos3 = 0;
             if (Cpos3 >= 0)
             {
               for (char Osm = 0; Osm <= 1; Osm++) for (char Csm = 0; Csm <= 1; Csm++)
               {
                 char PosSH[7][3] = {{13, 17, 18}, {6, 17, 18}, {9, 21, 22},
                                     {17, 18, 19}, {9, 10, 15}, {11, 14, 18}, {2, 14, 18}};
                 for (char PosNi = 0; PosNi < 7; PosNi++)
                 {
                   bool IsPosR = 1;
                   for (char ShNi = 0; ShNi < 3; ShNi++)
                   {
                     char DNomSh = (Csm == 1) ? (31 - PosSH[PosNi][ShNi])
                                                    : PosSH[PosNi][ShNi];
                     if (Osm == 1) {int x, y; NumToPole(DNomSh, x, y);
                                             DNomSh = PoleToNum(y, x);}
                     if (Pos.SH[DNomSh] != 3 + (Cpos3 == Pos.Move)) IsPosR = 0;
                   }
                   if (IsPosR)
                   {
                     if (PosNi == 3)
                     {
                       if (Cpos3 == 1) {if (Pos.SH[(Csm == 1) ? 29 : 2] !=
                           4 - (Cpos3 == Pos.Move) && Pos.SH[(Csm == 1) ? 11 : 20]
                             != 4 - (Cpos3 == Pos.Move)) PrP = 216;}
                       else
                       {
                         bool PrPZ = 1;
                         for (int i = 0; i < 6; i++)
                            if (Pos.SH[PoleToNum((Csm == 1) ? (i + 2) : i,
                               (Csm == 1) ? (7 - i) : (5 - i))] ==
                                4 - (Cpos3 == Pos.Move)) PrPZ = 0;
                         if (PrPZ) PrP = -216;
                       }
                     }
                     else if (PosNi == 4)
                     {
                       if (Cpos3 == 1)
                         {if (Pos.SH[ 0] != 4 - (Cpos3 == Pos.Move)
                           && Pos.SH[ 4] != 4 - (Cpos3 == Pos.Move)
                           && Pos.SH[27] != 4 - (Cpos3 == Pos.Move)
                           && Pos.SH[31] != 4 - (Cpos3 == Pos.Move)) PrP = 216;}
                       else {if (Pos.SH[(Csm == Osm) ?  4 :  0] != 4 - (Cpos3 == Pos.Move)
                              && Pos.SH[(Csm == Osm) ?  8 :  5] != 4 - (Cpos3 == Pos.Move)
                              && Pos.SH[(Csm == Osm) ? 26 : 23] != 4 - (Cpos3 == Pos.Move)
                              && Pos.SH[(Csm == Osm) ? 31 : 27] != 4 - (Cpos3 == Pos.Move))
                                         PrP = -216;}
                     }
                     else if (PosNi == 5)
                     {
                       char DNomSh = (Cpos3 == 1) ? ((Osm == 1) ? 16 : 6)
                                                  : ((Osm == 1) ? 20 : 2);
                       if (Csm == 1) DNomSh = 31 - DNomSh;
                       if (Pos.SH[DNomSh] == 4 - (Cpos3 == Pos.Move))
                           PrP = (Cpos3 == 1) ? 160 : -160;
                     }
                     else if (PosNi == 6)
                     {
                       if (Cpos3 == 1)
                            {if (Pos.SH[ 1] == 4 - (Cpos3 == Pos.Move)
                              || Pos.SH[12] == 4 - (Cpos3 == Pos.Move)
                              || Pos.SH[19] == 4 - (Cpos3 == Pos.Move)
                              || Pos.SH[30] == 4 - (Cpos3 == Pos.Move)) PrP = 168;}
                       else
                         {if (Pos.SH[(Csm == 1) ? 15 :  6] == 4 - (Cpos3 == Pos.Move)
                           || Pos.SH[(Csm == 1) ? 25 : 16] == 4 - (Cpos3 == Pos.Move))
                                      PrP = -168;}
                     }
                     else PrP = ((Cpos3 == 1) ? 1 : -1) * ((PosNi == 0) ? 200 : 208);
                   }
                 }
               }
             }
             if (PrP == 0)
             {
               unsigned char HwoBD = 0; char NShSBD = 0;
               for (int i = 0; i < 8; i++)
               {
                 char ShBD = Pos.SH[PoleToNum(i, 7 - i)];
                 if (ShBD == 3 + Pos.Move) {HwoBD |= 2; NShSBD++;}
                 else if (ShBD == 4 - Pos.Move) {HwoBD |= 1; NShSBD++;}
               }
               if (NDM1 >= 3 && NDM0 == 1 && NSH1 == 0 && NSH0 == 0 && HwoBD == 2)
               {
                 if (NShSBD >= 1) NextPreim -= NShSBD - 1;
                 if (Pos.SH[ 3] == 3 + Pos.Move) NextPreim--;
                 if (Pos.SH[28] == 3 + Pos.Move) NextPreim--;
                 char Drg1 = 0; bool Drg1p = 0; char DrgPS = 0;
                 for (int i = 0; i < 7; i++)
                 {
                   char Sh7D = Pos.SH[PoleToNum(i, i + 1)];
                   if (Sh7D == 3 + Pos.Move) {Drg1++; DrgPS |= 1;}
                   else if (Sh7D == 4 - Pos.Move) Drg1p = 1;
                   Sh7D = Pos.SH[PoleToNum(i + 1, i)];
                   if (Sh7D == 3 + Pos.Move) {Drg1++; DrgPS |= 2;}
                   else if (Sh7D == 4 - Pos.Move) Drg1p = 1;
                 }
                 if (Pos.SH[0] == 3 + Pos.Move || Pos.SH[4] == 3 + Pos.Move ||
                     Pos.SH[27] == 3 + Pos.Move || Pos.SH[31] == 3 + Pos.Move)
                         {if (Drg1p) NextPreim += 4; else NextPreim -= 1;}
                 if ((Pos.SH[14] == 3 + Pos.Move) == (Pos.SH[17] == 3 + Pos.Move))
                     {if (Drg1 == 1) NextPreim += 2;}
                 else
                 {
                   if (Drg1 >= 2)
                   {
                     if (Drg1 > 2) NextPreim -= 1;
                     if (DrgPS == 3) NextPreim += 4;
                     if (Drg1p) NextPreim += 4; else NextPreim += 16;
                     if (!Drg1p && DrgPS)
                     {
                       Drg1 = 0; Drg1p = 0; DrgPS = 0;
                       for (int i = 0; i < 6; i++)
                       {
                         char Sh7D = Pos.SH[PoleToNum(i, 5 - i)];
                         if (Sh7D == 3 + Pos.Move) {Drg1++; DrgPS |= 1;}
                         else if (Sh7D == 4 - Pos.Move) Drg1p = 1;
                         Sh7D = Pos.SH[PoleToNum(i + 2, 7 - i)];
                         if (Sh7D == 3 + Pos.Move) {Drg1++; DrgPS |= 2;}
                         else if (Sh7D == 4 - Pos.Move) Drg1p = 1;
                       }
                       if (Pos.SH[2] == 3 + Pos.Move || Pos.SH[11] == 3 + Pos.Move ||
                           Pos.SH[20] == 3 + Pos.Move || Pos.SH[29] == 3 + Pos.Move)
                               NextPreim += 4;
                       if ((Pos.SH[14] == 3 + Pos.Move)
                          ? (Pos.SH[13] == 3 + Pos.Move || Pos.SH[22] == 3 + Pos.Move)
                          : (Pos.SH[ 9] == 3 + Pos.Move || Pos.SH[18] == 3 + Pos.Move))
                       {
                         if (Drg1 >= 2)
                         {
                           if (DrgPS == 3) NextPreim += 4;
                           if (Drg1p) NextPreim += 4; else NextPreim += 16;
                         }
                         else if (Drg1 == 1) NextPreim += 1;
                       }
                       else if (Drg1 == 1) NextPreim += 2;
                     }
                   }
                 }
               }
               else if (NDM0 >= 3 && NDM1 == 1 && NSH0 == 0 && NSH1 == 0 && HwoBD == 1)
               {
                 if (NShSBD >= 1) NextPreim += NShSBD - 1;
                 if (Pos.SH[ 3] == 4 - Pos.Move) NextPreim++;
                 if (Pos.SH[28] == 4 - Pos.Move) NextPreim++;
                 char Drg1 = 0; bool Drg1p = 0; char DrgPS = 0;
                 for (int i = 0; i < 7; i++)
                 {
                   char Sh7D = Pos.SH[PoleToNum(i, i + 1)];
                   if (Sh7D == 4 - Pos.Move) {Drg1++; DrgPS |= 1;}
                   else if (Sh7D == 3 + Pos.Move) Drg1p = 1;
                   Sh7D = Pos.SH[PoleToNum(i + 1, i)];
                   if (Sh7D == 4 - Pos.Move) {Drg1++; DrgPS |= 2;}
                   else if (Sh7D == 3 + Pos.Move) Drg1p = 1;
                 }
                 if (Pos.SH[0] == 4 - Pos.Move || Pos.SH[4] == 4 - Pos.Move ||
                     Pos.SH[27] == 4 - Pos.Move || Pos.SH[31] == 4 - Pos.Move)
                         {if (Drg1p) NextPreim -= 4; else NextPreim += 1;}
                 if ((Pos.SH[14] == 4 - Pos.Move) == (Pos.SH[17] == 4 - Pos.Move))
                     {if (Drg1 == 1) NextPreim -= 2;}
                 else
                 {
                   if (Drg1 >= 2)
                   {
                     if (Drg1 > 2) NextPreim += 1;
                     if (DrgPS == 3) NextPreim -= 4;
                     if (Drg1p) NextPreim -= 4; else NextPreim -= 16;
                     if (!Drg1p && DrgPS)
                     {
                       Drg1 = 0; Drg1p = 0; DrgPS = 0;
                       for (int i = 0; i < 6; i++)
                       {
                         char Sh7D = Pos.SH[PoleToNum(i, 5 - i)];
                         if (Sh7D == 4 - Pos.Move) {Drg1++; DrgPS |= 1;}
                         else if (Sh7D == 3 + Pos.Move) Drg1p = 1;
                         Sh7D = Pos.SH[PoleToNum(i + 2, 7 - i)];
                         if (Sh7D == 4 - Pos.Move) {Drg1++; DrgPS |= 2;}
                         else if (Sh7D == 3 + Pos.Move) Drg1p = 1;
                       }
                       if (Pos.SH[2] == 4 - Pos.Move || Pos.SH[11] == 4 - Pos.Move ||
                           Pos.SH[20] == 4 - Pos.Move || Pos.SH[29] == 4 - Pos.Move)
                               NextPreim -= 4;
                       if ((Pos.SH[14] == 4 - Pos.Move)
                          ? (Pos.SH[13] == 4 - Pos.Move || Pos.SH[22] == 4 - Pos.Move)
                          : (Pos.SH[ 9] == 4 - Pos.Move || Pos.SH[18] == 4 - Pos.Move))
                       {
                         if (Drg1 >= 2)
                         {
                           if (DrgPS == 3) NextPreim -= 4;
                           if (Drg1p) NextPreim -= 4; else NextPreim -= 16;
                         }
                         else if (Drg1 == 1) NextPreim -= 1;
                       }
                       else if (Drg1 == 1) NextPreim -= 2;
                     }
                   }
                 }
               }
               else if (NDM1 >= 3 && NDM0 == 1 && NSH1 == 0 && NSH0 == 0 && HwoBD == 1)
               {
                 char Best4P[4][2] = {{0,9}, {4,13}, {31,22}, {27,18}};
                 char Add = 0;
                 for (int i = 0; i < 4; i++) Add |=
                     (char(Pos.SH[Best4P[i][0]] == 3 + Pos.Move) * 3 +
                      char(Pos.SH[Best4P[i][1]] == 3 + Pos.Move));
                 if (Add >= 4) NextPreim += 3; else if (Add == 3) NextPreim += 2;
                 else if (Add >= 1) NextPreim += 1;
               }
               else if (NDM0 >= 3 && NDM1 == 1 && NSH0 == 0 && NSH1 == 0 && HwoBD == 2)
               {
                 char Best4P[4][2] = {{0,9}, {4,13}, {31,22}, {27,18}};
                 char Add = 0;
                 for (int i = 0; i < 4; i++) Add |=
                     (char(Pos.SH[Best4P[i][0]] == 4 - Pos.Move) * 3 +
                      char(Pos.SH[Best4P[i][1]] == 4 - Pos.Move));
                 if (Add >= 4) NextPreim -= 3; else if (Add == 3) NextPreim -= 2;
                 else if (Add >= 1) NextPreim -= 1;
               }
             }
             else NextPreim += PrP;
           }
         }
       }
       Parent->TakeMove(Pos, -NextPreim);
     }

     void TakeMove(CompPosition &Pos1, long Preim)
     {
       if (Preim >= 16777216) Preim--; else if (Preim <= -16777216) Preim++;
       bool b = (NMax == 0 || Preim > NextPreim); if (NMax == 0) NMax = 1;
       if (!b && Preim == NextPreim) {b = (random(NMax + 1) == 0); NMax++;}
       if (b) {if (NomP == BCM_START) *NextPos = Pos1; NextPreim = Preim;}
     }

     CompPosition Pos; int NomP; BestMove *Parent;

     CompPosition *NextPos; char NMax; long NextPreim;
  };

  class MoveEatParam
  {
  public:
    char Home;
    char Eaten[15]; char NomEat;
    BestMove* BM;

    MoveEatParam(){}

    char RealPlase(char N)
    {
      char RP;
      if (N == Home) RP = 0; else RP = BM->Pos.SH[N];
      for (int i = 0; i < NomEat; i++) if (N == Eaten[i]) RP = 0;
      return RP;
    }
  };

  void EatContinue(MoveEatParam& MEP, bool IsDMC, char Start, char Kurs)
  {
    bool IsCanEat = 0;
    int x, y; NumToPole(Start, x, y);
    if (y == 7 - MEP.BM->Pos.Move * 7) IsDMC = 1;
    if (IsDMC)
    {
      int KursY = Kurs / abs(Kurs); int KursX = Kurs - KursY * 2;
      for (int i = x, j = y;
           i >= 0 && j >= 0 && i < 8 && j < 8;
              i += KursX, j += KursY)
      {
        int Shash = MEP.RealPlase(PoleToNum(i, j));
        if (Shash == 2 - MEP.BM->Pos.Move || Shash == 4 - MEP.BM->Pos.Move)
        {
         if (i + KursX >= 0 && j + KursY >= 0 && i + KursX < 8 && j + KursY < 8)
          if (MEP.RealPlase(PoleToNum(i + KursX, j + KursY)) == 0)
         {
           IsCanEat = 1;
           MEP.Eaten[MEP.NomEat++] = PoleToNum(i, j);
           EatContinue(MEP, 1, PoleToNum(i + KursX, j + KursY),
                                     KursX + 2 * KursY);
           MEP.NomEat--;
         }
         break;
        }
        else if (Shash == 1 + MEP.BM->Pos.Move || Shash == 3 + MEP.BM->Pos.Move) break;
        else for (int Rotate = -1; Rotate <= 1; Rotate += 2)
         for (int i1 = i + KursY * Rotate, j1 = j - KursX * Rotate;
                  i1 >= 0 && j1 >= 0 && i1 < 8 && j1 < 8;
                  i1 += KursY * Rotate, j1 -= KursX * Rotate)
         {
           int Shash = MEP.RealPlase(PoleToNum(i1, j1));
           if (Shash == 2 - MEP.BM->Pos.Move || Shash == 4 - MEP.BM->Pos.Move)
           {
             if (i1 + KursY * Rotate >= 0 && j1 - KursX * Rotate >= 0 &&
                   i1 + KursY * Rotate < 8 && j1 - KursX * Rotate < 8)
               if (MEP.RealPlase(PoleToNum(i1 + KursY * Rotate, j1 - KursX * Rotate)) == 0)
               {
                 IsCanEat = 1;
                 MEP.Eaten[MEP.NomEat++] = PoleToNum(i1, j1);
                 EatContinue(MEP, 1, PoleToNum(i1 + KursY * Rotate,
                                    j1 - KursX * Rotate),
                              KursY * Rotate - 2 * KursX * Rotate);
                 MEP.NomEat--;
               }
               break;
           }
           else if (Shash == 1 + MEP.BM->Pos.Move || Shash == 3 + MEP.BM->Pos.Move) break;
         }
      }
      if (!IsCanEat)
      {
        for (int ii = x, jj = y; ii >= 0 && jj >= 0 &&
              ii < 8 && jj < 8; ii += KursX, jj += KursY)
        {
          if (MEP.RealPlase(PoleToNum(ii, jj)) != 0) break;
          BestMove BMove(MEP.BM->Pos, MEP.BM); BMove.Pos.SH[MEP.Home] = 0;
          for (int i = 0; i < MEP.NomEat; i++) {BMove.Pos.SH[MEP.Eaten[i]] = 0;}
          BMove.Pos.SH[PoleToNum(ii, jj)] = 3 + MEP.BM->Pos.Move;
          BMove.Pos.Move = !BMove.Pos.Move;
          BMove.StartMove();
        }
      }
    }
    else
    {
      for (int i = -1; i <= 1; i += 2) for (int j = -1; j <= 1; j += 2)
           if (i + 2 * j != -Kurs)  if (x + 2 * i >= 0
                && y + 2 * j >= 0 && x + 2*i < 8 && y + 2 * j < 8)
      {
        char FESH = PoleToNum(x + i, y + j); char FESHrp = MEP.RealPlase(FESH);
        char FMSH = PoleToNum(x + 2 * i, y + 2 * j);
        if ((FESHrp == 2 - MEP.BM->Pos.Move || FESHrp == 4 - MEP.BM->Pos.Move)
                     && MEP.RealPlase(FMSH) == 0)
        {
          IsCanEat = 1;
          MEP.Eaten[MEP.NomEat++] = FESH;
          EatContinue(MEP, 0, FMSH, i + 2 * j);
          MEP.NomEat--;
        }
      }
      if (!IsCanEat)
      {
        BestMove BMove(MEP.BM->Pos, MEP.BM); BMove.Pos.SH[MEP.Home] = 0;
        for (int i = 0; i < MEP.NomEat; i++) {BMove.Pos.SH[MEP.Eaten[i]] = 0;}
        BMove.Pos.SH[Start] = 1 + MEP.BM->Pos.Move;
        BMove.Pos.Move = !BMove.Pos.Move;
        BMove.StartMove();
      }
    }
  }

  bool MoveEat(BestMove* BM)
  {
     bool IsCanEat = 0;
     MoveEatParam MEP; MEP.BM = BM;
     for (MEP.Home = 0; MEP.Home < 32; MEP.Home++)
     {
       if (BM->Pos.SH[MEP.Home] == 1 + BM->Pos.Move)
       {
         int x, y; NumToPole(MEP.Home, x, y);
         for (int i = -1; i <= 1; i += 2) for (int j = -1; j <= 1; j += 2)
            if (x + 2*i >= 0 && y + 2*j >= 0 && x + 2*i < 8 && y + 2*j < 8)
         {
           int FESH = PoleToNum(x + i, y + j);
           int FMSH = PoleToNum(x + 2 * i, y + 2 * j);
           if ((BM->Pos.SH[FESH] == 2 - BM->Pos.Move || BM->Pos.SH[FESH] == 4 - BM->Pos.Move)
                      && BM->Pos.SH[FMSH] == 0)
           {
             IsCanEat = 1;
             MEP.NomEat = 1; MEP.Eaten[0] = FESH;
             EatContinue(MEP, 0, FMSH, i + 2 * j);
           }
         }
       }
       else if (BM->Pos.SH[MEP.Home] == 3 + BM->Pos.Move)
       {
         int x, y; NumToPole(MEP.Home, x, y);
         for (int i = -1; i <= 1; i += 2) for (int j = -1; j <= 1; j += 2)
              for (int a = x + i, b = y + j;
                   a >= 0 && b >= 0 && a < 8 && b < 8;
                         a += i, b += j)
         {
           char Shash = PoleToNum(a, b);
           char Shash1 = PoleToNum(a + i, b + j);
           if (BM->Pos.SH[Shash] == 2 - BM->Pos.Move || BM->Pos.SH[Shash] == 4 - BM->Pos.Move)
           {
             if (a + i >= 0 && b + j >= 0 && a + i < 8 && b + j < 8)
               if (BM->Pos.SH[Shash1] == 0)
             {
               IsCanEat = 1;
               MEP.NomEat = 1; MEP.Eaten[0] = Shash;
               EatContinue(MEP, 1, Shash1, i + 2 * j);
             }
             break;
           }
           else if (BM->Pos.SH[Shash] == 1 + BM->Pos.Move || BM->Pos.SH[Shash] == 3 + BM->Pos.Move) break;
         }
       }
     }
     return IsCanEat;
  }

  bool MovePr(BestMove* BM)
  {
    bool IsCanMove = 0;
    for (int N = 0; N < 32; N++)
    {
      if (BM->Pos.SH[N] == 1 + BM->Pos.Move)
      {
        int x, y; NumToPole(N, x, y);
        int j = 1 - 2 * BM->Pos.Move;
        for (int i = -1; i <= 1; i += 2)
           if (x + i >= 0 && y + j >= 0 && x + i < 8 && y + j < 8)
        {
          int FMSH = PoleToNum(x + i, y + j);
          if (BM->Pos.SH[FMSH] == 0)
          {
            IsCanMove = 1;
            BestMove BMove(BM->Pos, BM); BMove.Pos.SH[N] = 0;
            BMove.Pos.SH[FMSH] = 1 + BM->Pos.Move + 2 *
                                    (y + j == 7 - BM->Pos.Move * 7);
            BMove.Pos.Move = !BMove.Pos.Move;
            BMove.StartMove();
          }
        }
      }
      else if (BM->Pos.SH[N] == 3 + BM->Pos.Move)
      {
        int x, y; NumToPole(N, x, y);
        for (int i = -1; i <= 1; i += 2) for (int j = -1; j <= 1; j += 2)
             for (int a = x + i, b = y + j;
                  a >= 0 && b >= 0 && a < 8 && b < 8;
                        a += i, b += j)
        {
          if (BM->Pos.SH[PoleToNum(a, b)] != 0) break;
          IsCanMove = 1;
          BestMove BMove(BM->Pos, BM); BMove.Pos.SH[N] = 0;
          BMove.Pos.SH[PoleToNum(a, b)] = 3 + BM->Pos.Move;
          BMove.Pos.Move = !BMove.Pos.Move;
          BMove.StartMove();
        }
      }
    }
    return IsCanMove;
  }

  int StartMove(Position& Pos, bool CompColor)
  {
     CompPosition Pos1; Pos1.Init(Pos, CompColor);
     BestMove MainMove(Pos1); MainMove.NomP = BCM_START;
     MainMove.NextPos = &Pos1;
     if (!MoveEat(&MainMove)) if (!MovePr(&MainMove)) return 1;
     Pos = Pos1; MainMove.Pos = Pos1;
     MainMove.NomP = BCM_ISM1;
     if (!MoveEat(&MainMove)) if (!MovePr(&MainMove)) return -1;
     return 0;
  }

#undef    BCM_ISM1
#undef    BCM_START
}


class TemporaryPlayer : public TChPlayer
{
public:
  TemporaryPlayer() {}

  virtual int Move(PMv &pmv);
};

int TemporaryPlayer::Move(PMv &pmv)
{
  TComputerPlayer::Z z;
  z.FindAllMoves(pmv);
  if (z.narr <= 0) return 0;
  int r, s = ComputerMove::StartMove(pmv.pos, pmv.pos.wmove);
  if (s > 0) return 0;
  for (r = 0; r < z.narr; r++)
  {
    if (memcmp(z.array[r].pos.SH, pmv.pos.SH, sizeof(pmv.pos.SH)) == 0)
    {
      pmv = z.array[r];
      return 1;
    }
  }
  return 0;
}

#endif  //_HEADER_TMP_PLAYER_H
