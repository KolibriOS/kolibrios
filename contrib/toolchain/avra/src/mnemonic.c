/***********************************************************************
 *
 *  avra - Assembler for the Atmel AVR microcontroller series
 *
 *  Copyright (C) 1998-2004 Jon Anders Haugum, Tobias Weber
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 *
 *  Authors of avra can be reached at:
 *     email: jonah@omegav.ntnu.no, tobiw@suprafluid.com
 *     www: http://sourceforge.net/projects/avra
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "misc.h"
#include "avra.h"
#include "device.h"

#define MAX_MNEMONIC_LEN	8	// Maximum mnemonic length

enum {
	MNEMONIC_NOP = 0,  //          0000 0000 0000 0000
	MNEMONIC_SEC,      //          1001 0100 0000 1000
	MNEMONIC_CLC,      //          1001 0100 1000 1000
	MNEMONIC_SEN,      //          1001 0100 0010 1000
	MNEMONIC_CLN,      //          1001 0100 1010 1000
	MNEMONIC_SEZ,      //          1001 0100 0001 1000
	MNEMONIC_CLZ,      //          1001 0100 1001 1000
	MNEMONIC_SEI,      //          1001 0100 0111 1000
	MNEMONIC_CLI,      //          1001 0100 1111 1000
	MNEMONIC_SES,      //          1001 0100 0100 1000
	MNEMONIC_CLS,      //          1001 0100 1100 1000
	MNEMONIC_SEV,      //          1001 0100 0011 1000
	MNEMONIC_CLV,      //          1001 0100 1011 1000
	MNEMONIC_SET,      //          1001 0100 0110 1000
	MNEMONIC_CLT,      //          1001 0100 1110 1000
	MNEMONIC_SEH,      //          1001 0100 0101 1000
	MNEMONIC_CLH,      //          1001 0100 1101 1000
	MNEMONIC_SLEEP,    //          1001 0101 1000 1000
	MNEMONIC_WDR,      //          1001 0101 1010 1000
	MNEMONIC_IJMP,     //          1001 0100 0000 1001
	MNEMONIC_EIJMP,    //          1001 0100 0001 1001
	MNEMONIC_ICALL,    //          1001 0101 0000 1001
	MNEMONIC_EICALL,   //          1001 0101 0001 1001
	MNEMONIC_RET,      //          1001 0101 0000 1000
	MNEMONIC_RETI,     //          1001 0101 0001 1000
	MNEMONIC_SPM,      //          1001 0101 1110 1000
	MNEMONIC_ESPM,     //          1001 0101 1111 1000
	MNEMONIC_BREAK,    //          1001 0101 1001 1000
	MNEMONIC_LPM,      //          1001 0101 1100 1000
	MNEMONIC_ELPM,     //          1001 0101 1101 1000
	MNEMONIC_BSET,     // s        1001 0100 0sss 1000
	MNEMONIC_BCLR,     // s        1001 0100 1sss 1000
	MNEMONIC_SER,      // Rd       1110 1111 dddd 1111
	MNEMONIC_COM,      // Rd       1001 010d dddd 0000
	MNEMONIC_NEG,      // Rd       1001 010d dddd 0001
	MNEMONIC_INC,      // Rd       1001 010d dddd 0011
	MNEMONIC_DEC,      // Rd       1001 010d dddd 1010
	MNEMONIC_LSR,      // Rd       1001 010d dddd 0110
	MNEMONIC_ROR,      // Rd       1001 010d dddd 0111
	MNEMONIC_ASR,      // Rd       1001 010d dddd 0101
	MNEMONIC_SWAP,     // Rd       1001 010d dddd 0010
	MNEMONIC_PUSH,     // Rr       1001 001r rrrr 1111
	MNEMONIC_POP,      // Rd       1001 000d dddd 1111
	MNEMONIC_TST,      // Rd       0010 00dd dddd dddd
	MNEMONIC_CLR,      // Rd       0010 01dd dddd dddd
	MNEMONIC_LSL,      // Rd       0000 11dd dddd dddd
	MNEMONIC_ROL,      // Rd       0001 11dd dddd dddd
	MNEMONIC_BREQ,     // k        1111 00kk kkkk k001
	MNEMONIC_BRNE,     // k        1111 01kk kkkk k001
	MNEMONIC_BRCS,     // k        1111 00kk kkkk k000
	MNEMONIC_BRCC,     // k        1111 01kk kkkk k000
	MNEMONIC_BRSH,     // k        1111 01kk kkkk k000
	MNEMONIC_BRLO,     // k        1111 00kk kkkk k000
	MNEMONIC_BRMI,     // k        1111 00kk kkkk k010
	MNEMONIC_BRPL,     // k        1111 01kk kkkk k010
	MNEMONIC_BRGE,     // k        1111 01kk kkkk k100
	MNEMONIC_BRLT,     // k        1111 00kk kkkk k100
	MNEMONIC_BRHS,     // k        1111 00kk kkkk k101
	MNEMONIC_BRHC,     // k        1111 01kk kkkk k101
	MNEMONIC_BRTS,     // k        1111 00kk kkkk k110
	MNEMONIC_BRTC,     // k        1111 01kk kkkk k110
	MNEMONIC_BRVS,     // k        1111 00kk kkkk k011
	MNEMONIC_BRVC,     // k        1111 01kk kkkk k011
	MNEMONIC_BRIE,     // k        1111 00kk kkkk k111
	MNEMONIC_BRID,     // k        1111 01kk kkkk k111
	MNEMONIC_RJMP,     // k        1100 kkkk kkkk kkkk
	MNEMONIC_RCALL,    // k        1101 kkkk kkkk kkkk
	MNEMONIC_JMP,      // k        1001 010k kkkk 110k + 16k
	MNEMONIC_CALL,     // k        1001 010k kkkk 111k + 16k
	MNEMONIC_BRBS,     // s, k     1111 00kk kkkk ksss
	MNEMONIC_BRBC,     // s, k     1111 01kk kkkk ksss
	MNEMONIC_ADD,      // Rd, Rr   0000 11rd dddd rrrr
	MNEMONIC_ADC,      // Rd, Rr   0001 11rd dddd rrrr
	MNEMONIC_SUB,      // Rd, Rr   0001 10rd dddd rrrr
	MNEMONIC_SBC,      // Rd, Rr   0000 10rd dddd rrrr
	MNEMONIC_AND,      // Rd, Rr   0010 00rd dddd rrrr
	MNEMONIC_OR,       // Rd, Rr   0010 10rd dddd rrrr
	MNEMONIC_EOR,      // Rd, Rr   0010 01rd dddd rrrr
	MNEMONIC_CP,       // Rd, Rr   0001 01rd dddd rrrr
	MNEMONIC_CPC,      // Rd, Rr   0000 01rd dddd rrrr
	MNEMONIC_CPSE,     // Rd, Rr   0001 00rd dddd rrrr
	MNEMONIC_MOV,      // Rd, Rr   0010 11rd dddd rrrr
	MNEMONIC_MUL,      // Rd, Rr   1001 11rd dddd rrrr
	MNEMONIC_MOVW,     // Rd, Rr   0000 0001 dddd rrrr
	MNEMONIC_MULS,     // Rd, Rr   0000 0010 dddd rrrr
	MNEMONIC_MULSU,    // Rd, Rr   0000 0011 0ddd 0rrr
	MNEMONIC_FMUL,     // Rd, Rr   0000 0011 0ddd 1rrr
	MNEMONIC_FMULS,    // Rd, Rr   0000 0011 1ddd 0rrr
	MNEMONIC_FMULSU,   // Rd, Rr   0000 0011 1ddd 1rrr
	MNEMONIC_ADIW,     // Rd, K    1001 0110 KKdd KKKK
	MNEMONIC_SBIW,     // Rd, K    1001 0111 KKdd KKKK
	MNEMONIC_SUBI,     // Rd, K    0101 KKKK dddd KKKK
	MNEMONIC_SBCI,     // Rd, K    0100 KKKK dddd KKKK
	MNEMONIC_ANDI,     // Rd, K    0111 KKKK dddd KKKK
	MNEMONIC_ORI,      // Rd, K    0110 KKKK dddd KKKK
	MNEMONIC_SBR,      // Rd, K    0110 KKKK dddd KKKK
	MNEMONIC_CPI,      // Rd, K    0011 KKKK dddd KKKK
	MNEMONIC_LDI,      // Rd, K    1110 KKKK dddd KKKK
	MNEMONIC_CBR,      // Rd, K    0111 KKKK dddd KKKK ~K
	MNEMONIC_SBRC,     // Rr, b    1111 110r rrrr 0bbb
	MNEMONIC_SBRS,     // Rr, b    1111 111r rrrr 0bbb
	MNEMONIC_BST,      // Rr, b    1111 101d dddd 0bbb
	MNEMONIC_BLD,      // Rd, b    1111 100d dddd 0bbb
	MNEMONIC_IN,       // Rd, P    1011 0PPd dddd PPPP
	MNEMONIC_OUT,      // P, Rr    1011 1PPr rrrr PPPP
	MNEMONIC_SBIC,     // P, b     1001 1001 PPPP Pbbb
	MNEMONIC_SBIS,     // P, b     1001 1011 PPPP Pbbb
	MNEMONIC_SBI,      // P, b     1001 1010 PPPP Pbbb
	MNEMONIC_CBI,      // P, b     1001 1000 PPPP Pbbb
	MNEMONIC_LDS,      // Rd, k    1001 000d dddd 0000 + 16k
	MNEMONIC_STS,      // k, Rr    1001 001d dddd 0000 + 16k
	MNEMONIC_LD,       // Rd, __   dummy
	MNEMONIC_ST,       // __, Rr   dummy
	MNEMONIC_LDD,      // Rd, _+q  dummy
	MNEMONIC_STD,      // _+q, Rr  dummy
	MNEMONIC_COUNT,
	MNEMONIC_LPM_Z,    // Rd, Z    1001 000d dddd 0100
	MNEMONIC_LPM_ZP,   // Rd, Z+   1001 000d dddd 0101
	MNEMONIC_ELPM_Z,   // Rd, Z    1001 000d dddd 0110
	MNEMONIC_ELPM_ZP,  // Rd, Z+   1001 000d dddd 0111
	MNEMONIC_LD_X,     // Rd, X    1001 000d dddd 1100
	MNEMONIC_LD_XP,    // Rd, X+   1001 000d dddd 1101
	MNEMONIC_LD_MX,    // Rd, -X   1001 000d dddd 1110
	MNEMONIC_LD_Y,     // Rd, Y    1000 000d dddd 1000
	MNEMONIC_LD_YP,    // Rd, Y+   1001 000d dddd 1001
	MNEMONIC_LD_MY,    // Rd, -Y   1001 000d dddd 1010
	MNEMONIC_LD_Z,     // Rd, Z    1000 000d dddd 0000
	MNEMONIC_LD_ZP,    // Rd, Z+   1001 000d dddd 0001
	MNEMONIC_LD_MZ,    // Rd, -Z   1001 000d dddd 0010
	MNEMONIC_ST_X,     // X, Rr    1001 001d dddd 1100
	MNEMONIC_ST_XP,    // X+, Rr   1001 001d dddd 1101
	MNEMONIC_ST_MX,    // -X, Rr   1001 001d dddd 1110
	MNEMONIC_ST_Y,     // Y, Rr    1000 001d dddd 1000
	MNEMONIC_ST_YP,    // Y+, Rr   1001 001d dddd 1001
	MNEMONIC_ST_MY,    // -Y, Rr   1001 001d dddd 1010
	MNEMONIC_ST_Z,     // Z, Rr    1000 001d dddd 0000
	MNEMONIC_ST_ZP,    // Z+, Rr   1001 001d dddd 0001
	MNEMONIC_ST_MZ,    // -Z, Rr   1001 001d dddd 0010
	MNEMONIC_LDD_Y,    // Rd, Y+q  10q0 qq0d dddd 1qqq
	MNEMONIC_LDD_Z,    // Rd, Z+q  10q0 qq0d dddd 0qqq
	MNEMONIC_STD_Y,    // Y+q, Rr  10q0 qq1r rrrr 1qqq
	MNEMONIC_STD_Z,    // Z+q, Rr  10q0 qq1r rrrr 0qqq
	MNEMONIC_END
};

struct instruction {
	char *mnemonic;
	int opcode;
        int flag;	/* Device flags meaning the instruction is not
                           supported */
};

