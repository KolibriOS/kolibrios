/*====================================================================/*
  opcodes.c -> This file executes the single-byte Z80 opcodes.

  The CPU fetchs the byte pointed by the PC register (Program Counter)
  into de IR (Instruction Register) and decodes it. The value of this
  fetched byte (opcode) determines what operation the CPU must do.
  On Z80 (which uses 8 bit for the IW register) this means that we
  can have 256 (2^8) different opcodes. The z80 uses a simple trick
  called PREFIXES to obtain more opcodes by using more than one byte
  in the decoding (see opcodes_cb.c to know how it does it).

  This file executes the whole list of single-byte opcodes.

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 Copyright (c) 2000 Santiago Romero Iglesias.
 Email: sromero@escomposlinux.org
 =====================================================================*/

/* About the AddCycles(4) -> Remember that reading from memory
                             takes 3 cycles plus 1 of the decoding.
                             Add 3 cycles for each operand fetch, and
                             3 more for each memory write/read. */


case  NOP       :  AddCycles( 4 ); break;
case  LD_BC_NN  :  LD_rr_nn(r_BC); AddCycles( 4+3+3 ); break;
case  LD_xBC_A  :  STORE_r(r_BC, r_A); AddCycles( 4+3 ); break;
case  INC_BC    :  r_BC++; AddCycles( 4+2 ); break;

case  INC_B     :  INC(r_B); AddCycles( 4 ); break;
case  DEC_B     :  DEC(r_B); AddCycles( 4 ); break;

case  LD_B_N    :  LD_r_n(r_B);
                   AddCycles( 4+3 );
                   break;

case  EX_AF_AF  :  EX_WORD(r_AF, r_AFs);
                   AddCycles( 4 );
                   break;

case  LD_A_xBC  :  LOAD_r(r_A, r_BC);
                   AddCycles( 4+3 );
                   break;

case  DEC_BC    :  r_BC--;
                   AddCycles( 4+2 );
                   break;

case  INC_C     :  INC(r_C);
                   AddCycles( 4 );
                   break;

case  DEC_C     :  DEC(r_C);
                   AddCycles( 4 );
                   break;

case  LD_C_N    :  LD_r_n(r_C);
                   AddCycles( 4+3 );
                   break;

case  LD_DE_NN  :  LD_rr_nn(r_DE);
                   AddCycles( 4+3+3 );
                   break;

case  LD_xDE_A  :  STORE_r(r_DE, r_A);
                   AddCycles( 4+3 );
                   break;

case  INC_DE    :  r_DE++;
                   AddCycles( 4+2 );
                   break;

case  INC_D     :  INC(r_D);
                   AddCycles( 4 );
                   break;

case  DEC_D     :  DEC(r_D);
                   AddCycles( 4 );
                   break;

case  LD_D_N    :  LD_r_n(r_D);
                   AddCycles( 4+3 );
                   break;

case  ADD_HL_BC :  ADD_WORD(r_HL, r_BC); AddCycles( 4+3+3+1 ); break;
case  ADD_HL_DE :  ADD_WORD(r_HL, r_DE); AddCycles( 4+3+3+1 ); break;
case  ADD_HL_HL :  ADD_WORD(r_HL, r_HL); AddCycles( 4+3+3+1 ); break;
case  ADD_HL_SP :  ADD_WORD(r_HL, r_SP); AddCycles( 4+3+3+1 ); break;

case  LD_A_xDE  :  LOAD_r(r_A, r_DE);
                   AddCycles( 4+3 );
                   break;

case  DEC_DE    :  r_DE--;
                   AddCycles( 4+2 );
                   break;

case  INC_E     :  INC(r_E);
                   AddCycles( 4 );
                   break;

case  DEC_E     :  DEC(r_E);
                   AddCycles( 4 );
                   break;

case  LD_E_N    :  LD_r_n(r_E);
                   AddCycles( 4+3 );
                   break;

case  LD_HL_NN  :  LD_rr_nn(r_HL);
                   AddCycles( 4+3+3 );
                   break;

case  LD_xNN_HL :  STORE_nn_rr(r_HL);
                   AddCycles( 4+3+3+3+3 );
                   break;

case  INC_HL    :  r_HL++;
                   AddCycles( 4+2 );
                   break;

