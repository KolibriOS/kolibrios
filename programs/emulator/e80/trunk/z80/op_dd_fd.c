/*====================================================================/*
  opcodes_dd_fd.c -> This file executes the DD/FD PREFIX opcodes.

  The DD prefix "creates" some new instructions by changing HL to IX
  on the opcode defined by the next byte on memory.

  The FD prefix "creates" some new instructions by changing HL to IY
  on the opcode defined by the next byte on memory.

  Change the REGISTER variable to IX or HY before including this file.
  Something like:

        #define REGISTER  regs->IX
            #include "op_dd_fd.c"
        #undef  REGISTER

 On this code, this REGISTER variable is used as REGISTER.W or
 REGISTER.B.h and REGISTER.B.l ...

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

/* 8 clock cycles minimum = DD opcode = FD opcode = 4 + 4 */

#define REG  REGISTER.W
#define REGL REGISTER.B.l
#define REGH REGISTER.B.h

opcode = Z80ReadMem( r_PC );
r_PC++;

switch(opcode)
{
case  ADD_IXY_BC    : ADD_WORD(REG, r_BC); AddCycles( 4+4+7 ); break;
case  ADD_IXY_DE    : ADD_WORD(REG, r_DE); AddCycles( 4+4+7 ); break;
case  ADD_IXY_SP    : ADD_WORD(REG, r_SP); AddCycles( 4+4+7 ); break;
case  ADD_IXY_IXY   : ADD_WORD(REG, REG); AddCycles( 4+4+7 ); break;
case  DEC_IXY       : REG--; AddCycles( 4+4+2 ); break;
case  INC_IXY       : REG++; AddCycles( 4+4 ); break;

case  JP_IXY        : r_PC = REG; AddCycles( 4+4 );break;
case  LD_SP_IXY     : r_SP = REG; AddCycles( 4+4+2 ); break;

case  PUSH_IXY      : PUSH_IXYr(); AddCycles( 4+4+3+3+1 ); break;
case  POP_IXY       : POP_IXYr();  AddCycles( 4+4+3+3 ); break;

case  EX_IXY_xSP    : r_meml = Z80ReadMem(r_SP);
                      r_memh = Z80ReadMem(r_SP+1);
                      Z80WriteMem( r_SP, REGL, regs );
                      Z80WriteMem( r_SP+1, REGH, regs );
                      REGL=r_meml; REGH=r_memh;
                      AddCycles( 4+4+3+3+3+3+3 ); break;

case  LD_A_xIXY     : r_A = Z80ReadMem( REG+ ((offset) Z80ReadMem(r_PC)) );
                      r_PC++; AddCycles( 4+3+3+3+3+3 ); break;
case  LD_B_xIXY     : r_B = Z80ReadMem( REG+ ((offset) Z80ReadMem(r_PC)) );
                      r_PC++; AddCycles( 4+3+3+3+3+3 ); break;
case  LD_C_xIXY     : r_C = Z80ReadMem( REG+ ((offset) Z80ReadMem(r_PC)) );
                      r_PC++; AddCycles( 4+3+3+3+3+3 ); break;
case  LD_D_xIXY     : r_D = Z80ReadMem( REG+ ((offset) Z80ReadMem(r_PC)) );
                      r_PC++; AddCycles( 4+3+3+3+3+3 ); break;
case  LD_E_xIXY     : r_E = Z80ReadMem( REG+ ((offset) Z80ReadMem(r_PC)) );
                      r_PC++; AddCycles( 4+3+3+3+3+3 ); break;

case  LD_xIXY_A     : Z80WriteMem( REG+(offset)Z80ReadMem(r_PC), r_A, regs );
                      r_PC++; AddCycles( 4+3+3+3+3+3 ); break;
case  LD_xIXY_B     : Z80WriteMem( REG+(offset)Z80ReadMem(r_PC), r_B, regs );
                      r_PC++; AddCycles( 4+3+3+3+3+3 ); break;
case  LD_xIXY_C     : Z80WriteMem( REG+(offset)Z80ReadMem(r_PC), r_C, regs );
                      r_PC++; AddCycles( 4+3+3+3+3+3 ); break;
case  LD_xIXY_D     : Z80WriteMem( REG+(offset)Z80ReadMem(r_PC), r_D, regs );
                      r_PC++; AddCycles( 4+3+3+3+3+3 ); break;
case  LD_xIXY_E     : Z80WriteMem( REG+(offset)Z80ReadMem(r_PC), r_E, regs );
                      r_PC++; AddCycles( 4+3+3+3+3+3 ); break;

case  INC_xIXY      : r_mem = REG+(offset)Z80ReadMem(r_PC);
                      r_PC++;
                      tmpreg.B.l = Z80ReadMem(r_mem);
                      INC(tmpreg.B.l);
                      Z80WriteMem(r_mem, tmpreg.B.l, regs );
                      AddCycles( 4+3+3 + 3+3+3+ 3+1); break;
case  DEC_xIXY      : r_mem = REG+(offset)Z80ReadMem(r_PC);
                      r_PC++;
                      tmpreg.B.l = Z80ReadMem(r_mem);
                      DEC(tmpreg.B.l);
                      Z80WriteMem(r_mem, tmpreg.B.l, regs );
                      AddCycles( 4+3+3 + 3+3+3+ 3+1); break;

case  ADC_xIXY      : r_meml = Z80ReadMem(REG+(offset)Z80ReadMem(r_PC)); r_PC++;
                      ADC(r_meml); AddCycles( 4+3+3+3+3+3 ); break;
case  SBC_xIXY      : r_meml = Z80ReadMem(REG+(offset)Z80ReadMem(r_PC));
                      r_PC++;
                      SBC(r_meml); AddCycles( 4+3+3+3+3+3 ); break;
case  ADD_xIXY      : r_meml = Z80ReadMem(REG+(offset)Z80ReadMem(r_PC));
                      r_PC++;
                      ADD(r_meml); AddCycles( 4+3+3+3+3+3 ); break;
case  SUB_xIXY      : r_meml = Z80ReadMem(REG+(offset)Z80ReadMem(r_PC));
                      r_PC++;
                      SUB(r_meml); AddCycles( 4+3+3+3+3+3 ); break;
case  AND_xIXY      : r_meml = Z80ReadMem(REG+(offset)Z80ReadMem(r_PC));
                      r_PC++;
                      AND(r_meml); AddCycles( 4+3+3+3+3+3 ); break;
case  OR_xIXY       : r_meml = Z80ReadMem(REG+(offset)Z80ReadMem(r_PC));
                      r_PC++;
                      OR(r_meml); AddCycles( 4+3+3+3+3+3 ); break;
case  XOR_xIXY      : r_meml = Z80ReadMem(REG+(offset)Z80ReadMem(r_PC));
                      r_PC++;
                      XOR(r_meml); AddCycles( 4+3+3+3+3+3 ); break;

case  CP_xIXY       : r_meml = Z80ReadMem(REG+(offset)Z80ReadMem(r_PC));
                      r_PC++;
                      CP(r_meml); AddCycles( 4+3+3+3+3+3 ); break;

case  LD_IXY_NN     : REGL = Z80ReadMem(r_PC); r_PC++;
                      REGH = Z80ReadMem(r_PC); r_PC++;
                      AddCycles( 4+1+3+3+3 ); break;

case  LD_xIXY_N     : r_mem = REG + (offset) Z80ReadMem(r_PC); r_PC++;
                      Z80WriteMem( r_mem, Z80ReadMem(r_PC), regs );  r_PC++;
                      AddCycles( 4+3+3+3+3+3 ); break;

case  LD_IXY_xNN    : LOAD_rr_nn(REG);
                      AddCycles( 4+3+3+3+3+3+1 ); break;

case  LD_xNN_IXY    : STORE_nn_rr(REG);
                      AddCycles( 4+3+3+ 3+3+3+1 ); break;


/* some undocumented opcodes: may be wrong: */
case  LD_A_IXYh     : r_A  = REGH; AddCycles(4+4); break;
case  LD_A_IXYl     : r_A  = REGL; AddCycles(4+4); break;
case  LD_B_IXYh     : r_B  = REGH; AddCycles(4+4); break;
case  LD_B_IXYl     : r_B  = REGL; AddCycles(4+4); break;
case  LD_C_IXYh     : r_C  = REGH; AddCycles(4+4); break;
case  LD_C_IXYl     : r_C  = REGL; AddCycles(4+4); break;
case  LD_D_IXYh     : r_D  = REGH; AddCycles(4+4); break;
case  LD_D_IXYl     : r_D  = REGL; AddCycles(4+4); break;
case  LD_E_IXYh     : r_E  = REGH; AddCycles(4+4); break;
case  LD_E_IXYl     : r_E  = REGL; AddCycles(4+4); break;
case  LD_IXYh_A     : REGH = r_A;  AddCycles(4+4); break;
case  LD_IXYh_B     : REGH = r_B;  AddCycles(4+4); break;
case  LD_IXYh_C     : REGH = r_C;  AddCycles(4+4); break;
case  LD_IXYh_D     : REGH = r_D;  AddCycles(4+4); break;
case  LD_IXYh_E     : REGH = r_E;  AddCycles(4+4); break;
case  LD_IXYh_IXYh  : AddCycles(4+4); break;
case  LD_IXYh_IXYl  : REGH = REGL; AddCycles(4+4); break;
case  LD_IXYl_A     : REGL = r_A;  AddCycles(4+4); break;
case  LD_IXYl_B     : REGL = r_B;  AddCycles(4+4); break;
case  LD_IXYl_C     : REGL = r_C;  AddCycles(4+4); break;
case  LD_IXYl_D     : REGL = r_D;  AddCycles(4+4); break;
case  LD_IXYl_E     : REGL = r_E;  AddCycles(4+4); break;
case  LD_IXYl_IXYh  : REGL = REGH; AddCycles(4+4); break;
case  LD_IXYl_IXYl  : AddCycles(4+4); break;
case  LD_IXYh_N     : REGH = Z80ReadMem(r_PC); r_PC++;
                      AddCycles(4+4+3); break;
case  LD_IXYl_N     : REGL = Z80ReadMem(r_PC); r_PC++;
                      AddCycles(4+4+3); break;


case  ADD_IXYh      : ADD(REGH); AddCycles(4+4); break;
case  ADD_IXYl      : ADD(REGL); AddCycles(4+4); break;
case  ADC_IXYh      : ADC(REGH); AddCycles(4+4); break;
case  ADC_IXYl      : ADC(REGL); AddCycles(4+4); break;
case  SUB_IXYh      : SUB(REGH); AddCycles(4+4); break;
case  SUB_IXYl      : SUB(REGL); AddCycles(4+4); break;
case  SBC_IXYh      : SBC(REGH); AddCycles(4+4); break;
case  SBC_IXYl      : SBC(REGL); AddCycles(4+4); break;
case  AND_IXYh      : AND(REGH); AddCycles(4+4); break;
case  AND_IXYl      : AND(REGL); AddCycles(4+4); break;
case  XOR_IXYh      : XOR(REGH); AddCycles(4+4); break;
case  XOR_IXYl      : XOR(REGL); AddCycles(4+4); break;
case  OR_IXYh       : OR(REGH);  AddCycles(4+4); break;
case  OR_IXYl       : OR(REGL);  AddCycles(4+4); break;
case  CP_IXYh       : CP(REGH);  AddCycles(4+4); break;
case  CP_IXYl       : CP(REGL);  AddCycles(4+4); break;
case  INC_IXYh      : INC(REGH); AddCycles(4+4); break;
case  INC_IXYl      : INC(REGL); AddCycles(4+4); break;
case  DEC_IXYh      : DEC(REGH); AddCycles(4+4); break;
case  DEC_IXYl      : DEC(REGL); AddCycles(4+4); break;

case  LD_xIXY_H     : r_meml =Z80ReadMem(r_PC); r_PC++;
                      Z80WriteMem( REG+(offset)(r_meml), r_H, regs );
                      AddCycles( 4+3+3+3+3+3 ); break;
case  LD_xIXY_L     : r_meml =Z80ReadMem(r_PC); r_PC++;
                      Z80WriteMem( REG+(offset)(r_meml), r_L, regs );
                      AddCycles( 4+3+3+3+3+3 ); break;
case  LD_H_xIXY     : r_meml =Z80ReadMem(r_PC); r_PC++;
                      r_H = Z80ReadMem( REG+(offset)(r_meml));
                      AddCycles( 4+3+3+3+3+3 ); break;
case  LD_L_xIXY     : r_meml =Z80ReadMem(r_PC); r_PC++;
                      r_L = Z80ReadMem( REG+(offset)(r_meml));
                      AddCycles( 4+3+3+3+3+3 ); break;

case PREFIX_CB:
                      #include "opddfdcb.c"
                      break;

/*
case PREFIX_DD:
case PREFIX_FD:       AddCycles( 4 );
                      r_PC--;                 // decode it the next time :)
                      break;
*/

default:              AddCycles( 4 );
                      r_PC--;                 /* decode it the next time :) */
                      SubR(1);

//    exit(1);
//    if( regs->DecodingErrors )
//    {
//      printf("z80 core: Unknown instruction: ");
//      if ( regs->we_are_on_ddfd == WE_ARE_ON_DD )
//         printf("DD ");
//      else
//         printf("FD ");
//      printf("%02Xh at PC=%04Xh.\n", Z80ReadMem(r_PC-1), r_PC-2 );
//    }
    break;
}

#undef REG
#undef REGL
#undef REGH