struct instruction instruction_list[] = {
	{"nop",   0x0000,          0},
	{"sec",   0x9408,          0},
	{"clc",   0x9488,          0},
	{"sen",   0x9428,          0},
	{"cln",   0x94a8,          0},
	{"sez",   0x9418,          0},
	{"clz",   0x9498,          0},
	{"sei",   0x9478,          0},
	{"cli",   0x94f8,          0},
	{"ses",   0x9448,          0},
	{"cls",   0x94c8,          0},
	{"sev",   0x9438,          0},
	{"clv",   0x94b8,          0},
	{"set",   0x9468,          0},
	{"clt",   0x94e8,          0},
	{"seh",   0x9458,          0},
	{"clh",   0x94d8,          0},
	{"sleep", 0x9588,          0},
	{"wdr",   0x95a8,          0},
	{"ijmp",  0x9409,  DF_TINY1X},
	{"eijmp", 0x9419, DF_NO_EIJMP},
	{"icall", 0x9509,  DF_TINY1X},
	{"eicall",0x9519, DF_NO_EICALL},
	{"ret",   0x9508,          0},
	{"reti",  0x9518,          0},
	{"spm",   0x95e8, DF_NO_SPM},
	{"espm",  0x95f8, DF_NO_ESPM},
	{"break", 0x9598, DF_NO_BREAK},
	{"lpm",   0x95c8, DF_NO_LPM},
	{"elpm",  0x95d8, DF_NO_ELPM},
	{"bset",  0x9408,          0},
	{"bclr",  0x9488,          0},
	{"ser",   0xef0f,          0},
	{"com",   0x9400,          0},
	{"neg",   0x9401,          0},
	{"inc",   0x9403,          0},
	{"dec",   0x940a,          0},
	{"lsr",   0x9406,          0},
	{"ror",   0x9407,          0},
	{"asr",   0x9405,          0},
	{"swap",  0x9402,          0},
	{"push",  0x920f,  DF_TINY1X},
	{"pop",   0x900f,  DF_TINY1X},
	{"tst",   0x2000,          0},
	{"clr",   0x2400,          0},
	{"lsl",   0x0c00,          0},
	{"rol",   0x1c00,          0},
	{"breq",  0xf001,          0},
	{"brne",  0xf401,          0},
	{"brcs",  0xf000,          0},
	{"brcc",  0xf400,          0},
	{"brsh",  0xf400,          0},
	{"brlo",  0xf000,          0},
	{"brmi",  0xf002,          0},
	{"brpl",  0xf402,          0},
	{"brge",  0xf404,          0},
	{"brlt",  0xf004,          0},
	{"brhs",  0xf005,          0},
	{"brhc",  0xf405,          0},
	{"brts",  0xf006,          0},
	{"brtc",  0xf406,          0},
	{"brvs",  0xf003,          0},
	{"brvc",  0xf403,          0},
	{"brie",  0xf007,          0},
	{"brid",  0xf407,          0},
	{"rjmp",  0xc000,          0},
	{"rcall", 0xd000,          0},
	{"jmp",   0x940c,  DF_NO_JMP},
	{"call",  0x940e,  DF_NO_JMP},
	{"brbs",  0xf000,          0},
	{"brbc",  0xf400,          0},
	{"add",   0x0c00,          0},
	{"adc",   0x1c00,          0},
	{"sub",   0x1800,          0},
	{"sbc",   0x0800,          0},
	{"and",   0x2000,          0},
	{"or",    0x2800,          0},
	{"eor",   0x2400,          0},
	{"cp",    0x1400,          0},
	{"cpc",   0x0400,          0},
	{"cpse",  0x1000,          0},
	{"mov",   0x2c00,          0},
	{"mul",   0x9c00, DF_NO_MUL},
	{"movw",  0x0100, DF_NO_MOVW},
	{"muls",  0x0200, DF_NO_MUL},
	{"mulsu", 0x0300, DF_NO_MUL},
	{"fmul",  0x0308, DF_NO_MUL},
	{"fmuls", 0x0380, DF_NO_MUL},
	{"fmulsu",0x0388, DF_NO_MUL},
	{"adiw",  0x9600,  DF_TINY1X},
	{"sbiw",  0x9700,  DF_TINY1X},
	{"subi",  0x5000,          0},
	{"sbci",  0x4000,          0},
	{"andi",  0x7000,          0},
	{"ori",   0x6000,          0},
	{"sbr",   0x6000,          0},
	{"cpi",   0x3000,          0},
	{"ldi",   0xe000,          0},
	{"cbr",   0x7000,          0},
	{"sbrc",  0xfc00,          0},
	{"sbrs",  0xfe00,          0},
	{"bst",   0xfa00,          0},
	{"bld",   0xf800,          0},
	{"in",    0xb000,          0},
	{"out",   0xb800,          0},
	{"sbic",  0x9900,          0},
	{"sbis",  0x9b00,          0},
	{"sbi",   0x9a00,          0},
	{"cbi",   0x9800,          0},
	{"lds",   0x9000,  DF_TINY1X},
	{"sts",   0x9200,  DF_TINY1X},
	{"ld",    0,          0},
	{"st",    0,          0},
	{"ldd",   0,  DF_TINY1X},
	{"std",   0,  DF_TINY1X},
	{"count", 0,          0},
	{"lpm",   0x9004, DF_NO_LPM|DF_NO_LPM_X},
	{"lpm",   0x9005, DF_NO_LPM|DF_NO_LPM_X},
	{"elpm",  0x9006, DF_NO_ELPM|DF_NO_ELPM_X},
	{"elpm",  0x9007, DF_NO_ELPM|DF_NO_ELPM_X},
	{"ld",    0x900c, DF_NO_XREG},
	{"ld",    0x900d, DF_NO_XREG},
	{"ld",    0x900e, DF_NO_XREG},
	{"ld",    0x8008, DF_NO_YREG},
	{"ld",    0x9009, DF_NO_YREG},
	{"ld",    0x900a, DF_NO_YREG},
	{"ld",    0x8000,          0},
	{"ld",    0x9001, DF_TINY1X},
	{"ld",    0x9002, DF_TINY1X},
	{"st",    0x920c, DF_NO_XREG},
	{"st",    0x920d, DF_NO_XREG},
	{"st",    0x920e, DF_NO_XREG},
	{"st",    0x8208, DF_NO_YREG},
	{"st",    0x9209, DF_NO_YREG},
	{"st",    0x920a, DF_NO_YREG},
	{"st",    0x8200,          0},
	{"st",    0x9201, DF_TINY1X},
	{"st",    0x9202, DF_TINY1X},
	{"ldd",   0x8008, DF_TINY1X},
	{"ldd",   0x8000, DF_TINY1X},
	{"std",   0x8208, DF_TINY1X},
	{"std",   0x8200, DF_TINY1X},
	{"end", 0, 0}
	};