case  INC_H     :  INC(r_H);
                   AddCycles( 4 );
                   break;

case  DEC_H     :  DEC(r_H);
                   AddCycles( 4 );
                   break;

case  LD_H_N    :  LD_r_n(r_H);
                   AddCycles( 4+3 );
                   break;

case  LD_HL_xNN :  LOAD_rr_nn(r_HL);
                   AddCycles( 4+3+3+3+3 );
                   break;

case  DEC_HL    :  r_HL--;
                   AddCycles( 4+2 );
                   break;

case  INC_L     :  INC(r_L);
                   AddCycles( 4 );
                   break;

case  DEC_L     :  DEC(r_L);
                   AddCycles( 4 );
                   break;

case  LD_L_N    :  LD_r_n(r_L);
                   AddCycles( 4+3 );
                   break;

case  LD_SP_NN  :  LD_rr_nn(r_SP);
                   AddCycles( 10 );
                   break;

case  LD_xNN_A  :  STORE_nn_r(r_A);
                   AddCycles( 13 );
                   break;

case  INC_SP    :  r_SP++;
                   AddCycles( 6 );
                   break;

case  LD_xHL_N  :  r_meml = Z80ReadMem(r_PC); r_PC++;
                   STORE_r( r_HL, r_meml );
                   AddCycles( 10 );
                   break;

case  LD_A_xNN  :  LOAD_r_nn( r_A );
                   AddCycles( 13 );
                   break;

case  DEC_SP    :  r_SP--;
                   AddCycles( 6 );
                   break;

case  INC_A     :  INC(r_A);
                   AddCycles( 4 );
                   break;

case  DEC_A     :  DEC(r_A);
                   AddCycles( 4 );
                   break;

case  LD_A_N    :  LD_r_n(r_A);
                   AddCycles( 4+3 );
                   break;

case  LD_B_B    :  LD_r_r( r_B, r_B );
                   AddCycles( 4 );
                   break;

case  LD_B_C    :  LD_r_r( r_B, r_C );
                   AddCycles( 4 );
                   break;

case  LD_B_D    :  LD_r_r( r_B, r_D );
                   AddCycles( 4 );
                   break;

case  LD_B_E    :  LD_r_r( r_B, r_E );
                   AddCycles( 4 );
                   break;

case  LD_B_H    :  LD_r_r( r_B, r_H );
                   AddCycles( 4 );
                   break;

case  LD_B_L    :  LD_r_r( r_B, r_L );
                   AddCycles( 4 );
                   break;

case  LD_B_xHL  :  LOAD_r(r_B, r_HL);
                   AddCycles( 4+3 );
                   break;

case  LD_B_A    :  LD_r_r( r_B, r_A );
                   AddCycles( 4 );
                   break;

case  LD_C_B    :  LD_r_r( r_C, r_B );
                   AddCycles( 4 );
                   break;

case  LD_C_C    :  LD_r_r( r_C, r_C );
                   AddCycles( 4 );
                   break;

case  LD_C_D    :  LD_r_r( r_C, r_D );
                   AddCycles( 4 );
                   break;

case  LD_C_E    :  LD_r_r( r_C, r_E );
                   AddCycles( 4 );
                   break;
case  LD_C_H    :  LD_r_r( r_C, r_H );
                   AddCycles( 4 );
                   break;

case  LD_C_L    :  LD_r_r( r_C, r_L );
                   AddCycles( 4 );
                   break;

case  LD_C_xHL  :  LOAD_r(r_C, r_HL);
                   AddCycles( 4+3 );
                   break;

case  LD_C_A    :  LD_r_r( r_C, r_A );
                   AddCycles( 4 );
                   break;

case  LD_D_B    :  LD_r_r( r_D, r_B );
                   AddCycles( 4 );
                   break;

case  LD_D_C    :  LD_r_r( r_D, r_C );
                   AddCycles( 4 );
                   break;

case  LD_D_D    :  LD_r_r( r_D, r_D );
                   AddCycles( 4 );
                   break;

case  LD_D_E    :  LD_r_r( r_D, r_E );
                   AddCycles( 4 );
                   break;

case  LD_D_H    :  LD_r_r( r_D, r_H );
                   AddCycles( 4 );
                   break;

