/*====================================================================/*
  opcodes_cb.c -> This file executes the CB PREFIX opcodes.

  When you find the CB opcode, it means that you must fetch another
  byte from memory and treat it as a new opcode with different
  meaning than the single-byte opcodes. This is a common way to extend
  the number of opcodes (8 bits of instruction word = just 256 opcodes)
  and it's called an OPCODE PREFIX (now we have another 256 new opcodes
  by using this trick).

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

/* 8 clock cycles minimum = CB opcode = 4+4 */

opcode = Z80ReadMem( r_PC );
r_PC++;

switch(opcode)
{

  case  RLC_B     :  RLC(r_B); AddCycles( 4+4 ); break;
  case  RLC_C     :  RLC(r_C); AddCycles( 4+4 ); break;
  case  RLC_D     :  RLC(r_D); AddCycles( 4+4 ); break;
  case  RLC_E     :  RLC(r_E); AddCycles( 4+4 ); break;
  case  RLC_H     :  RLC(r_H); AddCycles( 4+4 ); break;
  case  RLC_L     :  RLC(r_L); AddCycles( 4+4 ); break;
  case  RLC_xHL   :  r_meml = Z80ReadMem( r_HL );
                     RLC(r_meml);
                     Z80WriteMem( r_HL, r_meml, regs );
                     AddCycles( 4+4+3+3+1 ); break;
  case  RLC_A     :  RLC(r_A); AddCycles( 4+4 ); break;

  case  RRC_B     :  RRC(r_B); AddCycles( 4+4 ); break;
  case  RRC_C     :  RRC(r_C); AddCycles( 4+4 ); break;
  case  RRC_D     :  RRC(r_D); AddCycles( 4+4 ); break;
  case  RRC_E     :  RRC(r_E); AddCycles( 4+4 ); break;
  case  RRC_H     :  RRC(r_H); AddCycles( 4+4 ); break;
  case  RRC_L     :  RRC(r_L); AddCycles( 4+4 ); break;
  case  RRC_xHL   :  r_meml = Z80ReadMem( r_HL );
                     RRC(r_meml);
                     Z80WriteMem( r_HL, r_meml, regs );
                     AddCycles( 4+4+3+3+1 ); break;
  case  RRC_A     :  RRC(r_A); AddCycles( 4+4 ); break;

  case  RL_B      :  RL(r_B); AddCycles( 4+4 ); break;
  case  RL_C      :  RL(r_C); AddCycles( 4+4 ); break;
  case  RL_D      :  RL(r_D); AddCycles( 4+4 ); break;
  case  RL_E      :  RL(r_E); AddCycles( 4+4 ); break;
  case  RL_H      :  RL(r_H); AddCycles( 4+4 ); break;
  case  RL_L      :  RL(r_L); AddCycles( 4+4 ); break;
  case  RL_xHL    :  r_meml = Z80ReadMem( r_HL );
                     RL(r_meml);
                     Z80WriteMem( r_HL, r_meml, regs );
                     AddCycles( 4+4+3+3+1 ); break;
  case  RL_A      :  RL(r_A); AddCycles( 4+4 ); break;

  case  RR_B      :  RR(r_B); AddCycles( 4+4 ); break;
  case  RR_C      :  RR(r_C); AddCycles( 4+4 ); break;
  case  RR_D      :  RR(r_D); AddCycles( 4+4 ); break;
  case  RR_E      :  RR(r_E); AddCycles( 4+4 ); break;
  case  RR_H      :  RR(r_H); AddCycles( 4+4 ); break;
  case  RR_L      :  RR(r_L); AddCycles( 4+4 ); break;
  case  RR_xHL    :  r_meml = Z80ReadMem( r_HL );
                     RR(r_meml);
                     Z80WriteMem( r_HL, r_meml, regs );
                     AddCycles( 4+4+3+3+1 ); break;
  case  RR_A      :  RR(r_A); AddCycles( 4+4 ); break;

  case  SLA_B     :  SLA(r_B); AddCycles( 4+4 ); break;
  case  SLA_C     :  SLA(r_C); AddCycles( 4+4 ); break;
  case  SLA_D     :  SLA(r_D); AddCycles( 4+4 ); break;
  case  SLA_E     :  SLA(r_E); AddCycles( 4+4 ); break;
  case  SLA_H     :  SLA(r_H); AddCycles( 4+4 ); break;
  case  SLA_L     :  SLA(r_L); AddCycles( 4+4 ); break;
  case  SLA_xHL   :  r_meml = Z80ReadMem( r_HL );
                     SLA(r_meml);
                     Z80WriteMem( r_HL, r_meml, regs );
                     AddCycles( 4+4+3+3+1 ); break;
  case  SLA_A     :  SLA(r_A); AddCycles( 4+4 ); break;

  case  SRA_B     :  SRA(r_B); AddCycles( 4+4 ); break;
  case  SRA_C     :  SRA(r_C); AddCycles( 4+4 ); break;
  case  SRA_D     :  SRA(r_D); AddCycles( 4+4 ); break;
  case  SRA_E     :  SRA(r_E); AddCycles( 4+4 ); break;
  case  SRA_H     :  SRA(r_H); AddCycles( 4+4 ); break;
  case  SRA_L     :  SRA(r_L); AddCycles( 4+4 ); break;
  case  SRA_xHL   :  r_meml = Z80ReadMem( r_HL );
                     SRA(r_meml);
                     Z80WriteMem( r_HL, r_meml, regs );
                     AddCycles( 4+4+3+3+1 ); break;
  case  SRA_A     :  SRA(r_A); AddCycles( 4+4 ); break;

  case  SLL_B     :  SLL(r_B); AddCycles( 4+4 ); break;
  case  SLL_C     :  SLL(r_C); AddCycles( 4+4 ); break;
  case  SLL_D     :  SLL(r_D); AddCycles( 4+4 ); break;
  case  SLL_E     :  SLL(r_E); AddCycles( 4+4 ); break;
  case  SLL_H     :  SLL(r_H); AddCycles( 4+4 ); break;
  case  SLL_L     :  SLL(r_L); AddCycles( 4+4 ); break;
  case  SLL_xHL   :  r_meml = Z80ReadMem( r_HL );
                     SLL(r_meml);
                     Z80WriteMem( r_HL, r_meml, regs );
                     AddCycles( 4+4+3+3+1 ); break;
  case  SLL_A     :  SLL(r_A); AddCycles( 4+4 ); break;

  case  SRL_B     :  SRL(r_B); AddCycles( 4+4 ); break;
  case  SRL_C     :  SRL(r_C); AddCycles( 4+4 ); break;
  case  SRL_D     :  SRL(r_D); AddCycles( 4+4 ); break;
  case  SRL_E     :  SRL(r_E); AddCycles( 4+4 ); break;
  case  SRL_H     :  SRL(r_H); AddCycles( 4+4 ); break;
  case  SRL_L     :  SRL(r_L); AddCycles( 4+4 ); break;
  case  SRL_xHL   :  r_meml = Z80ReadMem( r_HL );
                     SRL(r_meml);
                     Z80WriteMem( r_HL, r_meml, regs );
                     AddCycles( 4+4+3+3+1 ); break;
  case  SRL_A     :  SRL(r_A); AddCycles( 4+4 ); break;

  case  BIT_0_B   :  BIT_BIT(0, r_B); AddCycles( 4+4 ); break;
  case  BIT_0_C   :  BIT_BIT(0, r_C); AddCycles( 4+4 ); break;
  case  BIT_0_D   :  BIT_BIT(0, r_D); AddCycles( 4+4 ); break;
  case  BIT_0_E   :  BIT_BIT(0, r_E); AddCycles( 4+4 ); break;
  case  BIT_0_H   :  BIT_BIT(0, r_H); AddCycles( 4+4 ); break;
  case  BIT_0_L   :  BIT_BIT(0, r_L); AddCycles( 4+4 ); break;
  case  BIT_0_xHL :  BIT_mem_BIT(0, r_HL); AddCycles( 12 ); break;
  case  BIT_0_A   :  BIT_BIT(0, r_A); AddCycles( 4+4 ); break;

  case  BIT_1_B   :  BIT_BIT(1, r_B); AddCycles( 4+4 ); break;
  case  BIT_1_C   :  BIT_BIT(1, r_C); AddCycles( 4+4 ); break;
  case  BIT_1_D   :  BIT_BIT(1, r_D); AddCycles( 4+4 ); break;
  case  BIT_1_E   :  BIT_BIT(1, r_E); AddCycles( 4+4 ); break;
  case  BIT_1_H   :  BIT_BIT(1, r_H); AddCycles( 4+4 ); break;
  case  BIT_1_L   :  BIT_BIT(1, r_L); AddCycles( 4+4 ); break;
  case  BIT_1_xHL :  BIT_mem_BIT(1, r_HL); AddCycles( 12 ); break;
  case  BIT_1_A   :  BIT_BIT(1, r_A); AddCycles( 4+4 ); break;

  case  BIT_2_B   :  BIT_BIT(2, r_B); AddCycles( 4+4 ); break;
  case  BIT_2_C   :  BIT_BIT(2, r_C); AddCycles( 4+4 ); break;
  case  BIT_2_D   :  BIT_BIT(2, r_D); AddCycles( 4+4 ); break;
  case  BIT_2_E   :  BIT_BIT(2, r_E); AddCycles( 4+4 ); break;
  case  BIT_2_H   :  BIT_BIT(2, r_H); AddCycles( 4+4 ); break;
  case  BIT_2_L   :  BIT_BIT(2, r_L); AddCycles( 4+4 ); break;
  case  BIT_2_xHL :  BIT_mem_BIT(2, r_HL); AddCycles( 12 ); break;
  case  BIT_2_A   :  BIT_BIT(2, r_A); AddCycles( 4+4 ); break;

  case  BIT_3_B   :  BIT_BIT(3, r_B); AddCycles( 4+4 ); break;
  case  BIT_3_C   :  BIT_BIT(3, r_C); AddCycles( 4+4 ); break;
  case  BIT_3_D   :  BIT_BIT(3, r_D); AddCycles( 4+4 ); break;
  case  BIT_3_E   :  BIT_BIT(3, r_E); AddCycles( 4+4 ); break;
  case  BIT_3_H   :  BIT_BIT(3, r_H); AddCycles( 4+4 ); break;
  case  BIT_3_L   :  BIT_BIT(3, r_L); AddCycles( 4+4 ); break;
  case  BIT_3_xHL :  BIT_mem_BIT(3, r_HL); AddCycles( 12 ); break;
  case  BIT_3_A   :  BIT_BIT(3, r_A); AddCycles( 4+4 ); break;

  case  BIT_4_B   :  BIT_BIT(4, r_B); AddCycles( 4+4 ); break;
  case  BIT_4_C   :  BIT_BIT(4, r_C); AddCycles( 4+4 ); break;
  case  BIT_4_D   :  BIT_BIT(4, r_D); AddCycles( 4+4 ); break;
  case  BIT_4_E   :  BIT_BIT(4, r_E); AddCycles( 4+4 ); break;
  case  BIT_4_H   :  BIT_BIT(4, r_H); AddCycles( 4+4 ); break;
  case  BIT_4_L   :  BIT_BIT(4, r_L); AddCycles( 4+4 ); break;
  case  BIT_4_xHL :  BIT_mem_BIT(4, r_HL); AddCycles( 12 ); break;
  case  BIT_4_A   :  BIT_BIT(4, r_A); AddCycles( 4+4 ); break;

  case  BIT_5_B   :  BIT_BIT(5, r_B); AddCycles( 4+4 ); break;
  case  BIT_5_C   :  BIT_BIT(5, r_C); AddCycles( 4+4 ); break;
  case  BIT_5_D   :  BIT_BIT(5, r_D); AddCycles( 4+4 ); break;
  case  BIT_5_E   :  BIT_BIT(5, r_E); AddCycles( 4+4 ); break;
  case  BIT_5_H   :  BIT_BIT(5, r_H); AddCycles( 4+4 ); break;
  case  BIT_5_L   :  BIT_BIT(5, r_L); AddCycles( 4+4 ); break;
  case  BIT_5_xHL :  BIT_mem_BIT(5, r_HL); AddCycles( 12 ); break;
  case  BIT_5_A   :  BIT_BIT(5, r_A); AddCycles( 4+4 ); break;

  case  BIT_6_B   :  BIT_BIT(6, r_B); AddCycles( 4+4 ); break;
  case  BIT_6_C   :  BIT_BIT(6, r_C); AddCycles( 4+4 ); break;
  case  BIT_6_D   :  BIT_BIT(6, r_D); AddCycles( 4+4 ); break;
  case  BIT_6_E   :  BIT_BIT(6, r_E); AddCycles( 4+4 ); break;
  case  BIT_6_H   :  BIT_BIT(6, r_H); AddCycles( 4+4 ); break;
  case  BIT_6_L   :  BIT_BIT(6, r_L); AddCycles( 4+4 ); break;
  case  BIT_6_xHL :  BIT_mem_BIT(6, r_HL); AddCycles( 12 ); break;
  case  BIT_6_A   :  BIT_BIT(6, r_A); AddCycles( 4+4 ); break;

  case  BIT_7_B   :  BIT_BIT7(r_B); AddCycles( 4+4 ); break;
  case  BIT_7_C   :  BIT_BIT7(r_C); AddCycles( 4+4 ); break;
  case  BIT_7_D   :  BIT_BIT7(r_D); AddCycles( 4+4 ); break;
  case  BIT_7_E   :  BIT_BIT7(r_E); AddCycles( 4+4 ); break;
  case  BIT_7_H   :  BIT_BIT7(r_H); AddCycles( 4+4 ); break;
  case  BIT_7_L   :  BIT_BIT7(r_L); AddCycles( 4+4 ); break;
  case  BIT_7_xHL :  BIT_mem_BIT7(r_HL); AddCycles( 12 ); break;
  case  BIT_7_A   :  BIT_BIT7(r_A); AddCycles( 4+4 ); break;

  case  RES_0_B   :  BIT_RES(0, r_B); AddCycles( 4+4 ); break;
  case  RES_0_C   :  BIT_RES(0, r_C); AddCycles( 4+4 ); break;
  case  RES_0_D   :  BIT_RES(0, r_D); AddCycles( 4+4 ); break;
  case  RES_0_E   :  BIT_RES(0, r_E); AddCycles( 4+4 ); break;
  case  RES_0_H   :  BIT_RES(0, r_H); AddCycles( 4+4 ); break;
  case  RES_0_L   :  BIT_RES(0, r_L); AddCycles( 4+4 ); break;
  case  RES_0_xHL :  BIT_mem_RES(0, r_HL); AddCycles( 4+4+7 ); break;
  case  RES_0_A   :  BIT_RES(0, r_A); AddCycles( 4+4 ); break;

  case  RES_1_B   :  BIT_RES(1, r_B); AddCycles( 4+4 ); break;
  case  RES_1_C   :  BIT_RES(1, r_C); AddCycles( 4+4 ); break;
  case  RES_1_D   :  BIT_RES(1, r_D); AddCycles( 4+4 ); break;
  case  RES_1_E   :  BIT_RES(1, r_E); AddCycles( 4+4 ); break;
  case  RES_1_H   :  BIT_RES(1, r_H); AddCycles( 4+4 ); break;
  case  RES_1_L   :  BIT_RES(1, r_L); AddCycles( 4+4 ); break;
  case  RES_1_xHL :  BIT_mem_RES(1, r_HL); AddCycles( 4+4+7 ); break;
  case  RES_1_A   :  BIT_RES(1, r_A); AddCycles( 4+4 ); break;

  case  RES_2_B   :  BIT_RES(2, r_B); AddCycles( 4+4 ); break;
  case  RES_2_C   :  BIT_RES(2, r_C); AddCycles( 4+4 ); break;
  case  RES_2_D   :  BIT_RES(2, r_D); AddCycles( 4+4 ); break;
  case  RES_2_E   :  BIT_RES(2, r_E); AddCycles( 4+4 ); break;
  case  RES_2_H   :  BIT_RES(2, r_H); AddCycles( 4+4 ); break;
  case  RES_2_L   :  BIT_RES(2, r_L); AddCycles( 4+4 ); break;
  case  RES_2_xHL :  BIT_mem_RES(2, r_HL); AddCycles( 4+4+7 ); break;
  case  RES_2_A   :  BIT_RES(2, r_A); AddCycles( 4+4 ); break;

  case  RES_3_B   :  BIT_RES(3, r_B); AddCycles( 4+4 ); break;
  case  RES_3_C   :  BIT_RES(3, r_C); AddCycles( 4+4 ); break;
  case  RES_3_D   :  BIT_RES(3, r_D); AddCycles( 4+4 ); break;
  case  RES_3_E   :  BIT_RES(3, r_E); AddCycles( 4+4 ); break;
  case  RES_3_H   :  BIT_RES(3, r_H); AddCycles( 4+4 ); break;
  case  RES_3_L   :  BIT_RES(3, r_L); AddCycles( 4+4 ); break;
  case  RES_3_xHL :  BIT_mem_RES(3, r_HL); AddCycles( 4+4+7 ); break;
  case  RES_3_A   :  BIT_RES(3, r_A); AddCycles( 4+4 ); break;

  case  RES_4_B   :  BIT_RES(4, r_B); AddCycles( 4+4 ); break;
  case  RES_4_C   :  BIT_RES(4, r_C); AddCycles( 4+4 ); break;
  case  RES_4_D   :  BIT_RES(4, r_D); AddCycles( 4+4 ); break;
  case  RES_4_E   :  BIT_RES(4, r_E); AddCycles( 4+4 ); break;
  case  RES_4_H   :  BIT_RES(4, r_H); AddCycles( 4+4 ); break;
  case  RES_4_L   :  BIT_RES(4, r_L); AddCycles( 4+4 ); break;
  case  RES_4_xHL :  BIT_mem_RES(4, r_HL); AddCycles( 4+4+7 ); break;
  case  RES_4_A   :  BIT_RES(4, r_A); AddCycles( 4+4 ); break;

  case  RES_5_B   :  BIT_RES(5, r_B); AddCycles( 4+4 ); break;
  case  RES_5_C   :  BIT_RES(5, r_C); AddCycles( 4+4 ); break;
  case  RES_5_D   :  BIT_RES(5, r_D); AddCycles( 4+4 ); break;
  case  RES_5_E   :  BIT_RES(5, r_E); AddCycles( 4+4 ); break;
  case  RES_5_H   :  BIT_RES(5, r_H); AddCycles( 4+4 ); break;
  case  RES_5_L   :  BIT_RES(5, r_L); AddCycles( 4+4 ); break;
  case  RES_5_xHL :  BIT_mem_RES(5, r_HL); AddCycles( 4+4+7 ); break;
  case  RES_5_A   :  BIT_RES(5, r_A); AddCycles( 4+4 ); break;

  case  RES_6_B   :  BIT_RES(6, r_B); AddCycles( 4+4 ); break;
  case  RES_6_C   :  BIT_RES(6, r_C); AddCycles( 4+4 ); break;
  case  RES_6_D   :  BIT_RES(6, r_D); AddCycles( 4+4 ); break;
  case  RES_6_E   :  BIT_RES(6, r_E); AddCycles( 4+4 ); break;
  case  RES_6_H   :  BIT_RES(6, r_H); AddCycles( 4+4 ); break;
  case  RES_6_L   :  BIT_RES(6, r_L); AddCycles( 4+4 ); break;
  case  RES_6_xHL :  BIT_mem_RES(6, r_HL); AddCycles( 4+4+7 ); break;
  case  RES_6_A   :  BIT_RES(6, r_A); AddCycles( 4+4 ); break;

  case  RES_7_B   :  BIT_RES(7, r_B); AddCycles( 4+4 ); break;
  case  RES_7_C   :  BIT_RES(7, r_C); AddCycles( 4+4 ); break;
  case  RES_7_D   :  BIT_RES(7, r_D); AddCycles( 4+4 ); break;
  case  RES_7_E   :  BIT_RES(7, r_E); AddCycles( 4+4 ); break;
  case  RES_7_H   :  BIT_RES(7, r_H); AddCycles( 4+4 ); break;
  case  RES_7_L   :  BIT_RES(7, r_L); AddCycles( 4+4 ); break;
  case  RES_7_xHL :  BIT_mem_RES(7, r_HL); AddCycles( 4+4+7 ); break;
  case  RES_7_A   :  BIT_RES(7, r_A); AddCycles( 4+4 ); break;

  case  SET_0_B   :  BIT_SET(0, r_B); AddCycles( 4+4 ); break;
  case  SET_0_C   :  BIT_SET(0, r_C); AddCycles( 4+4 ); break;
  case  SET_0_D   :  BIT_SET(0, r_D); AddCycles( 4+4 ); break;
  case  SET_0_E   :  BIT_SET(0, r_E); AddCycles( 4+4 ); break;
  case  SET_0_H   :  BIT_SET(0, r_H); AddCycles( 4+4 ); break;
  case  SET_0_L   :  BIT_SET(0, r_L); AddCycles( 4+4 ); break;
  case  SET_0_xHL :  BIT_mem_SET(0, r_HL); AddCycles( 4+4+7 ); break;
  case  SET_0_A   :  BIT_SET(0, r_A); AddCycles( 4+4 ); break;

  case  SET_1_B   :  BIT_SET(1, r_B); AddCycles( 4+4 ); break;
  case  SET_1_C   :  BIT_SET(1, r_C); AddCycles( 4+4 ); break;
  case  SET_1_D   :  BIT_SET(1, r_D); AddCycles( 4+4 ); break;
  case  SET_1_E   :  BIT_SET(1, r_E); AddCycles( 4+4 ); break;
  case  SET_1_H   :  BIT_SET(1, r_H); AddCycles( 4+4 ); break;
  case  SET_1_L   :  BIT_SET(1, r_L); AddCycles( 4+4 ); break;
  case  SET_1_xHL :  BIT_mem_SET(1, r_HL); AddCycles( 4+4+7 ); break;
  case  SET_1_A   :  BIT_SET(1, r_A); AddCycles( 4+4 ); break;

  case  SET_2_B   :  BIT_SET(2, r_B); AddCycles( 4+4 ); break;
  case  SET_2_C   :  BIT_SET(2, r_C); AddCycles( 4+4 ); break;
  case  SET_2_D   :  BIT_SET(2, r_D); AddCycles( 4+4 ); break;
  case  SET_2_E   :  BIT_SET(2, r_E); AddCycles( 4+4 ); break;
  case  SET_2_H   :  BIT_SET(2, r_H); AddCycles( 4+4 ); break;
  case  SET_2_L   :  BIT_SET(2, r_L); AddCycles( 4+4 ); break;
  case  SET_2_xHL :  BIT_mem_SET(2, r_HL); AddCycles( 4+4+7 ); break;
  case  SET_2_A   :  BIT_SET(2, r_A); AddCycles( 4+4 ); break;

  case  SET_3_B   :  BIT_SET(3, r_B); AddCycles( 4+4 ); break;
  case  SET_3_C   :  BIT_SET(3, r_C); AddCycles( 4+4 ); break;
  case  SET_3_D   :  BIT_SET(3, r_D); AddCycles( 4+4 ); break;
  case  SET_3_E   :  BIT_SET(3, r_E); AddCycles( 4+4 ); break;
  case  SET_3_H   :  BIT_SET(3, r_H); AddCycles( 4+4 ); break;
  case  SET_3_L   :  BIT_SET(3, r_L); AddCycles( 4+4 ); break;
  case  SET_3_xHL :  BIT_mem_SET(3, r_HL); AddCycles( 4+4+7 ); break;
  case  SET_3_A   :  BIT_SET(3, r_A); AddCycles( 4+4 ); break;

  case  SET_4_B   :  BIT_SET(4, r_B); AddCycles( 4+4 ); break;
  case  SET_4_C   :  BIT_SET(4, r_C); AddCycles( 4+4 ); break;
  case  SET_4_D   :  BIT_SET(4, r_D); AddCycles( 4+4 ); break;
  case  SET_4_E   :  BIT_SET(4, r_E); AddCycles( 4+4 ); break;
  case  SET_4_H   :  BIT_SET(4, r_H); AddCycles( 4+4 ); break;
  case  SET_4_L   :  BIT_SET(4, r_L); AddCycles( 4+4 ); break;
  case  SET_4_xHL :  BIT_mem_SET(4, r_HL); AddCycles( 4+4+7 ); break;
  case  SET_4_A   :  BIT_SET(4, r_A); AddCycles( 4+4 ); break;

  case  SET_5_B   :  BIT_SET(5, r_B); AddCycles( 4+4 ); break;
  case  SET_5_C   :  BIT_SET(5, r_C); AddCycles( 4+4 ); break;
  case  SET_5_D   :  BIT_SET(5, r_D); AddCycles( 4+4 ); break;
  case  SET_5_E   :  BIT_SET(5, r_E); AddCycles( 4+4 ); break;
  case  SET_5_H   :  BIT_SET(5, r_H); AddCycles( 4+4 ); break;
  case  SET_5_L   :  BIT_SET(5, r_L); AddCycles( 4+4 ); break;
  case  SET_5_xHL :  BIT_mem_SET(5, r_HL); AddCycles( 4+4+7 ); break;
  case  SET_5_A   :  BIT_SET(5, r_A); AddCycles( 4+4 ); break;

  case  SET_6_B   :  BIT_SET(6, r_B); AddCycles( 4+4 ); break;
  case  SET_6_C   :  BIT_SET(6, r_C); AddCycles( 4+4 ); break;
  case  SET_6_D   :  BIT_SET(6, r_D); AddCycles( 4+4 ); break;
  case  SET_6_E   :  BIT_SET(6, r_E); AddCycles( 4+4 ); break;
  case  SET_6_H   :  BIT_SET(6, r_H); AddCycles( 4+4 ); break;
  case  SET_6_L   :  BIT_SET(6, r_L); AddCycles( 4+4 ); break;
  case  SET_6_xHL :  BIT_mem_SET(6, r_HL); AddCycles( 4+4+7 ); break;
  case  SET_6_A   :  BIT_SET(6, r_A); AddCycles( 4+4 ); break;

  case  SET_7_B   :  BIT_SET(7, r_B); AddCycles( 4+4 ); break;
  case  SET_7_C   :  BIT_SET(7, r_C); AddCycles( 4+4 ); break;
  case  SET_7_D   :  BIT_SET(7, r_D); AddCycles( 4+4 ); break;
  case  SET_7_E   :  BIT_SET(7, r_E); AddCycles( 4+4 ); break;
  case  SET_7_H   :  BIT_SET(7, r_H); AddCycles( 4+4 ); break;
  case  SET_7_L   :  BIT_SET(7, r_L); AddCycles( 4+4 ); break;
  case  SET_7_xHL :  BIT_mem_SET(7, r_HL); AddCycles( 4+4+7 ); break;
  case  SET_7_A   :  BIT_SET(7, r_A); AddCycles( 4+4 ); break;

  default:
//    exit(1);
///!!!   if( regs->DecodingErrors )
///!!!    printf("z80 core: Unknown instruction: CB %02Xh at PC=%04Xh.\n",
///!!!            Z80ReadMem(r_PC-1), r_PC-2 );
    break;
}