/* We try to parse the command name. Is it a assembler mnemonic or anything else ?
 * If so, it may be a macro.
*/

int parse_mnemonic(struct prog_info *pi) 
{
  int mnemonic;
  int i;
  int opcode = 0;
  int opcode2 = 0;
  int instruction_long = False;
  char *operand1;
  char *operand2;
  struct macro *macro;
  char temp[MAX_MNEMONIC_LEN + 1];

  operand1 = get_next_token(pi->fi->scratch, TERM_SPACE);  // we get the first word on line
  mnemonic = get_mnemonic_type(my_strlwr(pi->fi->scratch));
  if(mnemonic == -1) {				// if -1 this must be a macro name
	macro = get_macro(pi, pi->fi->scratch); // and so, we try to get the corresponding macro struct.
	if(macro) {
		return(expand_macro(pi, macro, operand1)); // we expand the macro
	} else { 				// if we cant find a name, this is a unknown word.
		print_msg(pi, MSGTYPE_ERROR, "Unknown mnemonic/macro: %s", pi->fi->scratch);
		return(True);
	}
  }
  if(pi->pass == PASS_2) {
	if(mnemonic <= MNEMONIC_BREAK) {
		if(operand1) {
			print_msg(pi, MSGTYPE_WARNING, "Garbage after instruction %s: %s", instruction_list[mnemonic].mnemonic, operand1);		}
		opcode = 0;			// No operand
	} else if(mnemonic <= MNEMONIC_ELPM) {
		if(operand1) {
			operand2 = get_next_token(operand1, TERM_COMMA);
			if(!operand2) {
				print_msg(pi, MSGTYPE_ERROR, "%s needs a second operand", instruction_list[mnemonic].mnemonic);
				return(True);			}
			get_next_token(operand2, TERM_END);
			i = get_register(pi, operand1);
			opcode = i << 4;
			i = get_indirect(pi, operand2);
			if(i == 6) { // Means Z
				if(mnemonic == MNEMONIC_LPM)
					mnemonic = MNEMONIC_LPM_Z;
				else if(mnemonic == MNEMONIC_ELPM)
					mnemonic = MNEMONIC_ELPM_Z;
			} else if(i == 7) { // Means Z+
				if(mnemonic == MNEMONIC_LPM)
					mnemonic = MNEMONIC_LPM_ZP;
				else if(mnemonic == MNEMONIC_ELPM)
					mnemonic = MNEMONIC_ELPM_ZP;
			} else {
				print_msg(pi, MSGTYPE_ERROR, "Unsupported operand: %s", operand2);
				return(True);
			}
		} else
			opcode = 0;
	} else {
		if(!operand1) {
			print_msg(pi, MSGTYPE_ERROR, "%s needs an operand", instruction_list[mnemonic].mnemonic);
			return(True);
		}
		operand2 = get_next_token(operand1, TERM_COMMA);
		if(mnemonic >= MNEMONIC_BRBS) {
			if(!operand2) {
				print_msg(pi, MSGTYPE_ERROR, "%s needs a second operand", instruction_list[mnemonic].mnemonic);
				return(True);
			}
			get_next_token(operand2, TERM_END);
		}
		if(mnemonic <= MNEMONIC_BCLR) {
			if(!get_bitnum(pi, operand1, &i))
				return(False);
			opcode = i << 4;
		} else if(mnemonic <= MNEMONIC_ROL) {
			i = get_register(pi, operand1);
			if((mnemonic == MNEMONIC_SER) && (i < 16)) {
				print_msg(pi, MSGTYPE_ERROR, "%s can only use a high register (r16 - r31)", instruction_list[mnemonic].mnemonic);
				i &= 0x0f;
			}
			opcode = i << 4;
			if(mnemonic >= MNEMONIC_TST)
				opcode |= ((i & 0x10) << 5) | (i & 0x0f);
		} else if(mnemonic <= MNEMONIC_RCALL) {
			if(!get_expr(pi, operand1, &i))
				return(False);
			i -= pi->cseg_addr + 1;
			if(mnemonic <= MNEMONIC_BRID) {
				if((i < -64) || (i > 63))
					print_msg(pi, MSGTYPE_ERROR, "Branch out of range (-64 <= k <= 63)");
				opcode = (i & 0x7f) << 3;
			} else {
				if(((i < -2048) || (i > 2047)) && (pi->device->flash_size != 4096))
					print_msg(pi, MSGTYPE_ERROR, "Relative address out of range (-2048 <= k <= 2047)");
				opcode = i & 0x0fff;
			}
		} else if(mnemonic <= MNEMONIC_CALL) {
			if(!get_expr(pi, operand1, &i))
				return(False);
			if((i < 0) || (i > 4194303))
				print_msg(pi, MSGTYPE_ERROR, "Address out of range (0 <= k <= 4194303)");
			opcode = ((i & 0x3e0000) >> 13) | ((i & 0x010000) >> 16);
			opcode2 = i & 0xffff;
			instruction_long = True;
		} else if(mnemonic <= MNEMONIC_BRBC) {
			if(!get_bitnum(pi, operand1, &i))
				return(False);
			opcode = i;
			if(!get_expr(pi, operand2, &i))
				return(False);
			i -= pi->cseg_addr + 1;
			if((i < -64) || (i > 63))
				print_msg(pi, MSGTYPE_ERROR, "Branch out of range (-64 <= k <= 63)");
			opcode |= (i & 0x7f) << 3;
		} else if(mnemonic <= MNEMONIC_MUL) {
			i = get_register(pi, operand1);
			opcode = i << 4;
			i = get_register(pi, operand2);
			opcode |= ((i & 0x10) << 5) | (i & 0x0f);
		} else if(mnemonic <= MNEMONIC_MOVW) {
			i = get_register(pi, operand1);
			if((i % 2) == 1)
				print_msg(pi, MSGTYPE_ERROR, "%s must use a even numbered register for Rd", instruction_list[mnemonic].mnemonic);
			opcode = (i / 2) << 4;
			i = get_register(pi, operand2);
			if((i % 2) == 1)
				print_msg(pi, MSGTYPE_ERROR, "%s must use a even numbered register for Rr", instruction_list[mnemonic].mnemonic);
			opcode |= i / 2;
		} else if(mnemonic <= MNEMONIC_MULS) {
			i = get_register(pi, operand1);
			if(i < 16)
				print_msg(pi, MSGTYPE_ERROR, "%s can only use a high register (r16 - r31)", instruction_list[mnemonic].mnemonic);
			opcode = (i & 0x0f) << 4;
			i = get_register(pi, operand2);
			if(i < 16)
				print_msg(pi, MSGTYPE_ERROR, "%s can only use a high register (r16 - r31)", instruction_list[mnemonic].mnemonic);
			opcode |= (i & 0x0f);
		} else if(mnemonic <= MNEMONIC_FMULSU) {
			i = get_register(pi, operand1);
			if((i < 16) || (i >= 24))
				print_msg(pi, MSGTYPE_ERROR, "%s can only use registers (r16 - r23)", instruction_list[mnemonic].mnemonic);
			opcode = (i & 0x07) << 4;
			i = get_register(pi, operand2);
			if((i < 16) || (i >= 24))
				print_msg(pi, MSGTYPE_ERROR, "%s can only use registers (r16 - r23)", instruction_list[mnemonic].mnemonic);
			opcode |= (i & 0x07);
		} else if(mnemonic <= MNEMONIC_SBIW) {
			i = get_register(pi, operand1);
			if(!((i == 24) || (i == 26) || (i == 28) || (i == 30)))
				print_msg(pi, MSGTYPE_ERROR, "%s can only use registers R24, R26, R28 or R30", instruction_list[mnemonic].mnemonic);
			opcode = ((i - 24) / 2) << 4;
			if(!get_expr(pi, operand2, &i))
			return(False);
			if((i < 0) || (i > 63))
				print_msg(pi, MSGTYPE_ERROR, "Constant out of range (0 <= k <= 63)");
			opcode |= ((i & 0x30) << 2) | (i & 0x0f);
		} else if(mnemonic <= MNEMONIC_CBR) {
			i = get_register(pi, operand1);
			if(i < 16)
				print_msg(pi, MSGTYPE_ERROR, "%s can only use a high register (r16 - r31)", instruction_list[mnemonic].mnemonic);
			opcode = (i & 0x0f) << 4;
			if(!get_expr(pi, operand2, &i))
				return(False);
			if((i < -128) || (i > 255))
				print_msg(pi, MSGTYPE_WARNING, "Constant out of range (-128 <= k <= 255). Will be masked");
			if(mnemonic == MNEMONIC_CBR)
				i = ~i;
			opcode |= ((i & 0xf0) << 4) | (i & 0x0f);
		} else if(mnemonic <= MNEMONIC_BLD) {
			i = get_register(pi, operand1);
			opcode = i << 4;
			if(!get_bitnum(pi, operand2, &i))
				return(False);
			opcode |= i;
		} else if(mnemonic == MNEMONIC_IN) {
			i = get_register(pi, operand1);
			opcode = i << 4;
			if(!get_expr(pi, operand2, &i))
				return(False);
			if((i < 0) || (i > 63))
				print_msg(pi, MSGTYPE_ERROR, "I/O out of range (0 <= P <= 63)");
			opcode |= ((i & 0x30) << 5) | (i & 0x0f);
		} else if(mnemonic == MNEMONIC_OUT) {
			if(!get_expr(pi, operand1, &i))
				return(False);
			if((i < 0) || (i > 63))
				print_msg(pi, MSGTYPE_ERROR, "I/O out of range (0 <= P <= 63)");
			opcode = ((i & 0x30) << 5) | (i & 0x0f);
			i = get_register(pi, operand2);
			opcode |= i << 4;
		} else if(mnemonic <= MNEMONIC_CBI) {
			if(!get_expr(pi, operand1, &i))
				return(False);
			if((i < 0) || (i > 31))
				print_msg(pi, MSGTYPE_ERROR, "I/O out of range (0 <= P <= 31)");
			opcode = i << 3;
			if(!get_bitnum(pi, operand2, &i))
				return(False);
			opcode |= i;
		} else if(mnemonic == MNEMONIC_LDS) {
			i = get_register(pi, operand1);
			opcode = i << 4;
			if(!get_expr(pi, operand2, &i))
				return(False);
			if((i < 0) || (i > 65535))
				print_msg(pi, MSGTYPE_ERROR, "SRAM out of range (0 <= k <= 65535)");
			opcode2 = i;
			instruction_long = True;
		} else if(mnemonic == MNEMONIC_STS) {
			if(!get_expr(pi, operand1, &i))
				return(False);
			if((i < 0) || (i > 65535))
				print_msg(pi, MSGTYPE_ERROR, "SRAM out of range (0 <= k <= 65535)");
			opcode2 = i;
			i = get_register(pi, operand2);
			opcode = i << 4;
			instruction_long = True;
		} else if(mnemonic == MNEMONIC_LD) {
			i = get_register(pi, operand1);
			opcode = i << 4;
			mnemonic = MNEMONIC_LD_X + get_indirect(pi, operand2);
		} else if(mnemonic == MNEMONIC_ST) {
			mnemonic = MNEMONIC_ST_X + get_indirect(pi, operand1);
			i = get_register(pi, operand2);
			opcode = i << 4;
		} else if(mnemonic == MNEMONIC_LDD) {
			i = get_register(pi, operand1);
			opcode = i << 4;
			if(tolower(operand2[0]) == 'z')
				mnemonic = MNEMONIC_LDD_Z;
			else if(tolower(operand2[0]) == 'y')
					mnemonic = MNEMONIC_LDD_Y;
				else
					print_msg(pi, MSGTYPE_ERROR, "Garbage in second operand (%s)", operand2);
			i = 1;
			while((operand2[i] != '\0') && (operand2[i] != '+')) i++;
			if(operand2[i] == '\0')	{
				print_msg(pi, MSGTYPE_ERROR, "Garbage in second operand (%s)", operand2);
				return(False);
			}
			if(!get_expr(pi, &operand2[i + 1], &i))
				return(False);
			if((i < 0) || (i > 63))
				print_msg(pi, MSGTYPE_ERROR, "Displacement out of range (0 <= q <= 63)");
			opcode |= ((i & 0x20) << 8) | ((i & 0x18) << 7) | (i & 0x07);
		} else if(mnemonic == MNEMONIC_STD) {
			if(tolower(operand1[0]) == 'z')
				mnemonic = MNEMONIC_STD_Z;
			else if(tolower(operand1[0]) == 'y')
					mnemonic = MNEMONIC_STD_Y;
				else
					print_msg(pi, MSGTYPE_ERROR, "Garbage in first operand (%s)", operand1);
			i = 1;
			while((operand1[i] != '\0') && (operand1[i] != '+')) i++;
			if(operand1[i] == '\0')	{
				print_msg(pi, MSGTYPE_ERROR, "Garbage in first operand (%s)", operand1);
				return(False);
			}
			if(!get_expr(pi, &operand1[i + 1], &i))
				return(False);
			if((i < 0) || (i > 63))
				print_msg(pi, MSGTYPE_ERROR, "Displacement out of range (0 <= q <= 63)");
			opcode = ((i & 0x20) << 8) | ((i & 0x18) << 7) | (i & 0x07);
			i = get_register(pi, operand2);
			opcode |= i << 4;
		} else
			print_msg(pi, MSGTYPE_ERROR, "Shit! Missing opcode check [%d]...", mnemonic);
	}
	if (pi->device->flag & instruction_list[mnemonic].flag)	{
		strncpy(temp, instruction_list[mnemonic].mnemonic, MAX_MNEMONIC_LEN);
   		print_msg(pi, MSGTYPE_ERROR, "%s instruction is not supported on %s",
   		          my_strupr(temp), pi->device->name);
	}
	opcode |= instruction_list[mnemonic].opcode;
	if(pi->list_on && pi->list_line) {
		if(instruction_long)
			fprintf(pi->list_file, "C:%06x %04x %04x %s\n", pi->cseg_addr, opcode, opcode2, pi->list_line);
		else
			fprintf(pi->list_file, "C:%06x %04x      %s\n", pi->cseg_addr, opcode, pi->list_line);
		pi->list_line = NULL;
	}
	if(pi->hfi) {
		write_prog_word(pi, pi->cseg_addr, opcode);
		if(instruction_long)
			write_prog_word(pi, pi->cseg_addr + 1, opcode2);
	}
	if(instruction_long)
		pi->cseg_addr += 2;
	else
		pi->cseg_addr++;
  } else { // Pass 1
	if((mnemonic == MNEMONIC_JMP) || (mnemonic == MNEMONIC_CALL) 
        	|| (mnemonic == MNEMONIC_LDS) || (mnemonic == MNEMONIC_STS)) {
		pi->cseg_addr += 2;
		pi->cseg_count += 2;
	} else {
		pi->cseg_addr++;
		pi->cseg_count++;
	}
  }
  return(True);
}