case  LD_D_L    :  LD_r_r( r_D, r_L );
                   AddCycles( 4 );
                   break;

case  LD_D_xHL  :  LOAD_r(r_D, r_HL);
                   AddCycles( 4+3 );
                   break;

case  LD_D_A    :  LD_r_r( r_D, r_A );
                   AddCycles( 4 );
                   break;

case  LD_E_B    :  LD_r_r( r_E, r_B );
                   AddCycles( 4 );
                   break;

case  LD_E_C    :  LD_r_r( r_E, r_C );
                   AddCycles( 4 );
                   break;

case  LD_E_D    :  LD_r_r( r_E, r_D );
                   AddCycles( 4 );
                   break;

case  LD_E_E    :  LD_r_r( r_E, r_E );
                   AddCycles( 4 );
                   break;

case  LD_E_H    :  LD_r_r( r_E, r_H );
                   AddCycles( 4 );
                   break;

case  LD_E_L    :  LD_r_r( r_E, r_L );
                   AddCycles( 4 );
                   break;

case  LD_E_xHL  :  LOAD_r(r_E, r_HL);
                   AddCycles( 4+3 );
                   break;

case  LD_E_A    :  LD_r_r( r_E, r_A );
                   AddCycles( 4 );
                   break;

case  LD_H_B    :  LD_r_r( r_H, r_B );
                   AddCycles( 4 );
                   break;

case  LD_H_C    :  LD_r_r( r_H, r_C );
                   AddCycles( 4 );
                   break;

case  LD_H_D    :  LD_r_r( r_H, r_D );
                   AddCycles( 4 );
                   break;

case  LD_H_E    :  LD_r_r( r_H, r_E );
                   AddCycles( 4 );
                   break;

case  LD_H_H    :  LD_r_r( r_H, r_H );
                   AddCycles( 4 );
                   break;

case  LD_H_L    :  LD_r_r( r_H, r_L );
                   AddCycles( 4 );
                   break;

case  LD_H_xHL  :  LOAD_r(r_H, r_HL);
                   AddCycles( 4+3 );
                   break;

case  LD_H_A    :  LD_r_r( r_H, r_A );
                   AddCycles( 4 );
                   break;

case  LD_L_B    :  LD_r_r( r_L, r_B );
                   AddCycles( 4 );
                   break;

case  LD_L_C    :  LD_r_r( r_L, r_C );
                   AddCycles( 4 );
                   break;

case  LD_L_D    :  LD_r_r( r_L, r_D );
                   AddCycles( 4 );
                   break;

case  LD_L_E    :  LD_r_r( r_L, r_E );
                   AddCycles( 4 );
                   break;

case  LD_L_H    :  LD_r_r( r_L, r_H );
                   AddCycles( 4 );
                   break;

case  LD_L_L    :  LD_r_r( r_L, r_L );
                   AddCycles( 4 );
                   break;

case  LD_L_xHL  :  LOAD_r(r_L, r_HL);
                   AddCycles( 7 );
                   break;

case  LD_L_A    :  LD_r_r( r_L, r_A );
                   AddCycles( 4 );
                   break;

case  LD_xHL_B  :  STORE_r( r_HL, r_B );
                   AddCycles( 4+3 );
                   break;

case  LD_xHL_C  :  STORE_r( r_HL, r_C );
                   AddCycles( 4+3 );
                   break;

case  LD_xHL_D  :  STORE_r( r_HL, r_D );
                   AddCycles( 4+3 );
                   break;

case  LD_xHL_E  :  STORE_r( r_HL, r_E );
                   AddCycles( 4+3 );
                   break;

case  LD_xHL_H  :  STORE_r( r_HL, r_H );
                   AddCycles( 4+3 );
                   break;

case  LD_xHL_L  :  STORE_r( r_HL, r_L );
                   AddCycles( 4+3 );
                   break;

case  LD_xHL_A  :  STORE_r(r_HL, r_A );
                   AddCycles( 4+3 );
                   break;

case  LD_A_B    :  LD_r_r( r_A, r_B );
                   AddCycles( 4 );
                   break;

case  LD_A_C    :  LD_r_r( r_A, r_C );
                   AddCycles( 4 );
                   break;

