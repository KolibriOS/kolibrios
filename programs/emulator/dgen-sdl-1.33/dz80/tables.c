/*
	Tables used by dZ80
*/

#include <stdio.h>
#include <stdlib.h>

#include "dissz80p.h"

char    *VersionString  = "2.0";                         /* Disassembler version */

char	*IMModes[4] 	= {"0", "0", "1", "2"};

char *Conditions[8]=
{
	"nz",
	"z",
	"nc",
	"c",
	"po",
	"pe",
	"p",
	"m"
};

char	*AccRotType[8]=
{
	"rlca",
	"rrca",
	"rla",
	"rra",
	"daa",
	"cpl",
	"scf",
	"ccf"
};

char	*CBRotType[8] =
{
	"rlc",
	"rrc",
	"rl",
	"rr",
	"sla",
	"sra",
	"sll",
	"srl"
};

char	*Reg8Idx[8] =
{
	"b",
	"c",
	"d",
	"e",
	"h",
	"l",
	"(hl)",
	"a"    
};

char	*Reg8AFIdx[8] =
{
	"b",
	"c",
	"d",
	"e",
	"h",
	"l",
	"f",
	"a"    
};


char	*Reg16Idx[5] =
{
	"bc",
	"de",
	"hl",
	"sp",
	"af"
};

char	*BasicOps[8] =
{
	"add",
	"adc",
	"sub",
	"sbc",
	"and",
	"xor",
	"or",
	"cp"
};

char	*RepeatOps[16] =
{
	"ldi",
	"cpi",
	"ini",
	"outi",

	"ldd",
	"cpd",
	"ind",
	"outd",

	"ldir",
	"cpir",
	"inir",
	"otir",

	"lddr",
	"cpdr",
	"indr",
	"otdr"
};


char *Z180RepeatOps[4] =
{
	"otim",
	"otdm",
	"otimr",
	"otdmr"
};


char *dZ80CpuTypeNames[DCPU_TOTAL] =
{
	"Z80GB",
	"Z80",
	"Z180"
};

char *dZ80ErrorMsgs[DERR_TOTAL] =
{
	"No Error",
	"Out of Memory",
	"Couldn't create file",
	"Couldn't write to file",
	"Bad opcode length",
	"Invalid parameters",
	"Script error",
	"Wrong argument type",
	"File not found",
	"Scripting not available"
};