int get_mnemonic_type(char *mnemonic)
{
	int i;

	for(i = 0; i < MNEMONIC_COUNT; i++) {
		if(!strcmp(mnemonic, instruction_list[i].mnemonic)) {
			return(i);
		}
	}
	return(-1);
}


int get_register(struct prog_info *pi, char *data)
{
	char *second_reg;
	int reg = 0;
	struct def *def;

	// Check for any occurence of r1:r0 pairs, and if so skip to second register
	second_reg = strchr(data, ':');
	if(second_reg != NULL)
		data = second_reg + 1;

	for(def = pi->first_def; def; def = def->next)
		if(!nocase_strcmp(def->name, data))
			{
			reg = def->reg;
			return(reg);
			}
	if((tolower(data[0]) == 'r') && isdigit(data[1])) {
		reg = atoi(&data[1]);
		if(reg > 31)
			print_msg(pi, MSGTYPE_ERROR, "R%d is not a valid register", reg);
	}
	else
		print_msg(pi, MSGTYPE_ERROR, "No register associated with %s", data);
	return(reg);
}


int get_bitnum(struct prog_info *pi, char *data, int *ret)
{
	if(!get_expr(pi, data, ret))
		return(False);
	if((*ret < 0) || (*ret > 7)) {
		print_msg(pi, MSGTYPE_ERROR, "Operand out of range (0 <= s <= 7)");
		return(False);
	}
	return(True);
}