case  LD_A_D    :  LD_r_r( r_A, r_D );
                   AddCycles( 4 );
                   break;

case  LD_A_E    :  LD_r_r( r_A, r_E );
                   AddCycles( 4 );
                   break;

case  LD_A_H    :  LD_r_r( r_A, r_H );
                   AddCycles( 4 );
                   break;

case  LD_A_L    :  LD_r_r( r_A, r_L );
                   AddCycles( 4 );
                   break;

case  LD_A_xHL  :  LOAD_r( r_A, r_HL);
                   AddCycles( 4+3 );
                   break;

case  LD_A_A    :  LD_r_r( r_A, r_A );
                   AddCycles( 4 );
                   break;

case  LD_SP_HL  :  LD_r_r( r_SP, r_HL );
                   AddCycles( 6 ); break;

case  ADD_B     :  ADD(r_B); AddCycles( 4 ); break;
case  ADD_C     :  ADD(r_C); AddCycles( 4 ); break;
case  ADD_D     :  ADD(r_D); AddCycles( 4 ); break;
case  ADD_E     :  ADD(r_E); AddCycles( 4 ); break;
case  ADD_H     :  ADD(r_H); AddCycles( 4 ); break;
case  ADD_L     :  ADD(r_L); AddCycles( 4 ); break;
case  ADD_xHL   :  r_meml = Z80ReadMem(r_HL);
                   ADD(r_meml);
                   AddCycles( 4+3 );
                   break;
case  ADD_A     :  ADD(r_A); AddCycles( 4 ); break;
case  ADC_B     :  ADC(r_B); AddCycles( 4 ); break;
case  ADC_C     :  ADC(r_C); AddCycles( 4 ); break;
case  ADC_D     :  ADC(r_D); AddCycles( 4 ); break;
case  ADC_E     :  ADC(r_E); AddCycles( 4 ); break;
case  ADC_H     :  ADC(r_H); AddCycles( 4 ); break;
case  ADC_L     :  ADC(r_L); AddCycles( 4 ); break;
case  ADC_xHL   :  r_meml = Z80ReadMem(r_HL);
                   ADC(r_meml);
                   AddCycles( 4+3 );
                   break;
case  ADC_A     :  ADC(r_A); AddCycles( 4 ); break;
case  ADC_N     :  r_meml = Z80ReadMem(r_PC); r_PC++;
                   ADC(r_meml);
                   AddCycles( 4+3 );
                   break;

case  SUB_A     :  SUB(r_A); AddCycles( 4 ); break;
case  SUB_B     :  SUB(r_B); AddCycles( 4 ); break;
case  SUB_C     :  SUB(r_C); AddCycles( 4 ); break;
case  SUB_D     :  SUB(r_D); AddCycles( 4 ); break;
case  SUB_E     :  SUB(r_E); AddCycles( 4 ); break;
case  SUB_H     :  SUB(r_H); AddCycles( 4 ); break;
case  SUB_L     :  SUB(r_L); AddCycles( 4 ); break;
case  SUB_xHL   :  r_meml = Z80ReadMem(r_HL);
                   SUB(r_meml);
                   AddCycles( 4+3 );
                   break;
case  SUB_N     :  r_meml = Z80ReadMem(r_PC); r_PC++;
                   SUB(r_meml);
                   AddCycles( 4+3 );
                   break;

case  SBC_A     :  SBC(r_A); AddCycles( 4 ); break;
case  SBC_B     :  SBC(r_B); AddCycles( 4 ); break;
case  SBC_C     :  SBC(r_C); AddCycles( 4 ); break;
case  SBC_D     :  SBC(r_D); AddCycles( 4 ); break;
case  SBC_E     :  SBC(r_E); AddCycles( 4 ); break;
case  SBC_H     :  SBC(r_H); AddCycles( 4 ); break;
case  SBC_L     :  SBC(r_L); AddCycles( 4 ); break;
case  SBC_xHL   :  r_meml = Z80ReadMem(r_HL);
                   SBC(r_meml);
                   AddCycles( 4+3 );
                   break;
case  SBC_N     :  r_meml = Z80ReadMem(r_PC); r_PC++;
                   SBC(r_meml);
                   AddCycles( 4 );
                   break;

case  AND_B     :  AND( r_B ); AddCycles( 4 ); break;
case  AND_C     :  AND( r_C ); AddCycles( 4 ); break;
case  AND_D     :  AND( r_D ); AddCycles( 4 ); break;
case  AND_E     :  AND( r_E ); AddCycles( 4 ); break;
case  AND_H     :  AND( r_H ); AddCycles( 4 ); break;
case  AND_L     :  AND( r_L ); AddCycles( 4 ); break;
case  AND_xHL   :  AND_mem( r_HL ); AddCycles( 4+3 ); break;
case  AND_A     :  AND( r_A ); AddCycles( 4 ); break;
case  XOR_B     :  XOR( r_B ); AddCycles( 4 ); break;
case  XOR_C     :  XOR( r_C ); AddCycles( 4 ); break;
case  XOR_D     :  XOR( r_D ); AddCycles( 4 ); break;
case  XOR_E     :  XOR( r_E ); AddCycles( 4 ); break;
case  XOR_H     :  XOR( r_H ); AddCycles( 4 ); break;
case  XOR_L     :  XOR( r_L ); AddCycles( 4 ); break;
case  XOR_xHL   :  XOR_mem( r_HL ); AddCycles( 4+3 ); break;
case  XOR_A     :  XOR( r_A ); AddCycles( 4 ); break;
case  OR_B      :  OR( r_B ); AddCycles( 4 ); break;
case  OR_C      :  OR( r_C ); AddCycles( 4 ); break;
case  OR_D      :  OR( r_D ); AddCycles( 4 ); break;
case  OR_E      :  OR( r_E ); AddCycles( 4 ); break;
case  OR_H      :  OR( r_H ); AddCycles( 4 ); break;
case  OR_L      :  OR( r_L ); AddCycles( 4 ); break;
case  OR_xHL    :  OR_mem( r_HL ); AddCycles( 4+3 ); break;
case  OR_A      :  OR( r_A ); AddCycles( 4 ); break;
case  CP_A      :  CP(r_A); AddCycles( 4 ); break;
case  CP_B      :  CP(r_B); AddCycles( 4 ); break;
case  CP_C      :  CP(r_C); AddCycles( 4 ); break;
case  CP_D      :  CP(r_D); AddCycles( 4 ); break;
case  CP_E      :  CP(r_E); AddCycles( 4 ); break;
case  CP_H      :  CP(r_H); AddCycles( 4 ); break;
case  CP_L      :  CP(r_L); AddCycles( 4 ); break;
case  CP_xHL    :  r_meml = Z80ReadMem( r_HL );
                   CP(r_meml); AddCycles( 4+3 ); break;
case  CP_N      :  r_meml = Z80ReadMem(r_PC); r_PC++;
                   CP(r_meml);
                   AddCycles( 4+3);
                   break;

case  RET_Z     :  if( TEST_FLAG(Z_FLAG) )
                   { RET_nn(); AddCycles( 4+1+3+3 ); }
                   else { AddCycles( 4+1 ); }
                   break;

case  RET_C     :  if( TEST_FLAG(C_FLAG) )
                   { RET_nn(); AddCycles( 4+1+3+3 ); }
                   else { AddCycles( 4+1 ); }
                   break;

case  RET_M     :  if( TEST_FLAG(S_FLAG) )
                   { RET_nn(); AddCycles( 4+1+3+3 ); }
                   else { AddCycles( 4+1 ); }
                   break;

case  RET_PE    :  if( TEST_FLAG(P_FLAG) )
                   { RET_nn(); AddCycles( 4+1+3+3 ); }
                   else { AddCycles( 4+1 ); }
                   break;

case  RET_PO    :  if( TEST_FLAG(P_FLAG) )
                   { AddCycles( 4+1 ); }
                   else { RET_nn(); AddCycles( 4+1+3+3 ); }
                   break;

case  RET_P     :  if( TEST_FLAG(S_FLAG) )
                   { AddCycles( 4+1 ); }
                   else { RET_nn(); AddCycles( 4+1+3+3 ); }
                   break;

case  RET       :  RET_nn();
                   AddCycles( 4+3+3 );
                   break;

case  RET_NZ    :  if( TEST_FLAG(Z_FLAG) )
                   { AddCycles( 4+1 ); }
                   else { RET_nn(); AddCycles( 4+1+3+3 ); }
                   break;