int get_indirect(struct prog_info *pi, char *operand)
{
	int i = 1;

	switch(tolower(operand[0])) {
		case '-':
			while(IS_HOR_SPACE(operand[i])) i++;
			if(operand[i + 1] != '\0')
				print_msg(pi, MSGTYPE_ERROR, "Garbage in operand (%s)", operand);
			switch(tolower(operand[i])) {
				case 'x':
        	   	                if (pi->device->flag & DF_NO_XREG)
                                           print_msg(pi, MSGTYPE_ERROR, "X register is not supported on %s", pi->device->name);
					return(2);
				case 'y':
        	   	                if (pi->device->flag & DF_NO_YREG)
                                           print_msg(pi, MSGTYPE_ERROR, "Y register is not supported on %s", pi->device->name);
					return(5);
				case 'z':
					return(8);
				default:
					print_msg(pi, MSGTYPE_ERROR, "Garbage in operand (%s)", operand);
					return(0);
			}
		case 'x':
                        if (pi->device->flag & DF_NO_XREG)
                           print_msg(pi, MSGTYPE_ERROR, "X register is not supported on %s", pi->device->name);
			while(IS_HOR_SPACE(operand[i])) i++;
			if(operand[i] == '+') {
				if(operand[i + 1] != '\0')
					print_msg(pi, MSGTYPE_ERROR, "Garbage in operand (%s)", operand);
				return(1);
			}
			else if(operand[i] == '\0')
				return(0);
			else
				print_msg(pi, MSGTYPE_ERROR, "Garbage after operand (%s)", operand);
			return(0);
		case 'y':
                        if (pi->device->flag & DF_NO_YREG)
                           print_msg(pi, MSGTYPE_ERROR, "Y register is not supported on %s", pi->device->name);
			while(IS_HOR_SPACE(operand[i])) i++;
			if(operand[i] == '+') {
				if(operand[i + 1] != '\0')
					print_msg(pi, MSGTYPE_ERROR, "Garbage in operand (%s)", operand);
				return(4);
			}
			else if(operand[i] == '\0')
				return(3);
			else
				print_msg(pi, MSGTYPE_ERROR, "Garbage after operand (%s)", operand);
			return(0);
		case 'z':
			while(IS_HOR_SPACE(operand[i])) i++;
			if(operand[i] == '+') {
				if(operand[i + 1] != '\0')
					print_msg(pi, MSGTYPE_ERROR, "Garbage in operand (%s)", operand);
				return(7);
			}
			else if(operand[i] == '\0')
				return(6);
			else
				print_msg(pi, MSGTYPE_ERROR, "Garbage after operand (%s)", operand);
			return(0);
		default:
			print_msg(pi, MSGTYPE_ERROR, "Garbage in operand (%s)", operand);
	}
	return(0);
}

/* Return 1 if instruction name is supported by the current device,
   0 if unsupported, -1 if it is invalid */
int is_supported(struct prog_info *pi, char *name) {
   char temp[MAX_MNEMONIC_LEN+1];
   int mnemonic;

   strncpy(temp,name,MAX_MNEMONIC_LEN);
   mnemonic = get_mnemonic_type(my_strlwr(temp));
   if (mnemonic == -1) return -1;
   if (pi->device->flag & instruction_list[mnemonic].flag) return 0;
   return 1;
}

int count_supported_instructions(int flags)
{
  int i = 0, count = 0;
  while(i < MNEMONIC_END) {
    if((i < MNEMONIC_LD) || (i > MNEMONIC_COUNT))
      if(!(flags & instruction_list[i].flag))
        count++;
    i++;
  }
  return(count);
}

/* end of mnemonic.c */