case  RET_NC    :  if( TEST_FLAG(C_FLAG) )
                   { AddCycles( 4+1 ); }
                   else { RET_nn(); AddCycles( 4+1+3+3 ); }
                   break;

case  ADD_N     :  r_meml = Z80ReadMem(r_PC); r_PC++;
                   ADD(r_meml);
                   AddCycles( 4+3 );
                   break;

case  JR        :  JR_n();
                   AddCycles( 4+3+3+2 );
                   break;

case  JR_NZ     :  if( TEST_FLAG(Z_FLAG) )
                   { r_PC++;  AddCycles( 4+3 ); }
                   else
                   { JR_n();  AddCycles( 4+8 ); }
                   break;

case  JR_Z      :  if( TEST_FLAG(Z_FLAG) )
                   { JR_n();  AddCycles( 4+8 ); }
                   else
                   { r_PC++;  AddCycles( 4+3 ); }
                   break;

case  JR_NC     :  if( TEST_FLAG(C_FLAG) )
                   { r_PC++;  AddCycles( 4+3 ); }
                   else
                   { JR_n();  AddCycles( 4+8 ); }
                   break;

case  JR_C      :  if( TEST_FLAG(C_FLAG) )
                   { JR_n();  AddCycles( 4+8 ); }
                   else
                   { r_PC++;  AddCycles( 4+3 ); }
                   break;

case  JP_NZ     :  if( TEST_FLAG(Z_FLAG) ) { r_PC += 2; }
                   else                    { JP_nn();   }
                   AddCycles( 4+3+3 );
                   break;

case  JP        :  JP_nn();
                   AddCycles( 4+3+3 );
                   break;

case  JP_Z      :  if( TEST_FLAG(Z_FLAG) ) { JP_nn();   }
                   else                    { r_PC += 2; }
                   AddCycles( 4+3+3 );
                   break;

case  JP_NC     :  if( TEST_FLAG(C_FLAG) ) { r_PC += 2; }
                   else                    { JP_nn();   }
                   AddCycles( 4+3+3 );
                   break;

case  JP_C      :  if( TEST_FLAG(C_FLAG) ) { JP_nn();   }
                   else                    { r_PC += 2; }
                   AddCycles( 4+3+3 );
                   break;

case  JP_PO     :  if( TEST_FLAG(P_FLAG) ) { r_PC += 2; }
                   else                    { JP_nn();   }
                   AddCycles( 4+3+3 );
                   break;

case  JP_PE     :  if( TEST_FLAG(P_FLAG) ) { JP_nn();   }
                   else                    { r_PC += 2; }
                   AddCycles( 4+3+3 );
                   break;

case  JP_P      :  if( TEST_FLAG(S_FLAG) ) { r_PC += 2; }
                   else                    { JP_nn();   }
                   AddCycles( 4+3+3 );
                   break;


case  JP_M      :  if( TEST_FLAG(S_FLAG) ) { JP_nn();   }
                   else                    { r_PC += 2; }
                   AddCycles( 4+3+3 );
                   break;

case  JP_xHL    :  r_PC = r_HL; AddCycles( 4 ); break;

case  CPL       :  r_A ^= 0xFF;
                   r_F = ( r_F & (FLAG_C|FLAG_P|FLAG_Z|FLAG_S)) |
                         ( r_A & (FLAG_3|FLAG_5))|(FLAG_N|FLAG_H);
                   AddCycles( 4 ); break;

case  INC_xHL   :  r_meml = Z80ReadMem( r_HL );
                   INC(r_meml);
                   Z80WriteMem( r_HL, r_meml, regs );
                   AddCycles( 4+3+3+1 ); break;

case  DEC_xHL   :  r_meml = Z80ReadMem( r_HL );
                   DEC(r_meml);
                   Z80WriteMem( r_HL, r_meml, regs );
                   AddCycles( 4+3+3+1 ); break;

case  SCF       :  r_F = r_F | FLAG_C;
                   r_F &= FLAG_Z | FLAG_S | FLAG_P;
                   if(r_F & FLAG_H) r_F ^= FLAG_H;
                   r_F |= FLAG_C;
                   AddCycles( 4 ); break;

case  CCF       :  r_F = (r_F & (FLAG_P|FLAG_Z|FLAG_S) )    |
                         ((r_F & FLAG_C) ? FLAG_H : FLAG_C) |
                         ( r_A & (FLAG_3 | FLAG_5) );
                   AddCycles( 4 ); break;

case HALT       : regs->halted = 1;
                  AddCycles(4);
                  break;

case  POP_BC    :  POP(BC);  AddCycles( 10 ); break;
case  PUSH_BC   :  PUSH(BC); AddCycles( 11 ); break;
case  POP_HL    :  POP(HL);  AddCycles( 10 ); break;
case  PUSH_HL   :  PUSH(HL); AddCycles( 11 ); break;
case  POP_AF    :  POP(AF);  AddCycles( 10 ); break;
case  PUSH_AF   :  PUSH(AF); AddCycles( 11 ); break;
case  POP_DE    :  POP(DE);  AddCycles( 10 ); break;
case  PUSH_DE   :  PUSH(DE); AddCycles( 11 ); break;

case  RLCA      :  r_A = (r_A << 1) | (r_A >> 7);
                   r_F = ( r_F & (FLAG_P|FLAG_Z|FLAG_S)) |
                         ( r_A & (FLAG_C|FLAG_3|FLAG_5) );
                   AddCycles( 4 ); break;

case  RRCA      :  r_F = ( r_F & (FLAG_P|FLAG_Z|FLAG_S)) |
                         ( r_A & FLAG_C );
                   r_A = ( r_A >> 1) | ( r_A << 7 );
                   r_F |= ( r_A & ( FLAG_3 | FLAG_5 ) );
                   AddCycles( 4 ); break;

case  DJNZ      :  r_B--;
                   if( r_B )   { JR_n();  AddCycles(13); }
                   else        { r_PC++ ; AddCycles(8);  }
                   break;

case  RLA       :  r_meml = r_A;
                   r_A = ( r_A << 1 ) | ( r_F & FLAG_C );
                   r_F = ( r_F & ( FLAG_P | FLAG_Z | FLAG_S ) ) |
                   (r_A & ( FLAG_3 | FLAG_5 ) ) | ( r_meml >> 7);
                   AddCycles( 4 ); break;

case  RRA       :  r_meml = r_A;
                   r_A = ( r_A >> 1 ) | ( r_F << 7 );
                   r_F = ( r_F & ( FLAG_P | FLAG_Z | FLAG_S ) ) |
                   ( r_A & ( FLAG_3 | FLAG_5 ) ) | ( r_meml & FLAG_C );
                   AddCycles( 4 ); break;

case  DAA       :  r_meml = 0;
                   r_memh = ( r_F & FLAG_C );
                   if( ( r_F & FLAG_H ) || ( (r_A & 0x0f)>9 ) ) r_meml=6;
                   if( r_memh || (r_A > 0x9f ) ) r_meml |= 0x60;
                   if( r_A > 0x99 ) r_memh=1;
                   if ( r_F & FLAG_N ) { SUB(r_meml); }
                   else
                   {
                       if( (r_A>0x90) && ( (r_A & 0x0f)>9) ) r_meml|=0x60;
                       ADD(r_meml);
                   }
                   r_F = ( r_F & ~( FLAG_C | FLAG_P) ) | r_memh |
                   parity_table[r_A];
                   AddCycles( 4 ); break;

case  OUT_N_A   :  Z80OutPort( regs, Z80ReadMem( r_PC ), r_A ); r_PC++;
                   AddCycles( 11 ); break;

case  IN_A_N    :  r_A = Z80InPort( Z80ReadMem( r_PC ) + (r_A << 8));
                   r_PC++; AddCycles( 11 ); break;

case  EX_HL_xSP :  r_meml = Z80ReadMem(r_SP);
                   r_memh = Z80ReadMem(r_SP+1);
                   Z80WriteMem(r_SP, r_L, regs);
                   Z80WriteMem(r_SP+1, r_H, regs);
                   r_L = r_meml;
                   r_H = r_memh;
                   AddCycles( 19 ); break;

case  EXX       :  EX_WORD(r_BC, r_BCs); EX_WORD(r_DE, r_DEs);
                   EX_WORD(r_HL, r_HLs);
                   AddCycles( 4 ); break;

case  EX_DE_HL  :  EX_WORD( r_DE, r_HL );
                   AddCycles( 4 );
                   break;

case  AND_N     :  AND_mem( r_PC );
                   r_PC++;
                   AddCycles( 4+3 );
                   break;

case  XOR_N     :  XOR_mem( r_PC );
                   r_PC++;
                   AddCycles( 4+3 );
                   break;

case  OR_N      :  OR_mem( r_PC );
                   r_PC++;
                   AddCycles( 4+3 );
                   break;

case  DI        :  r_IFF1 = r_IFF2 = 0;
                   AddCycles( 4 );
                   break;

case  CALL      :
                   CALL_nn();
                   AddCycles( 4+3+3+3+3+1 );
                   break;

case  CALL_NZ   :  if( TEST_FLAG(Z_FLAG) )
                    { r_PC += 2; AddCycles( 4+3+3 ); }
                   else
                    {
                      CALL_nn();
                      AddCycles( 4+3+3+3+3+1 );
                    }
                   break;

case  CALL_NC   :  if( TEST_FLAG(C_FLAG) )
                    { r_PC += 2; AddCycles( 4+3+3 ); }
                   else
                    { CALL_nn();
                      AddCycles( 4+3+3+3+3+1 );
                    }
                   break;

case  CALL_PO   :  if( TEST_FLAG(P_FLAG) )
                    { r_PC += 2; AddCycles( 4+3+3 ); }
                   else
                    { CALL_nn();
                      AddCycles( 4+3+3+ 3+3+1 );
                   }
                   break;

case  CALL_P    :  if( TEST_FLAG(S_FLAG) )
                    { r_PC += 2; AddCycles( 4+3+3 ); }
                   else
                    { CALL_nn();
                      AddCycles( 4+3+3+3+3+1 );
                    }
                   break;


case  CALL_Z    :  if( TEST_FLAG(Z_FLAG) )
                   { CALL_nn();
                      AddCycles( 4+3+3+3+3+1 );
                   }
                   else
                   {  r_PC += 2; AddCycles( 4+3+3 ); }
                   break;

case  CALL_C    :  if( TEST_FLAG(C_FLAG) )
                   { CALL_nn();
                      AddCycles( 4+3+3+ 3+3+4 );
                   }
                   else
                   { r_PC += 2; AddCycles( 4+3+3 ); }
                   break;

case  CALL_PE   :  if( TEST_FLAG(P_FLAG) )
                    { CALL_nn();
                      AddCycles( 4+3+3+3+3+1 );
                    }
                   else
                    { r_PC += 2; AddCycles( 4+3+3 ); }
                   break;

case  CALL_M    :  if( TEST_FLAG(S_FLAG) )
                    { CALL_nn();
                      AddCycles( 4+3+3+3+3+1 );
                   }
                   else
                    { r_PC += 2; AddCycles( 4+3+3 ); }
                   break;

case  EI        :   r_IFF1 = r_IFF2 = 1;
                    /*
                    Why Marat Fayzullin does this? ->

                    regs->IFF2 |= 0x01;
                    if( regs->IRequest != INT_NOINT )
                    {
                       regs->IBackup = regs->ICount;
                       regs->ICount = 0x1;
                       r_IFF |= 0x20;
                    }*/
                   AddCycles( 4 );
                   break;

case  RST_00    :  RST(0x00); AddCycles( 11 ); break;
case  RST_08    :  RST(0x08); AddCycles( 11 ); break;
case  RST_10    :  RST(0x10); AddCycles( 11 ); break;
case  RST_18    :  RST(0x18); AddCycles( 11 ); break;
case  RST_20    :  RST(0x20); AddCycles( 11 ); break;
case  RST_28    :  RST(0x28); AddCycles( 11 ); break;
case  RST_30    :  RST(0x30); AddCycles( 11 ); break;
case  RST_38    :  RST(0x38); AddCycles( 11 ); break;

default:
//    exit(1);
///!!!    if( regs->DecodingErrors )
///!!!    printf("z80 core: Unknown instruction: %02Xh at PC=%04Xh.\n",
///!!!            Z80ReadMem(r_PC-1), r_PC-1 );
    break;
