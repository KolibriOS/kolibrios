/* Multi-Z80 32 Bit emulator */

/* Copyright 1996, 1997, 1998, 1999, 2000 Neil Bradley, All rights reserved
 *
 * License agreement:
 *
 * (MZ80 Refers to both the assembly and C code emitted by makeZ80.c and 
 *	 makeZ80.c itself)
 *
 * MZ80 May be distributed in unmodified form to any medium.
 *
 * MZ80 May not be sold, or sold as a part of a commercial package without
 * the express written permission of Neil Bradley (neil@synthcom.com). This
 * includes shareware.
 *
 * Modified versions of MZ80 may not be publicly redistributed without author
 * approval (neil@synthcom.com). This includes distributing via a publicly
 * accessible LAN. You may make your own source modifications and distribute
 * MZ80 in source or object form, but if you make modifications to MZ80
 * then it should be noted in the top as a comment in makeZ80.c.
 *
 * MZ80 Licensing for commercial applications is available. Please email
 * neil@synthcom.com for details.
 *
 * Synthcom Systems, Inc, and Neil Bradley will not be held responsible for
 * any damage done by the use of MZ80. It is purely "as-is".
 *
 * If you use MZ80 in a freeware application, credit in the following text:
 *
 * "Multi-Z80 CPU emulator by Neil Bradley (neil@synthcom.com)"
 *
 * must accompany the freeware application within the application itself or
 * in the documentation.
 *
 * Legal stuff aside:
 *
 * If you find problems with MZ80, please email the author so they can get
 * resolved. If you find a bug and fix it, please also email the author so
 * that those bug fixes can be propogated to the installed base of MZ80
 * users. If you find performance improvements or problems with MZ80, please
 * email the author with your changes/suggestions and they will be rolled in
 * with subsequent releases of MZ80.
 *
 * The whole idea of this emulator is to have the fastest available 32 bit
 * Multi-Z80 emulator for the PC, giving maximum performance. 
 *
 */ 

/*
  DGen/SDL modifications for compilation issues and bugfixes:

  2011-08-28 - Rename VERSION to MZ80_VERSION.
             - Some fprintf() calls had too many arguments.
             - Use C99 uint*_t/int*_t for portability.
  2011-09-11 - Replace assert(0) occurences with abort() as these checks
               shouldn't go away when defining NDEBUG.
             - Add default case to switch statement in IRHandler().
             - Append -dgen to version number.
  2011-10-08 - Fix segfault on reset in the C version.
  2014-06-22 - Remove malloc() calls from mz80init() as they cannot be
               checked nor freed.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <inttypes.h>

#define	MZ80_VERSION "3.4-dgen"

#define TRUE            		0xff
#define FALSE           		0x0
#define INVALID					0xff

#define UINT32 uint32_t
#define UINT8 uint8_t
#define INT8 char

#define	TIMING_REGULAR			0x00
#define	TIMING_XXCB				0x01
#define	TIMING_CB				0xcb
#define	TIMING_DDFD				0xdd
#define	TIMING_ED				0xed
#define	TIMING_EXCEPT			0x02

FILE *fp = NULL;
char string[150];
char cpubasename[150];
static char mz80Index[50];
static char mz80IndexHalfHigh[50];
static char mz80IndexHalfLow[50];
char majorOp[50];
char procname[150];
UINT32 dwGlobalLabel = 0;

enum
{
	MZ80_ASSEMBLY_X86,
	MZ80_C,
	MZ80_UNKNOWN
};

UINT8 bPlain = FALSE;
UINT8 bNoTiming = FALSE;
UINT8 bUseStack = 0;
UINT8 bCurrentMode = TIMING_REGULAR;	// Current timing mode
UINT8 b16BitIo = FALSE;
UINT8 bThroughCallHandler = FALSE;
UINT8 bOS2 = FALSE;
UINT8 bWhat = MZ80_UNKNOWN;

void ProcBegin(UINT32 dwOpcode);

INT8 *pbLocalReg[8] =
{
	"ch",
	"cl",
	"dh",
	"dl",
	"bh",
	"bl",
	"dl",
	"al"
};

INT8 *pbLocalRegC[8] =
{
	"cpu.z80B",
	"cpu.z80C",
	"cpu.z80D",
	"cpu.z80E",
	"cpu.z80H",
	"cpu.z80L",
	"barf",
	"cpu.z80A"
};

INT8 *pbPushReg[8] = 
{
	"cl",
	"ch",
	"byte [_z80de]",
	"byte [_z80de + 1]",
	"bl",
	"bh",
	"ah",
	"al"
};

INT8 *pbFlags[8] =
{
	"nz",
	"z",
	"nc",
	"c",
	"po",
	"pe",
	"ns",
	"s"
};

INT8 *pbRegPairC[] =
{
	"cpu.z80BC",
	"cpu.z80DE",
	"cpu.z80HL",
	"cpu.z80sp"
};

INT8 *pbFlagsC[8] =
{
	"(!(cpu.z80F & Z80_FLAG_ZERO))",
	"(cpu.z80F & Z80_FLAG_ZERO)",
	"(!(cpu.z80F & Z80_FLAG_CARRY))",
	"(cpu.z80F & Z80_FLAG_CARRY)",
	"(!(cpu.z80F & Z80_FLAG_OVERFLOW_PARITY))",
	"(cpu.z80F & Z80_FLAG_OVERFLOW_PARITY)",
	"(!(cpu.z80F & Z80_FLAG_SIGN))",
	"(cpu.z80F & Z80_FLAG_SIGN)"
};

INT8 *pbMathReg[8] =
{
	"ch",
	"cl",
	"byte [_z80de + 1]",
	"byte [_z80de]",
	"bh",
	"bl",
	"INVALID",
	"al"
};

INT8 *pbMathRegC[8] =
{
	"cpu.z80B",
	"cpu.z80C",
	"cpu.z80D",
	"cpu.z80E",
	"cpu.z80H",
	"cpu.z80L",
	"bTemp",
	"cpu.z80A"
};

INT8 *pbRegPairs[4] = 
{
	"cx",	// BC
	"word [_z80de]", // DE
	"bx",	// HL
	"word [_z80sp]"  // SP
};

INT8 *pbRegPairsC[4] = 
{
	"cpu.z80BC",	// BC
	"cpu.z80DE", // DE
	"cpu.z80HL",	// HL
	"cpu.z80sp"  // SP
};

INT8 *pbPopRegPairs[4] = 
{
	"cx",	// BC
	"word [_z80de]", // DE
	"bx",	// HL
	"ax"  // SP
};

INT8 *pbPopRegPairC[4] = 
{
	"cpu.z80BC",
	"cpu.z80DE",
	"cpu.z80HL",
	"cpu.z80AF"
};

INT8 *pbIndexedRegPairs[4] = 
{
	"cx",	// BC
	"word [_z80de]", // DE
	"di",	// IX/IY
	"word [_z80sp]"  // SP
};

// Timing tables

UINT8 bTimingRegular[0x100] =
{
	0x04, 0x0a, 0x07, 0x06, 0x04, 0x04, 0x07, 0x04, 0x04, 0x0b, 0x07, 0x06, 0x04, 0x04, 0x07, 0x04,
	0x08, 0x0a, 0x07, 0x06, 0x04, 0x04, 0x07, 0x04, 0x0c, 0x0b, 0x07, 0x06, 0x04, 0x04, 0x07, 0x04,
	0x07, 0x0a, 0x10, 0x06, 0x04, 0x04, 0x07, 0x04, 0x07, 0x0b, 0x10, 0x06, 0x04, 0x04, 0x07, 0x04,
	0x07, 0x0a, 0x0d, 0x06, 0x0b, 0x0b, 0x0a, 0x04, 0x07, 0x0b, 0x0d, 0x06, 0x04, 0x04, 0x07, 0x04,

	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04,
	0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x04, 0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04,

	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04,
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x07, 0x04,

	0x05, 0x0a, 0x0a, 0x0a, 0x0a, 0x0b, 0x07, 0x0b, 0x05, 0x0a, 0x0a, 0x00, 0x0a, 0x11, 0x07, 0x0b,
	0x05, 0x0a, 0x0a, 0x0b, 0x0a, 0x0b, 0x07, 0x0b, 0x05, 0x04, 0x0a, 0x0b, 0x0a, 0x00, 0x07, 0x0b,
	0x05, 0x0a, 0x0a, 0x13, 0x0a, 0x0b, 0x07, 0x0b, 0x05, 0x04, 0x0a, 0x04, 0x0a, 0x00, 0x07, 0x0b,
	0x05, 0x0a, 0x0a, 0x04, 0x0a, 0x0b, 0x07, 0x0b, 0x05, 0x06, 0x0a, 0x04, 0x0a, 0x00, 0x07, 0x0b
};

UINT8 bTimingCB[0x100] =
{
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 

	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0c, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0c, 0x08, 
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0c, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0c, 0x08, 
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0c, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0c, 0x08, 
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0c, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0c, 0x08, 
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0f, 0x08
};

UINT8 bTimingXXCB[0x100] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 
	0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14,
	0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14,
	0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14,
	0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00
};

UINT8 bTimingDDFD[0x100] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x0e, 0x14, 0x0a, 0x09, 0x09, 0x09, 0x00, 0x00, 0x0f, 0x14, 0x0a, 0x09, 0x09, 0x09, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x17, 0x17, 0x13, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 

	0x00, 0x00, 0x00, 0x00, 0x09, 0x09, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x09, 0x13, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x09, 0x09, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x09, 0x13, 0x00, 

	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x13, 0x09,	0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x13, 0x09,
	0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,	0x00, 0x00, 0x00, 0x00, 0x09, 0x09, 0x13, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x09, 0x09, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x09, 0x13, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x09, 0x09, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x09, 0x13, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x09, 0x09, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x09, 0x13, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x09, 0x09, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x09, 0x13, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x0e, 0x00, 0x17, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

UINT8 bTimingED[0x100] = 
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 

	0x0c, 0x0c, 0x0f, 0x14, 0x08, 0x0e, 0x08, 0x09, 0x0c, 0x0c, 0x0f, 0x14, 0x08, 0x0e, 0x08, 0x09,
	0x0c, 0x0c, 0x0f, 0x14, 0x08, 0x08, 0x08, 0x09, 0x0c, 0x0c, 0x0f, 0x14, 0x08, 0x08, 0x08, 0x09,
	0x0c, 0x0c, 0x0f, 0x14, 0x08, 0x08, 0x08, 0x12, 0x0c, 0x0c, 0x0f, 0x14, 0x08, 0x08, 0x08, 0x12,
	0x0c, 0x0c, 0x0f, 0x14, 0x08, 0x08, 0x08, 0x00, 0x0c, 0x0c, 0x0f, 0x14, 0x08, 0x08, 0x08, 0x00,

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 
	0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x10, 0x10, 0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
};

void EDHandler(UINT32 dwOpcode);
void DDHandler(UINT32 dwOpcode);
void FDHandler(UINT32 dwOpcode);
void CBHandler(UINT32 dwOpcode);

void PushPopOperations(UINT32 dwOpcode);
void AddRegpairOperations(UINT32 dwOpcode);
void CallHandler(UINT32 dwOpcode);
void MiscHandler(UINT32 dwOpcode);
void IMHandler(UINT32 dwOpcode);
void IRHandler(UINT32 dwOpcode);
void LdRegPairImmediate(UINT32 dwOpcode);
void LoadImmediate(UINT32 dwOpcode);
void LdRegpairPtrByte(UINT32 dwOpcode);
void MathOperation(UINT32 dwOpcode);
void RegIntoMemory(UINT32 dwOpcode);
void JpHandler(UINT32 dwOpcode);
void LdRegImmediate(UINT32 dwOpcode);
void IncRegister(UINT32 dwOpcode);
void DecRegister(UINT32 dwOpcode);
void IncDecRegpair(UINT32 dwOpcode);
void LdRegReg(UINT32 dwOpcode);
void MathOperationDirect(UINT32 dwOpcode);
void JrHandler(UINT32 dwOpcode);
void RetHandler(UINT32 dwOpcode);
void RestartHandler(UINT32 dwOpcode);
void ToRegFromHl(UINT32);
void RraRlaHandler(UINT32);
void LdByteRegpair(UINT32);
void IncDecHLPtr(UINT32 dwOpcode);
void InOutHandler(UINT32 dwOpcode);
void RLCRRCRLRRSLASRASRLHandler(UINT32 dwOpcode);
void BITHandler(UINT32 dwOpcode);
void RESSETHandler(UINT32 dwOpcode);
void PushPopOperationsIndexed(UINT32 dwOpcode);
void LDILDRLDIRLDDRHandler(UINT32);
void LdRegpair(UINT32 dwOpcode);
void ExtendedRegIntoMemory(UINT32 dwOpcode);
void NegHandler(UINT32 dwOpcode);
void ExtendedInHandler(UINT32 dwOpcode);
void ExtendedOutHandler(UINT32 dwOpcode);
void RetIRetNHandler(UINT32 dwOcode);
void AdcSbcRegpair(UINT32 dwOpcode);
void CPICPDCPIRCPDRHandler(UINT32 dwOpcode);
void RRDRLDHandler(UINT32 dwOpcode);
void UndocRegToIndex(UINT32 dwOpcode);
void UndocIndexToReg(UINT32 dwOpcode);
void MathOperationIndexed(UINT32 dwOpcode);
void IncDecIndexed(UINT32 dwOpcode);
void DDFDCBHandler(UINT32 dwOpcode);
void JPIXIYHandler(UINT32 dwOpcode);
void AddIndexHandler(UINT32 dwOpcode);
void SPToIndex(UINT32 dwOpcode);
void LdByteToIndex(UINT32 dwOpcode);
void LdRegIndexOffset(UINT32 dwOpcode);
void IncDecIndexReg(UINT32 dwOpcode);
void ExIndexed(UINT32 dwOpcode);
void UndocIncDecIndexReg(UINT32 dwOpcode);
void UndocLoadHalfIndexReg(UINT32 dwOpcode);
void UndocMathIndex(UINT32 dwOpcode);
void ddcbBitWise(UINT32 dwOpcode);
void LdIndexPtrReg(UINT32 dwOpcode);
void StoreIndexReg(UINT32 dwOpcode);
void LoadIndexReg(UINT32 dwOpcode);
void OTIROTDROUTIOUTDHandler(UINT32 dwOpcode);
void INIRINDRINIINDHandler(UINT32 dwOpcode);

struct sOp
{
	UINT32 bOpCode;
	void (*Emitter)(UINT32);
};

struct sOp StandardOps[] =
{
	{0xd3,  InOutHandler},		// V
	{0xdb,  InOutHandler},		// V

	{0x0a, LdByteRegpair},		// V
	{0x1a, LdByteRegpair},		// V

	{0x17,  RraRlaHandler},		// V
	{0x1f,  RraRlaHandler},		// V

	{0x05,  DecRegister},		// V
	{0x0d,  DecRegister},		// V
	{0x15,  DecRegister},		// V
	{0x1d,  DecRegister},		// V
	{0x25,  DecRegister},		// V
	{0x2d,  DecRegister},		// V
	{0x3d,  DecRegister},		// V

	{0x04,  IncRegister},		// V
	{0x0c,  IncRegister},		// V
	{0x14,  IncRegister},		// V
	{0x1c,  IncRegister},		// V
	{0x24,  IncRegister},		// V
	{0x2c,  IncRegister},		// V
	{0x3c,  IncRegister},		// V

	{0x32,	RegIntoMemory},	// V
	{0x22,	RegIntoMemory},	// V

	{0xc3,	JpHandler},			// V
	{0xc2, JpHandler},			// V
	{0xca, JpHandler},			// V
	{0xd2, JpHandler},			// V
	{0xda, JpHandler},			// V
	{0xe2, JpHandler},			// V
	{0xea, JpHandler},			// V
	{0xf2, JpHandler},			// V
	{0xfa, JpHandler},			// V


	{0x06, LdRegImmediate},		// V
	{0x0e, LdRegImmediate},		// V
	{0x16, LdRegImmediate},		// V
	{0x1e, LdRegImmediate},		// V
	{0x26, LdRegImmediate},		// V
	{0x2e, LdRegImmediate},		// V
	{0x3e, LdRegImmediate},		// V

	{0x0b,  IncDecRegpair},		// V
	{0x1b,  IncDecRegpair},		// V
	{0x2b,  IncDecRegpair},		// V
	{0x3b,  IncDecRegpair},		// V

	{0x03,  IncDecRegpair},		// V
	{0x13,  IncDecRegpair},		// V
	{0x23,  IncDecRegpair},		// V
	{0x33,  IncDecRegpair},		// V

	{0x34,	IncDecHLPtr},		// V
	{0x35,	IncDecHLPtr},		// V

	{0xcb,	CBHandler},
	{0xdd,	DDHandler},
	{0xed,	EDHandler},
	{0xfd,	FDHandler},

	{0x01,	LdRegPairImmediate},	// V
	{0x11,	LdRegPairImmediate},	// V
	{0x21,	LdRegPairImmediate},	// V
	{0x31,	LdRegPairImmediate},	// V

	{0xe3,	MiscHandler},	// V
	{0x2a,	MiscHandler},	// V
	{0xfb,	MiscHandler},	// V
	{0xf9,	MiscHandler},	// V
	{0xd9,	MiscHandler},	// V
	{0x76,	MiscHandler},	// V
	{0x3f,	MiscHandler},	// V
	{0x37,	MiscHandler},	// V
	{0x27,	MiscHandler},	// V
	{0x07,	MiscHandler},	// V
	{0x08,	MiscHandler},	// V
	{0x00,	MiscHandler},	// V
	{0xe9,	MiscHandler},	// V
	{0xeb,	MiscHandler},	// V
	{0xf3,	MiscHandler},	// V
	{0x3a,	MiscHandler},	// V
	{0x10,	MiscHandler},	// V
	{0x2f,	MiscHandler},	// V
	{0x0f,	MiscHandler},	// V

	{0x02, LdRegpairPtrByte},	// V
	{0x12, LdRegpairPtrByte},	// V

	{0x70, LdRegpairPtrByte},	// V
	{0x71, LdRegpairPtrByte},	// V
	{0x72, LdRegpairPtrByte},	// V
	{0x73, LdRegpairPtrByte},	// V
	{0x74, LdRegpairPtrByte},	// V
	{0x75, LdRegpairPtrByte},	// V
	{0x77, LdRegpairPtrByte},	// V

	{0x36, LdRegpairPtrByte},	// V

	{0x80,  MathOperation},	// V
	{0x81,  MathOperation},	// V
	{0x82,  MathOperation},	// V
	{0x83,  MathOperation},	// V
	{0x84,  MathOperation},	// V
	{0x85,  MathOperation},	// V
	{0x86,  MathOperation},	// V
	{0x87,  MathOperation},	// V
	{0x88,  MathOperation},	// V
	{0x89,  MathOperation},	// V
	{0x8a,  MathOperation},	// V
	{0x8b,  MathOperation},	// V
	{0x8c,  MathOperation},	// V
	{0x8d,  MathOperation},	// V
	{0x8e,  MathOperation},	// V
	{0x8f,  MathOperation},	// V
	{0x90,  MathOperation},	// V
	{0x91,  MathOperation},	// V
	{0x92,  MathOperation},	// V
	{0x93,  MathOperation},	// V
	{0x94,  MathOperation},	// V
	{0x95,  MathOperation},	// V
	{0x96,  MathOperation},	// V
	{0x97,  MathOperation},	// V
	{0x98,  MathOperation},	// V
	{0x99,  MathOperation},	// V
	{0x9a,  MathOperation},	// V
	{0x9b,  MathOperation},	// V
	{0x9c,  MathOperation},	// V
	{0x9d,  MathOperation},	// V
	{0x9e,  MathOperation},	// V
	{0x9f,  MathOperation},	// V
	{0xa0,  MathOperation},	// V
	{0xa1,  MathOperation},	// V
	{0xa2,  MathOperation},	// V
	{0xa3,  MathOperation},	// V
	{0xa4,  MathOperation},	// V
	{0xa5,  MathOperation},	// V
	{0xa6,  MathOperation},	// V
	{0xa7,  MathOperation},	// V
	{0xa8,  MathOperation},	// V
	{0xa9,  MathOperation},	// V
	{0xaa,  MathOperation},	// V
	{0xab,  MathOperation},	// V
	{0xac,  MathOperation},	// V
	{0xad,  MathOperation},	// V
	{0xae,  MathOperation},	// V
	{0xaf,  MathOperation},	// V
	{0xb0,  MathOperation},	// V
	{0xb1,  MathOperation},	// V
	{0xb2,  MathOperation},	// V
	{0xb3,  MathOperation},	// V
	{0xb4,  MathOperation},	// V
	{0xb5,  MathOperation},	// V
	{0xb6,  MathOperation},	// V
	{0xb7,  MathOperation},	// V
	{0xb8,  MathOperation},	// V
	{0xb9,  MathOperation},	// V
	{0xba,  MathOperation},	// V
	{0xbb,  MathOperation},	// V
	{0xbc,  MathOperation},	// V
	{0xbd,  MathOperation},	// V
	{0xbe,  MathOperation},	// V
	{0xbf,  MathOperation},	// V

	{0x40, LdRegReg},	// V
	{0x41, LdRegReg},	// V
	{0x42, LdRegReg},	// V
	{0x43, LdRegReg},	// V
	{0x44, LdRegReg},	// V
	{0x45, LdRegReg},	// V
	{0x47, LdRegReg},	// V
	{0x48, LdRegReg},	// V
	{0x49, LdRegReg},	// V
	{0x4a, LdRegReg},	// V
	{0x4b, LdRegReg},	// V
	{0x4c, LdRegReg},	// V
	{0x4d, LdRegReg},	// V
	{0x4f, LdRegReg},	// V
	{0x50, LdRegReg},	// V
	{0x51, LdRegReg},	// V
	{0x52, LdRegReg},	// V
	{0x53, LdRegReg},	// V
	{0x54, LdRegReg},	// V
	{0x55, LdRegReg},	// V
	{0x57, LdRegReg},	// V
	{0x58, LdRegReg},	// V
	{0x59, LdRegReg},	// V
	{0x5a, LdRegReg},	// V
	{0x5b, LdRegReg},	// V
	{0x5c, LdRegReg},	// V
	{0x5d, LdRegReg},	// V
	{0x5f, LdRegReg},	// V
	{0x60, LdRegReg},	// V
	{0x61, LdRegReg},	// V
	{0x62, LdRegReg},	// V
	{0x63, LdRegReg},	// V
	{0x64, LdRegReg},	// V
	{0x65, LdRegReg},	// V
	{0x67, LdRegReg},	// V
	{0x68, LdRegReg},	// V
	{0x69, LdRegReg},	// V
	{0x6a, LdRegReg},	// V
	{0x6b, LdRegReg},	// V
	{0x6c, LdRegReg},	// V
	{0x6d, LdRegReg},	// V
	{0x6f, LdRegReg},	// V
	{0x78, LdRegReg},	// V
	{0x79, LdRegReg},	// V
	{0x7a, LdRegReg},	// V
	{0x7b, LdRegReg},	// V
	{0x7c, LdRegReg},	// V
	{0x7d, LdRegReg},	// V
	{0x7f, LdRegReg},	// V

	{0xc6,  MathOperationDirect},	// V
	{0xce,  MathOperationDirect},	// V
	{0xd6,  MathOperationDirect},	// V
	{0xde,  MathOperationDirect},	// V
	{0xe6,  MathOperationDirect},	// V
	{0xee,  MathOperationDirect},	// V
	{0xf6,  MathOperationDirect},	// V
	{0xfe,  MathOperationDirect},	// V

	{0x18,  JrHandler},	// V
	{0x20,  JrHandler},	// V
	{0x28,  JrHandler},	// V
	{0x30,  JrHandler},	// V
	{0x38,  JrHandler},

	{0xc4, CallHandler},	// V
	{0xcc, CallHandler},	// V
	{0xcd, CallHandler},	// V
	{0xd4, CallHandler},	// V
	{0xdc, CallHandler},	// V
	{0xe4, CallHandler},	// V
	{0xec, CallHandler},	// V
	{0xf4, CallHandler},	// V
	{0xfc, CallHandler},	// V

	{0xc9,  RetHandler},	// V
	{0xc0,  RetHandler},	// V
	{0xc8,  RetHandler},	// V
	{0xd0,  RetHandler},	// V
	{0xd8,  RetHandler},	// V
	{0xe0,  RetHandler},	// V
	{0xe8,  RetHandler},	// V
	{0xf0,  RetHandler},	// V
	{0xf8,  RetHandler},	// V

	{0xc7,  RestartHandler}, // V
	{0xcf,  RestartHandler}, // V
	{0xd7,  RestartHandler}, // V
	{0xdf,  RestartHandler}, // V
	{0xe7,  RestartHandler}, // V
	{0xef,  RestartHandler}, // V
	{0xf7,  RestartHandler}, // V
	{0xff,  RestartHandler}, // V

	{0x46,  ToRegFromHl},	// V
	{0x4e,  ToRegFromHl},	// V
	{0x56,  ToRegFromHl},	// V
	{0x5e,  ToRegFromHl},	// V
	{0x66,  ToRegFromHl},	// V
	{0x6e,  ToRegFromHl},	// V
	{0x7e,  ToRegFromHl},

	{0x09,  AddRegpairOperations},	// V
	{0x19,  AddRegpairOperations},	// V
	{0x29,  AddRegpairOperations},	// V
	{0x39,  AddRegpairOperations},	// V

	{0xc5,  PushPopOperations},	// V
	{0xd5,  PushPopOperations},	// V
	{0xe5,  PushPopOperations},	// V
	{0xf5,  PushPopOperations},	// V
	{0xc1,  PushPopOperations},	// V
	{0xd1,  PushPopOperations},	// V
	{0xe1,  PushPopOperations},	// V
	{0xf1,  PushPopOperations},	// V

	// Terminator

	{0xffffffff, NULL}  
};

struct sOp CBOps[] =
{
	{0x00,  RLCRRCRLRRSLASRASRLHandler},
	{0x01,  RLCRRCRLRRSLASRASRLHandler},
	{0x02,  RLCRRCRLRRSLASRASRLHandler},
	{0x03,  RLCRRCRLRRSLASRASRLHandler},
	{0x04,  RLCRRCRLRRSLASRASRLHandler},
	{0x05,  RLCRRCRLRRSLASRASRLHandler},
	{0x06,  RLCRRCRLRRSLASRASRLHandler},
	{0x07,  RLCRRCRLRRSLASRASRLHandler},
	{0x08,  RLCRRCRLRRSLASRASRLHandler},
	{0x09,  RLCRRCRLRRSLASRASRLHandler},
	{0x0a,  RLCRRCRLRRSLASRASRLHandler},
	{0x0b,  RLCRRCRLRRSLASRASRLHandler},
	{0x0c,  RLCRRCRLRRSLASRASRLHandler},
	{0x0d,  RLCRRCRLRRSLASRASRLHandler},
	{0x0e,  RLCRRCRLRRSLASRASRLHandler},
	{0x0f,  RLCRRCRLRRSLASRASRLHandler},

	{0x10,  RLCRRCRLRRSLASRASRLHandler},
	{0x11,  RLCRRCRLRRSLASRASRLHandler},
	{0x12,  RLCRRCRLRRSLASRASRLHandler},
	{0x13,  RLCRRCRLRRSLASRASRLHandler},
	{0x14,  RLCRRCRLRRSLASRASRLHandler},
	{0x15,  RLCRRCRLRRSLASRASRLHandler},
	{0x16,  RLCRRCRLRRSLASRASRLHandler},
	{0x17,  RLCRRCRLRRSLASRASRLHandler},
	{0x18,  RLCRRCRLRRSLASRASRLHandler},
	{0x19,  RLCRRCRLRRSLASRASRLHandler},
	{0x1a,  RLCRRCRLRRSLASRASRLHandler},
	{0x1b,  RLCRRCRLRRSLASRASRLHandler},
	{0x1c,  RLCRRCRLRRSLASRASRLHandler},
	{0x1d,  RLCRRCRLRRSLASRASRLHandler},
	{0x1e,  RLCRRCRLRRSLASRASRLHandler},
	{0x1f,  RLCRRCRLRRSLASRASRLHandler},

	{0x20,  RLCRRCRLRRSLASRASRLHandler},
	{0x21,  RLCRRCRLRRSLASRASRLHandler},
	{0x22,  RLCRRCRLRRSLASRASRLHandler},
	{0x23,  RLCRRCRLRRSLASRASRLHandler},
	{0x24,  RLCRRCRLRRSLASRASRLHandler},
	{0x25,  RLCRRCRLRRSLASRASRLHandler},
	{0x26,  RLCRRCRLRRSLASRASRLHandler},
	{0x27,  RLCRRCRLRRSLASRASRLHandler},
	{0x28,  RLCRRCRLRRSLASRASRLHandler},
	{0x29,  RLCRRCRLRRSLASRASRLHandler},
	{0x2a,  RLCRRCRLRRSLASRASRLHandler},
	{0x2b,  RLCRRCRLRRSLASRASRLHandler},
	{0x2c,  RLCRRCRLRRSLASRASRLHandler},
	{0x2d,  RLCRRCRLRRSLASRASRLHandler},
	{0x2e,  RLCRRCRLRRSLASRASRLHandler},
	{0x2f,  RLCRRCRLRRSLASRASRLHandler},

	{0x30,  RLCRRCRLRRSLASRASRLHandler},
	{0x31,  RLCRRCRLRRSLASRASRLHandler},
	{0x32,  RLCRRCRLRRSLASRASRLHandler},
	{0x33,  RLCRRCRLRRSLASRASRLHandler},
	{0x34,  RLCRRCRLRRSLASRASRLHandler},
	{0x35,  RLCRRCRLRRSLASRASRLHandler},
	{0x36,  RLCRRCRLRRSLASRASRLHandler},
	{0x37,  RLCRRCRLRRSLASRASRLHandler},

	{0x38,  RLCRRCRLRRSLASRASRLHandler},
	{0x39,  RLCRRCRLRRSLASRASRLHandler},
	{0x3a,  RLCRRCRLRRSLASRASRLHandler},
	{0x3b,  RLCRRCRLRRSLASRASRLHandler},
	{0x3c,  RLCRRCRLRRSLASRASRLHandler},
	{0x3d,  RLCRRCRLRRSLASRASRLHandler},
	{0x3e,  RLCRRCRLRRSLASRASRLHandler},
	{0x3f,  RLCRRCRLRRSLASRASRLHandler},

	{0x40,  BITHandler},
	{0x41,  BITHandler},
	{0x42,  BITHandler},
	{0x43,  BITHandler},
	{0x44,  BITHandler},
	{0x45,  BITHandler},
	{0x46,  BITHandler},
	{0x47,  BITHandler},
	{0x48,  BITHandler},
	{0x49,  BITHandler},
	{0x4a,  BITHandler},
	{0x4b,  BITHandler},
	{0x4c,  BITHandler},
	{0x4d,  BITHandler},
	{0x4e,  BITHandler},
	{0x4f,  BITHandler},

	{0x50,  BITHandler},
	{0x51,  BITHandler},
	{0x52,  BITHandler},
	{0x53,  BITHandler},
	{0x54,  BITHandler},
	{0x55,  BITHandler},
	{0x56,  BITHandler},
	{0x57,  BITHandler},
	{0x58,  BITHandler},
	{0x59,  BITHandler},
	{0x5a,  BITHandler},
	{0x5b,  BITHandler},
	{0x5c,  BITHandler},
	{0x5d,  BITHandler},
	{0x5e,  BITHandler},
	{0x5f,  BITHandler},

	{0x60,  BITHandler},
	{0x61,  BITHandler},
	{0x62,  BITHandler},
	{0x63,  BITHandler},
	{0x64,  BITHandler},
	{0x65,  BITHandler},
	{0x66,  BITHandler},
	{0x67,  BITHandler},
	{0x68,  BITHandler},
	{0x69,  BITHandler},
	{0x6a,  BITHandler},
	{0x6b,  BITHandler},
	{0x6c,  BITHandler},
	{0x6d,  BITHandler},
	{0x6e,  BITHandler},
	{0x6f,  BITHandler},

	{0x70,  BITHandler},
	{0x71,  BITHandler},
	{0x72,  BITHandler},
	{0x73,  BITHandler},
	{0x74,  BITHandler},
	{0x75,  BITHandler},
	{0x76,  BITHandler},
	{0x77,  BITHandler},
	{0x78,  BITHandler},
	{0x79,  BITHandler},
	{0x7a,  BITHandler},
	{0x7b,  BITHandler},
	{0x7c,  BITHandler},
	{0x7d,  BITHandler},
	{0x7e,  BITHandler},
	{0x7f,  BITHandler},

	// RES

	{0x80,  RESSETHandler},
	{0x81,  RESSETHandler},
	{0x82,  RESSETHandler},
	{0x83,  RESSETHandler},
	{0x84,  RESSETHandler},
	{0x85,  RESSETHandler},
	{0x86,  RESSETHandler},
	{0x87,  RESSETHandler},
	{0x88,  RESSETHandler},
	{0x89,  RESSETHandler},
	{0x8a,  RESSETHandler},
	{0x8b,  RESSETHandler},
	{0x8c,  RESSETHandler},
	{0x8d,  RESSETHandler},
	{0x8e,  RESSETHandler},
	{0x8f,  RESSETHandler},

	{0x90,  RESSETHandler},
	{0x91,  RESSETHandler},
	{0x92,  RESSETHandler},
	{0x93,  RESSETHandler},
	{0x94,  RESSETHandler},
	{0x95,  RESSETHandler},
	{0x96,  RESSETHandler},
	{0x97,  RESSETHandler},
	{0x98,  RESSETHandler},
	{0x99,  RESSETHandler},
	{0x9a,  RESSETHandler},
	{0x9b,  RESSETHandler},
	{0x9c,  RESSETHandler},
	{0x9d,  RESSETHandler},
	{0x9e,  RESSETHandler},
	{0x9f,  RESSETHandler},

	{0xa0,  RESSETHandler},
	{0xa1,  RESSETHandler},
	{0xa2,  RESSETHandler},
	{0xa3,  RESSETHandler},
	{0xa4,  RESSETHandler},
	{0xa5,  RESSETHandler},
	{0xa6,  RESSETHandler},
	{0xa7,  RESSETHandler},
	{0xa8,  RESSETHandler},
	{0xa9,  RESSETHandler},
	{0xaa,  RESSETHandler},
	{0xab,  RESSETHandler},
	{0xac,  RESSETHandler},
	{0xad,  RESSETHandler},
	{0xae,  RESSETHandler},
	{0xaf,  RESSETHandler},

	{0xb0,  RESSETHandler},
	{0xb1,  RESSETHandler},
	{0xb2,  RESSETHandler},
	{0xb3,  RESSETHandler},
	{0xb4,  RESSETHandler},
	{0xb5,  RESSETHandler},
	{0xb6,  RESSETHandler},
	{0xb7,  RESSETHandler},
	{0xb8,  RESSETHandler},
	{0xb9,  RESSETHandler},
	{0xba,  RESSETHandler},
	{0xbb,  RESSETHandler},
	{0xbc,  RESSETHandler},
	{0xbd,  RESSETHandler},
	{0xbe,  RESSETHandler},
	{0xbf,  RESSETHandler},

	// SET

	{0xc0,  RESSETHandler},
	{0xc1,  RESSETHandler},
	{0xc2,  RESSETHandler},
	{0xc3,  RESSETHandler},
	{0xc4,  RESSETHandler},
	{0xc5,  RESSETHandler},
	{0xc6,  RESSETHandler},
	{0xc7,  RESSETHandler},
	{0xc8,  RESSETHandler},
	{0xc9,  RESSETHandler},
	{0xca,  RESSETHandler},
	{0xcb,  RESSETHandler},
	{0xcc,  RESSETHandler},
	{0xcd,  RESSETHandler},
	{0xce,  RESSETHandler},
	{0xcf,  RESSETHandler},

	{0xd0,  RESSETHandler},
	{0xd1,  RESSETHandler},
	{0xd2,  RESSETHandler},
	{0xd3,  RESSETHandler},
	{0xd4,  RESSETHandler},
	{0xd5,  RESSETHandler},
	{0xd6,  RESSETHandler},
	{0xd7,  RESSETHandler},
	{0xd8,  RESSETHandler},
	{0xd9,  RESSETHandler},
	{0xda,  RESSETHandler},
	{0xdb,  RESSETHandler},
	{0xdc,  RESSETHandler},
	{0xdd,  RESSETHandler},
	{0xde,  RESSETHandler},
	{0xdf,  RESSETHandler},

	{0xe0,  RESSETHandler},
	{0xe1,  RESSETHandler},
	{0xe2,  RESSETHandler},
	{0xe3,  RESSETHandler},
	{0xe4,  RESSETHandler},
	{0xe5,  RESSETHandler},
	{0xe6,  RESSETHandler},
	{0xe7,  RESSETHandler},
	{0xe8,  RESSETHandler},
	{0xe9,  RESSETHandler},
	{0xea,  RESSETHandler},
	{0xeb,  RESSETHandler},
	{0xec,  RESSETHandler},
	{0xed,  RESSETHandler},
	{0xee,  RESSETHandler},
	{0xef,  RESSETHandler},

	{0xf0,  RESSETHandler},
	{0xf1,  RESSETHandler},
	{0xf2,  RESSETHandler},
	{0xf3,  RESSETHandler},
	{0xf4,  RESSETHandler},
	{0xf5,  RESSETHandler},
	{0xf6,  RESSETHandler},
	{0xf7,  RESSETHandler},
	{0xf8,  RESSETHandler},
	{0xf9,  RESSETHandler},
	{0xfa,  RESSETHandler},
	{0xfb,  RESSETHandler},
	{0xfc,  RESSETHandler},
	{0xfd,  RESSETHandler},
	{0xfe,  RESSETHandler},
	{0xff,  RESSETHandler},

	// Terminator

	{0xffffffff, NULL}  
};

struct sOp EDOps[] =
{
	{0x67,  RRDRLDHandler},
	{0x6f,  RRDRLDHandler},
	{0x42,  AdcSbcRegpair},
	{0x4a,  AdcSbcRegpair},
	{0x52,  AdcSbcRegpair},
	{0x5a,  AdcSbcRegpair},
	{0x62,  AdcSbcRegpair},
	{0x6a,  AdcSbcRegpair},
	{0x72,  AdcSbcRegpair},
	{0x7a,  AdcSbcRegpair},  
	{0x45,  RetIRetNHandler},
	{0x4d,  RetIRetNHandler},
	{0x44,	NegHandler},
	{0xa0,  LDILDRLDIRLDDRHandler},
	{0xa8,  LDILDRLDIRLDDRHandler},
	{0xb0,  LDILDRLDIRLDDRHandler},
	{0xb8,  LDILDRLDIRLDDRHandler},
	{0x57, IRHandler},
	{0x5F, IRHandler},
	{0x47, IRHandler},
	{0x4F, IRHandler},
	{0x46,  IMHandler},
	{0x56,  IMHandler},
	{0x5e,  IMHandler},
	{0x4b,	LdRegpair},
	{0x5b,	LdRegpair},
	{0x7b,	LdRegpair},
	{0x43,	ExtendedRegIntoMemory},
	{0x53,	ExtendedRegIntoMemory},
	{0x63,	ExtendedRegIntoMemory},
	{0x73,	ExtendedRegIntoMemory},
	{0x40, ExtendedInHandler},
	{0x48, ExtendedInHandler},
	{0x50, ExtendedInHandler},
	{0x58, ExtendedInHandler},
	{0x60, ExtendedInHandler},
	{0x68, ExtendedInHandler},
	{0x78, ExtendedInHandler},
	{0x41, ExtendedOutHandler},
	{0x49, ExtendedOutHandler},
	{0x51, ExtendedOutHandler},
	{0x59, ExtendedOutHandler},
	{0x61, ExtendedOutHandler},
	{0x69, ExtendedOutHandler},
	{0x79, ExtendedOutHandler}, 
	{0xa1,	CPICPDCPIRCPDRHandler},
	{0xa9,	CPICPDCPIRCPDRHandler},
	{0xb1,	CPICPDCPIRCPDRHandler},
	{0xb9,	CPICPDCPIRCPDRHandler},

	{0xbb,	OTIROTDROUTIOUTDHandler},			// OTDR
	{0xb3,	OTIROTDROUTIOUTDHandler},			// OTIR
	{0xab,	OTIROTDROUTIOUTDHandler},			// OUTD
	{0xa3,	OTIROTDROUTIOUTDHandler},			// OUTI

	{0xb2,	INIRINDRINIINDHandler},				// INIR
	{0xba,	INIRINDRINIINDHandler},				// INDR
	{0xa2,	INIRINDRINIINDHandler},				// INI
	{0xaa,	INIRINDRINIINDHandler},				// IND

	// Terminator

	{0xffffffff, NULL}  
};

struct sOp DDFDOps[] =
{
	{0x35,  IncDecIndexed},
	{0x34,  IncDecIndexed},
 	{0xcb,  DDFDCBHandler},
	{0x86,  MathOperationIndexed},
	{0x8e,  MathOperationIndexed},
	{0x96,  MathOperationIndexed},
	{0x9e,  MathOperationIndexed},
	{0xa6,  MathOperationIndexed},
	{0xae,  MathOperationIndexed},
	{0xb6,  MathOperationIndexed},
	{0xbe,  MathOperationIndexed},

	{0xe1,  PushPopOperationsIndexed},
	{0xe5,  PushPopOperationsIndexed},
	{0x21,  LoadImmediate},
	{0xe9,  JPIXIYHandler},
	{0x09,  AddIndexHandler},
	{0x19,  AddIndexHandler},
	{0x29,  AddIndexHandler},
	{0x39,  AddIndexHandler},
	{0xf9,  SPToIndex},
	{0x36,  LdByteToIndex},
	{0x46,  LdRegIndexOffset},
	{0x4e,  LdRegIndexOffset},
	{0x56,  LdRegIndexOffset},
	{0x5e,  LdRegIndexOffset},
	{0x66,  LdRegIndexOffset},
	{0x6e,  LdRegIndexOffset},
	{0x7e,  LdRegIndexOffset}, 

	{0x70,  LdIndexPtrReg},
	{0x71,  LdIndexPtrReg},
	{0x72,  LdIndexPtrReg},
	{0x73,  LdIndexPtrReg},
	{0x74,  LdIndexPtrReg},
	{0x75,  LdIndexPtrReg},
	{0x77,  LdIndexPtrReg},

	{0x23,  IncDecIndexReg},
	{0x2b,  IncDecIndexReg},

	{0x22,  StoreIndexReg},
	{0x2a,  LoadIndexReg},
	{0xe3,  ExIndexed},

	{0x44,	UndocRegToIndex},
	{0x45,	UndocRegToIndex},
	{0x4c,	UndocRegToIndex},
	{0x4d,	UndocRegToIndex},
	{0x54,	UndocRegToIndex},
	{0x55,	UndocRegToIndex},
	{0x5c,	UndocRegToIndex},
	{0x5d,	UndocRegToIndex},
	{0x7c,	UndocRegToIndex},
	{0x7d,	UndocRegToIndex},

	{0x60,	UndocIndexToReg},
	{0x61,	UndocIndexToReg},
	{0x62,	UndocIndexToReg},
	{0x63,	UndocIndexToReg},
	{0x64,	UndocIndexToReg},
	{0x65,	UndocIndexToReg},
	{0x67,	UndocIndexToReg},
	{0x68,	UndocIndexToReg},
	{0x69,	UndocIndexToReg},
	{0x6a,	UndocIndexToReg},
	{0x6b,	UndocIndexToReg},
	{0x6c,	UndocIndexToReg},
	{0x6d,	UndocIndexToReg},
	{0x6f,	UndocIndexToReg},

	{0x24,	UndocIncDecIndexReg},
	{0x25,	UndocIncDecIndexReg},
	{0x2c,	UndocIncDecIndexReg},
	{0x2d,	UndocIncDecIndexReg},

	{0x26,	UndocLoadHalfIndexReg},
	{0x2e,	UndocLoadHalfIndexReg},

	{0x84,	UndocMathIndex},
	{0x85,	UndocMathIndex},
	{0x8c,	UndocMathIndex},
	{0x8d,	UndocMathIndex},

	{0x94,	UndocMathIndex},
	{0x95,	UndocMathIndex},
	{0x9c,	UndocMathIndex},
	{0x9d,	UndocMathIndex},

	{0xa4,	UndocMathIndex},
	{0xa5,	UndocMathIndex},
	{0xac,	UndocMathIndex},
	{0xad,	UndocMathIndex},

	{0xb4,	UndocMathIndex},
	{0xb5,	UndocMathIndex},
	{0xbc,	UndocMathIndex},
	{0xbd,	UndocMathIndex},

	// Terminator

	{0xffffffff, NULL}
};

struct sOp DDFDCBOps[] =
{
	{0x06,  ddcbBitWise},
	{0x0e,  ddcbBitWise},
	{0x16,  ddcbBitWise},
	{0x1e,  ddcbBitWise},
	{0x26,  ddcbBitWise},
	{0x2e,  ddcbBitWise},
	{0x3e,  ddcbBitWise},
	{0x46,  ddcbBitWise},
	{0x4e,  ddcbBitWise},
	{0x56,  ddcbBitWise},
	{0x5e,  ddcbBitWise},
	{0x66,  ddcbBitWise},
	{0x6e,  ddcbBitWise},
	{0x76,  ddcbBitWise},
	{0x7e,  ddcbBitWise},
	{0x86,  ddcbBitWise},
	{0x8e,  ddcbBitWise},
	{0x96,  ddcbBitWise},
	{0x9e,  ddcbBitWise},
	{0xa6,  ddcbBitWise},
	{0xae,  ddcbBitWise},
	{0xb6,  ddcbBitWise},
	{0xbe,  ddcbBitWise},
	{0xc6,  ddcbBitWise},
	{0xce,  ddcbBitWise},
	{0xd6,  ddcbBitWise},
	{0xde,  ddcbBitWise},
	{0xe6,  ddcbBitWise},
	{0xee,  ddcbBitWise},
	{0xf6,  ddcbBitWise},
	{0xfe,  ddcbBitWise},

	// Terminator

	{0xffffffff, NULL}
};

void InvalidInstructionC(UINT32 dwCount)
{
	fprintf(fp, "				InvalidInstruction(%" PRIu32 ");\n", dwCount);
}

UINT32 Timing(UINT8 bWho, UINT32 dwOpcode)
{
	UINT32 dwTiming = 0;

	assert(dwOpcode < 0x100);

	if (TIMING_REGULAR == bWho)	// Regular?
		dwTiming = bTimingRegular[dwOpcode];
	else
	if (TIMING_CB == bWho)
		dwTiming = bTimingCB[dwOpcode];
	else
	if (TIMING_DDFD == bWho)
		dwTiming = bTimingDDFD[dwOpcode];
	else
	if (TIMING_ED == bWho)
		dwTiming = bTimingED[dwOpcode];
	else
	if (TIMING_XXCB == bWho)
 		dwTiming = bTimingXXCB[dwOpcode];
	else
	if (TIMING_EXCEPT == bWho)
		dwTiming = dwOpcode;
	else
		abort();

	if (0 == dwTiming)
	{	
		fprintf(stderr, "Opcode: %.2x:%.2x - Not zero!\n", bWho, dwOpcode);
		fclose(fp);
		exit(1);
	}

	return(dwTiming);
}

void IndexedOffset(INT8 *Localmz80Index)
{
	fprintf(fp, "		mov	dl, [esi]	; Fetch our offset\n");
	fprintf(fp, "		inc	esi		; Move past the offset\n");
	fprintf(fp, "		or	dl, dl		; Is this bad boy signed?\n");
	fprintf(fp, "		jns	notSigned%" PRIu32 "	; Nope!\n", dwGlobalLabel);
	fprintf(fp, "		dec	dh			; Make it FFable\n");
	fprintf(fp, "notSigned%" PRIu32 ":\n", dwGlobalLabel);
	fprintf(fp, "		add	dx, [_z80%s]	; Our offset!\n", Localmz80Index);
	++dwGlobalLabel;
}

void CBHandler(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp, ";\n");
		fprintf(fp, "; Handler for all CBxx instructions\n");
		fprintf(fp, ";\n");
		sprintf(string, "RegInst%.2x", dwOpcode);
		ProcBegin(0xffffffff);
		fprintf(fp, "		mov	dl, [esi]\n");
		fprintf(fp, "		inc	esi\n");
		fprintf(fp, "		jmp	dword [z80PrefixCB+edx*4]\n\n");
		fprintf(fp, "\n\n");
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				CBHandler();\n");
	}
	else
	{
		abort();	
	}
}

void EDHandler(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp, ";\n");
		fprintf(fp, "; Handler for all EDxx instructions\n");
		fprintf(fp, ";\n");
		sprintf(string, "RegInst%.2x", dwOpcode);
		ProcBegin(0xffffffff);
		fprintf(fp,     "		mov	dl, [esi]\n");
		fprintf(fp, "		inc	esi\n");
		fprintf(fp,     "		jmp	dword [z80PrefixED+edx*4]\n\n");
		fprintf(fp, "\n\n");
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				EDHandler();\n");
	}
	else
	{
		abort();	
	}
}

void DDHandler(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp, ";\n");
		fprintf(fp, "; Handler for all DDxx instructions\n");
		fprintf(fp, ";\n");
		sprintf(string, "RegInst%.2x", dwOpcode);
		ProcBegin(0xffffffff);
		fprintf(fp,     "		mov	dl, [esi]\n");
		fprintf(fp, "		inc	esi\n");
		fprintf(fp,     "		jmp	dword [z80PrefixDD+edx*4]\n\n");
		fprintf(fp, "\n\n");
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				DDHandler();\n");
	}
	else
	{
		abort();	
	}
}

void FDHandler(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp, ";\n");
		fprintf(fp, "; Handler for all FDxx instructions\n");
		fprintf(fp, ";\n");
		sprintf(string, "RegInst%.2x", dwOpcode);
		ProcBegin(0xffffffff);
		fprintf(fp,     "		mov	dl, [esi]\n");
		fprintf(fp, "		inc	esi\n");
		fprintf(fp,     "		jmp	dword [z80PrefixFD+edx*4]\n\n");
		fprintf(fp, "\n\n");
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				FDHandler();\n");
	}
	else
	{
		abort();	
	}
}

void StandardHeader(void)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp,"; For assembly by NASM only\n");
		fprintf(fp,"bits 32\n\n");

		fprintf(fp,"; Theory of operation\n\n");
		fprintf(fp,"; EDI=General purpose\n");
		fprintf(fp,"; ESI=Program counter + base address\n");
		fprintf(fp,"; EBP=z80Base\n");
		fprintf(fp,"; AX=AF\n");
		fprintf(fp,"; BX=HL\n");
		fprintf(fp,"; CX=BC\n");
		fprintf(fp,"; DX=General purpose\n\n"); 

		if (bUseStack)
			fprintf(fp, "; Using stack calling conventions\n");
		else
			fprintf(fp, "; Using register calling conventions\n");

		if (b16BitIo)
			fprintf(fp, "; Extended input/output instructions treat (BC) as I/O address\n");
		else
			fprintf(fp, "; Extended input/output instructions treat (C) as I/O address\n\n");

		fprintf(fp, "IFF1		equ	01h\n");
		fprintf(fp, "IFF2		equ	02h\n");

		fprintf(fp, "CPUREG_PC		equ	00h\n");
		fprintf(fp, "CPUREG_SP		equ	01h\n");
		fprintf(fp, "CPUREG_AF		equ	02h\n");
		fprintf(fp, "CPUREG_BC		equ	03h\n");
		fprintf(fp, "CPUREG_DE		equ	04h\n");
		fprintf(fp, "CPUREG_HL		equ	05h\n");
		fprintf(fp, "CPUREG_AFPRIME		equ	06h\n");
		fprintf(fp, "CPUREG_BCPRIME		equ	07h\n");
		fprintf(fp, "CPUREG_DEPRIME		equ	08h\n");
		fprintf(fp, "CPUREG_HLPRIME		equ	09h\n");
		fprintf(fp, "CPUREG_IX		equ	0ah\n");
		fprintf(fp, "CPUREG_IY		equ	0bh\n");
		fprintf(fp, "CPUREG_I		equ	0ch\n");
		fprintf(fp, "CPUREG_A		equ	0dh\n");
		fprintf(fp, "CPUREG_F		equ	0eh\n");
		fprintf(fp, "CPUREG_B		equ	0fh\n");
		fprintf(fp, "CPUREG_C		equ	10h\n");
		fprintf(fp, "CPUREG_D		equ	11h\n");
		fprintf(fp, "CPUREG_E		equ	12h\n");
		fprintf(fp, "CPUREG_H		equ	13h\n");
		fprintf(fp, "CPUREG_L		equ	14h\n");
		fprintf(fp, "CPUREG_IFF1		equ	15h\n");
		fprintf(fp, "CPUREG_IFF2		equ	16h\n");
		fprintf(fp, "CPUREG_CARRY		equ	17h\n");
		fprintf(fp, "CPUREG_NEGATIVE		equ	18h\n");
		fprintf(fp, "CPUREG_PARITY		equ	19h\n");
		fprintf(fp, "CPUREG_OVERFLOW		equ	1ah\n");
		fprintf(fp, "CPUREG_HALFCARRY		equ	1bh\n");
		fprintf(fp, "CPUREG_ZERO		equ	1ch\n");
		fprintf(fp, "CPUREG_SIGN		equ	1dh\n");
		fprintf(fp, "CPUREG_MAXINDEX		equ	1eh\n\n");
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "/* Multi-Z80 32 Bit emulator */\n");
		fprintf(fp, "\n");
		fprintf(fp, "/* Copyright 1996-2000 Neil Bradley, All rights reserved\n");
		fprintf(fp, " *\n");
		fprintf(fp, " * License agreement:\n");
		fprintf(fp, " *\n");
		fprintf(fp, " * (MZ80 Refers to both the assembly code emitted by makeZ80.c and makeZ80.c\n");
		fprintf(fp, " * itself)\n");
		fprintf(fp, " *\n");
		fprintf(fp, " * MZ80 May be distributed in unmodified form to any medium.\n");
		fprintf(fp, " *\n");
		fprintf(fp, " * MZ80 May not be sold, or sold as a part of a commercial package without\n");
		fprintf(fp, " * the express written permission of Neil Bradley (neil@synthcom.com). This\n");
		fprintf(fp, " * includes shareware.\n");
		fprintf(fp, " *\n");
		fprintf(fp, " * Modified versions of MZ80 may not be publicly redistributed without author\n");
		fprintf(fp, " * approval (neil@synthcom.com). This includes distributing via a publicly\n");
		fprintf(fp, " * accessible LAN. You may make your own source modifications and distribute\n");
		fprintf(fp, " * MZ80 in source or object form, but if you make modifications to MZ80\n");
		fprintf(fp, " * then it should be noted in the top as a comment in makeZ80.c.\n");
		fprintf(fp, " *\n");
		fprintf(fp, " * MZ80 Licensing for commercial applications is available. Please email\n");
		fprintf(fp, " * neil@synthcom.com for details.\n");
		fprintf(fp, " *\n");
		fprintf(fp, " * Synthcom Systems, Inc, and Neil Bradley will not be held responsible for\n");
		fprintf(fp, " * any damage done by the use of MZ80. It is purely \"as-is\".\n");
		fprintf(fp, " *\n");
		fprintf(fp, " * If you use MZ80 in a freeware application, credit in the following text:\n");
		fprintf(fp, " *\n");
		fprintf(fp, " * \"Multi-Z80 CPU emulator by Neil Bradley (neil@synthcom.com)\"\n");
		fprintf(fp, " *\n");
		fprintf(fp, " * must accompany the freeware application within the application itself or\n");
		fprintf(fp, " * in the documentation.\n");
		fprintf(fp, " *\n");
		fprintf(fp, " * Legal stuff aside:\n");
		fprintf(fp, " *\n");
		fprintf(fp, " * If you find problems with MZ80, please email the author so they can get\n");
		fprintf(fp, " * resolved. If you find a bug and fix it, please also email the author so\n");
		fprintf(fp, " * that those bug fixes can be propogated to the installed base of MZ80\n");
		fprintf(fp, " * users. If you find performance improvements or problems with MZ80, please\n");
		fprintf(fp, " * email the author with your changes/suggestions and they will be rolled in\n");
		fprintf(fp, " * with subsequent releases of MZ80.\n");
		fprintf(fp, " *\n");
		fprintf(fp, " * The whole idea of this emulator is to have the fastest available 32 bit\n");
		fprintf(fp, " * Multi-Z80 emulator for the PC, giving maximum performance. \n");
		fprintf(fp, " */\n\n");
		fprintf(fp, "#include <stdio.h>\n");
		fprintf(fp, "#include <stdlib.h>\n");
		fprintf(fp, "#include <string.h>\n");
		fprintf(fp,
			"#ifdef HAVE_MEMCPY_H\n"
			"#include \"memcpy.h\"\n"
			"#endif\n");
		fprintf(fp, "#include \"mz80.h\"\n");

		// HACK HACK

		fprintf(fp, "UINT32 z80intAddr;\n");
		fprintf(fp, "UINT32 z80pc;\n");
	}			  
	else
	{
		// Whoops. Unknown emission type.

		abort();
	}

	fprintf(fp, "\n\n");
}

void Alignment(void)
{
	fprintf(fp, "\ntimes ($$-$) & 3 nop	; pad with NOPs to 4-byte boundary\n\n");
}

void ProcBegin(UINT32 dwOpcode)
{
	(void)dwOpcode;
	Alignment();
	fprintf(fp, "%s:\n", procname);
}

void SetSubFlagsSZHVC(INT8 *pszLeft, INT8 *pszRight)
{
	fprintf(fp, "				cpu.z80F = (cpu.z80F & ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_OVERFLOW_PARITY | \n");
	fprintf(fp, "							   Z80_FLAG_HALF_CARRY | Z80_FLAG_ZERO | Z80_FLAG_SIGN)) |\n");
	fprintf(fp, "								pbSubSbcTable[((UINT32) %s << 8) | %s];\n", pszLeft, pszRight);
}

void SetSbcFlagsSZHVC(INT8 *pszLeft, INT8 *pszRight)
{
	fprintf(fp, "				cpu.z80F = (cpu.z80F & ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_OVERFLOW_PARITY | \n");
	fprintf(fp, "							   Z80_FLAG_HALF_CARRY | Z80_FLAG_ZERO | Z80_FLAG_SIGN)) |\n");
	fprintf(fp, "								pbSubSbcTable[((UINT32) %s << 8) | %s | (((UINT32) cpu.z80F & Z80_FLAG_CARRY) << 16)];\n", pszLeft, pszRight);
}

void SetAddFlagsSZHVC(INT8 *pszLeft, INT8 *pszRight)
{
	fprintf(fp, "				cpu.z80F = (cpu.z80F & ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_OVERFLOW_PARITY | \n");
	fprintf(fp, "							   Z80_FLAG_HALF_CARRY | Z80_FLAG_ZERO | Z80_FLAG_SIGN)) |\n");
	fprintf(fp, "								pbAddAdcTable[((UINT32) %s << 8) | %s];\n", pszLeft, pszRight);
}

void SetAdcFlagsSZHVC(INT8 *pszLeft, INT8 *pszRight)
{
	fprintf(fp, "				cpu.z80F = (cpu.z80F & ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_OVERFLOW_PARITY | \n");
	fprintf(fp, "							   Z80_FLAG_HALF_CARRY | Z80_FLAG_ZERO | Z80_FLAG_SIGN)) |\n");
	fprintf(fp, "								pbAddAdcTable[((UINT32) %s << 8) | %s | (((UINT32) cpu.z80F & Z80_FLAG_CARRY) << 16)];\n", pszLeft, pszRight);
}

UINT32 dwOverflowCount = 0;

void SetOverflow(void)
{
	fprintf(fp, "		seto	dl\n");
	fprintf(fp, "		and	ah, 0fbh	; Knock out parity/overflow\n");
	fprintf(fp, "		shl	dl, 2\n");
	fprintf(fp, "		or		ah, dl\n");
}
	
void FetchNextInstruction(UINT32 dwOpcode)
{
	if (0xffffffff != dwOpcode)
	{
		fprintf(fp, "		sub	edi, byte %" PRIu32 "\n", Timing(bCurrentMode, dwOpcode));
		
		if (bCurrentMode == TIMING_REGULAR)
			fprintf(fp, "		js	near noMoreExec\n");
		else
			fprintf(fp, "		js	near noMoreExec\n");
	}

	fprintf(fp, "		mov	dl, byte [esi]	; Get our next instruction\n");
	fprintf(fp, "		inc	esi		; Increment PC\n");
	fprintf(fp, "		jmp	dword [z80regular+edx*4]\n\n");
}

void WriteValueToMemory(INT8 *pszAddress, INT8 *pszValue)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp, "		mov	[cyclesRemaining], edi\n");
		fprintf(fp, "		mov	[_z80af], ax	; Store AF\n");

		// First off, load our byte to write into al after we've saved AF

		if (strcmp(pszValue, "al") != 0)
			fprintf(fp, "		mov	al, %s	; And our data to write\n", pszValue);
		if (strcmp(pszValue, "[esi]") == 0)	// Immediate value?
			fprintf(fp, "		inc	esi	; Increment our program counter\n");

		// Now get the address in DX - regardless of what it is

		if (strcmp(pszAddress, "[_z80de]") == 0 ||
			 strcmp(pszAddress, "[_orgval]") == 0 ||
			 strcmp(pszAddress, "[_z80ix]") == 0 ||
			 strcmp(pszAddress, "[_z80iy]") == 0)
			fprintf(fp, "		mov	dx, %s\n", pszAddress);

		fprintf(fp, "		mov	edi, [_z80MemWrite]	; Point to the write array\n\n");
		fprintf(fp, "checkLoop%" PRIu32 ":\n", dwGlobalLabel);
		fprintf(fp, "		cmp	[edi], word 0ffffh ; End of our list?\n");
		fprintf(fp, "		je	memoryWrite%" PRIu32 "	; Yes - go write it!\n", dwGlobalLabel);

		if (strcmp(pszAddress, "[_z80de]") == 0 ||
			 strcmp(pszAddress, "[_orgval]") == 0 ||
			 strcmp(pszAddress, "[_z80ix]") == 0 ||
			 strcmp(pszAddress, "[_z80iy]") == 0)
			fprintf(fp, "		cmp	dx, [edi]	; Are we smaller?\n");
		else
			fprintf(fp, "		cmp	%s, [edi]	; Are we smaller?\n", pszAddress);

		fprintf(fp, "		jb	nextAddr%" PRIu32 "	; Yes... go to the next addr\n", dwGlobalLabel);

		if (strcmp(pszAddress, "[_z80de]") == 0 ||
			 strcmp(pszAddress, "[_orgval]") == 0 ||
			 strcmp(pszAddress, "[_z80ix]") == 0 ||
			 strcmp(pszAddress, "[_z80iy]") == 0)
			fprintf(fp, "		cmp	dx, [edi+4]	; Are we smaller?\n");
		else
			fprintf(fp, "		cmp	%s, [edi+4]	; Are we smaller?\n", pszAddress);
	
		fprintf(fp, "		jbe	callRoutine%" PRIu32 "	; If not, go call it!\n\n", dwGlobalLabel);
		fprintf(fp, "nextAddr%" PRIu32 ":\n", dwGlobalLabel);
		fprintf(fp, "		add	edi, 10h		; Next structure, please\n");
		fprintf(fp, "		jmp	short checkLoop%" PRIu32 "\n\n", dwGlobalLabel);
		fprintf(fp, "callRoutine%" PRIu32 ":\n", dwGlobalLabel);
	
		// Save off our registers!
	
		if ((strcmp(pszAddress, "dx") != 0) && (strcmp(pszAddress, "[_z80de]") != 0) &&
			 (strcmp(pszAddress, "[_z80ix]") != 0) &&
			 (strcmp(pszAddress, "[_orgval]") != 0) &&
			 (strcmp(pszAddress, "[_z80iy]") != 0))
			fprintf(fp, "		mov	dx, %s	; Get our address to target\n", pszAddress);
	
		fprintf(fp, "		call	WriteMemoryByte	; Go write the data!\n");
		fprintf(fp, "		jmp	short WriteMacroExit%" PRIu32 "\n", dwGlobalLabel);
	
		fprintf(fp, "memoryWrite%" PRIu32 ":\n", dwGlobalLabel);
	
		if (strcmp(pszValue, "[esi]") == 0)
			fprintf(fp, "		mov	[ebp + e%s], al	; Store our direct value\n", pszAddress);
		else
		{
			if (pszValue[0] == 'b' && pszValue[1] == 'y' && pszValue[2] == 't')
			{
				fprintf(fp, "		mov	edi, edx\n");
				assert(strcmp(pszValue, "dl") != 0);
	
				fprintf(fp, "		mov	dl, %s\n", pszValue);
	
				if (strcmp(pszAddress, "dx") == 0)
					fprintf(fp, "		mov	[ebp + edi], dl\n");
				else
					fprintf(fp, "		mov	[ebp + e%s], dl\n", pszAddress);
	
				fprintf(fp, "		mov	edx, edi\n");
			}
			else
			{
				if (strcmp(pszAddress, "[_z80de]") != 0 &&
					 strcmp(pszAddress, "[_orgval]") != 0 &&
					 strcmp(pszAddress, "[_z80ix]") != 0 &&
					 strcmp(pszAddress, "[_z80iy]") != 0)
					fprintf(fp, "		mov	[ebp + e%s], %s\n", pszAddress, pszValue);
				else
					fprintf(fp, "		mov	[ebp + edx], al\n");
			}
		}

		fprintf(fp, "		mov	ax, [_z80af] ; Get our accumulator and flags\n");
	
		fprintf(fp, "WriteMacroExit%" PRIu32 ":\n", dwGlobalLabel);
		fprintf(fp, "		mov	edi, [cyclesRemaining]\n");

		++dwGlobalLabel;
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				psMemWrite = cpu.z80MemWrite;	/* Beginning of our handler */\n");
		fprintf(fp, "				while (psMemWrite->lowAddr != 0xffffffff)\n");
		fprintf(fp, "				{\n");
		fprintf(fp, "					if ((%s >= psMemWrite->lowAddr) && (%s <= psMemWrite->highAddr))\n", pszAddress, pszAddress);
		fprintf(fp, "					{\n");
		fprintf(fp, "						cpu.z80pc = (UINTPTR) pbPC - (UINTPTR) cpu.z80Base;\n");
		fprintf(fp, "						if (psMemWrite->memoryCall)\n");
		fprintf(fp, "						{\n");
		fprintf(fp, "							psMemWrite->memoryCall(%s, %s, psMemWrite);\n", pszAddress, pszValue);
		fprintf(fp, "						}\n");
		fprintf(fp, "						else\n");
		fprintf(fp, "						{\n");
		fprintf(fp, "							*((UINT8 *) psMemWrite->pUserArea + (%s - psMemWrite->lowAddr)) = %s;\n", pszAddress, pszValue);
		fprintf(fp, "						}\n");
		fprintf(fp, "						psMemWrite = NULL;\n");
		fprintf(fp, "						break;\n");
		fprintf(fp, "					}\n");
		fprintf(fp, "					++psMemWrite;\n");
		fprintf(fp, "				}\n\n");
		fprintf(fp, "				if (psMemWrite)\n");
		fprintf(fp, "				{\n");
		fprintf(fp, "					cpu.z80Base[%s] = (UINT8) %s;\n", pszAddress, pszValue);
		fprintf(fp, "				}\n\n");
	}
}

void WriteWordToMemory(INT8 *pszAddress, INT8 *pszTarget)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp, "		mov	[cyclesRemaining], edi\n");
		fprintf(fp, "		mov	edi, [_z80MemWrite]	; Point to the write array\n\n");
		fprintf(fp, "checkLoop%" PRIu32 ":\n", dwGlobalLabel);
		fprintf(fp, "		cmp	[edi], word 0ffffh ; End of the list?\n");
		fprintf(fp, "		je		memoryWrite%" PRIu32 "\n", dwGlobalLabel);
		fprintf(fp, "		cmp	%s, [edi]	; Are we smaller?\n", pszAddress);
		fprintf(fp, "		jb		nextAddr%" PRIu32 "		; Yes, go to the next address\n", dwGlobalLabel);
		fprintf(fp, "		cmp	%s, [edi+4]	; Are we bigger?\n", pszAddress);
		fprintf(fp, "		jbe	callRoutine%" PRIu32 "\n\n", dwGlobalLabel);
		fprintf(fp, "nextAddr%" PRIu32 ":\n", dwGlobalLabel);
		fprintf(fp, "		add	edi, 10h		; Next structure!\n");
		fprintf(fp, "		jmp	short checkLoop%" PRIu32 "\n\n", dwGlobalLabel);
		fprintf(fp, "callRoutine%" PRIu32 ":\n", dwGlobalLabel);

		fprintf(fp, "		push	ax		; Save this for later\n");

		// Write the LSB

		fprintf(fp, "		push	dx\n");

		if (strcmp(pszTarget, "ax") != 0)
		{
			fprintf(fp, "		mov	ax, %s\n", pszTarget);
		}
		else
		{
			fprintf(fp, "		xchg	ah, al\n");
		}

		fprintf(fp, "		call	WriteMemoryByte\n");
		fprintf(fp, "		pop	dx\n");
		fprintf(fp, "		pop	ax\n");
		fprintf(fp, "		inc	dx\n\n");

		fprintf(fp, "		push	ax\n");
		fprintf(fp, "		push	dx\n");

		if (strcmp(pszTarget, "ax") != 0)
		{
			fprintf(fp, "		mov	ax, %s\n", pszTarget);
			fprintf(fp, "		xchg	ah, al\n");
		}

		fprintf(fp, "		call	WriteMemoryByte\n");
		fprintf(fp, "		pop	dx\n");
		fprintf(fp, "		pop	ax	; Restore us!\n");

		fprintf(fp, "		jmp	writeExit%" PRIu32 "\n\n", dwGlobalLabel);

		fprintf(fp, "memoryWrite%" PRIu32 ":\n", dwGlobalLabel);

		if (strlen(pszTarget) != 2)
		{
			fprintf(fp, "		mov	di, %s\n", pszTarget);
			fprintf(fp, "		mov	[ebp + e%s], di	; Store our word\n", pszAddress);
		}
		else
		{
			if (strcmp(pszTarget, "ax") != 0)
			{
				fprintf(fp, "		mov	[ebp + e%s], %s	; Store our word\n", pszAddress, pszTarget);
			}
			else
			{
				fprintf(fp, "		xchg	ah, al	; Swap for later\n");
				fprintf(fp, "		mov	[ebp + e%s], %s	; Store our word\n", pszAddress, pszTarget);
				fprintf(fp, "		xchg	ah, al	; Restore\n");
			}
		}
	
		fprintf(fp, "writeExit%" PRIu32 ":\n", dwGlobalLabel);
		fprintf(fp, "		mov	edi, [cyclesRemaining]\n");
	
		dwGlobalLabel++;
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				psMemWrite = cpu.z80MemWrite;	/* Beginning of our handler */\n");
		fprintf(fp, "				while (psMemWrite->lowAddr != 0xffffffff)\n");
		fprintf(fp, "				{\n");
		fprintf(fp, "					if ((%s >= psMemWrite->lowAddr) && (%s <= psMemWrite->highAddr))\n", pszAddress, pszAddress);
		fprintf(fp, "					{\n");
		fprintf(fp, "						cpu.z80pc = (UINTPTR) pbPC - (UINTPTR) cpu.z80Base;\n");

		fprintf(fp, "						if (psMemWrite->memoryCall)\n");
		fprintf(fp, "						{\n");
		fprintf(fp, "							psMemWrite->memoryCall(%s, (%s & 0xff), psMemWrite);\n", pszAddress, pszTarget);
		fprintf(fp, "							psMemWrite->memoryCall(%s + 1, (%s >> 8), psMemWrite);\n", pszAddress, pszTarget);
		fprintf(fp, "						}\n");
		fprintf(fp, "						else\n");
		fprintf(fp, "						{\n");
		fprintf(fp, "							*((UINT8 *) psMemWrite->pUserArea + (%s - psMemWrite->lowAddr)) = %s;\n", pszAddress, pszTarget);
		fprintf(fp, "							*((UINT8 *) psMemWrite->pUserArea + (%s - psMemWrite->lowAddr) + 1) = %s >> 8;\n", pszAddress, pszTarget);
		fprintf(fp, "						}\n");

		fprintf(fp, "						psMemWrite = NULL;\n");
		fprintf(fp, "						break;\n");
		fprintf(fp, "					}\n");
		fprintf(fp, "					++psMemWrite;\n");
		fprintf(fp, "				}\n\n");
		fprintf(fp, "				if (psMemWrite)\n");
		fprintf(fp, "				{\n");
		fprintf(fp, "					cpu.z80Base[%s] = (UINT8) %s;\n", pszAddress, pszTarget);
		fprintf(fp, "					cpu.z80Base[%s + 1] = (UINT8) ((UINT32) %s >> 8);\n", pszAddress, pszTarget);
		fprintf(fp, "				}\n\n");
	}
	else
	{
		abort();
	}
}

void WriteValueToIo(INT8 *pszIoAddress, INT8 *pszValue)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp, "		mov	[cyclesRemaining], edi\n");
		fprintf(fp, "		mov	[_z80af], ax	; Store AF\n");

		if (strcmp(pszValue, "al") != 0)
			fprintf(fp, "		mov	al, %s	; And our data to write\n", pszValue);
		if (strcmp(pszValue, "[esi]") == 0)	// Immediate value?
			fprintf(fp, "		inc	esi	; Increment our program counter\n");

		fprintf(fp, "		mov	edi, [_z80IoWrite]	; Point to the I/O write array\n\n");
		fprintf(fp, "checkLoop%" PRIu32 ":\n", dwGlobalLabel);
		fprintf(fp, "		cmp	[edi], word 0ffffh ; End of our list?\n");
		fprintf(fp, "		je	WriteMacroExit%" PRIu32 "	; Yes - ignore it!\n", dwGlobalLabel);
		fprintf(fp, "		cmp	%s, [edi]	; Are we smaller?\n", pszIoAddress);
		fprintf(fp, "		jb	nextAddr%" PRIu32 "	; Yes... go to the next addr\n", dwGlobalLabel);
		fprintf(fp, "		cmp	%s, [edi+2]	; Are we bigger?\n", pszIoAddress);
		fprintf(fp, "		jbe	callRoutine%" PRIu32 "	; If not, go call it!\n\n", dwGlobalLabel);
		fprintf(fp, "nextAddr%" PRIu32 ":\n", dwGlobalLabel);
		fprintf(fp, "		add	edi, 0ch		; Next structure, please\n");
		fprintf(fp, "		jmp	short checkLoop%" PRIu32 "\n\n", dwGlobalLabel);
		fprintf(fp, "callRoutine%" PRIu32 ":\n", dwGlobalLabel);

		// Save off our registers!

		if (strcmp(pszIoAddress, "dx") != 0)
			fprintf(fp, "		mov	dx, %s	; Get our address to target\n", pszIoAddress);

		fprintf(fp, "		call	WriteIOByte	; Go write the data!\n");
	
		fprintf(fp, "WriteMacroExit%" PRIu32 ":\n", dwGlobalLabel);
		fprintf(fp, "		mov	edi, [cyclesRemaining]\n");
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				psIoWrite = cpu.z80IoWrite;	/* Beginning of our handler */\n");
		fprintf(fp, "				while (psIoWrite->lowIoAddr != 0xffff)\n");
		fprintf(fp, "				{\n");
		fprintf(fp, "					if ((%s >= psIoWrite->lowIoAddr) && (%s <= psIoWrite->highIoAddr))\n", pszIoAddress, pszIoAddress);
		fprintf(fp, "					{\n");
		fprintf(fp, "						cpu.z80pc = (UINTPTR) pbPC - (UINTPTR) cpu.z80Base;\n");
		fprintf(fp, "						psIoWrite->IOCall(%s, %s, psIoWrite);\n", pszIoAddress, pszValue);
		fprintf(fp, "						psIoWrite = NULL;\n");
		fprintf(fp, "						break;\n");
		fprintf(fp, "					}\n");
		fprintf(fp, "					++psIoWrite;\n");
		fprintf(fp, "				}\n\n");
	}
	else
	{
		abort();
	}	
	
	++dwGlobalLabel;
}

void ReadValueFromMemory(INT8 *pszAddress, INT8 *pszTarget)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp, "		mov	[cyclesRemaining], edi\n");
		fprintf(fp, "		mov	edi, [_z80MemRead]	; Point to the read array\n\n");
		fprintf(fp, "checkLoop%" PRIu32 ":\n", dwGlobalLabel);
		fprintf(fp, "		cmp	[edi], word 0ffffh ; End of the list?\n");
		fprintf(fp, "		je		memoryRead%" PRIu32 "\n", dwGlobalLabel);
		fprintf(fp, "		cmp	e%s, [edi]	; Are we smaller?\n", pszAddress);
		fprintf(fp, "		jb		nextAddr%" PRIu32 "		; Yes, go to the next address\n", dwGlobalLabel);
		fprintf(fp, "		cmp	e%s, [edi+4]	; Are we bigger?\n", pszAddress);
		fprintf(fp, "		jbe	callRoutine%" PRIu32 "\n\n", dwGlobalLabel);
		fprintf(fp, "nextAddr%" PRIu32 ":\n", dwGlobalLabel);
		fprintf(fp, "		add	edi, 10h		; Next structure!\n");
		fprintf(fp, "		jmp	short checkLoop%" PRIu32 "\n\n", dwGlobalLabel);
		fprintf(fp, "callRoutine%" PRIu32 ":\n", dwGlobalLabel);

		if (strcmp(pszAddress, "dx") != 0)
			fprintf(fp, "		mov	dx, %s	; Get our address\n", pszAddress);
	
		fprintf(fp, "		call	ReadMemoryByte	; Standard read routine\n");
	
		// Yes, these are intentionally reversed!
	
		if (strcmp(pszTarget, "al") == 0)
			fprintf(fp, "		mov	[_z80af], al	; Save our new accumulator\n");
		else
		if (strcmp(pszTarget, "ah") == 0)
			fprintf(fp, "		mov	[_z80af + 1], al	; Save our new flags\n");
		else
			fprintf(fp, "		mov	%s, al	; Put our returned value here\n", pszTarget);
	
		// And are properly restored HERE:
	
		fprintf(fp, "		mov	ax, [_z80af]	; Get our AF back\n");
	
		// Restore registers here...
	
		fprintf(fp, "		jmp	short readExit%" PRIu32 "\n\n", dwGlobalLabel);
		fprintf(fp, "memoryRead%" PRIu32 ":\n", dwGlobalLabel);
	
		if (pszTarget[0] == 'b' && pszTarget[1] == 'y' && pszTarget[2] == 't')
		{
			fprintf(fp, "		mov	di, dx\n");
			fprintf(fp, "		mov	dl, [ebp + e%s]\n", pszAddress);
			fprintf(fp, "		mov	%s, dl\n", pszTarget);
			fprintf(fp, "		mov	dx, di\n");
		}
		else
			fprintf(fp, "		mov	%s, [ebp + e%s]	; Get our data\n\n", pszTarget, pszAddress);
	
		fprintf(fp, "readExit%" PRIu32 ":\n", dwGlobalLabel);
		fprintf(fp, "		mov	edi, [cyclesRemaining]\n");
	
		dwGlobalLabel++;
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				psMemRead = cpu.z80MemRead;	/* Beginning of our handler */\n");
		fprintf(fp, "				while (psMemRead->lowAddr != 0xffffffff)\n");
		fprintf(fp, "				{\n");
		fprintf(fp, "					if ((%s >= psMemRead->lowAddr) && (%s <= psMemRead->highAddr))\n", pszAddress, pszAddress);
		fprintf(fp, "					{\n");
		fprintf(fp, "						cpu.z80pc = (UINTPTR) pbPC - (UINTPTR) cpu.z80Base;\n");
		fprintf(fp, "						if (psMemRead->memoryCall)\n");
		fprintf(fp, "						{\n");
		fprintf(fp, "							%s = psMemRead->memoryCall(%s, psMemRead);\n", pszTarget, pszAddress);
		fprintf(fp, "						}\n");
		fprintf(fp, "						else\n");
		fprintf(fp, "						{\n");
		fprintf(fp, "							%s = *((UINT8 *) psMemRead->pUserArea + (%s - psMemRead->lowAddr));\n", pszTarget, pszAddress);
		fprintf(fp, "						}\n");
		fprintf(fp, "						psMemRead = NULL;\n");
		fprintf(fp, "						break;\n");
		fprintf(fp, "					}\n");
		fprintf(fp, "					++psMemRead;\n");
		fprintf(fp, "				}\n\n");
		fprintf(fp, "				if (psMemRead)\n");
		fprintf(fp, "				{\n");
		fprintf(fp, "					%s = cpu.z80Base[%s];\n", pszTarget, pszAddress);
		fprintf(fp, "				}\n\n");
	}
	else
	{
		abort();
	}
}


void ReadWordFromMemory(INT8 *pszAddress, INT8 *pszTarget)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp, "		mov	[cyclesRemaining], edi\n");
		fprintf(fp, "		mov	edi, [_z80MemRead]	; Point to the read array\n\n");
		fprintf(fp, "checkLoop%" PRIu32 ":\n", dwGlobalLabel);
		fprintf(fp, "		cmp	[edi], word 0ffffh ; End of the list?\n");
		fprintf(fp, "		je		memoryRead%" PRIu32 "\n", dwGlobalLabel);
		fprintf(fp, "		cmp	%s, [edi]	; Are we smaller?\n", pszAddress);
		fprintf(fp, "		jb		nextAddr%" PRIu32 "		; Yes, go to the next address\n", dwGlobalLabel);
		fprintf(fp, "		cmp	%s, [edi+4]	; Are we bigger?\n", pszAddress);
		fprintf(fp, "		jbe	callRoutine%" PRIu32 "\n\n", dwGlobalLabel);
		fprintf(fp, "nextAddr%" PRIu32 ":\n", dwGlobalLabel);
		fprintf(fp, "		add	edi, 10h		; Next structure!\n");
		fprintf(fp, "		jmp	short checkLoop%" PRIu32 "\n\n", dwGlobalLabel);
		fprintf(fp, "callRoutine%" PRIu32 ":\n", dwGlobalLabel);

		if (strcmp(pszAddress, "dx") != 0)
			fprintf(fp, "		mov	dx, %s	; Get our address\n", pszAddress);

		if (strcmp(pszTarget, "ax") != 0)
			fprintf(fp, "		push	ax		; Save this for later\n");

		fprintf(fp, "		push	dx		; Save address\n");
		fprintf(fp, "		call	ReadMemoryByte	; Standard read routine\n");
		fprintf(fp, "		pop	dx		; Restore our address\n");

		fprintf(fp, "		inc	dx		; Next byte, please\n");

		fprintf(fp, "		push	ax		; Save returned byte\n");
		fprintf(fp, "		call	ReadMemoryByte	; Standard read routine\n");
		fprintf(fp, "		xchg	ah, al	; Swap for endian's sake\n");
		fprintf(fp, "		pop	dx	; Restore LSB\n");

		fprintf(fp, "		mov	dh, ah	; Our word is now in DX\n");

		// DX Now has our data and our address is toast
	
		if (strcmp(pszTarget, "ax") != 0)
		{
			fprintf(fp, "		pop	ax		; Restore this\n");
	
			if (strcmp(pszTarget, "dx") != 0)
			{
				fprintf(fp, "		mov	%s, dx	; Store our word\n", pszTarget);
			}
		}
		else
			fprintf(fp, "		mov	ax, dx\n");

		if (strcmp(pszTarget, "ax") == 0)
		{
			fprintf(fp, "		xchg	ah, al\n");
		}
	
		fprintf(fp, "		jmp	readExit%" PRIu32 "\n\n", dwGlobalLabel);
	
		fprintf(fp, "memoryRead%" PRIu32 ":\n", dwGlobalLabel);
	
		if (strlen(pszTarget) == 2)
		{
			fprintf(fp, "		mov	%s, [ebp + e%s]\n", pszTarget, pszAddress);
			if (strcmp(pszTarget, "ax") == 0)
			{
				fprintf(fp, "		xchg	ah, al\n");
			}
		}
		else
		{
			fprintf(fp, "		mov	dx, [ebp + e%s]\n", pszAddress);
			fprintf(fp, "		mov	%s, dx\n", pszTarget);
		}
	
		fprintf(fp, "readExit%" PRIu32 ":\n", dwGlobalLabel);
		fprintf(fp, "		mov	edi, [cyclesRemaining]\n");
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				psMemRead = cpu.z80MemRead;	/* Beginning of our handler */\n");
		fprintf(fp, "				while (psMemRead->lowAddr != 0xffffffff)\n");
		fprintf(fp, "				{\n");
		fprintf(fp, "					if ((%s >= psMemRead->lowAddr) && (%s <= psMemRead->highAddr))\n", pszAddress, pszAddress);
		fprintf(fp, "					{\n");
		fprintf(fp, "						cpu.z80pc = (UINTPTR) pbPC - (UINTPTR) cpu.z80Base;\n");
		fprintf(fp, "						if (psMemRead->memoryCall)\n");
		fprintf(fp, "						{\n");
		fprintf(fp, "							%s = psMemRead->memoryCall(%s, psMemRead);\n", pszTarget, pszAddress);
		fprintf(fp, "							%s |= (UINT32) ((UINT32) psMemRead->memoryCall(%s + 1, psMemRead) << 8);\n", pszTarget, pszAddress);
		fprintf(fp, "						}\n");
		fprintf(fp, "						else\n");
		fprintf(fp, "						{\n");
		fprintf(fp, "							%s = *((UINT8 *) psMemRead->pUserArea + (%s - psMemRead->lowAddr));\n", pszTarget, pszAddress);
		fprintf(fp, "							%s |= (UINT32) ((UINT32) *((UINT8 *) psMemRead->pUserArea + (%s - psMemRead->lowAddr + 1)) << 8);\n", pszTarget, pszAddress);
		fprintf(fp, "						}\n");
		fprintf(fp, "						psMemRead = NULL;\n");
		fprintf(fp, "						break;\n");
		fprintf(fp, "					}\n");
		fprintf(fp, "					++psMemRead;\n");
		fprintf(fp, "				}\n\n");
		fprintf(fp, "				if (psMemRead)\n");
		fprintf(fp, "				{\n");
		fprintf(fp, "					%s = cpu.z80Base[%s];\n", pszTarget, pszAddress);
		fprintf(fp, "					%s |= (UINT32) ((UINT32) cpu.z80Base[%s + 1] << 8);\n", pszTarget, pszAddress);
		fprintf(fp, "				}\n\n");
	}
	else
	{
		abort();
	}

	dwGlobalLabel++;
}


void ReadValueFromIo(INT8 *pszIoAddress, INT8 *pszTarget)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp, "		mov	[cyclesRemaining], edi\n");
		fprintf(fp, "		mov	edi, [_z80IoRead]	; Point to the read array\n\n");
		fprintf(fp, "checkLoop%" PRIu32 ":\n", dwGlobalLabel);
		fprintf(fp, "		cmp	[edi], word 0ffffh ; End of the list?\n");
		fprintf(fp, "		je		ioRead%" PRIu32 "\n", dwGlobalLabel);
		fprintf(fp, "		cmp	%s, [edi]	; Are we smaller?\n", pszIoAddress);
		fprintf(fp, "		jb		nextAddr%" PRIu32 "		; Yes, go to the next address\n", dwGlobalLabel);
		fprintf(fp, "		cmp	%s, [edi+2]	; Are we bigger?\n", pszIoAddress);
		fprintf(fp, "		jbe	callRoutine%" PRIu32 "\n\n", dwGlobalLabel);
		fprintf(fp, "nextAddr%" PRIu32 ":\n", dwGlobalLabel);
		fprintf(fp, "		add	edi, 0ch		; Next structure!\n");
		fprintf(fp, "		jmp	short checkLoop%" PRIu32 "\n\n", dwGlobalLabel);
		fprintf(fp, "callRoutine%" PRIu32 ":\n", dwGlobalLabel);

		if (strcmp(pszIoAddress, "dx") != 0)
			fprintf(fp, "		mov	dx, %s	; Get our address\n", pszIoAddress);

		fprintf(fp, "		call	ReadIOByte	; Standard read routine\n");

		// Yes, these are intentionally reversed!
	
		if (strcmp(pszTarget, "al") == 0)
			fprintf(fp, "		mov	[_z80af], al	; Save our new accumulator\n");
		else
		if (strcmp(pszTarget, "ah") == 0)
			fprintf(fp, "		mov	[_z80af + 1], ah	; Save our new flags\n");
		else
		if (strcmp(pszTarget, "dl") == 0)
			fprintf(fp, "		mov	[_z80de], al	; Put it in E\n");
		else
		if (strcmp(pszTarget, "dh") == 0)
			fprintf(fp, "		mov	[_z80de + 1], al ; Put it in D\n");
		else
		if (strcmp(pszTarget, "*dl") == 0)
			fprintf(fp, "		mov	dl, al	; Put it in DL for later consumption\n");
		else
			fprintf(fp, "		mov	%s, al	; Put our returned value here\n", pszTarget);

		// And are properly restored HERE:

		fprintf(fp, "		mov	ax, [_z80af]	; Get our AF back\n");

		// Restore registers here...

		fprintf(fp, "		jmp	short readExit%" PRIu32 "\n\n", dwGlobalLabel);
		fprintf(fp, "ioRead%" PRIu32 ":\n", dwGlobalLabel);
	
		if (strcmp(pszTarget, "*dl") == 0)
			fprintf(fp, "		mov	dl, 0ffh	; An unreferenced read\n");
		else
			fprintf(fp, "		mov	%s, 0ffh	; An unreferenced read\n", pszTarget);
		fprintf(fp, "readExit%" PRIu32 ":\n", dwGlobalLabel);
		fprintf(fp, "		mov	edi, [cyclesRemaining]\n");
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				psIoRead = cpu.z80IoRead;	/* Beginning of our handler */\n");
		fprintf(fp, "				while (psIoRead->lowIoAddr != 0xffff)\n");
		fprintf(fp, "				{\n");
		fprintf(fp, "					if ((%s >= psIoRead->lowIoAddr) && (%s <= psIoRead->highIoAddr))\n", pszIoAddress, pszIoAddress);
		fprintf(fp, "					{\n");
		fprintf(fp, "						cpu.z80pc = (UINTPTR) pbPC - (UINTPTR) cpu.z80Base;\n");
		fprintf(fp, "						%s = psIoRead->IOCall(%s, psIoRead);\n", pszTarget, pszIoAddress);
		fprintf(fp, "						psIoRead = NULL;\n");
		fprintf(fp, "						break;\n");
		fprintf(fp, "					}\n");
		fprintf(fp, "					++psIoRead;\n");
		fprintf(fp, "				}\n\n");
		fprintf(fp, "				if (psIoRead)\n");
		fprintf(fp, "				{\n");
		fprintf(fp, "					%s = 0xff; /* Unclaimed I/O read */\n", pszTarget);
		fprintf(fp, "				}\n\n");
	}
	else
	{
		abort();
	}

	dwGlobalLabel++;
}

// Basic instruction set area

void MiscHandler(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if (dwOpcode == 0xe3)
		{
			if (bThroughCallHandler)
			{
				fprintf(fp, "		call	PopWord\n");
				fprintf(fp, "		xchg	bx, [_wordval]\n");
				fprintf(fp, "		call	PushWord\n");
			}
			else
			{
				fprintf(fp, "		mov	dx, word [_z80sp]\n");
				fprintf(fp, "		xchg	bx, [ebp+edx]\n");
				fprintf(fp, "		xor	edx, edx\n");
			}
		}

		if (dwOpcode == 0x2a)
		{
			fprintf(fp, "		mov	dx, [esi]	; Get address to load\n");
			fprintf(fp, "		add	esi, 2	; Skip over it so we don't execute it\n");

			ReadWordFromMemory("dx", "bx");
			fprintf(fp, "		xor	edx, edx\n");
		}

		if (dwOpcode == 0xfb)
		{
			fprintf(fp, "		or		dword [_z80iff], IFF1	; Indicate interrupts are enabled now\n");
			fprintf(fp, "		sub	edi, 4	; Takes 4 cycles!\n");
			fprintf(fp, "		mov	[dwEITiming], edi	; Snapshot our current timing\n");
			fprintf(fp, "		mov	[bEIExit], byte 1	; Indicate we're exiting because of an EI\n");
			fprintf(fp, "		xor	edi, edi	; Force next instruction to exit\n");
			fprintf(fp, "		mov	dl, byte [esi]	; Get our next instruction\n");
			fprintf(fp, "		inc	esi	; Next PC\n");
			fprintf(fp, "		jmp	dword [z80regular+edx*4]\n\n");
		}

		if (dwOpcode == 0xf9)
			fprintf(fp, "		mov	word [_z80sp], bx\n");

		if (dwOpcode == 0xd9)
		{
			fprintf(fp, "		mov	[cyclesRemaining], edi\n");
			fprintf(fp, "		mov	di, [_z80de]\n");
			fprintf(fp, "		xchg	cx, [_z80bcprime]\n");
			fprintf(fp, "		xchg	di, [_z80deprime]\n");
			fprintf(fp, "		xchg	bx, [_z80hlprime]\n");
			fprintf(fp, "		mov	[_z80de], di\n");
			fprintf(fp, "		mov	edi, [cyclesRemaining]\n");
		}

		if (dwOpcode == 0x76)
		{
			fprintf(fp, "		mov	dword [_z80halted], 1	; We've halted the chip!\n");
	
			if (FALSE == bNoTiming)
			{
				fprintf(fp, "		xor	edi, edi\n");
				fprintf(fp, "		mov	[cyclesRemaining], edi\n");
			}
	
			fprintf(fp, "		jmp	noMoreExec\n");
			return;
		}
	
		if (dwOpcode == 0x3f)
		{
			fprintf(fp, "		mov	dl, ah\n");
			fprintf(fp, "		and	dl, 01h\n");
			fprintf(fp, "		shl	dl, 4\n");
			fprintf(fp, "		xor	ah, 01h\n");
			fprintf(fp, "		and	ah, 0edh\n");
			fprintf(fp, "		or	ah, dl\n");
		}
	
		if (dwOpcode == 0x37)
		{
			fprintf(fp, "		or	ah, 1\n");
			fprintf(fp, "		and	ah,0edh\n");
		}
	
		if (dwOpcode == 0x27)
		{
			fprintf(fp, "		mov	dh, ah\n");
			fprintf(fp, "		and	dh, 02ah\n");
			fprintf(fp, "		test	ah, 02h	; Were we doing a subtraction?\n");
			fprintf(fp, "		jnz	handleNeg ; Nope!\n");
			fprintf(fp, "		sahf\n");
			fprintf(fp, "		daa\n");
			fprintf(fp, "		lahf\n");
			fprintf(fp, "		jmp	short endDaa\n");
			fprintf(fp, "handleNeg:\n");
			fprintf(fp, "		sahf\n");
			fprintf(fp, "		das\n");
			fprintf(fp, "		lahf\n");
			fprintf(fp, "endDaa:\n");
			fprintf(fp, "		and	ah, 0d5h\n");
			fprintf(fp, "		or	ah, dh\n");
			fprintf(fp, "		xor	edx, edx\n");
		}
	
		if (dwOpcode == 0x08)
		{
			fprintf(fp, "		xchg	ah, al\n");
			fprintf(fp, "		xchg	ax, [_z80afprime]\n");
			fprintf(fp, "		xchg	ah, al\n");
		}
	
		if (dwOpcode == 0x07)
		{
			fprintf(fp, "		sahf\n");
			fprintf(fp, "		rol	al, 1\n");
			fprintf(fp, "		lahf\n");
			fprintf(fp, "		and	ah, 0edh\n");
		}
	
		if (dwOpcode == 0x0f)
		{
			fprintf(fp, "		sahf\n");
			fprintf(fp, "		ror	al, 1\n");
			fprintf(fp, "		lahf\n");
			fprintf(fp, "		and	ah, 0edh\n");
		}
	
		if (dwOpcode == 0xe9)
		{
			fprintf(fp, "		mov	si, bx\n");
			fprintf(fp, "		and	esi, 0ffffh\n");
			fprintf(fp, "		add	esi, ebp\n");
		}
	
		if (dwOpcode == 0xeb)
			fprintf(fp, "		xchg	[_z80de], bx	; Exchange DE & HL\n");
	
		if (dwOpcode == 0x2f)
		{
			fprintf(fp, "		not	al\n");
			fprintf(fp, "		or	ah, 012h	; N And H are now on!\n");
		}
	
		if (dwOpcode == 0x10)	// DJNZ
		{
			fprintf(fp, "		mov	dl, [esi] ; Get our relative offset\n");
			fprintf(fp, "		inc	esi	; Next instruction, please!\n");
			fprintf(fp, "		dec	ch	; Decrement B\n");
			fprintf(fp, "		jz	noJump	; Don't take the jump if it's done!\n");
			fprintf(fp, "; Otherwise, take the jump\n");
	
			fprintf(fp, "		sub	edi, 5\n");
	
			fprintf(fp, "		xchg	eax, edx\n");
			fprintf(fp, "		cbw\n");
			fprintf(fp, "		xchg 	eax, edx\n");
			fprintf(fp, "		sub	esi, ebp\n");
			fprintf(fp, "		add	si, dx\n");
			fprintf(fp, "		add	esi, ebp\n");
			fprintf(fp, "noJump:\n");
			fprintf(fp, "		xor	edx, edx\n");
		}
	
		if (dwOpcode == 0x3a)	// LD A,(xxxx)
		{
			fprintf(fp, "		mov	dx, [esi]	; Get our address\n");
			fprintf(fp, "		add	esi, 2		; Skip past the address\n");
			ReadValueFromMemory("dx", "al");
			fprintf(fp, "		xor	edx, edx	; Make sure we don't hose things\n");
		}
	
		if (dwOpcode == 0xf3)  	// DI
		{
			fprintf(fp, "		and	dword [_z80iff], (~IFF1)	; Not in an interrupt\n");
		}
	
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (dwOpcode == 0x76)		// HALT!
		{
			fprintf(fp, "				cpu.z80halted = 1;\n");
			fprintf(fp, "				dwElapsedTicks += sdwCyclesRemaining;\n");

			fprintf(fp, "				sdwCyclesRemaining = 0;\n");
		}
		else
		if (dwOpcode == 0x2f)		// CPL
		{
			fprintf(fp, "				cpu.z80A ^= 0xff;\n");
			fprintf(fp, "				cpu.z80F |= (Z80_FLAG_NEGATIVE | Z80_FLAG_HALF_CARRY);\n");
		}
		else
		if (dwOpcode == 0xd9)		// EXX
		{
			fprintf(fp, "				dwTemp = cpu.z80DE;\n");
			fprintf(fp, "				cpu.z80DE = cpu.z80deprime;\n");
			fprintf(fp, "				cpu.z80deprime = dwTemp;\n");

			fprintf(fp, "				dwTemp = cpu.z80BC;\n");
			fprintf(fp, "				cpu.z80BC = cpu.z80bcprime;\n");
			fprintf(fp, "				cpu.z80bcprime = dwTemp;\n");

			fprintf(fp, "				dwTemp = cpu.z80HL;\n");
			fprintf(fp, "				cpu.z80HL = cpu.z80hlprime;\n");
			fprintf(fp, "				cpu.z80hlprime = dwTemp;\n");
		}
		else
		if (dwOpcode == 0xf9)		// LD SP, HL
		{
			fprintf(fp, "				cpu.z80sp = cpu.z80HL;\n");
		}
		else
		if (dwOpcode == 0x27)		// DAA
		{
			fprintf(fp, "				dwAddr = (((cpu.z80F & Z80_FLAG_CARRY) | \n");
			fprintf(fp, "						((cpu.z80F & Z80_FLAG_HALF_CARRY) >> 3) | \n");
			fprintf(fp, "						((cpu.z80F & Z80_FLAG_NEGATIVE) << 1)) << 8) | cpu.z80A;\n");
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_HALF_CARRY | Z80_FLAG_ZERO | Z80_FLAG_SIGN);\n");
			fprintf(fp, "				cpu.z80F |= (wDAATable[dwAddr] >> 8);\n");
			fprintf(fp, "				cpu.z80A = wDAATable[dwAddr] & 0xff;\n");
		}
		else
		if (dwOpcode == 0x2a)
		{
			fprintf(fp, "				dwAddr = *pbPC++;\n");
			fprintf(fp, "				dwAddr |= ((UINT32) *pbPC++ << 8);\n");
			ReadWordFromMemory("dwAddr", "cpu.z80HL");
		}
		else
		if (dwOpcode == 0xe3)		// EX (SP), HL
		{
			ReadWordFromMemory("cpu.z80sp", "dwAddr");
			WriteWordToMemory("cpu.z80sp", "cpu.z80HL");
			fprintf(fp, "				cpu.z80HL = dwAddr;\n");
		}
		else
		if (dwOpcode == 0xe9)		// JP (HL)
		{
			fprintf(fp, "				pbPC = cpu.z80Base + cpu.z80HL;\n");
		}
		else
		if (0x08 == dwOpcode)		// EX AF, AF'
		{
			fprintf(fp, "				dwAddr = (UINT32) cpu.z80AF;\n");
			fprintf(fp, "				cpu.z80AF = cpu.z80afprime;\n");
			fprintf(fp, "				cpu.z80afprime = dwAddr;\n");
		}
		else
		if (0xeb == dwOpcode)		// EX DE, HL
		{
			fprintf(fp, "				dwAddr = cpu.z80DE;\n");
			fprintf(fp, "				cpu.z80DE = cpu.z80HL;\n");
			fprintf(fp, "				cpu.z80HL = dwAddr;\n");
		}
		else
		if (0x10 == dwOpcode)		// DJNZ
		{
			fprintf(fp, "				sdwAddr = (INT8) *pbPC++;	/* Get LSB first */\n");
			fprintf(fp, "				if (--cpu.z80B)\n");
			fprintf(fp, "				{\n");
			fprintf(fp, "					dwElapsedTicks += 5;	/* 5 More for jump taken */\n");
			fprintf(fp, "					cpu.z80pc = (UINTPTR) pbPC - (UINTPTR) cpu.z80Base;\n");
			fprintf(fp, "					sdwAddr = (sdwAddr + (INT32) cpu.z80pc) & 0xffff;\n");
			fprintf(fp, "					pbPC = cpu.z80Base + sdwAddr;	/* Normalize the address */\n");
			fprintf(fp, "				}\n");
		}
		else
		if (0x37 == dwOpcode)	// SCF
		{
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_HALF_CARRY | Z80_FLAG_NEGATIVE);\n");
			fprintf(fp, "				cpu.z80F |= Z80_FLAG_CARRY;\n");
		}
		else
		if (0x3f == dwOpcode)	// CCF
		{
			fprintf(fp, "				bTemp = (cpu.z80F & Z80_FLAG_CARRY) << 4;\n");
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_HALF_CARRY | Z80_FLAG_NEGATIVE);\n");
			fprintf(fp, "				cpu.z80F ^= Z80_FLAG_CARRY;\n");
		}
		else
		if (0x07 == dwOpcode)	// RLCA
		{
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_HALF_CARRY);\n");
			fprintf(fp, "				cpu.z80F |= (cpu.z80A >> 7);\n");
			fprintf(fp, "				cpu.z80A = (cpu.z80A << 1) | (cpu.z80A >> 7);\n");
		}
		else
		if (0x0f == dwOpcode)	// RRCA
		{
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_HALF_CARRY);\n");
			fprintf(fp, "				cpu.z80F |= (cpu.z80A & Z80_FLAG_CARRY);\n");
			fprintf(fp, "				cpu.z80A = (cpu.z80A >> 1) | (cpu.z80A << 7);\n");
		}
		else
		if (0x3a == dwOpcode)	// LD A, (xxxxh)
		{
			fprintf(fp, "				dwTemp = *pbPC++;\n");
			fprintf(fp, "				dwTemp |= (((UINT32) *pbPC++) << 8);\n");
			ReadValueFromMemory("dwTemp", "cpu.z80A");
		}
		else
		if (0xf3 == dwOpcode)	// DI
		{
			fprintf(fp, "				cpu.z80iff &= (~IFF1);\n");
		}
		else
		if (0xfb == dwOpcode)	// EI
		{
			fprintf(fp, "				cpu.z80iff |= IFF1;\n");
		}
		else
		if (0x00 == dwOpcode)	// NOP
		{
			fprintf(fp, "				/* Intentionally not doing anything - NOP! */\n");
		}
		else
		{
			InvalidInstructionC(1);
		}
	}
	else
	{
		abort();
	}
	
}

void LdRegPairImmediate(UINT32 dwOpcode)
{
	UINT8 bOp = 0;

	bOp = (dwOpcode >> 4) & 0x3;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if (bOp == 0)
			fprintf(fp, "		mov	cx, [esi]	; Get our immediate value of BC\n");
		else
		if (bOp == 2)
			fprintf(fp, "		mov	bx, [esi]	; Get our immediate value of HL\n");
		else
		if (bOp == 1)
		{
			fprintf(fp, "		mov	dx, [esi]	; Get our immediate value of DE\n");
			fprintf(fp, "		mov	word [_z80de], dx ; Store DE\n");
			fprintf(fp, "		xor	edx, edx\n");
		}
		else
		if (bOp == 3)
		{
			fprintf(fp, "		mov	dx, [esi]	; Get our immediate value of SP\n");
			fprintf(fp, "		mov	word [_z80sp], dx	; Store it!\n");
			fprintf(fp, "		xor	edx, edx\n");
		}
	
		fprintf(fp, "		add	esi, 2\n");
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				%s = *pbPC++;	/* LSB First */\n", pbRegPairC[bOp]);
		fprintf(fp, "				%s |= (((UINT32) *pbPC++ << 8));	/* Now the MSB */\n", pbRegPairC[bOp]);
	}
	else
	{
		abort();
	}
}

void LdRegpairPtrByte(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if (dwOpcode == 0x36)	// Immediate into (HL)
			WriteValueToMemory("bx", "[esi]");

		if (dwOpcode == 0x12)
			WriteValueToMemory("[_z80de]", "al");	// (DE), A

		if (dwOpcode == 0x2)		// (BC), A
			WriteValueToMemory("cx", "al");

		if (dwOpcode >= 0x70 && dwOpcode < 0x78)
			WriteValueToMemory("bx", pbMathReg[dwOpcode & 0x07]);

		fprintf(fp, "		xor	edx, edx\n");
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (dwOpcode == 0x36)
			WriteValueToMemory("cpu.z80HL", "*pbPC++");

		if (dwOpcode == 0x12)
			WriteValueToMemory("cpu.z80DE", "cpu.z80A");

		if (dwOpcode == 0x02)
			WriteValueToMemory("cpu.z80BC", "cpu.z80A");

		if (dwOpcode >= 0x70 && dwOpcode < 0x78)
			WriteValueToMemory("cpu.z80HL", pbMathRegC[dwOpcode & 0x07]);
	}
	else
	{
		abort();
	}
}

void MathOperation(UINT32 dwOrgOpcode)
{
	UINT8 bRegister;
	UINT32 dwOpcode;
	INT8 tempstr[150];

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOrgOpcode);

		dwOpcode = dwOrgOpcode;
		bRegister = dwOpcode & 0x07;
		dwOpcode &= 0xf8;

		if (dwOpcode == 0x80)
			strcpy(tempstr, "add");
		if (dwOpcode == 0x88)
			strcpy(tempstr, "adc");
		if (dwOpcode == 0x90)
			strcpy(tempstr, "sub");
		if (dwOpcode == 0x98)
			strcpy(tempstr, "sbb");
		if (dwOpcode == 0xa0)
			strcpy(tempstr, "and");
		if (dwOpcode == 0xa8)
			strcpy(tempstr, "xor");
		if (dwOpcode == 0xb0)
			strcpy(tempstr, "or");
		if (dwOpcode == 0xb8)
			strcpy(tempstr, "cmp");
	
		// Let's see if we have to deal with (HL) or #xxh
	
		if (bRegister == 0x6)
		{
			// We have to deal with (HL)
	
			ReadValueFromMemory("bx", "dl");
		}
	
		if (bRegister != 0x06 && bRegister < 0xff)
		{
			fprintf(fp, "		sahf\n");
			fprintf(fp, "		%s	al, %s\n", tempstr, pbMathReg[bRegister]);
			fprintf(fp, "		lahf\n");
		}
		else	// If it's (HL)....
		{
			fprintf(fp, "		sahf\n");
			fprintf(fp, "		%s	al, dl\n", tempstr);
			fprintf(fp, "		lahf\n");
		}
	
		if (dwOpcode != 0xa8 && dwOpcode != 0xa0 && dwOpcode != 0xb0)
			SetOverflow();
	
		if (dwOpcode == 0xa8)
			fprintf(fp, "		and	ah, 0ech	; Only these flags matter!\n");
	
		if (dwOpcode == 0xa0)
		{
			fprintf(fp, "		and	ah, 0ech	; Only these flags matter!\n");
			fprintf(fp, "		or	ah, 010h	; Half carry gets set\n");
		}
	
		if (dwOpcode == 0xb0)
			fprintf(fp, "		and	ah, 0ech ; No H, N, or C\n");
	
		if (dwOpcode == 0xb8)
			fprintf(fp, "		or	ah, 02h	; Set N for compare!\n");
	
		if (dwOpcode == 0x80 || dwOpcode == 0x88)
			fprintf(fp, "		and	ah, 0fdh ; No N!\n");
	
		if (dwOpcode == 0x90 || dwOpcode == 0x98)
			fprintf(fp, "		or	ah, 02h	; N Gets set!\n");

		if (bRegister == 0x6)
			fprintf(fp, "		xor	edx, edx	; Zero this...\n");
	
		FetchNextInstruction(dwOrgOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		dwOpcode = dwOrgOpcode;
		bRegister = dwOpcode & 0x07;
		dwOpcode &= 0xf8;

		if (6 == bRegister)		// Deal with (HL)
		{
			ReadValueFromMemory("cpu.z80HL", "bTemp");
		}

		if (dwOpcode == 0xa0)
		{
			fprintf(fp, "				cpu.z80A &= %s;\n", pbMathRegC[bRegister]);
		}
		else
		if (dwOpcode == 0xa8)
		{
			fprintf(fp, "				cpu.z80A ^= %s;\n", pbMathRegC[bRegister]);
		}
		else
		if (dwOpcode == 0xb0)
		{
			fprintf(fp, "				cpu.z80A |= %s;\n", pbMathRegC[bRegister]);
		}
		else
		if (dwOpcode == 0xb8)
		{
			// Don't do anything. We just do flags!
		}
		else
		if (dwOpcode == 0x88)		// ADC
		{
			fprintf(fp, "				bTemp2 = cpu.z80A + %s + (cpu.z80F & Z80_FLAG_CARRY);\n", pbMathRegC[bRegister]);
		}
		else
		if (dwOpcode == 0x90)		// SUB
		{
			fprintf(fp, "				bTemp2 = cpu.z80A - %s;\n", pbMathRegC[bRegister]);
		}										  
		else
		if (dwOpcode == 0x80)		// ADD
		{
			fprintf(fp, "				bTemp2 = cpu.z80A + %s;\n", pbMathRegC[bRegister]);
		}
		else
		if (dwOpcode == 0x98)		// SBC
		{
			fprintf(fp, "				bTemp2 = cpu.z80A - %s - (cpu.z80F & Z80_FLAG_CARRY);\n", pbMathRegC[bRegister]);
		}
		else
		{
			InvalidInstructionC(1);
		}

		// Now do flag fixup

		if (0xb0 == dwOpcode || 0xa8 == dwOpcode)
		{
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_HALF_CARRY | Z80_FLAG_ZERO | Z80_FLAG_SIGN);\n");
			fprintf(fp, "				cpu.z80F |= bPostORFlags[cpu.z80A];\n\n");
		}

		if (0xa0 == dwOpcode)
		{
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_HALF_CARRY | Z80_FLAG_ZERO | Z80_FLAG_SIGN);\n");
			fprintf(fp, "				cpu.z80F |= bPostANDFlags[cpu.z80A];\n\n");
		}

		if (0xb8 == dwOpcode || 0x90 == dwOpcode)
		{
			SetSubFlagsSZHVC("cpu.z80A", pbMathRegC[bRegister]);

			if (0x90 == dwOpcode)
			{
				fprintf(fp, "				cpu.z80A = bTemp2;\n");
			}
		}

		if (0x80 == dwOpcode)		// Add fixup
		{
			SetAddFlagsSZHVC("cpu.z80A", pbMathRegC[bRegister]);
			fprintf(fp, "				cpu.z80A = bTemp2;\n");
		}

		if (0x88 == dwOpcode)		// Adc fixup
		{
			SetAdcFlagsSZHVC("cpu.z80A", pbMathRegC[bRegister]);
			fprintf(fp, "				cpu.z80A = bTemp2;\n");
		}

		if (0x98 == dwOpcode)		// Sbc fixup
		{
			SetSbcFlagsSZHVC("cpu.z80A", pbMathRegC[bRegister]);
			fprintf(fp, "				cpu.z80A = bTemp2;\n");
		}
	}
	else
	{
		abort();
	}
}

void RegIntoMemory(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		fprintf(fp, "		mov	dx, [esi]	; Get our address to write to\n");
		fprintf(fp, "		add	esi, 2		; Next address, please...\n");

		if (0x32 == dwOpcode)		// LD (xxxx), A
			WriteValueToMemory("dx", "al");
		if (0x22 == dwOpcode)		// LD (xxxx), HL
		{
			WriteWordToMemory("dx", "bx");
		}

		fprintf(fp, "		xor	edx, edx	; Zero our upper byte\n");

		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				dwTemp = *pbPC++;\n");
		fprintf(fp, "				dwTemp |= ((UINT32) *pbPC++ << 8);\n");

		if (0x32 == dwOpcode)
			WriteValueToMemory("dwTemp", "cpu.z80A");
		if (0x22 == dwOpcode)
			WriteWordToMemory("dwTemp", "cpu.z80HL");

		return;
	}
	else
	{
		abort();
	}
}

void JpHandler(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if (0xc3 == dwOpcode)	// If it's a straight jump...
		{
			fprintf(fp, "		mov	si, [esi]	; Get our new address\n");
			fprintf(fp, "		and	esi, 0ffffh	; Only the lower 16 bits\n");
			fprintf(fp, "		add	esi, ebp		; Our new address!\n");
		}
		else	// It's a conditional handler...
		{
			fprintf(fp, "		sahf		; Restore our flags\n");
			fprintf(fp, "		j%s	takeJump%" PRIu32 "	; We're going to take a jump\n", pbFlags[(dwOpcode >> 3) & 0x07], dwGlobalLabel);
			fprintf(fp, "		add	esi, 2		; Skip past the address\n");
			fprintf(fp, "		jmp	short nextInst%" PRIu32 "	 ; Go execute the next instruction\n", dwGlobalLabel);
			fprintf(fp, "takeJump%" PRIu32 ":\n", dwGlobalLabel);
	
			fprintf(fp, "		mov	si, [esi]	; Get our new offset\n");
			fprintf(fp, "		and	esi, 0ffffh	; Only the lower WORD is valid\n");
			fprintf(fp, "		add	esi, ebp		; Our new address!\n");
			fprintf(fp, "nextInst%" PRIu32 ":\n", dwGlobalLabel);
			++dwGlobalLabel;
		}
	
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "					dwAddr = *pbPC++;	/* Get LSB first */\n");
		fprintf(fp, "					dwAddr |= ((UINT32) *pbPC++ << 8); /* Get MSB last */\n");

		if (0xc3 != dwOpcode)
		{
			fprintf(fp, "				if %s\n", pbFlagsC[(dwOpcode >> 3) & 0x07]);
			fprintf(fp, "				{\n");
			fprintf(fp, "					pbPC = cpu.z80Base + dwAddr;	/* Normalize the address */\n");
			fprintf(fp, "				}\n");
		}
		else		// Regular jump here
		{
			fprintf(fp, "				pbPC = cpu.z80Base + dwAddr;	/* Normalize the address */\n");
		}
	}
	else
	{
		abort();
	}
}

void LdRegImmediate(UINT32 dwOpcode)
{
	UINT8 bOp;

	bOp = (dwOpcode >> 3) & 0x7;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);


		if (bOp != 2 && bOp != 3)
			fprintf(fp, "		mov	%s, [esi]	; Get our immediate value\n", pbMathReg[bOp]);
		else
		{
			fprintf(fp, "		mov	dl, [esi]	; Get our immediate value\n");
			fprintf(fp, "		mov	%s, dl	; Store our new value\n", pbMathReg[bOp]);
		}
	
		fprintf(fp, "		inc	esi\n");
	
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				%s = *pbPC++;	/* Get immediate byte into register */\n", pbMathRegC[bOp]);
	}
	else
	{
		abort();
	}
}

void IncRegister(UINT32 dwOpcode)
{
	UINT32 dwOpcode1 = 0;

	dwOpcode1 = (dwOpcode >> 3) & 0x07;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		fprintf(fp, "		sahf\n");
		fprintf(fp,     "		inc	%s\n", pbMathReg[dwOpcode1]);
		fprintf(fp,     "		lahf\n");
		SetOverflow();
		fprintf(fp, "		and	ah, 0fdh	; Knock out N!\n");

		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_SIGN | Z80_FLAG_ZERO | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE);\n");
		fprintf(fp ,"				cpu.z80F |= bPostIncFlags[%s++];\n", pbMathRegC[dwOpcode1]);
	}
	else
	{
		abort();
	}
}

void DecRegister(UINT32 dwOpcode)
{
	UINT32 dwOpcode1 = 0;

	dwOpcode1 = (dwOpcode >> 3) & 0x07;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		fprintf(fp, "		sahf\n");
		fprintf(fp,     "		dec	%s\n", pbMathReg[dwOpcode1]);
		fprintf(fp,     "		lahf\n");
		SetOverflow();
		fprintf(fp, "		or	ah, 02h	; Set negative!\n");

		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_SIGN | Z80_FLAG_ZERO | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY);\n");
		fprintf(fp ,"				cpu.z80F |= bPostDecFlags[%s--];\n", pbMathRegC[dwOpcode1]);
	}
	else
	{
		abort();
	}
}

void IncDecRegpair(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if ((dwOpcode & 0x0f) == 3)	// Increment?
			fprintf(fp,     "		inc	%s\n", pbRegPairs[(dwOpcode >> 4) & 0x03]);
		else
			fprintf(fp,     "		dec	%s\n", pbRegPairs[(dwOpcode >> 4) & 0x03]);

		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if ((dwOpcode & 0x0f) == 3)	// Increment
			fprintf(fp, "				%s++;\n", pbRegPairC[(dwOpcode >> 4) & 0x03]);
		else
			fprintf(fp, "				%s--;\n", pbRegPairC[(dwOpcode >> 4) & 0x03]);
		fprintf(fp, "				%s &= 0xffff;\n", pbRegPairC[(dwOpcode >> 4) & 0x03]);
	}
	else
	{
		abort();
	}
}

void LdRegReg(UINT32 dwOpcode)
{
	UINT8 bDestination;
	UINT8 bSource;

	bDestination = (dwOpcode >> 3) & 0x07;
	bSource = (dwOpcode) & 0x07;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{

		ProcBegin(dwOpcode);
	
		if (bSource != bDestination)
		{
			if (bSource == 2 && bDestination == 3)
			{
				fprintf(fp, "		mov	dl, byte [_z80de + 1]\n");
				fprintf(fp, "		mov	[_z80de], dl\n");
			}
			else
			if (bSource == 3 && bDestination == 2)
			{
				fprintf(fp, "		mov	dl, byte [_z80de]\n");
				fprintf(fp, "		mov	[_z80de + 1], dl\n");
			}
			else
				fprintf(fp, "		mov	%s, %s\n", pbMathReg[bDestination], pbMathReg[bSource]);
		}
	
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (bDestination != bSource)
		{
			fprintf(fp, "				%s = %s;\n",
					  pbMathRegC[bDestination],
					  pbMathRegC[bSource]);
		}
	}
	else
	{
		abort();
	}
}

void MathOperationDirect(UINT32 dwOpcode)
{
	INT8 tempstr[4];

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		if (dwOpcode == 0xc6)
			strcpy(tempstr, "add");
		if (dwOpcode == 0xce)
			strcpy(tempstr, "adc");
		if (dwOpcode == 0xd6)
			strcpy(tempstr, "sub");
		if (dwOpcode == 0xde)
			strcpy(tempstr, "sbb");
		if (dwOpcode == 0xe6)
			strcpy(tempstr, "and");
		if (dwOpcode == 0xee)
			strcpy(tempstr, "xor");
		if (dwOpcode == 0xf6)
			strcpy(tempstr, "or");
		if (dwOpcode == 0xfe)
			strcpy(tempstr, "cmp");
	
		ProcBegin(dwOpcode);
	
		// Let's see if we have to deal with (HL) or #xxh
	
		fprintf(fp, "		sahf\n");
		fprintf(fp, "		%s	al, [esi]\n", tempstr);
		fprintf(fp, "		lahf\n");
	
		if (dwOpcode != 0xee && dwOpcode != 0xe6 && dwOpcode != 0xf6)
		{
			SetOverflow();
		}
	
		if (dwOpcode == 0xe6)
		{
			fprintf(fp, "		and	ah, 0ech ; Only parity, half carry, sign, zero\n");
			fprintf(fp, "		or	ah, 10h	; Half carry\n");
		}
	
		if (dwOpcode == 0xc6 || dwOpcode == 0xce)
			fprintf(fp, "		and	ah, 0fdh ; Knock out N!\n");
	
		if (dwOpcode == 0xd6 || dwOpcode == 0xde || dwOpcode == 0xfe)
			fprintf(fp, "		or	ah, 02h	; Set negative!\n");

		if (dwOpcode == 0xf6 || dwOpcode == 0xee)
			fprintf(fp, "		and	ah, 0ech	; No H, N, or C\n");
	
		fprintf(fp, "		inc	esi\n");
	
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (0xfe == dwOpcode)	// Cp
		{
			SetSubFlagsSZHVC("cpu.z80A", "*pbPC++");
		}
		else
		if (0xe6 == dwOpcode)	// And
		{
			fprintf(fp, "				cpu.z80A &= *pbPC++;\n");
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_HALF_CARRY | Z80_FLAG_ZERO | Z80_FLAG_SIGN);\n");
			fprintf(fp, "				cpu.z80F |= bPostANDFlags[cpu.z80A];\n\n");
		}
		else
		if (0xf6 == dwOpcode) 	// Or
		{
			fprintf(fp, "				cpu.z80A |= *pbPC++;\n");
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_HALF_CARRY | Z80_FLAG_ZERO | Z80_FLAG_SIGN);\n");
			fprintf(fp, "				cpu.z80F |= bPostORFlags[cpu.z80A];\n\n");
		}
		else
		if (0xc6 == dwOpcode) 	// Add
		{
			fprintf(fp, "				bTemp = *pbPC++;\n");
			SetAddFlagsSZHVC("cpu.z80A", "bTemp");
			fprintf(fp, "				cpu.z80A += bTemp;\n");
		}
		else
		if (0xce == dwOpcode) 	// Adc
		{
			fprintf(fp, "				bTemp = *pbPC++ + (cpu.z80F & Z80_FLAG_CARRY);\n");
			SetAdcFlagsSZHVC("cpu.z80A", "bTemp");
			fprintf(fp, "				cpu.z80A += bTemp;\n");
		}
		else
		if (0xd6 == dwOpcode) 	// Sub
		{
			fprintf(fp, "				bTemp = *pbPC++;\n");
			SetSubFlagsSZHVC("cpu.z80A", "bTemp");
			fprintf(fp, "				cpu.z80A -= bTemp;\n");
		}
		else
		if (0xde == dwOpcode)	// Sbc
		{
			fprintf(fp, "				bTemp = *pbPC++ + (cpu.z80F & Z80_FLAG_CARRY);\n");
			SetSbcFlagsSZHVC("cpu.z80A", "bTemp");
			fprintf(fp, "				cpu.z80A = cpu.z80A - bTemp;\n");
		}
		else
		if (0xee == dwOpcode)	// Xor
		{
			fprintf(fp, "				cpu.z80A ^= *pbPC++;\n");
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_HALF_CARRY | Z80_FLAG_ZERO | Z80_FLAG_SIGN);\n");
			fprintf(fp, "				cpu.z80F |= bPostORFlags[cpu.z80A];\n\n");
		}
		else
			InvalidInstructionC(1);
	}
	else
	{
		abort();
	}
}

// JR cc, addr

void JrHandler(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);
	
		fprintf(fp, "		sub	esi, ebp\n");
		fprintf(fp, "		and	esi, 0ffffh\n");
		fprintf(fp, "		add	esi, ebp\n");

		fprintf(fp, "		mov	dl, [esi] ; Get our relative offset\n");
		fprintf(fp, "		inc	esi	; Next instruction, please!\n");

		if (dwOpcode != 0x18)
		{
			fprintf(fp, "		sahf\n");
			fprintf(fp,     "		j%s	takeJump%" PRIu32 "\n", pbFlags[(dwOpcode >> 3) & 0x3], dwGlobalLabel);
			fprintf(fp, "		jmp	short noJumpMan%" PRIu32 "\n", dwGlobalLabel);
			fprintf(fp, "takeJump%" PRIu32 ":\n", dwGlobalLabel);

			if (FALSE == bNoTiming)
			{
				fprintf(fp, "		sub	edi, 5\n");
			}
		}
		else	// It's a JR
		{
			fprintf(fp, "		cmp	dl, 0feh	; Jump to self?\n");
			fprintf(fp, "		je		yesJrMan	; Yup! Bail out!\n");
		}
	
		fprintf(fp, "		xchg	eax, edx\n");
		fprintf(fp, "		cbw\n");
		fprintf(fp, "		xchg	eax, edx\n");
		fprintf(fp, "		sub	esi, ebp\n");
		fprintf(fp, "		add	si, dx\n");
		fprintf(fp, "		and	esi, 0ffffh	; Only the lower 16 bits\n");
		fprintf(fp, "		add	esi, ebp\n");
		fprintf(fp, "		xor	dh, dh\n");
		fprintf(fp, "noJumpMan%" PRIu32 ":\n", dwGlobalLabel++);

		FetchNextInstruction(dwOpcode);
	
		if (0x18 == dwOpcode)
		{
			fprintf(fp,"yesJrMan:\n");

			fprintf(fp, "		xor	edx, edx		; Zero me for later\n");
			fprintf(fp, "		mov	edi, edx\n");
			fprintf(fp, "		mov	[cyclesRemaining], edx\n");
			fprintf(fp, "		sub	esi, 2	; Back to the instruction again\n");
			fprintf(fp, "		jmp	noMoreExec\n\n");
		}
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				sdwAddr = (INT8) *pbPC++;	/* Get LSB first */\n");
		fprintf(fp, "				cpu.z80pc = (UINTPTR) pbPC - (UINTPTR) cpu.z80Base;\n");
		fprintf(fp, "				sdwAddr = (sdwAddr + (INT32) cpu.z80pc) & 0xffff;\n");

		if (0x18 != dwOpcode)
		{
			fprintf(fp, "				if %s\n", pbFlagsC[(dwOpcode >> 3) & 0x03]);
		}

		fprintf(fp, "				{\n");

		fprintf(fp, "				sdwCyclesRemaining -= 5;\n");

		fprintf(fp, "					pbPC = cpu.z80Base + sdwAddr;	/* Normalize the address */\n");
		fprintf(fp, "				}\n");
		
	}
	else
	{
		abort();
	}
}

void CallHandler(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if (dwOpcode != 0xcd)
		{
			fprintf(fp, "		sahf		; Restore our flags\n");
			fprintf(fp, "		j%s	takeJump%" PRIu32 "	; We're going call in this case\n", pbFlags[(dwOpcode >> 3) & 0x07], dwGlobalLabel);
			fprintf(fp, "		add	esi, 2		; Skip past the address\n");
			fprintf(fp, "		jmp	short noCallTaken%" PRIu32 "	 ; Go execute the next instruction\n", dwGlobalLabel);
			fprintf(fp, "takeJump%" PRIu32 ":\n", dwGlobalLabel);
	
			fprintf(fp, "		sub	edi, 7\n");
		}


		if (bThroughCallHandler)
		{
			fprintf(fp, "		mov	dx, [esi]	; Get our call to address\n");
			fprintf(fp, "		mov	[_z80pc], dx ; Store our new program counter\n");
			fprintf(fp, "		add	esi, 2		; Skip to our new address to be pushed\n");
			fprintf(fp, "		sub	esi, ebp		; Value to push onto the \"stack\"\n");
			fprintf(fp, "		mov	[_wordval], si	; Store our return address on the stack\n");
			fprintf(fp, "		mov	si, dx		; Our new address\n");
			fprintf(fp, "		add	esi, ebp	; And our base address\n");
			fprintf(fp, "		call	PushWord	; Go push our orgval to the stack\n");
		}
		else
		{
			fprintf(fp, "		mov	dx, [esi]	; Get our call to address\n");
			fprintf(fp, "		mov	[_z80pc], dx ; Store our new program counter\n");
			fprintf(fp, "		add	esi, 2		; Skip to our new address to be pushed\n");
			fprintf(fp, "		sub	esi, ebp		; Value to push onto the \"stack\"\n");
			fprintf(fp, "		mov	dx, word [_z80sp] ; Get the current stack pointer\n");
			fprintf(fp, "		sub	dx, 2		; Back up two bytes\n");
			fprintf(fp, "		mov	[ebp+edx], si ; PUSH It!\n");
			fprintf(fp, "		mov	word [_z80sp], dx	; Store our new stack pointer\n");
			fprintf(fp, "		mov	si, [_z80pc] ; Get our new program counter\n");
			fprintf(fp, "		add	esi, ebp		; Naturalize it!\n");
		}

		if (dwOpcode != 0xcd)
			fprintf(fp, "noCallTaken%" PRIu32 ":\n", dwGlobalLabel++);

		fprintf(fp, "		xor	edx, edx\n");
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				dwAddr = *pbPC++;	/* Get LSB first */\n");
		fprintf(fp, "				dwAddr |= ((UINT32) *pbPC++ << 8); /* Get MSB last */\n");

		if (0xcd != dwOpcode)
		{
			fprintf(fp, "				if %s\n", pbFlagsC[(dwOpcode >> 3) & 0x07]);
			fprintf(fp, "				{\n");
			fprintf(fp, "					cpu.z80pc = (UINTPTR) pbPC - (UINTPTR) cpu.z80Base;\n");
			fprintf(fp, "					pbSP = (cpu.z80Base + cpu.z80sp - 1);	/* Normalize the stack pointer */\n");
			fprintf(fp, "					*pbSP-- = cpu.z80pc >> 8;	/* MSB */\n");
			fprintf(fp, "					*pbSP = (UINT8) cpu.z80pc;	/* LSB */\n");
			fprintf(fp, "					cpu.z80sp -= 2;	/* Back our stack up */\n");
			fprintf(fp, "					pbPC = cpu.z80Base + dwAddr;	/* Normalize the address */\n");
			fprintf(fp, "				}\n");
		}
		else		// Just a regular call
		{
			fprintf(fp, "				cpu.z80pc = (UINTPTR) pbPC - (UINTPTR) cpu.z80Base;\n");
			fprintf(fp, "				pbSP = (cpu.z80Base + cpu.z80sp - 1);	/* Normalize the stack pointer */\n");
			fprintf(fp, "				*pbSP-- = cpu.z80pc >> 8;	/* LSB */\n");
			fprintf(fp, "				*pbSP = (UINT8) cpu.z80pc;	/* MSB */\n");
			fprintf(fp, "				cpu.z80sp -= 2;	/* Back our stack up */\n");
			fprintf(fp, "				pbPC = cpu.z80Base + dwAddr;	/* Normalize the address */\n");
		}
	}
	else
	{
		abort();
	}
}

void RetHandler(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if (dwOpcode != 0xc9)
		{
			fprintf(fp, "		sahf\n");
			fprintf(fp, "		j%s	takeReturn%" PRIu32 "\n", pbFlags[(dwOpcode >> 3) & 0x07], dwGlobalLabel);
			fprintf(fp, "		jmp	short retNotTaken%" PRIu32 "\n", dwGlobalLabel);
			fprintf(fp, "takeReturn%" PRIu32 ":\n", dwGlobalLabel);

			if (FALSE == bNoTiming)
			{
				fprintf(fp, "		sub	edi, byte 6\n");
			}
		}


		if (bThroughCallHandler)
		{
			fprintf(fp, "		call	PopWord\n");
			fprintf(fp, "		xor	esi, esi\n");
			fprintf(fp, "		mov	si, dx\n");
			fprintf(fp,	"		add	esi, ebp\n");
			fprintf(fp, "		xor	edx, edx\n");
		}
		else 	 
		{
			fprintf(fp, "		mov	dx, word [_z80sp]	; Get our current stack pointer\n");
			fprintf(fp, "		mov	si, [edx+ebp]	; Get our return address\n");
			fprintf(fp, "		and	esi, 0ffffh		; Only within 64K!\n");
			fprintf(fp, "		add	esi, ebp			; Add in our base address\n");
			fprintf(fp, "		add	word [_z80sp], 02h	; Remove our two bytes from the stack\n");
			fprintf(fp, "		xor	edx, edx\n");
		}

		if (dwOpcode != 0xc9)
			fprintf(fp, "retNotTaken%" PRIu32 ":\n", dwGlobalLabel++);

		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (dwOpcode != 0xc9)
		{
			fprintf(fp, "				if %s\n", pbFlagsC[(dwOpcode >> 3) & 0x07]);
			fprintf(fp, "				{\n");
			fprintf(fp, "					dwElapsedTicks += 6;\n");
		}

		fprintf(fp, "				pbSP = cpu.z80Base + cpu.z80sp;	/* Normalize our stack PTR */\n");
		fprintf(fp, "				dwAddr = *pbSP++;	/* Pop LSB */\n");
		fprintf(fp, "				dwAddr |= ((UINT32) *pbSP << 8);	/* Pop MSB */\n");
		fprintf(fp, "				cpu.z80sp += 2;	/* Pop the word off */\n");
		fprintf(fp, "				pbPC = (cpu.z80Base + dwAddr);	/* Point PC to our return address */\n");

		if (dwOpcode != 0xc9)
		{
			fprintf(fp, "				}\n");
		}
	}
	else
	{
		abort();
	}
}

void RestartHandler(UINT32 dwOpcode)
{
	UINT32 dwOpcode1 = 0;

	dwOpcode1 = dwOpcode & 0x38;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if (bThroughCallHandler)
		{
			fprintf(fp, "		sub	esi, ebp\n");
			fprintf(fp, "		mov	[_wordval], si	; Store our return address\n");
			fprintf(fp, "		call	PushWord\n");
			fprintf(fp, "		xor	esi, esi\n");
			fprintf(fp, "		mov	si, %.4" PRIx32 "h\n", dwOpcode1);
			fprintf(fp, "		add	esi, ebp\n");
		}
		else 
		{
			fprintf(fp, "		mov	dx, word [_z80sp]	; Get our stack pointer\n");
			fprintf(fp, "		sub	dx, 2		; Make room for the new value!\n");
			fprintf(fp, "		mov	word [_z80sp], dx	; Store our new stack pointer\n");
			fprintf(fp, "		sub	esi, ebp		; Get our real PC\n");
			fprintf(fp, "		mov	[ebp+edx], si	; Our return address\n");
			fprintf(fp, "		mov	si, 0%.2xh	; Our new call address\n", dwOpcode1);
			fprintf(fp, "		add	esi, ebp	; Back to the base!\n");
		}
	
		fprintf(fp, "		xor	edx, edx\n");
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				cpu.z80pc = (UINTPTR) pbPC - (UINTPTR) cpu.z80Base;\n");
		fprintf(fp, "				pbSP = (cpu.z80Base + cpu.z80sp - 1);	/* Normalize the stack pointer */\n");
		fprintf(fp, "				*pbSP-- = cpu.z80pc >> 8;	/* LSB */\n");
		fprintf(fp, "				*pbSP = (UINT8) cpu.z80pc;	/* MSB */\n");
		fprintf(fp, "				cpu.z80sp -= 2;	/* Back our stack up */\n");
		fprintf(fp, "				pbPC = cpu.z80Base + 0x%.2x;	/* Normalize the address */\n", dwOpcode1);
	}
	else
	{
		abort();
	}
}

void ToRegFromHl(UINT32 dwOpcode)
{
	UINT8 bReg;

	bReg = (dwOpcode >> 3) & 0x07;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if (bReg != 2 && bReg != 3)
			ReadValueFromMemory("bx", pbMathReg[bReg]);
		else
		{
			ReadValueFromMemory("bx", pbLocalReg[bReg]);
			fprintf(fp, "		mov	%s, %s\n", pbMathReg[bReg], pbLocalReg[bReg]);
		}

		fprintf(fp, "		xor	edx, edx\n");
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		ReadValueFromMemory("cpu.z80HL", pbLocalRegC[bReg]);
	}
	else
	{
		abort();
	}
}

void AddRegpairOperations(UINT32 dwOpcode)
{
	UINT8 bRegpair;

	bRegpair = (dwOpcode >> 4) & 0x3;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		fprintf(fp, "		mov	dh, ah	; Get our flags\n");
		fprintf(fp, "		and	dh, 0ech	; Preserve the top three and bits 2 & 3\n");
	
		fprintf(fp, "		mov	[_orgval], bx	; Store our original value\n");
		fprintf(fp, "		add	bx, %s\n", pbRegPairs[bRegpair]);
		fprintf(fp, "		lahf\n");
	
		fprintf(fp, "		mov	[cyclesRemaining], edi\n");
		fprintf(fp, "		mov	di, [_orgval]	; Get original\n");
		fprintf(fp, "		xor	di, bx ; XOR It with our computed value\n");
		fprintf(fp, "		xor	di, %s\n", pbRegPairs[bRegpair]);
		fprintf(fp, "		and	di, 1000h	; Just our half carry\n");
		fprintf(fp, "		or		dx, di	; Or in our flags\n");
		fprintf(fp, "		and	ah, 01h	; Just carry\n");
		fprintf(fp, "		or	ah, dh\n");
		fprintf(fp, "		mov	edi, [cyclesRemaining]\n");	
		fprintf(fp, "		xor	edx, edx\n");	
	
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "			cpu.z80F &= ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_HALF_CARRY);\n");
		fprintf(fp, "			dwTemp = cpu.z80HL + %s;\n", pbRegPairsC[bRegpair]);
		fprintf(fp, "			cpu.z80F |= ((dwTemp >> 16) & Z80_FLAG_CARRY) | (((cpu.z80HL ^ dwTemp ^ %s) >> 8) & Z80_FLAG_HALF_CARRY);\n", pbRegPairsC[bRegpair]);
		fprintf(fp, "			cpu.z80HL = dwTemp & 0xffff;\n");

		return;
	}
	else
	{
		abort();
	}
}

void PushPopOperations(UINT32 dwOpcode)
{
	UINT8 bRegPair;

	bRegPair = ((dwOpcode >> 4) & 0x3) << 1;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if ((dwOpcode & 0xcf) == 0xc5)	// Push
		{
			fprintf(fp, "		sub	word [_z80sp], 2\n");
			fprintf(fp, "		mov	dx, [_z80sp]\n");
			WriteWordToMemory("dx", pbPopRegPairs[bRegPair >> 1]);
		}
		else	// Pop
		{
			fprintf(fp, "		mov	dx, [_z80sp]\n");
			ReadWordFromMemory("dx", pbPopRegPairs[bRegPair >> 1]);
			fprintf(fp, "		add	word [_z80sp], 2\n");
		}	

		fprintf(fp, "		xor	edx, edx\n");
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if ((dwOpcode & 0xcf) == 0xc5)		// Push?
		{
			fprintf(fp, "					cpu.z80sp -= 2;\n");
			fprintf(fp, "					pbSP = (cpu.z80Base + cpu.z80sp);	/* Normalize the stack pointer */\n");
			
			WriteWordToMemory("cpu.z80sp", pbPopRegPairC[bRegPair >> 1]);
			return;
		}
		else
		{
			ReadWordFromMemory("cpu.z80sp", pbPopRegPairC[bRegPair >> 1]);

			fprintf(fp, "					cpu.z80sp += 2;\n");
			fprintf(fp, "					pbSP = (cpu.z80Base + cpu.z80sp);	/* Normalize the stack pointer */\n");
			return;
		}
		
		InvalidInstructionC(1);
	}
	else
	{
		abort();
	}
}

void RraRlaHandler(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		fprintf(fp, "		sahf\n");
		if (dwOpcode == 0x1f)
			fprintf(fp, "		rcr	al, 1\n");
		else
			fprintf(fp, "		rcl	al, 1\n");
	
		fprintf(fp, "		lahf\n");
		fprintf(fp, "		and	ah, 0edh\n");
	
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (0x1f == dwOpcode)		// RRA
		{
			fprintf(fp, "				bTemp = (cpu.z80F & Z80_FLAG_CARRY) << 7;\n");
			fprintf(fp, "				cpu.z80F = (cpu.z80F & ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_HALF_CARRY)) | (cpu.z80A & Z80_FLAG_CARRY);\n");
			fprintf(fp, "				cpu.z80A = ((cpu.z80A >> 1) | bTemp);\n");
		}
		else								// RLA
		{
			fprintf(fp, "				bTemp = cpu.z80A >> 7;\n");
			fprintf(fp, "				cpu.z80A = (cpu.z80A << 1) | (cpu.z80F & Z80_FLAG_CARRY);\n");
			fprintf(fp, "				cpu.z80F = (cpu.z80F & ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_HALF_CARRY)) | bTemp;\n");
		}
	}
	else
	{
		abort();
	}
}

void LdByteRegpair(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if (dwOpcode == 0x0a)
			ReadValueFromMemory("cx", "al");
		if (dwOpcode == 0x1a)
		{
			fprintf(fp, "		mov	dx, [_z80de]\n");
			ReadValueFromMemory("dx", "al");
		}

		fprintf(fp, "		xor	edx, edx\n");
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (dwOpcode == 0x0a)
			ReadValueFromMemory("cpu.z80BC", "cpu.z80A");
		if (dwOpcode == 0x1a)
			ReadValueFromMemory("cpu.z80DE", "cpu.z80A");
	}
	else
	{
		abort();
	}
}

void IncDecHLPtr(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		ReadValueFromMemory("bx", "dl");

		fprintf(fp, "		sahf\n");

		if (dwOpcode == 0x34)
			fprintf(fp, "		inc	dl\n");
		else
			fprintf(fp, "		dec	dl\n");
		fprintf(fp, "		lahf\n");

		fprintf(fp, "		o16	pushf\n");
		fprintf(fp, "		shl	edx, 16\n");
		fprintf(fp, "		and	ah, 0fbh	;	Knock out parity/overflow\n");
		fprintf(fp, "		pop	dx\n");
		fprintf(fp, "		and	dh, 08h ; Just the overflow\n");
		fprintf(fp, "		shr	dh, 1	; Shift it into position\n");
		fprintf(fp, "		or	ah, dh	; OR It in with the real flags\n");
	
		fprintf(fp, "		shr	edx, 16\n");
	
		if (dwOpcode == 0x34)
			fprintf(fp, "		and	ah, 0fdh	; Knock out N!\n");
		else
			fprintf(fp, "		or		ah, 02h	; Make it N!\n");
	
		WriteValueToMemory("bx", "dl");
		fprintf(fp, "		xor	edx, edx\n");

		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		ReadValueFromMemory("cpu.z80HL", "bTemp");

		fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_SIGN | Z80_FLAG_ZERO | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE);\n");

		if (0x34 == dwOpcode)
			fprintf(fp ,"				cpu.z80F |= bPostIncFlags[bTemp];\n");
		else
			fprintf(fp ,"				cpu.z80F |= bPostDecFlags[bTemp];\n");
		
		if (0x34 == dwOpcode)
			fprintf(fp, "				bTemp++;\n");
		else
			fprintf(fp, "				bTemp--;\n");
	
		WriteValueToMemory("cpu.z80HL", "bTemp");
		return;
	}
	else
	{
		abort();
	}
}

void InOutHandler(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		fprintf(fp, "		mov	dl, [esi]	; Get our address to 'out' to\n");
		fprintf(fp, "		inc	esi	; Next address\n");

		if (b16BitIo)
		{
			fprintf(fp, "		mov	dh, al	; Upper 8 bits are the A register for 16 bit addressing\n");
		}

		if (0xd3 == dwOpcode)
			WriteValueToIo("dx", "al");
		else
			ReadValueFromIo("dx", "al");

		fprintf(fp, "		xor	edx, edx\n");
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp ,"			dwTemp = *pbPC++;\n");

		if (0xd3 == dwOpcode)
			WriteValueToIo("dwTemp", "cpu.z80A");
		else
			ReadValueFromIo("dwTemp", "cpu.z80A");

		// Not supposed to set flags for immediate instruction!

		return;
	}
	else
	{
		abort();
	}
}

// CB Area

void RESSETHandler(UINT32 dwOpcode)
{
	UINT8 op = 0;

	op = dwOpcode & 0x07;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if ((2 == op) || (3 == op))
			fprintf(fp, "		mov	dx, [_z80de]	; Move DE into something half usable\n");

		if ((dwOpcode & 0x07) == 6)     // (HL)?
			ReadValueFromMemory("bx", "dl");

		if ((dwOpcode & 0xc0) == 0x80)
			fprintf(fp, "		and %s, 0%.2xh	; Reset a bit\n",         
							pbLocalReg[op],
							0xff - (1 << ((dwOpcode >> 3) & 0x7)));

		if ((dwOpcode & 0xc0) == 0xc0)
			fprintf(fp, "		or	%s, 0%.2xh	; Set a bit\n",    
							pbLocalReg[op],
							(1 << ((dwOpcode >> 3) & 0x7)));

		if ((2 == op) || (3 == op))
		{
			fprintf(fp, "		mov	[_z80de], dx	; Once modified, put it back\n");
			fprintf(fp, "		xor	edx, edx\n");
		}

		if ((dwOpcode & 0x07) == 6)     // (HL)?
		{
			WriteValueToMemory("bx", "dl");
			fprintf(fp, "		xor	edx, edx\n");
		}

		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (6 == op)			// (HL)?
			ReadValueFromMemory("cpu.z80HL", "bTemp");

		if ((dwOpcode & 0xc0) == 0x80)	// RES
			fprintf(fp, "				%s &= 0x%.2x;\n", pbMathRegC[op], (UINT8) ~((UINT8) 1 << ((dwOpcode >> 3) & 0x07)));
		else										// SET
			fprintf(fp, "				%s |= 0x%.2x;\n", pbMathRegC[op], 1 << ((dwOpcode >> 3) & 0x07));

		if (6 == op)			// (HL)?
			WriteValueToMemory("cpu.z80HL", "bTemp");
	}
	else
		abort();
}

void BITHandler(UINT32 dwOpcode)
{
	UINT8 op = 0;
	UINT8 bBitVal = 0;

	op = dwOpcode & 0x07;
	bBitVal = 1 << ((dwOpcode >> 3) & 0x07);

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if ((dwOpcode & 0x07) == 6)     // (HL)?
			ReadValueFromMemory("bx", "dl");

		fprintf(fp, "		mov	byte [_z80af], ah ; Store F\n");
		fprintf(fp, "		sahf\n");

		if ((dwOpcode & 0x07) == 6)
			fprintf(fp, "		test	dl, 0%.2xh	; Do a bitwise check\n", 1 << ((dwOpcode >> 3) & 0x7));
		else
			fprintf(fp, "		test %s, 0%.2xh	; Do a bitwise check\n", pbMathReg[op], 1 << ((dwOpcode >> 3) & 0x7));

		fprintf(fp, "		lahf\n");
		fprintf(fp, "		and	ah, 0c0h	; Only care about Z and S\n");
		fprintf(fp, "		or	ah, 10h	; Set half carry to 1\n");

		fprintf(fp, "		and	byte [_z80af], 029h		; Only zero/non-zero!\n");
		fprintf(fp, "		or	ah, byte [_z80af]	; Put it in with the real flags\n");

		if (6 == (dwOpcode & 0x07))     // (HL)?
			fprintf(fp, "		xor	edx, edx\n");

		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (6 == op)			// (HL)?
			ReadValueFromMemory("cpu.z80HL", "bTemp");

		fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_HALF_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_ZERO);\n");
		fprintf(fp, "				cpu.z80F |= (Z80_FLAG_HALF_CARRY);\n");
		fprintf(fp, "				if (!(%s & 0x%.2" PRIx32 "))\n", pbMathRegC[op], bBitVal);
		fprintf(fp, "				{\n");
		fprintf(fp, "					cpu.z80F |= Z80_FLAG_ZERO;\n");
		fprintf(fp, "				}\n");
	}
	else
		abort();
}

void RLCRRCRLRRSLASRASRLHandler(UINT32 dwOpcode)
{
	UINT8 op = 0;

	op = dwOpcode & 0x07;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if ((2 == op) || (3 == op))
			fprintf(fp, "		mov	dx, [_z80de]	; Move DE into something half usable\n");

		if ((dwOpcode & 0x07) == 6)     // (HL)?
			ReadValueFromMemory("bx", "dl");

		fprintf(fp, "		sahf\n");

		if ((dwOpcode & 0xf8) == 0)
			fprintf(fp, "		rol	%s, 1\n", pbLocalReg[op]);
		else
		if ((dwOpcode & 0xf8) == 0x08)
			fprintf(fp, "		ror	%s, 1\n", pbLocalReg[op]);
		else
		if ((dwOpcode & 0xf8) == 0x10)
			fprintf(fp, "		rcl	%s, 1\n", pbLocalReg[op]);
		else
		if ((dwOpcode & 0xf8) == 0x18)
			fprintf(fp, "		rcr	%s, 1\n", pbLocalReg[op]);
		else
		if ((dwOpcode & 0xf8) == 0x20 || (dwOpcode & 0xf8) == 0x30)
			fprintf(fp, "		shl	%s, 1\n", pbLocalReg[op]);
		else
		if ((dwOpcode & 0xf8) == 0x28)
			fprintf(fp, "		sar	%s, 1\n", pbLocalReg[op]);
		else
		if ((dwOpcode & 0xf8) == 0x38)
			fprintf(fp, "		shr	%s, 1\n", pbLocalReg[op]);
		else
			abort();
	
		fprintf(fp, "		lahf\n");

		if ((dwOpcode & 0xf8) >= 0x20)
		{
			if ((dwOpcode & 0xf8) == 0x30)
				fprintf(fp, "		or	%s, 1	; Slide in a 1 bit (SLIA)\n", pbLocalReg[op]);
			fprintf(fp, "		and	ah, 0edh	; Clear H and N\n");
		}
		else
		{
			fprintf(fp, "		and	ah, 029h	; Clear H and N\n");
			fprintf(fp, "		mov	byte [_z80af], ah\n");

			fprintf(fp, "		or	%s, %s\n", pbLocalReg[op], pbLocalReg[op]);
	
			fprintf(fp,	"		lahf\n");
			fprintf(fp, "		and	ah, 0c4h	; Sign, zero, and parity\n");
			fprintf(fp, "		or	ah, byte [_z80af]\n");
		}

		if ((2 == op) || (3 == op))
		{
			fprintf(fp, "		mov	[_z80de], dx	; Once modified, put it back\n");
			fprintf(fp, "		xor	edx, edx\n");
		}

		if ((dwOpcode & 0x07) == 6)     // (HL)?
		{
			WriteValueToMemory("bx", "dl");
			fprintf(fp, "		xor	edx, edx\n");
		}

		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (6 == op)						// (HL)?
			ReadValueFromMemory("cpu.z80HL", "bTemp");

		dwOpcode &= 0xf8;			// Just the instruction

		if (0 == dwOpcode)		// RLC
		{
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_ZERO | Z80_FLAG_SIGN | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE | Z80_FLAG_CARRY);\n");
			fprintf(fp, "				bTemp2 = (%s >> 7);\n", pbMathRegC[op]);
			fprintf(fp, "				%s = (%s << 1) | bTemp2;\n", pbMathRegC[op], pbMathRegC[op]);
			fprintf(fp, "				cpu.z80F |= bTemp2 | bPostORFlags[%s];\n", pbMathRegC[op]);
		}
		else
		if (0x08 == dwOpcode)		// RRC
		{
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_ZERO | Z80_FLAG_SIGN | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE | Z80_FLAG_CARRY);\n");
			fprintf(fp, "				cpu.z80F |= (%s & Z80_FLAG_CARRY);\n", pbMathRegC[op]);
			fprintf(fp, "				%s = (%s >> 1) | (%s << 7);\n", pbMathRegC[op], pbMathRegC[op], pbMathRegC[op]);
			fprintf(fp, "				cpu.z80F |= bPostORFlags[%s];\n", pbMathRegC[op]);
		}
		else
		if (0x10 == dwOpcode)		// RL
		{
			fprintf(fp, "				bTemp2 = cpu.z80F & Z80_FLAG_CARRY;\n");
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_ZERO | Z80_FLAG_SIGN | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE | Z80_FLAG_CARRY);\n");
			fprintf(fp, "				cpu.z80F |= (%s >> 7);\n", pbMathRegC[op]);
			fprintf(fp, "				%s = (%s << 1) | bTemp2;\n", pbMathRegC[op], pbMathRegC[op]);
			fprintf(fp, "				cpu.z80F |= bPostORFlags[%s];\n", pbMathRegC[op]);
		}
		else
		if (0x18 == dwOpcode)		// RR
		{
			fprintf(fp, "				bTemp2 = (cpu.z80F & Z80_FLAG_CARRY) << 7;\n");
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_ZERO | Z80_FLAG_SIGN | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE | Z80_FLAG_CARRY);\n");
			fprintf(fp, "				cpu.z80F |= (%s & Z80_FLAG_CARRY);\n", pbMathRegC[op]);
			fprintf(fp, "				%s = (%s >> 1) | bTemp2;\n", pbMathRegC[op], pbMathRegC[op]);
			fprintf(fp, "				cpu.z80F |= bPostORFlags[%s];\n", pbMathRegC[op]);
		}
		else
		if ((0x20 == dwOpcode) || (0x30 == dwOpcode))	// SLA/SRL
		{
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_ZERO | Z80_FLAG_SIGN | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE | Z80_FLAG_CARRY);\n");
			fprintf(fp, "				cpu.z80F |= (%s >> 7);\n", pbMathRegC[op]);
			fprintf(fp, "				%s = (%s << 1);\n", pbMathRegC[op], pbMathRegC[op]);
			fprintf(fp, "				cpu.z80F |= bPostORFlags[%s];\n", pbMathRegC[op]);
		}
		else
		if (0x28 == dwOpcode)		// SRA
		{
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_ZERO | Z80_FLAG_SIGN | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE | Z80_FLAG_CARRY);\n");
			fprintf(fp, "				cpu.z80F |= (%s & Z80_FLAG_CARRY);\n", pbMathRegC[op]);
			fprintf(fp, "				%s = (%s >> 1) | (%s & 0x80);\n", pbMathRegC[op], pbMathRegC[op], pbMathRegC[op]);
			fprintf(fp, "				cpu.z80F |= bPostORFlags[%s];\n", pbMathRegC[op]);
		}
		else
		if (0x38 == dwOpcode)		// SRL
		{
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_ZERO | Z80_FLAG_SIGN | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE | Z80_FLAG_CARRY);\n");
			fprintf(fp, "				cpu.z80F |= (%s & Z80_FLAG_CARRY);\n", pbMathRegC[op]);
			fprintf(fp, "				%s = (%s >> 1);\n", pbMathRegC[op], pbMathRegC[op]);
			fprintf(fp, "				cpu.z80F |= bPostORFlags[%s];\n", pbMathRegC[op]);
		}
		else
		{
			InvalidInstructionC(2);
		}

		if (6 == op)						// (HL)?
			WriteValueToMemory("cpu.z80HL", "bTemp");
	}
	else
		abort();
}

// ED Area

void RRDRLDHandler(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		ReadValueFromMemory("bx", "dl");	// Get (HL)
		fprintf(fp, "		mov	dh, dl	; Put a copy in DH\n");

		if (0x6f == dwOpcode)	// RLD
		{
			fprintf(fp, "		shr	dh, 4	; Get our upper nibble in position\n");
			fprintf(fp, "		shl	dl, 4	; Get our lower nibble into the higher position\n");
			fprintf(fp, "		shl	ecx, 16	; Save this for later\n");
			fprintf(fp, "		mov	cl, al\n");
			fprintf(fp, "		and	cl, 0fh\n	; Only the lower nibble\n");
			fprintf(fp, "		or	dl, cl	; OR In A->(HL) transfer\n");
			fprintf(fp, "		and	al, 0f0h	; Only the upper 4 bits remain\n");
			fprintf(fp, "		or	al, dh	; OR It in to our accumulator\n");
			fprintf(fp, "		shr	ecx, 16	; Restore this\n");
		}
		else			//	RRD
		if (0x67 == dwOpcode)
		{
			fprintf(fp, "		shr	dl, 4	; Upper nibble to lower nibble\n");
			fprintf(fp, "		shl	ecx, 16	; Save this\n");
			fprintf(fp, "		mov	cl, al\n");
			fprintf(fp, "		shl	cl, 4\n");
			fprintf(fp, "		or	dl, cl	; OR In what was in A\n");
			fprintf(fp, "		and	al, 0f0h	; Knock out lower part\n");
			fprintf(fp, "		and	dh, 0fh	; Only the lower nibble\n");
			fprintf(fp, "		or	al, dh	; OR In our nibble\n");
			fprintf(fp, "		shr	ecx, 16	; Restore this\n");
		}
		else	// Whoops!
			abort();

		// This routine assumes that the new value to be placed at (HL) is in DL

		fprintf(fp, "		and	ah, 29h	; Retain carry & two undefined bits\n");
		fprintf(fp, "		mov	dh, ah	; Store our flags away for later\n");

		fprintf(fp, "		or	al, al	; Get our flags\n");
		fprintf(fp, "		lahf\n");
		fprintf(fp, "		and	ah,0c4h	; Only partiy, zero, and sign\n");
		fprintf(fp, "		or	ah, dh	; OR In our old flags\n");

		// Now go write the value back

		WriteValueToMemory("bx", "dl");
		fprintf(fp, "		xor	edx, edx	; Zero out this for later\n");
	
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (0x67 == dwOpcode)	//	RRD
		{
			ReadValueFromMemory("cpu.z80HL", "bTemp");
			fprintf(fp, "				bTemp2 = (cpu.z80A & 0x0f) << 4;\n");
			fprintf(fp, "				cpu.z80A = (cpu.z80A & 0xf0) | (bTemp & 0x0f);\n");
			fprintf(fp, "				bTemp = (bTemp >> 4) | bTemp2;\n");

			WriteValueToMemory("cpu.z80HL", "bTemp");
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_SIGN | Z80_FLAG_ZERO | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE);\n");
			fprintf(fp, "				cpu.z80F |= bPostORFlags[cpu.z80A];\n");
		}
		else
		if (0x6f == dwOpcode)	// RLD
		{
			ReadValueFromMemory("cpu.z80HL", "bTemp");

			fprintf(fp, "				bTemp2 = (cpu.z80A & 0x0f);\n");
			fprintf(fp, "				cpu.z80A = (cpu.z80A & 0xf0) | (bTemp >> 4);\n");
			fprintf(fp, "				bTemp = (bTemp << 4) | bTemp2;\n");

			WriteValueToMemory("cpu.z80HL", "bTemp");
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_SIGN | Z80_FLAG_ZERO | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE);\n");
			fprintf(fp, "				cpu.z80F |= bPostORFlags[cpu.z80A];\n");
		}
		else
			InvalidInstructionC(2);
	}
	else
		abort();
}

void CPICPDCPIRCPDRHandler(UINT32 dwOpcode)
{
	UINT32 dwRepeatOb = 0;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if (dwOpcode == 0xb1 || dwOpcode == 0xb9)
		{
			fprintf(fp, "cpRepeat%" PRIu32 ":\n", dwGlobalLabel);
			dwRepeatOb = dwGlobalLabel;
			++dwGlobalLabel;
		}

		// Now go get the data from the source

		ReadValueFromMemory("bx", "dl");

		// Target data is in DL

		fprintf(fp, "		mov	byte [_z80af], ah\n");
		fprintf(fp, "		sahf\n");
		fprintf(fp, "		cmp	al, dl	; Do our comparison\n");
		fprintf(fp, "		lahf\n");
		fprintf(fp, "		and	ah, 0fah	; No P/V or carry!\n");
		fprintf(fp, "		dec	cx	; Dec BC\n");
		fprintf(fp, "		jz	notBcZero%" PRIu32 "\n", dwGlobalLabel);
		fprintf(fp, "		or	ah, 04h	; P/V set when BC not zero\n");
		fprintf(fp, "notBcZero%" PRIu32 ":\n", dwGlobalLabel);
		fprintf(fp, "		or	ah, 02h	; N Gets set when we do compares\n");
		fprintf(fp, "		mov	dl, byte [_z80af]\n");
		fprintf(fp, "		and	dl, 01h\n");
		fprintf(fp, "		or	ah, dl	; Preserve carry!\n");
	
		if (dwOpcode == 0xa1 || dwOpcode == 0xb1)
			fprintf(fp, "		inc	bx	; Increment!\n");
		if (dwOpcode == 0xa9 || dwOpcode == 0xb9)
			fprintf(fp, "		dec	bx	; Decrement!\n");

		// Let's see if we repeat...
	
		if (dwOpcode == 0xb1 || dwOpcode == 0xb9)
		{
			fprintf(fp, "		sahf\n");
			fprintf(fp, "		jz	BCDone%" PRIu32 "\n", dwRepeatOb);
			fprintf(fp, "		jnp	BCDone%" PRIu32 "\n", dwRepeatOb);

			if (FALSE == bNoTiming)
			{
				fprintf(fp, "		sub	edi, dword 21\n");
				fprintf(fp, "		js		BCDoneExit%" PRIu32 "\n", dwRepeatOb);
			}

			fprintf(fp, "		jmp	cpRepeat%" PRIu32 "\n", dwRepeatOb);

			fprintf(fp, "BCDoneExit%" PRIu32 ":\n", dwRepeatOb);
			fprintf(fp, "		sub	esi, 2	;	Back up to the instruction again\n");
			fprintf(fp, "		jmp	noMoreExec\n\n");
			fprintf(fp, "BCDone%" PRIu32 ":\n", dwRepeatOb);
		}
	
		fprintf(fp, "		xor	edx, edx\n");
	
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (0xb1 == dwOpcode || 0xb9 == dwOpcode)	// Repeat instruction?
		{
			fprintf(fp, "				while ((sdwCyclesRemaining >= 0) && (cpu.z80BC))\n");
		}

		fprintf(fp, "				{\n");			

		ReadValueFromMemory("cpu.z80HL", "bTemp");

		if (0xb1 == dwOpcode || 0xa1 == dwOpcode)
		{
			fprintf(fp, "				cpu.z80HL++;\n");
			fprintf(fp, "				cpu.z80HL &= 0xffff;\n");
		}
		else
		{
			fprintf(fp, "				cpu.z80HL--;\n");
			fprintf(fp, "				cpu.z80HL &= 0xffff;\n");
		}

		fprintf(fp, "				cpu.z80BC--;\n");
		fprintf(fp, "				cpu.z80BC &= 0xffff;\n");

		if (0xb1 == dwOpcode || 0xb9 == dwOpcode)	// Repeat?
		{
			fprintf(fp, "				sdwCyclesRemaining -= 16;\n");
			fprintf(fp, "				if (cpu.z80A == bTemp)\n");
			fprintf(fp, "				{\n");
			fprintf(fp, "					break;\n");
			fprintf(fp, "				}\n");
		}

		fprintf(fp, "				}\n");

		// Now figure out what's going on

		fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_ZERO | Z80_FLAG_SIGN | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE);\n");
		fprintf(fp, "				cpu.z80F |= (pbSubSbcTable[((UINT32) cpu.z80A << 8) | bTemp] & (Z80_FLAG_SIGN | Z80_FLAG_NEGATIVE | Z80_FLAG_ZERO));\n");
		fprintf(fp, "				if (cpu.z80BC)\n");
		fprintf(fp, "				{\n");
		fprintf(fp, "					cpu.z80F |= Z80_FLAG_OVERFLOW_PARITY;\n");

		fprintf(fp, "				}\n");
	}
	else
		abort();
}

void INIRINDRINIINDHandler(UINT32 dwOpcode)
{
	UINT32 dwTempLabel = 0;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		dwTempLabel = dwGlobalLabel;
		dwGlobalLabel++;

		if (0xba == dwOpcode || 0xb2 == dwOpcode)
			fprintf(fp, "loopIt%" PRIu32 ":\n", dwTempLabel);

		// Fetch what's at (C) and put it in (HL)

		fprintf(fp, "		push	cx	; Save BC\n");
	
		if (b16BitIo == FALSE)
			fprintf(fp, "		xor	ch, ch ; We want 8 bit ports\n");
	
		ReadValueFromIo("cx", "*dl");		// Put our value in DL
		fprintf(fp, "		pop	cx	; Restore BC\n");
	
		WriteValueToMemory("bx", "dl");
	
		if (0xa2 == dwOpcode || 0xb2 == dwOpcode)
			fprintf(fp, "		inc	bx	; Increment HL\n");
		else
		if (0xaa == dwOpcode || 0xba == dwOpcode)
			fprintf(fp, "		dec	bx	; Decrement HL\n");
	
		// Now we decrement B
	
		fprintf(fp, "		dec	ch	; Decrement B (of C)\n");
	
		// Emit this instruction if we repeat
	
		if (0xba == dwOpcode || 0xb2 == dwOpcode)
		{
			fprintf(fp, "		jz	near finalExit%" PRIu32 "\n", dwTempLabel);

			// Otherwise, we need to loop again

			if (FALSE == bNoTiming)
			{
				fprintf(fp, "		sub	edi, dword 21\n");
				fprintf(fp, "		js		loopExit%" PRIu32 "\n", dwTempLabel);
			}

			fprintf(fp, "		jmp	loopIt%" PRIu32 "\n\n", dwTempLabel);
			fprintf(fp, "loopExit%" PRIu32 ":\n", dwTempLabel);
			fprintf(fp, "		sub	esi, 2\n");
			fprintf(fp, "		jmp	noMoreExec\n\n");
		}
	
		// Now let's fix up the flags

		fprintf(fp, "finalExit%" PRIu32 ":\n", dwTempLabel);	
		fprintf(fp, "		jnz	clearFlag%" PRIu32 "\n", dwTempLabel);
		fprintf(fp, "		or	ah, 040h	; Set the Zero flag!\n");
		fprintf(fp, "		jmp	short continue%" PRIu32 "\n", dwTempLabel);
		fprintf(fp, "clearFlag%" PRIu32 ":\n", dwTempLabel);
		fprintf(fp, "		and	ah, 0bfh	; Clear the zero flag\n");
		fprintf(fp, "continue%" PRIu32 ":\n", dwTempLabel);
		fprintf(fp, "		or	ah, 02h	; Set negative!\n");
		fprintf(fp, "		xor	edx, edx\n");
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (0xb2 == dwOpcode || 0xba == dwOpcode)	// Repeat instruction?
		{
			fprintf(fp, "				while ((sdwCyclesRemaining > 0) && (cpu.z80B))\n");
		}

		fprintf(fp, "				{\n");			

		ReadValueFromIo("cpu.z80B", "bTemp");
		WriteValueToMemory("cpu.z80HL", "bTemp");

		if (0xb2 == dwOpcode || 0xa2 == dwOpcode)
		{
			fprintf(fp, "				cpu.z80HL++;\n");
			fprintf(fp, "				cpu.z80HL &= 0xffff;\n");
		}
		else
		{
			fprintf(fp, "				cpu.z80HL--;\n");
			fprintf(fp, "				cpu.z80HL &= 0xffff;\n");
		}

		fprintf(fp, "				sdwCyclesRemaining -= 16;\n");
	
		fprintf(fp, "				cpu.z80B--;\n");
		fprintf(fp, "				}\n");

		// Now figure out what's going on

		fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_ZERO | Z80_FLAG_SIGN | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE);\n");
		fprintf(fp, "				cpu.z80F |= (bPostORFlags[bTemp] & (Z80_FLAG_SIGN | Z80_FLAG_NEGATIVE | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY));\n");
		fprintf(fp, "				if (cpu.z80B)\n");
		fprintf(fp, "				{\n");
		fprintf(fp, "					cpu.z80F |= Z80_FLAG_OVERFLOW_PARITY;\n");

		fprintf(fp, "					pbPC -= 2;\n");

		fprintf(fp, "				}\n");
	}
	else
		abort();
}

void OTIROTDROUTIOUTDHandler(UINT32 dwOpcode)
{
	UINT32 dwTempLabel = 0;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		dwTempLabel = dwGlobalLabel;
		dwGlobalLabel++;

		if (0xbb == dwOpcode || 0xb3 == dwOpcode)
			fprintf(fp, "loopIt%" PRIu32 ":\n", dwTempLabel);

		// Fetch what's at (HL) and put it in DL

		ReadValueFromMemory("bx", "dl");

		fprintf(fp, "		push	cx	; Save BC\n");
		if (b16BitIo == FALSE)
			fprintf(fp, "		xor	ch, ch	; No 16 bit for this instruction!\n");
		WriteValueToIo("cx", "dl");
		fprintf(fp, "		pop	cx	; Restore BC now that it has been \"OUT\"ed\n");
	
		if (0xa3 == dwOpcode || 0xb3 == dwOpcode)
			fprintf(fp, "		inc	bx	; Increment HL\n");
		else
		if (0xab == dwOpcode || 0xbb == dwOpcode)
			fprintf(fp, "		dec	bx	; Decrement HL\n");
	
		// Now we decrement B

		fprintf(fp, "		dec	ch	; Decrement B (of C)\n");
	
		// Emit this instruction if we repeat
	
		if (0xbb == dwOpcode || 0xb3 == dwOpcode)
		{
			fprintf(fp, "		jz	near finalExit%" PRIu32 "\n", dwTempLabel);

			// Otherwise, we need to loop again

			if (FALSE == bNoTiming)
			{
				fprintf(fp, "		sub	edi, dword 21\n");
				fprintf(fp, "		js		loopExit%" PRIu32 "\n", dwTempLabel);
			}

			fprintf(fp, "		jmp	loopIt%" PRIu32 "\n\n", dwTempLabel);
			fprintf(fp, "loopExit%" PRIu32 ":\n", dwTempLabel);
			fprintf(fp, "		sub	esi, 2\n");
			fprintf(fp, "		jmp	noMoreExec\n\n");
		}
	
		// Now let's fix up the flags

		fprintf(fp, "finalExit%" PRIu32 ":\n", dwTempLabel);	
		fprintf(fp, "		jnz	clearFlag%" PRIu32 "\n", dwTempLabel);
		fprintf(fp, "		or	ah, 040h	; Set the Zero flag!\n");
		fprintf(fp, "		jmp	short continue%" PRIu32 "\n", dwTempLabel);
		fprintf(fp, "clearFlag%" PRIu32 ":\n", dwTempLabel);
		fprintf(fp, "		and	ah, 0bfh	; Clear the zero flag\n");
		fprintf(fp, "continue%" PRIu32 ":\n", dwTempLabel);
		fprintf(fp, "		or	ah, 02h	; Set negative!\n");
		fprintf(fp, "		xor	edx, edx\n");
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (0xb3 == dwOpcode || 0xbb == dwOpcode)	// Repeat instruction?
		{
			fprintf(fp, "				while ((sdwCyclesRemaining > 0) && (cpu.z80B))\n");
		}

		fprintf(fp, "				{\n");
		
		ReadValueFromMemory("cpu.z80HL", "bTemp");
		WriteValueToIo("cpu.z80BC", "bTemp");

		if (0xb3 == dwOpcode || 0xa3 == dwOpcode)
		{
			fprintf(fp, "				cpu.z80HL++;\n");
			fprintf(fp, "				cpu.z80HL &= 0xffff;\n");
		}
		else
		{
			fprintf(fp, "				cpu.z80HL--;\n");
			fprintf(fp, "				cpu.z80HL &= 0xffff;\n");
		}

		fprintf(fp, "				sdwCyclesRemaining -= 16;\n");
	
		fprintf(fp, "				cpu.z80B--;\n");
		fprintf(fp, "				}\n");

		// Now figure out what's going on

		fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_ZERO | Z80_FLAG_SIGN | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE);\n");
		fprintf(fp, "				cpu.z80F |= (bPostORFlags[bTemp] & (Z80_FLAG_SIGN | Z80_FLAG_NEGATIVE | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY));\n");
		fprintf(fp, "				if (cpu.z80B)\n");
		fprintf(fp, "				{\n");
		fprintf(fp, "					cpu.z80F |= Z80_FLAG_OVERFLOW_PARITY;\n");

		fprintf(fp, "				}\n");
	}
	else
		abort();
}

void AdcSbcRegpair(UINT32 dwOpcode)
{
	UINT8 bOp = 0;

	bOp = (dwOpcode >> 4) & 0x03;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		fprintf(fp, "		mov	dx, %s	; Get our original register\n", pbRegPairs[bOp]);
		fprintf(fp, "		mov	[_orgval], dx	; Store this for later half carry computation\n");
		fprintf(fp, "		mov	[_orgval2], bx	; Store this, too\n");
		fprintf(fp, "		sahf		; Restore our flags\n");

		if ((dwOpcode & 0xcf) == 0x4a)
			fprintf(fp, "		adc	bx, dx	; Do the operation!\n");
		else
			fprintf(fp, "		sbb	bx, dx	; Do the operation!\n");

		fprintf(fp, "		lahf		; Get our new flags\n");
	
		if ((dwOpcode & 0xcf) != 0x4a)
		{
			SetOverflow();
			fprintf(fp, "		and	ah, 0edh	; Knock out negative & half carry flags\n");
	 		fprintf(fp, "		or	ah, 02h	; Negative!\n");
			fprintf(fp, "		mov	[_z80hl], bx\n");
			fprintf(fp, "		xor	bx, [_orgval]\n");
			fprintf(fp, "		xor	bx, [_orgval2]\n");
			fprintf(fp, "		and	bh, 10h	; Half carry?\n");
			fprintf(fp, "		or	ah, bh	; OR It in if so\n");
			fprintf(fp, "		mov	bx, [_z80hl]\n");
		}
		else
		{
			SetOverflow();
			fprintf(fp, "		and	ah, 0edh	; Knock out negative & half carry flags\n");
			fprintf(fp, "		mov	[_z80hl], bx\n");
			fprintf(fp, "		xor	bx, [_orgval]\n");
			fprintf(fp, "		xor	bx, [_orgval2]\n");
			fprintf(fp, "		and	bh, 10h	; Half carry?\n");
			fprintf(fp, "		or	ah, bh	; OR It in if so\n");
			fprintf(fp, "		mov	bx, [_z80hl]\n");
		}

		fprintf(fp, "		xor	edx, edx	; Make sure we don't hose things\n");
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if ((dwOpcode & 0xcf) == 0x4a)	// ADC
		{
			fprintf(fp, "				dwTemp = cpu.z80HL + %s + (cpu.z80F & Z80_FLAG_CARRY);\n", pbRegPairsC[bOp]);
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_HALF_CARRY | Z80_FLAG_ZERO | Z80_FLAG_SIGN);\n");
			fprintf(fp, "				cpu.z80F |= ((dwTemp >> 8) & Z80_FLAG_SIGN);\n");
			fprintf(fp, "				if (0 == (dwTemp & 0xffff))\n");
			fprintf(fp, "				{\n");
			fprintf(fp, "					cpu.z80F |= Z80_FLAG_ZERO;\n");
			fprintf(fp, "				}\n");
			fprintf(fp, "				cpu.z80F |= (((cpu.z80HL ^ dwTemp ^ %s) >> 8) & Z80_FLAG_HALF_CARRY);\n", pbRegPairsC[bOp]);
			fprintf(fp, "				cpu.z80F |= ((((%s ^ cpu.z80HL ^ 0x8000) & (%s ^ dwTemp)) >> 13) & Z80_FLAG_OVERFLOW_PARITY);\n", pbRegPairsC[bOp], pbRegPairsC[bOp]);
			fprintf(fp, "				cpu.z80F |= ((dwTemp >> 16) & Z80_FLAG_CARRY);\n");
			fprintf(fp, "				cpu.z80HL = dwTemp & 0xffff;\n");
			return;
		}
		else										// SBC
		{
			fprintf(fp, "				dwTemp = cpu.z80HL - %s - (cpu.z80F & Z80_FLAG_CARRY);\n", pbRegPairsC[bOp]);
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_HALF_CARRY | Z80_FLAG_ZERO | Z80_FLAG_SIGN);\n");
			fprintf(fp, "				cpu.z80F |= ((dwTemp >> 8) & Z80_FLAG_SIGN);\n");
			fprintf(fp, "				if (0 == (dwTemp & 0xffff))\n");
			fprintf(fp, "				{\n");
			fprintf(fp, "					cpu.z80F |= Z80_FLAG_ZERO;\n");
			fprintf(fp, "				}\n");
			fprintf(fp, "				cpu.z80F |= (((cpu.z80HL ^ dwTemp ^ %s) >> 8) & Z80_FLAG_HALF_CARRY);\n", pbRegPairsC[bOp]);
			fprintf(fp, "				cpu.z80F |= ((((%s ^ cpu.z80HL) & (%s ^ dwTemp)) >> 13) & Z80_FLAG_OVERFLOW_PARITY);\n", pbRegPairsC[bOp], pbRegPairsC[bOp]);
			fprintf(fp, "				cpu.z80F |= ((dwTemp >> 16) & Z80_FLAG_CARRY);\n");
			fprintf(fp, "				cpu.z80HL = dwTemp & 0xffff;\n");
			return;
		}
	}
	else
		abort();
}

void RetIRetNHandler(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if (bThroughCallHandler)
		{
			fprintf(fp, "		call	PopWord\n");
			fprintf(fp, "		xor	esi, esi\n");
			fprintf(fp, "		mov	si, dx\n");
			fprintf(fp,	"		add	esi, ebp\n");
		}
		else 
		{
			fprintf(fp,     "		mov	dx, word [_z80sp]	; Get our current stack pointer\n");
			fprintf(fp, "		mov	si, [edx+ebp]	; Get our return address\n");
			fprintf(fp, "		and	esi, 0ffffh		; Only within 64K!\n");
			fprintf(fp,     "		add	esi, ebp			; Add in our base address\n");
			fprintf(fp,     "		add	word [_z80sp], 02h	; Remove our two bytes from the stack\n");
		}
	
		if (dwOpcode == 0x45)
		{
			fprintf(fp, "		xor	edx, edx\n");
			fprintf(fp, "		mov	dl, [_z80iff]	; Get interrupt flags\n");
			fprintf(fp, "		shr	dl, 1		; Move IFF2->IFF1\n");
			fprintf(fp, "		and	[_z80iff], dword (~IFF1)	; Get rid of IFF 1\n");
			fprintf(fp, "		and	dl, IFF1	; Just want the IFF 1 value now\n");
			fprintf(fp, "		or	dword [_z80iff], edx\n");
		}

		fprintf(fp, "		xor	edx, edx	; Make sure we don't hose things\n");
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (0x4d == dwOpcode)		// RETI
		{
			fprintf(fp, "				pbSP = cpu.z80Base + cpu.z80sp;	/* Normalize our stack PTR */\n");
			fprintf(fp, "				dwAddr = *pbSP++;	/* Pop LSB */\n");
			fprintf(fp, "				dwAddr |= ((UINT32) *pbSP << 8);	/* Pop MSB */\n");
			fprintf(fp, "				cpu.z80sp += 2;	/* Pop the word off */\n");
			fprintf(fp, "				pbPC = (cpu.z80Base + dwAddr);	/* Point PC to our return address */\n");
		}
		else
		if (0x45 == dwOpcode)		// RETN
		{
			fprintf(fp, "				pbSP = cpu.z80Base + cpu.z80sp;	/* Normalize our stack PTR */\n");
			fprintf(fp, "				dwAddr = *pbSP++;	/* Pop LSB */\n");
			fprintf(fp, "				dwAddr |= ((UINT32) *pbSP << 8);	/* Pop MSB */\n");
			fprintf(fp, "				cpu.z80sp += 2;	/* Pop the word off */\n");
			fprintf(fp, "				pbPC = (cpu.z80Base + dwAddr);	/* Point PC to our return address */\n");
			fprintf(fp, "				cpu.z80iff &= ~(IFF1);	/* Keep IFF2 around */\n");
			fprintf(fp, "				cpu.z80iff |= ((cpu.z80iff >> 1) & IFF1);	/* IFF2->IFF1 */\n");
		}
		else
		{
			InvalidInstructionC(2);
		}
	}
	else
		abort();
}

void ExtendedOutHandler(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if (b16BitIo == FALSE)
			fprintf(fp, "		mov	dl, cl	; Address in DX... (C)\n");
		else
			fprintf(fp, "		mov	dx, cx	; Address in DX... (BC)\n");
	
		WriteValueToIo("dx", pbMathReg[(dwOpcode >> 3) & 0x07]);

		fprintf(fp, "		xor	edx, edx\n");
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (b16BitIo == FALSE)
			fprintf(fp, "				dwAddr = cpu.z80C;\n");
		else
			fprintf(fp, "				dwAddr = cpu.z80BC;\n");

		WriteValueToIo("dwAddr", pbMathRegC[(dwOpcode >> 3) & 0x07]);
	}
	else
		abort();
}

void ExtendedInHandler(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if (b16BitIo == FALSE)
			fprintf(fp, "		mov	dl, cl	; Address in DX... (C)\n");
		else
			fprintf(fp, "		mov	dx, cx	; Address in DX... (BC)\n");
	
		ReadValueFromIo("dx", pbMathReg[(dwOpcode >> 3) & 0x07]);

		fprintf(fp, ";\n; Remember, this variant of the IN instruction modifies the flags\n;\n\n");
		fprintf(fp, "		sahf	; Restore our flags\n");
		fprintf(fp, "		mov	dh, ah	; Save flags for later\n");
	
		if (0x50 == dwOpcode || 0x58 == dwOpcode)
		{
			fprintf(fp, "		mov	dl, %s\n", pbMathReg[(dwOpcode >> 3) & 0x07]);
			fprintf(fp, "		or	dl, dl\n");
		}
		else
			fprintf(fp, "		or	%s, %s;\n", pbMathReg[(dwOpcode >> 3) & 0x07], pbMathReg[(dwOpcode >> 3) & 0x07]);

		fprintf(fp, "		lahf\n");
		fprintf(fp, "		and	dh, 029h	; Only keep carry and two unused flags\n");
		fprintf(fp, "		and	ah, 0d4h\n");
		fprintf(fp, "		or	ah, dh\n");

		fprintf(fp, "		xor	edx, edx\n");

		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (b16BitIo == FALSE)
			fprintf(fp, "				dwAddr = cpu.z80C;\n");
		else
			fprintf(fp, "				dwAddr = cpu.z80BC;\n");

		ReadValueFromIo("dwAddr", pbMathRegC[(dwOpcode >> 3) & 0x07]);

		// Set flags!

		fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_SIGN | Z80_FLAG_ZERO | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE);\n");
		fprintf(fp, "				cpu.z80F |= bPostORFlags[%s];\n", pbMathRegC[(dwOpcode >> 3) & 0x07]);
	}
	else
		abort();
}

void NegHandler(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		fprintf(fp, "		sahf\n");
		fprintf(fp, "		sub	dh, al\n");
		fprintf(fp, "		lahf\n");
		fprintf(fp, "		mov	al, dh\n");
	
		SetOverflow();
		fprintf(fp, "		or	ah, 02h\n");
		fprintf(fp, "		xor	edx, edx\n");

		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		SetSubFlagsSZHVC("0", "cpu.z80A");
		fprintf(fp, "				cpu.z80A = 0 - cpu.z80A;\n");
	}
	else
		abort();
}

void ExtendedRegIntoMemory(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		fprintf(fp, "		mov	dx, [esi]	; Get our address to write to\n");
		fprintf(fp, "		add	esi, 2		; Next address, please...\n");

		if (dwOpcode == 0x43)
			WriteValueToMemory("dx", "cl");
		if (dwOpcode == 0x53)
			WriteValueToMemory("dx", "byte [_z80de]");
		if (dwOpcode == 0x63)
			WriteValueToMemory("dx", "bl");
		if (dwOpcode == 0x73)
			WriteValueToMemory("dx", "byte [_z80sp]");

		fprintf(fp, "		inc	dx\n");

		if (dwOpcode == 0x43)
			WriteValueToMemory("dx", "ch");
		if (dwOpcode == 0x53)
			WriteValueToMemory("dx", "byte [_z80de + 1]");
		if (dwOpcode == 0x63)
			WriteValueToMemory("dx", "bh");
		if (dwOpcode == 0x73)
			WriteValueToMemory("dx", "byte [_z80sp + 1]");
	
		fprintf(fp, "		xor	edx, edx	; Zero our upper word\n");

		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "		dwTemp = *pbPC++;\n");
		fprintf(fp, "		dwTemp |= ((UINT32) *pbPC++ << 8);\n");

		if (0x43 == dwOpcode)		//	LD (xxxxh), BC
			WriteWordToMemory("dwTemp", "cpu.z80BC");
		if (0x53 == dwOpcode)		//	LD (xxxxh), DE
			WriteWordToMemory("dwTemp", "cpu.z80DE");
		if (0x63 == dwOpcode)		//	LD (xxxxh), HL
			WriteWordToMemory("dwTemp", "cpu.z80HL");
		if (0x73 == dwOpcode)		//	LD (xxxxh), SP
			WriteWordToMemory("dwTemp", "cpu.z80sp");
	}
	else
		abort();
}

void LdRegpair(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		fprintf(fp, "		mov	dx, [esi]	; Get address to load\n");
		fprintf(fp, "		add	esi, 2	; Skip over it so we don't execute it\n");
	
		if (dwOpcode == 0x4b)
			ReadValueFromMemory("dx", "cl");
		if (dwOpcode == 0x5b)
			ReadValueFromMemory("dx", "byte [_z80de]");
		if (dwOpcode == 0x7b)
			ReadValueFromMemory("dx", "byte [_z80sp]");
	
		fprintf(fp, "		inc	dx\n");
	
		if (dwOpcode == 0x4b)
			ReadValueFromMemory("dx", "ch");
		if (dwOpcode == 0x5b)
			ReadValueFromMemory("dx", "byte [_z80de + 1]");
		if (dwOpcode == 0x7b)
			ReadValueFromMemory("dx", "byte [_z80sp + 1]");

		fprintf(fp, "		xor	edx, edx\n");

		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "		dwTemp = *pbPC++;\n");
		fprintf(fp, "		dwTemp |= ((UINT32) *pbPC++ << 8);\n");

		if (0x4b == dwOpcode)
			ReadWordFromMemory("dwTemp", "cpu.z80BC");
		if (0x5b == dwOpcode)
			ReadWordFromMemory("dwTemp", "cpu.z80DE");
		if (0x7b == dwOpcode)
			ReadWordFromMemory("dwTemp", "cpu.z80sp");
	}
	else
		abort();
}

void LDILDRLDIRLDDRHandler(UINT32 dwOpcode)
{
	UINT32 dwOrgGlobal = 0;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if (dwOpcode == 0xb0 || dwOpcode == 0xb8)
		{
			dwOrgGlobal = dwGlobalLabel;
			fprintf(fp, "ldRepeat%" PRIu32 ":\n", dwGlobalLabel);
		}

		ReadValueFromMemory("bx", "dl");	

		// Here we write the byte back to the target
	
		WriteValueToMemory("[_z80de]", "dl");

		// Now we decide what to do
	
		if ((dwOpcode & 0x0f) == 0)
		{
			fprintf(fp, "		inc	bx	; Increment HL\n");
			fprintf(fp, "		inc	word [_z80de]	; Increment DE\n");
		}
		else
		{
			fprintf(fp, "		dec	bx	; Decrement HL\n");
			fprintf(fp, "		dec	word [_z80de]	; Decrement DE\n");
		}
	
		fprintf(fp, "		dec	cx	; Decrement BC\n");
	
		if (dwOpcode == 0xb0 || dwOpcode == 0xb8)
		{
			if (FALSE == bNoTiming)
			{
				fprintf(fp, "		jz	noMore%" PRIu32 "\n", dwGlobalLabel);
				fprintf(fp, "		sub	edi, dword 16	; 16 T-States per iteration\n");
				fprintf(fp, "		js	noMore%" PRIu32 "\n", dwGlobalLabel);
			}
			else
			{
				fprintf(fp, "		jz	noMore%" PRIu32 "\n", dwGlobalLabel);
			}
	
			fprintf(fp, "		jmp	ldRepeat%" PRIu32 " ; Loop until we're done!\n", dwOrgGlobal);
			fprintf(fp, "noMore%" PRIu32 ":\n", dwGlobalLabel);
		}
	
		fprintf(fp, "		and	ah, 0e9h ; Knock out H & N and P/V\n");
		fprintf(fp, "		or		cx, cx	; Flag BC\n");
		fprintf(fp, "		jz	atZero%" PRIu32 " ; We're done!\n", dwGlobalLabel);
	
		if (dwOpcode == 0xb0 || dwOpcode == 0xb8)
		{
			// It's a repeat, so let's readjust ESI, shall we?
	
			fprintf(fp, "		or	ah, 04h	; Non-zero - we're still going!\n");
			fprintf(fp, "		sub	esi, 2	; Adjust back to the beginning of the instruction\n");
			fprintf(fp, "		jmp	noMoreExec\n\n");
		}
		else
		if (dwOpcode == 0xa0 || dwOpcode == 0xa8)
		{
			fprintf(fp, "		or	ah, 04h	; Non-zero - we're still going!\n");
		}
	
		fprintf(fp, "atZero%" PRIu32 ":\n", dwGlobalLabel);
		++dwGlobalLabel;
	
		fprintf(fp, "		xor	edx, edx	; Make sure we don't hose things\n");
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		// This is the actual move

		if (0xb0 == dwOpcode || 0xb8 == dwOpcode)	// Repeat instruction?
		{
			fprintf(fp, "				while ((sdwCyclesRemaining > 0) && (cpu.z80BC))\n");

			fprintf(fp, "				{\n");			
		}

		ReadValueFromMemory("cpu.z80HL", "bTemp");
		WriteValueToMemory("cpu.z80DE", "bTemp");

		if ((dwOpcode & 0x0f) == 0)
		{
			fprintf(fp, "					++cpu.z80HL;\n");
			fprintf(fp, "					++cpu.z80DE;\n");
		}
		else				
		{
			fprintf(fp, "					--cpu.z80HL;\n");
			fprintf(fp, "					--cpu.z80DE;\n");
		}

		fprintf(fp, "				--cpu.z80BC;\n");
		fprintf(fp, "				cpu.z80HL &= 0xffff;\n");
		fprintf(fp, "				cpu.z80DE &= 0xffff;\n");
		fprintf(fp, "				cpu.z80BC &= 0xffff;\n");

		if (0xb0 == dwOpcode || 0xb8 == dwOpcode)	// Repeat instruction?
		{
			fprintf(fp, "				sdwCyclesRemaining -= 21;\n");

			fprintf(fp, "				}\n");
		}

		// Time for a flag fixup!

		fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_HALF_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_OVERFLOW_PARITY);\n");
		fprintf(fp, "				if (cpu.z80BC)\n");
		fprintf(fp, "				{\n");

		if (0xb0 == dwOpcode || 0xb8 == dwOpcode)
		{
			fprintf(fp, "					pbPC -= 2;	/* Back up so we hit this instruction again */\n");
		}

		fprintf(fp, "					cpu.z80F |= Z80_FLAG_OVERFLOW_PARITY;\n");
		fprintf(fp, "				}\n");

		if (0xb0 == dwOpcode || 0xb8 == dwOpcode)	// Repeat instruction?
		{
			fprintf(fp, "				sdwCyclesRemaining -= 16;\n");
		}
	}
	else
		abort();
}

void IMHandler(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if (dwOpcode == 0x46)
			fprintf(fp, "		mov	dword [_z80interruptMode], 0 ; IM 0\n");

		if (dwOpcode == 0x56)
		{
			fprintf(fp, "		mov	dword [_z80interruptMode], 1 ; Interrupt mode 1\n");
			fprintf(fp, "		mov	word [_z80intAddr], 038h	; Interrupt mode 1 cmd!\n");
		}

		if (dwOpcode == 0x5e)
			fprintf(fp, "		mov	dword [_z80interruptMode], 2 ; IM 2\n");

		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (0x46 == dwOpcode)		// IM 0
			fprintf(fp, "				cpu.z80interruptMode = 0;\n");

		if (0x56 == dwOpcode)		// IM 1
		{
			fprintf(fp, "				cpu.z80interruptMode = 1;\n");
			fprintf(fp, "				cpu.z80intAddr = 0x38;\n");
		}

		if (0x5e == dwOpcode)		// IM 2
			fprintf(fp, "				cpu.z80interruptMode = 2;\n");
	}
	else
		abort();
	
}

void IRHandler(UINT32 dwOpcode)
{
   char *src, *dst;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
	   switch(dwOpcode) 
		{
	   	case 0x57:  
				dst = "al"; src="[_z80i]"; break;
		   case 0x5F:  
				dst = "al"; src="[_z80r]"; break;
		   case 0x47:  
				dst = "[_z80i]"; src="al"; break;
		   case 0x4F:  
				dst = "[_z80r]"; src="al"; break;
		default:
			abort();
	   }

	   ProcBegin(dwOpcode);

	   fprintf(fp, "           mov     %s, %s\n",dst,src);
	
		if (dwOpcode == 0x5f)
		{
			fprintf(fp, "		and	ah, 029h	; No N, H, Z, or S!\n");
			fprintf(fp, "		or	al,al	; Get appropriate flags\n");
			fprintf(fp, "		o16 pushf\n");
			fprintf(fp, "		pop	dx\n");
			fprintf(fp, "		and	dl, 0c0h\n");
			fprintf(fp, "		or	ah, dl	; OR In our S & Z flags\n");
	
			fprintf(fp, "		mov	dl, [_z80iff]\n");
			fprintf(fp, "		and	dl, IFF2\n");
			fprintf(fp, "		shl	dl, 1\n");
			fprintf(fp, "		or	ah, dl\n");

			// Randomize R

			fprintf(fp, "		mov	edx, [dwLastRSample]\n");
			fprintf(fp, "		sub	edx, edi\n");
			fprintf(fp, "		add	edx, [_z80rCounter]\n");
			fprintf(fp, "		shr	edx, 2\n");
			fprintf(fp, "		and	edx, 07fh\n");
			fprintf(fp, "		and	byte [_z80r], 80h\n");
			fprintf(fp, "		or		byte [_z80r], dl\n");

			fprintf(fp, "		xor	edx, edx\n");
			fprintf(fp, "		mov	[dwLastRSample], edi\n");
		}

		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (0x5f == dwOpcode)		// LD A, R
		{
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_HALF_CARRY | Z80_FLAG_SIGN | Z80_FLAG_ZERO | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE);\n");
			fprintf(fp, "				cpu.z80F |= bPostORFlags[cpu.z80r];\n");
			fprintf(fp, "				cpu.z80F = (cpu.z80F & ~(Z80_FLAG_OVERFLOW_PARITY)) | ((cpu.z80iff & IFF2) << 1);\n");
			fprintf(fp, "				cpu.z80A = cpu.z80r;\n");

			// Now randomize a little

			fprintf(fp, "				bTemp = (cpu.z80r + (cpu.z80B + sdwCyclesRemaining + 1 + cpu.z80H)) ^ cpu.z80A;\n");
			fprintf(fp, "				cpu.z80r = (cpu.z80r & 0x80) | (bTemp & 0x7f);\n");
		}
		else
		if (0x47 == dwOpcode)		// LD I, A
		{
		 	fprintf(fp, "				cpu.z80i = cpu.z80A;\n");
		}
		else
		if (0x57 == dwOpcode)		// LD A, I
		{
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE | Z80_FLAG_ZERO | Z80_FLAG_SIGN);\n");
			fprintf(fp, "				cpu.z80F |= ((cpu.z80iff & IFF2) << 1);\n");
			fprintf(fp, "				cpu.z80A = cpu.z80i;\n");
			fprintf(fp, "				cpu.z80F |= bPostORFlags[cpu.z80A];\n");
		}
		else
		if (0x4f == dwOpcode)		// LD R, A
		{
			fprintf(fp, "				cpu.z80r = cpu.z80A;\n");
		}
		else
		{
			InvalidInstructionC(2);
		}
	}
	else
		abort();
}

// DD/FD Area

void DDFDCBHandler(UINT32 dwOpcode)
{
	UINT32 dwData = 0;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp, "%sInst%.2x:\n", majorOp, dwOpcode);
		fprintf(fp, "		mov	dx, [esi]	; Get our instruction (and offset)\n");
		fprintf(fp, "		add	esi, 2	; Increment our PC\n");

		fprintf(fp, "		mov	byte [_orgval], dl ; Store our value\n");
		fprintf(fp, "		or	dl, dl\n");
		fprintf(fp, "		js	notNeg%" PRIu32 "\n", dwGlobalLabel);
		fprintf(fp, "		mov	byte [_orgval + 1], 00h;\n");

		fprintf(fp, " 		jmp	short jumpHandler%" PRIu32 "\n", dwGlobalLabel);
		fprintf(fp, "notNeg%" PRIu32 ":\n", dwGlobalLabel);
		fprintf(fp, "		mov	byte [_orgval + 1], 0ffh;	It's negative\n");
		fprintf(fp, "jumpHandler%" PRIu32 ":\n", dwGlobalLabel++);
		fprintf(fp, "		shl	ebx, 16	; Save BX away\n");
		fprintf(fp, "		mov	bx, [_z80%s]\n", mz80Index);
		fprintf(fp, "		add	[_orgval], bx\n");
		fprintf(fp, "		shr	ebx, 16	; Restore BX\n");
		fprintf(fp, "		mov	dl, dh	; Get our instruction\n");
		fprintf(fp, "		xor	dh, dh	; Zero this\n");
		fprintf(fp, "		jmp	dword [z80ddfdcbInstructions+edx*4]\n\n");
	}
	else
	if (MZ80_C == bWhat)
	{
		if (strcmp("cpu.z80IX", mz80Index) == 0)
			dwData = 0;
		else
			dwData = 1;

		fprintf(fp, "				DDFDCBHandler(%d);\n", dwData);
	}
	else
		abort();
}

void LoadIndexReg(UINT32 dwOpcode)
{
	INT8 string[150];

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		sprintf(string, "[_z80%s]", mz80Index);

		fprintf(fp, "		mov	dx, [esi]	 ; Get our address to store\n");
		fprintf(fp, "		add	esi, 2\n");

		ReadWordFromMemory("dx", string);
		fprintf(fp, "		xor	edx, edx\n");
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				dwAddr = *pbPC++;\n");
		fprintf(fp, "				dwAddr |= ((UINT32) *pbPC++ << 8);\n");
		ReadWordFromMemory("dwAddr", mz80Index);
	}
	else
		abort();
}

void StoreIndexReg(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		fprintf(fp, "		mov	dx, [esi]	 ; Get our address to store\n");
		fprintf(fp, "		add	esi, 2\n");
		fprintf(fp, "		mov	[_orgval], dx\n");

		fprintf(fp, "		mov	dl, [_z80%s]\n", mz80Index);
		WriteValueToMemory("[_orgval]", "dl");

		fprintf(fp, "		inc	word [_orgval]\n");

		fprintf(fp, "		mov	dl, [_z80%s + 1]\n", mz80Index);
		WriteValueToMemory("[_orgval]", "dl");
		fprintf(fp, "		xor	edx, edx\n");
	
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				dwAddr = *pbPC++;\n");
		fprintf(fp, "				dwAddr |= ((UINT32) *pbPC++ << 8);\n");
		WriteWordToMemory("dwAddr", mz80Index);
	}
	else
		abort();
}

void LdIndexPtrReg(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		IndexedOffset(mz80Index);

		// DX Contains the address

		WriteValueToMemory("dx", pbMathReg[dwOpcode & 0x07]);
		fprintf(fp, "		xor	edx, edx\n");

		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				sdwAddr = (INT8) *pbPC++;	// Get the offset\n");
		fprintf(fp, "				sdwAddr = ((INT32) %s + sdwAddr) & 0xffff;\n", mz80Index);

		WriteValueToMemory("sdwAddr", pbMathRegC[dwOpcode & 0x07]);
	}
	else
		abort();
}

void UndocMathIndex(UINT32 dwOpcode)
{
	UINT32 dwOpcode1 = 0;
	INT8 *pbIndexReg = NULL;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if (dwOpcode & 1)
			fprintf(fp, "		mov	dl, byte [_z80%s]\n", mz80Index);
		else
			fprintf(fp, "		mov	dl, byte [_z80%s + 1]\n", mz80Index);

		// Info is in DL - let's do the math operation

		fprintf(fp, "		sahf		; Store our flags in x86 flag reg\n");

		dwOpcode1 = (dwOpcode & 0xf8);	// Only the operation

		if (dwOpcode1 == 0x80)
			fprintf(fp, "		add	al, dl\n");
		else
		if (dwOpcode1 == 0x88)
			fprintf(fp, "		adc	al, dl\n");
		else
		if (dwOpcode1 == 0x90)
			fprintf(fp, "		sub	al, dl\n");
		else
		if (dwOpcode1 == 0x98)
			fprintf(fp, "		sbb	al, dl\n");
		else
		if (dwOpcode1 == 0xa0)
			fprintf(fp, "		and	al, dl\n");
		else
		if (dwOpcode1 == 0xa8)
			fprintf(fp, "		xor	al, dl\n");
		else
		if (dwOpcode1 == 0xb0)
			fprintf(fp, "		or	al, dl\n");
		else
		if (dwOpcode1 == 0xb8)
			fprintf(fp, "		cmp	al, dl\n");
		else
			abort();

		fprintf(fp, "		lahf		; Get flags back into AH\n");

		if (dwOpcode1 != 0xa8 && dwOpcode1 != 0xa0 && dwOpcode1 != 0xb0)
		{
			SetOverflow();
		}

		if (dwOpcode1 == 0xa8)
			fprintf(fp, "		and	ah, 0ech	; Only these flags matter!\n");

		if (dwOpcode1 == 0xa0)
		{
			fprintf(fp, "		and	ah, 0ech	; Only these flags matter!\n");
			fprintf(fp, "		or	ah, 010h	; Half carry gets set\n");
		}

		if (dwOpcode1 == 0xb0)
			fprintf(fp, "		and	ah, 0ech ; No H, N, or C\n");
	
		if (dwOpcode1 == 0xb8)
			fprintf(fp, "		or	ah, 02h	; Negative gets set on a compare\n");
	
		if (dwOpcode1 == 0x80 || dwOpcode1 == 0x88)
			fprintf(fp, "		and	ah, 0fdh ; No N!\n");
	
		if (dwOpcode1 == 0x90 || dwOpcode1 == 0x98)
			fprintf(fp, "		or	ah, 02h	; N Gets set!\n");
	
		if (dwOpcode1 == 0xb0)
			fprintf(fp, "		and	ah, 0ech ; No H, N, or C\n");
	
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (dwOpcode & 1)
			pbIndexReg = mz80IndexHalfLow;
		else
			pbIndexReg = mz80IndexHalfHigh;

		dwOpcode1 = (dwOpcode & 0xf8);	// Only the operation

		if (0x80 == dwOpcode1)	// ADD
		{
			fprintf(fp, "				bTemp2 = cpu.z80A + %s;\n", pbIndexReg);
			SetAddFlagsSZHVC("cpu.z80A", pbIndexReg);
		}
		else
		if (0x88 == dwOpcode1)	// ADC
		{
			fprintf(fp, "				bTemp2 = cpu.z80A + %s + (cpu.z80F & Z80_FLAG_CARRY);\n", pbIndexReg);
			SetAdcFlagsSZHVC("cpu.z80A", pbIndexReg);
		}
		else
		if (0x90 == dwOpcode1)	// SUB
		{
			fprintf(fp, "				bTemp2 = cpu.z80A - %s;\n", pbIndexReg);
			SetSubFlagsSZHVC("cpu.z80A", pbIndexReg);
		}
		else								
		if (0x98 == dwOpcode1)	// SBC
		{
			fprintf(fp, "				bTemp2 = cpu.z80A - %s - (cpu.z80F & Z80_FLAG_CARRY);\n", pbIndexReg);
			SetSbcFlagsSZHVC("cpu.z80A", pbIndexReg);
		}
		else
		if (0xa0 == dwOpcode1)	// AND
		{
			fprintf(fp, "				cpu.z80A &= %s;\n", pbIndexReg);
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_HALF_CARRY | Z80_FLAG_ZERO | Z80_FLAG_SIGN);\n");
			fprintf(fp, "				cpu.z80F |= bPostANDFlags[cpu.z80A];\n\n");
		}
		else
		if (0xa8 == dwOpcode1)	// XOR
		{
			fprintf(fp, "				cpu.z80A ^= %s;\n", pbIndexReg);
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_HALF_CARRY | Z80_FLAG_ZERO | Z80_FLAG_SIGN);\n");
			fprintf(fp, "				cpu.z80F |= bPostORFlags[cpu.z80A];\n\n");
		}
		else
		if (0xb0 == dwOpcode1)	// OR
		{
			fprintf(fp, "				cpu.z80A |= %s;\n", pbIndexReg);
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_HALF_CARRY | Z80_FLAG_ZERO | Z80_FLAG_SIGN);\n");
			fprintf(fp, "				cpu.z80F |= bPostORFlags[cpu.z80A];\n\n");
		}
		else
		if (0xb8 == dwOpcode1)	// CP - Don't do anything! Just flags!
		{
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_HALF_CARRY | Z80_FLAG_ZERO | Z80_FLAG_SIGN);\n");
			fprintf(fp, "				cpu.z80F |= bPostORFlags[cpu.z80A];\n\n");
		}
		else
		{
			abort();
		}

		InvalidInstructionC(2);
	}
	else
		abort();
}

void UndocLoadHalfIndexReg(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		fprintf(fp, "		mov	dl, [esi]	; Get immediate byte to load\n");
		fprintf(fp, "		inc	esi	; Next byte\n");

		if (dwOpcode == 0x26)
			fprintf(fp, "		mov	byte [_z80%s + 1], dl\n", mz80Index);
		if (dwOpcode == 0x2e)
			fprintf(fp, "		mov	byte [_z80%s], dl\n", mz80Index);

		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (dwOpcode & 0x08)
			fprintf(fp, "			%s = *pbPC++;\n", mz80IndexHalfLow);
		else
			fprintf(fp, "			%s = *pbPC++;\n", mz80IndexHalfHigh);
	}
	else
		abort();
}

void UndocIncDecIndexReg(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		fprintf(fp, "		sahf\n");

		if (dwOpcode == 0x24)
			fprintf(fp, "		inc	byte [_z80%s + 1]\n", mz80Index);
		if (dwOpcode == 0x25)
			fprintf(fp, "		dec	byte [_z80%s + 1]\n", mz80Index);

		if (dwOpcode == 0x2c)
			fprintf(fp, "		inc	byte [_z80%s]\n", mz80Index);
		if (dwOpcode == 0x2d)
			fprintf(fp, "		dec	byte [_z80%s]\n", mz80Index);

		fprintf(fp,     "		lahf\n");
		SetOverflow();

		if ((0x24 == dwOpcode) || (0x2c == dwOpcode))
			fprintf(fp, "		and	ah, 0fdh	; Knock out N!\n");
		else
			fprintf(fp, "		or	ah, 02h	; Set negative!\n");

		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_SIGN | Z80_FLAG_ZERO | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE);\n");

		if (0x24 == dwOpcode || 0x2c == dwOpcode)
		{
			if (dwOpcode & 0x08)
				fprintf(fp, "				cpu.z80F |= bPostIncFlags[%s++];\n", mz80IndexHalfLow);
			else	
				fprintf(fp, "				cpu.z80F |= bPostIncFlags[%s++];\n", mz80IndexHalfHigh);
		}
		else
		{
			if (dwOpcode & 0x08)
				fprintf(fp, "				cpu.z80F |= bPostDecFlags[%s--];\n", mz80IndexHalfLow);
			else	
				fprintf(fp, "				cpu.z80F |= bPostDecFlags[%s--];\n", mz80IndexHalfHigh);
		}
	}
	else
		abort();
}

void ExIndexed(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if( bThroughCallHandler )
		{
			fprintf(fp, "		mov dx, word [_z80%s]\n", mz80Index);
			fprintf(fp, "		push dx\n");
			fprintf(fp, "		call PopWord\n");
			fprintf(fp, "		mov	[_z80%s], dx\n", mz80Index);
			fprintf(fp, "		pop dx\n");
			fprintf(fp, "		mov [_wordval], dx\n" );
			fprintf(fp, "		call PushWord\n" );
		}  
		else  
		{
			fprintf(fp, "		mov	[cyclesRemaining], edi\n");
			fprintf(fp, "		mov	dx, word [_z80sp]\n");
			fprintf(fp, "		xor	edi, edi\n");
			fprintf(fp, "		mov	di, [_z80%s]\n", mz80Index);
			fprintf(fp, "		xchg	di, [ebp+edx]\n");
			fprintf(fp, "		mov	[_z80%s], di\n", mz80Index);
			fprintf(fp, "		xor	edx, edx\n");
			fprintf(fp, "		mov	edi, [cyclesRemaining]\n");
		}

		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		ReadWordFromMemory("cpu.z80sp", "dwAddr");
		WriteWordToMemory("cpu.z80sp", mz80Index);
		fprintf(fp, "				%s = dwAddr;\n", mz80Index);
	}
	else
		abort();
}

void IncDecIndexReg(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if (dwOpcode == 0x23)
			fprintf(fp, "		inc	word [_z80%s]	; Increment our mz80Index register\n", mz80Index);
		else
			fprintf(fp, "		dec	word [_z80%s]	; Increment our mz80Index register\n", mz80Index);

		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (0x23 == dwOpcode)
		{
			fprintf(fp, "				%s++;\n", mz80Index);
		}
		else
		{
			fprintf(fp, "				%s--;\n", mz80Index);
		}

		fprintf(fp, "				%s &= 0xffff;\n", mz80Index);
	}
	else
		abort();
}

void LdRegIndexOffset(UINT32 dwOpcode)
{
	UINT32 dwOpcode1 = 0;

	dwOpcode1 = (dwOpcode & 0x38) >> 3;
	
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		IndexedOffset(mz80Index);
	
		ReadValueFromMemory("dx", pbMathReg[dwOpcode1]);
	
		fprintf(fp, "		xor	edx, edx	; Make sure we don't hose things\n");
		dwGlobalLabel++;
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				sdwAddr = (INT8) *pbPC++;	// Get the offset\n");
		fprintf(fp, "				sdwAddr = ((INT32) %s + sdwAddr) & 0xffff;\n", mz80Index);

		ReadValueFromMemory("sdwAddr", pbMathRegC[dwOpcode1]);
	}
	else
		abort();
}

void LdByteToIndex(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		fprintf(fp, "		mov	dx, [esi]	; Get our address\n");
		fprintf(fp, "		add	esi, 2	; Skip over our storage bytes\n");
		fprintf(fp, "		mov	[cyclesRemaining], edi\n");
		fprintf(fp, "		mov	di, dx	; Store it here for later\n");
		fprintf(fp, "		xor	dh, dh\n");
		fprintf(fp, "		or	dl, dl\n");
		fprintf(fp, "		jns	noNegate%" PRIu32 "\n", dwGlobalLabel);
		fprintf(fp, "		dec	dh\n");
		fprintf(fp, "noNegate%" PRIu32 ":\n", dwGlobalLabel);
		fprintf(fp, "		add	dx, [_z80%s]	; Add in our index\n", mz80Index);
		fprintf(fp, "		mov	[_orgval], dx	; Store our address to write to\n");
		fprintf(fp, "		mov	dx, di\n");
		fprintf(fp, "		xchg	dh, dl\n");
		fprintf(fp, "		mov	edi, [cyclesRemaining]\n");
	
		WriteValueToMemory("[_orgval]", "dl");
	
		fprintf(fp, "		xor	edx, edx\n");
		++dwGlobalLabel;
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				sdwAddr = (INT8) *pbPC++;	// Get the offset\n");
		fprintf(fp, "				sdwAddr = ((INT32) %s + sdwAddr) & 0xffff;\n", mz80Index);

		WriteValueToMemory("sdwAddr", "*pbPC++");
	}
	else
		abort();
}


void SPToIndex(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		fprintf(fp, "		mov	dx, [_z80%s] ; Get our source register\n", mz80Index);
		fprintf(fp, "		mov	word [_z80sp], dx	; Store our new SP\n");
		fprintf(fp, "		xor	edx, edx\n");

		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				cpu.z80sp = %s;\n", mz80Index);
	}
	else
		abort();
}

void AddIndexHandler(UINT32 dwOpcode)
{
	UINT8 bRegPair;

	bRegPair = dwOpcode >> 4;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		fprintf(fp, "		mov	dh, ah	; Get our flags\n");
		fprintf(fp, "		and	dh, 0ech	; Preserve the top three and bits 2 & 3\n");

		fprintf(fp, "		mov	[cyclesRemaining], edi\n");
		fprintf(fp, "		mov	di, [_z80%s]	; Get our value\n", mz80Index);
		fprintf(fp, "		mov	[_orgval], di	; Store our original value\n");
		fprintf(fp, "		add	di, %s\n", pbIndexedRegPairs[(dwOpcode & 0x30) >> 4]);
		fprintf(fp, "		lahf\n");
		fprintf(fp, "		mov	[_z80%s], di	; Store our register back\n", mz80Index);

		fprintf(fp, "		mov	di, [_orgval]	; Get original\n");
		fprintf(fp, "		xor	di, word [_z80%s] ; XOR It with our computed value\n", mz80Index);
		fprintf(fp, "		xor	di, %s\n", pbIndexedRegPairs[(dwOpcode & 0x30) >> 4]);
		fprintf(fp, "		and	di, 1000h	; Just our half carry\n");
		fprintf(fp, "		or		dx, di	; Or in our flags\n");
		fprintf(fp, "		and	ah, 01h	; Just carry\n");
		fprintf(fp, "		or	ah, dh\n");
		fprintf(fp, "		mov	edi, [cyclesRemaining]\n");
		fprintf(fp, "		xor	edx, edx\n");
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (bRegPair != 2)
		{ 
			fprintf(fp, "			cpu.z80F &= ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_HALF_CARRY);\n");
			fprintf(fp, "			dwTemp = %s + %s;\n", mz80Index, pbRegPairsC[bRegPair]);
			fprintf(fp, "			cpu.z80F |= ((dwTemp >> 16) & Z80_FLAG_CARRY) | (((%s ^ dwTemp ^ %s) >> 8) & Z80_FLAG_HALF_CARRY);\n", mz80Index, pbRegPairsC[bRegPair]);
			fprintf(fp, "			%s = dwTemp & 0xffff;\n", mz80Index);
		}
		else
		{
			fprintf(fp, "			cpu.z80F &= ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_HALF_CARRY);\n");
			fprintf(fp, "			dwTemp = %s + %s;\n", mz80Index, mz80Index);
			fprintf(fp, "			cpu.z80F |= ((dwTemp >> 16) & Z80_FLAG_CARRY) | (((%s ^ dwTemp ^ %s) >> 8) & Z80_FLAG_HALF_CARRY);\n", mz80Index, pbRegPairsC[bRegPair]);
			fprintf(fp, "			%s = dwTemp & 0xffff;\n", mz80Index);
		}
	}
	else
		abort();
}

void JPIXIYHandler(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		fprintf(fp, "		mov	dx, [_z80%s]	; Get our value\n", mz80Index);
		fprintf(fp, "		mov	esi, edx		; New PC!\n");
		fprintf(fp, "		add	esi, ebp		; Add in our base\n");
		fprintf(fp, "		xor	edx, edx\n");
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				pbPC = cpu.z80Base + %s;\n", mz80Index);
	}
	else
		abort();
}

void IncDecIndexed(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		IndexedOffset(mz80Index);

		fprintf(fp, "		mov	[_orgval], dx\n");

		ReadValueFromMemory("dx", "dl");

		fprintf(fp, "		sahf\n");

		if (dwOpcode == 0x34)
			fprintf(fp, "		inc	dl\n");
		else
			fprintf(fp, "		dec	dl\n");
		fprintf(fp, "		lahf\n");

		fprintf(fp, "		o16	pushf\n");
		fprintf(fp, "		shl	edx, 16\n");
		fprintf(fp, "		and	ah, 0fbh	;	Knock out parity/overflow\n");
		fprintf(fp, "		pop	dx\n");
		fprintf(fp, "		and	dh, 08h ; Just the overflow\n");
		fprintf(fp, "		shr	dh, 1	; Shift it into position\n");
		fprintf(fp, "		or	ah, dh	; OR It in with the real flags\n");

		fprintf(fp, "		shr	edx, 16\n");

		if (dwOpcode == 0x34)
			fprintf(fp, "		and	ah, 0fdh	; Knock out N!\n");
		else
			fprintf(fp, "		or		ah, 02h	; Make it N!\n");

		WriteValueToMemory("[_orgval]", "dl");

		fprintf(fp, "		xor	edx, edx\n");

		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				sdwAddr = (INT8) *pbPC++;	/* Get LSB first */\n");
		fprintf(fp, "				dwAddr = (sdwAddr + (INT32) %s) & 0xffff;\n", mz80Index);

		ReadValueFromMemory("dwAddr", "bTemp");

		fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_SIGN | Z80_FLAG_ZERO | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE);\n");
		
		if (0x34 == dwOpcode)
		{
			fprintf(fp ,"				cpu.z80F |= bPostIncFlags[bTemp++];\n");
		}
		else
		{
			fprintf(fp ,"				cpu.z80F |= bPostDecFlags[bTemp--];\n");
		}
	
		WriteValueToMemory("dwAddr", "bTemp");
	}
	else
		abort();
}

void MathOperationIndexed(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		IndexedOffset(mz80Index);
		ReadValueFromMemory("dx", "dl");

		fprintf(fp, "		sahf\n");

		if (dwOpcode == 0x86)           // Add
			fprintf(fp, "		add	al, dl\n");
		if (dwOpcode == 0x8e)           // Adc
			fprintf(fp, "		adc	al, dl\n");
		if (dwOpcode == 0x96)           // Sub
			fprintf(fp, "		sub	al, dl\n");
		if (dwOpcode == 0x9e)           // Sbc
			fprintf(fp, "		sbb	al, dl\n");
		if (dwOpcode == 0xa6)           // And
			fprintf(fp, "		and	al, dl\n");
		if (dwOpcode == 0xae)           // Xor
			fprintf(fp, "		xor	al, dl\n");
		if (dwOpcode == 0xb6)           //      Or
			fprintf(fp, "		or	al, dl\n");
		if (dwOpcode == 0xbe)           // Cp
			fprintf(fp, "		cmp	al, dl\n");

		fprintf(fp, "		lahf\n");

		if (dwOpcode == 0x86 || dwOpcode == 0x8e)
		{
			SetOverflow();
			fprintf(fp, "		and	ah, 0fdh	; Knock out negative\n");
		}

		if (dwOpcode == 0x96 || dwOpcode == 0x9e || dwOpcode == 0xbe)
		{
			SetOverflow();
			fprintf(fp, "		or	ah, 02h	; Set negative\n");
		}

		if (dwOpcode == 0xae || dwOpcode == 0xb6)
			fprintf(fp, "		and	ah, 0ech	; Knock out H, N, and C\n");

		if (dwOpcode == 0xa6)
		{
			fprintf(fp, "		and	ah,0fch	; Knock out N & C\n");
			fprintf(fp, "		or	ah, 10h	; Set half carry\n");
		}

		fprintf(fp, "		xor	edx, edx\n");
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "				sdwAddr = (INT8) *pbPC++;	/* Get LSB first */\n");
		fprintf(fp, "				dwAddr = (sdwAddr + (INT32) %s) & 0xffff;\n", mz80Index);

		ReadValueFromMemory("dwAddr", "bTemp");

		if (0x86 == dwOpcode)		// ADD A, (IX/IY+nn)
		{
			SetAddFlagsSZHVC("cpu.z80A", "bTemp");
			fprintf(fp, "				cpu.z80A += bTemp;\n");
		}
		else
		if (0x8e == dwOpcode)		// ADC A, (IX/IY+nn)
		{
			fprintf(fp, "				bTemp2 = (cpu.z80F & Z80_FLAG_CARRY);\n");
			SetAdcFlagsSZHVC("cpu.z80A", "bTemp");
			fprintf(fp, "				cpu.z80A += bTemp + bTemp2;\n");
		}
		else
		if (0x96 == dwOpcode)		// SUB A, (IX/IY+nn)
		{
			SetSubFlagsSZHVC("cpu.z80A", "bTemp");
			fprintf(fp, "				cpu.z80A -= bTemp;\n");
		}
		else
		if (0x9e == dwOpcode)		// SBC A, (IX/IY+nn)
		{
			fprintf(fp, "				bTemp2 = cpu.z80A;\n");
			fprintf(fp, "				cpu.z80A = cpu.z80A - bTemp - (cpu.z80F & Z80_FLAG_CARRY);\n");
			SetSbcFlagsSZHVC("bTemp2", "bTemp");
		}
		else
		if (0xa6 == dwOpcode)		// AND A, (IX/IY+nn)
		{
			fprintf(fp, "				cpu.z80A &= bTemp;\n");
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_HALF_CARRY | Z80_FLAG_ZERO | Z80_FLAG_SIGN);\n");
			fprintf(fp, "				cpu.z80F |= bPostANDFlags[cpu.z80A];\n\n");
		}
		else
		if (0xae == dwOpcode)		// XOR A, (IX/IY+nn)
		{
			fprintf(fp, "				cpu.z80A ^= bTemp;\n");
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_HALF_CARRY | Z80_FLAG_ZERO | Z80_FLAG_SIGN);\n");
			fprintf(fp, "				cpu.z80F |= bPostORFlags[cpu.z80A];\n\n");
		}
		else
		if (0xb6 == dwOpcode)		// OR A, (IX/IY+nn)
		{
			fprintf(fp, "				cpu.z80A |= bTemp;\n");
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_CARRY | Z80_FLAG_NEGATIVE | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_HALF_CARRY | Z80_FLAG_ZERO | Z80_FLAG_SIGN);\n");
			fprintf(fp, "				cpu.z80F |= bPostORFlags[cpu.z80A];\n\n");
		}
		else
		if (0xbe == dwOpcode)		// CP A, (IX/IY+nn)
		{
			SetSubFlagsSZHVC("cpu.z80A", "bTemp");
		}
		else
			InvalidInstructionC(2);
	}
	else
		abort();
}

void UndocIndexToReg(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if ((dwOpcode & 0x07) == 2 || (dwOpcode & 0x07) == 3)
			fprintf(fp, "	mov	dx, [_z80de]	; Get DE\n");

		if ((dwOpcode & 0x07) == 4)
			fprintf(fp, "	mov	dh, byte [_z80%s + 1]\n", mz80Index);
		if ((dwOpcode & 0x07) == 5)
			fprintf(fp, "	mov	dl, byte [_z80%s]\n", mz80Index);

		fprintf(fp, "		mov   byte [_z80%s + %" PRIu32 "], %s\n", mz80Index, 1 - ((dwOpcode & 0x08) >> 3), pbLocalReg[dwOpcode & 0x07]);
		fprintf(fp, "		xor	edx, edx\n");
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (dwOpcode != 0x64 && dwOpcode != 0x65 && dwOpcode != 0x6c && dwOpcode != 0x6d)
		{
			if (dwOpcode & 0x08)
				fprintf(fp, "			%s = %s;\n", mz80IndexHalfLow, pbLocalRegC[dwOpcode & 0x07]);
			else
				fprintf(fp, "			%s = %s;\n", mz80IndexHalfHigh, pbLocalRegC[dwOpcode & 0x07]);
		}
		else		// IX/IY High/low weirdness afoot...
		{
			// We don't generate any code for ld indexH, indexH and ld indexL, indexL

			if (0x65 == dwOpcode)		// LD indexH, indexL
			{
				fprintf(fp, "			%s = %s;\n", mz80IndexHalfHigh, mz80IndexHalfLow);
			}
			else
			if (0x6c == dwOpcode)		// LD indexH, indexL
			{
				fprintf(fp, "			%s = %s;\n", mz80IndexHalfLow, mz80IndexHalfHigh);
			}
		}
	}
	else
		abort();
}

void UndocRegToIndex(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		if ((dwOpcode & 0x38) == 0x10 || (dwOpcode & 0x38) == 0x18)
			fprintf(fp, "		mov	dx, [_z80de]	; Get a usable copy of DE here\n");

		fprintf(fp, "		mov	%s, byte [_z80%s + %" PRIu32 "]\n", pbLocalReg[(dwOpcode >> 3) & 0x07], mz80Index, 1 - (dwOpcode & 1));

		if ((dwOpcode & 0x38) == 0x10 || (dwOpcode & 0x38) == 0x18)
			fprintf(fp, "		mov	[_z80de], dx	; Put it back!\n");

		fprintf(fp, "		xor	edx, edx\n");
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (dwOpcode & 1)
			fprintf(fp, "			%s = %s;\n", pbLocalRegC[(dwOpcode >> 3) & 0x07], mz80IndexHalfLow);
		else
			fprintf(fp, "			%s = %s;\n", pbLocalRegC[(dwOpcode >> 3) & 0x07], mz80IndexHalfHigh);
	}
	else
		abort();
}

void LoadImmediate(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);

		fprintf(fp, "		mov	dx, [esi]	; Get our word to load\n");
		fprintf(fp, "		add	esi, 2	; Advance past the word\n");
		fprintf(fp, "		mov	[_z80%s], dx ; Store our new value\n", mz80Index);
		fprintf(fp, "		xor	edx, edx\n");

		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "		%s = *pbPC++;\n", mz80Index);
		fprintf(fp, "		%s |= ((UINT32) *pbPC++ << 8);\n", mz80Index);
	}
	else
		abort();
}

void PushPopOperationsIndexed(UINT32 dwOpcode)
{
	INT8 bRegBaseLsb[25];
	INT8 bRegBaseMsb[25];
	INT8 string[150];

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		sprintf(bRegBaseLsb, "byte [_z80%s]", mz80Index);
		sprintf(bRegBaseMsb, "byte [_z80%s + 1]", mz80Index);

		sprintf(string, "[_z80%s]", mz80Index);

		ProcBegin(dwOpcode);

		if (dwOpcode == 0xe5)	// Push IX/IY
		{
			fprintf(fp, "		sub	word [_z80sp], 2\n");
			fprintf(fp, "		mov	dx, [_z80sp]\n");
	
			WriteWordToMemory("dx", string);		
		}
		else	// Pop
		{
			fprintf(fp, "		mov	dx, [_z80sp]\n");
			ReadWordFromMemory("dx", string);
			fprintf(fp, "		add	word [_z80sp], 2\n");
		}	

		fprintf(fp, "		xor	edx, edx\n");
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (0xe5 == dwOpcode)	// Push IX/IY
		{
			fprintf(fp, "					cpu.z80sp -= 2;\n");
			fprintf(fp, "					pbSP = (cpu.z80Base + cpu.z80sp);	/* Normalize the stack pointer */\n");
			
			WriteWordToMemory("cpu.z80sp", mz80Index);
		}
		else
		if (0xe1 == dwOpcode)	// Pop IX/IY
		{
			ReadWordFromMemory("cpu.z80sp", mz80Index);

			fprintf(fp, "					cpu.z80sp += 2;\n");
			fprintf(fp, "					pbSP = (cpu.z80Base + cpu.z80sp);	/* Normalize the stack pointer */\n");
			return;
		}
	}
	else
		abort();
}

// DDFD XXCB Instructions

void ddcbBitWise(UINT32 dwOpcode)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		ProcBegin(dwOpcode);
	
		// NOTE: _orgval contains the address to get from. It includes the offset
		// already computed plus the mz80Index register.

		// Read our byte

		fprintf(fp, "		mov	dx, [_orgval]	; Get our target address\n");
		ReadValueFromMemory("dx", "dl");

		// Do the operation

		if (dwOpcode != 0x06 && dwOpcode != 0x0e &&
			 dwOpcode != 0x16 && dwOpcode != 0x1e &&
			 dwOpcode != 0x26 && dwOpcode != 0x2e &&
			 dwOpcode != 0x3e && (dwOpcode & 0xc7) != 0x86 &&
			 (dwOpcode & 0xc7) != 0xc6)
		{
			fprintf(fp, "		mov	dh, ah	; Store our original flags\n");
			fprintf(fp, "		and	dh, 29h	; Keep our old flags\n");
		}

		if ((dwOpcode & 0xc7) != 0x86 && (dwOpcode & 0xc7) != 0xc6)
			fprintf(fp, "		sahf		; Restore our flags\n");

		if (dwOpcode == 0x06)
			fprintf(fp, "		rol	dl, 1\n");
		if (dwOpcode == 0x0e)
			fprintf(fp, "		ror	dl, 1\n");
		if (dwOpcode == 0x16)
			fprintf(fp, "		rcl	dl, 1\n");
		if (dwOpcode == 0x1e)
			fprintf(fp, "		rcr	dl, 1\n");
		if (dwOpcode == 0x26)
			fprintf(fp, "		shl	dl, 1\n");
		if (dwOpcode == 0x2e)
			fprintf(fp, "		sar	dl, 1\n");
		if (dwOpcode == 0x3e)
			fprintf(fp, "		shr	dl, 1\n");

		// BIT, AND, and OR

		if ((dwOpcode & 0xc7) == 0x46)
			fprintf(fp, "		test	dl, 0%.2xh	; Is it set?\n", (1 << ((dwOpcode >> 3) & 0x07)));
		else
		if ((dwOpcode & 0xc7) == 0x86)
			fprintf(fp, "		and	dl, 0%.2xh	; Reset the bit\n", 
						0xff - (1 << ((dwOpcode >> 3) & 0x07)));
		else
		if ((dwOpcode & 0xc7) == 0xc6)
			fprintf(fp, "		or	dl, 0%.2xh	; Set the bit\n",
						(1 << ((dwOpcode >> 3) & 0x07)));
	
		if ((dwOpcode & 0xc7) != 0x86 && (dwOpcode & 0xc7) != 0xc6)
			fprintf(fp, "		lahf		; Get our flags back\n");  

		// Do the flag fixup (if any)

		if (dwOpcode == 0x26 || dwOpcode == 0x2e || ((dwOpcode & 0xc7) == 0x46))
			fprintf(fp, "		and	ah, 0edh	; No Half carry or negative!\n");
	
		if (dwOpcode == 0x06 || dwOpcode == 0x0e ||
			 dwOpcode == 0x16 || dwOpcode == 0x1e ||
			 dwOpcode == 0x3e)
			fprintf(fp, "		and	ah, 0edh	; Knock out H & N\n");

		// BIT!

		if ((dwOpcode & 0xc7) == 0x46)
		{
			fprintf(fp, "		or	ah, 10h	; OR In our half carry\n");
			fprintf(fp, "		and	ah, 0d0h ; New flags\n");
			fprintf(fp, "		or	ah, dh	; OR In our old flags\n");
		}

		// Now write our data back if it's not a BIT instruction

		if ((dwOpcode & 0xc7) != 0x46)  // If it's not a BIT, write it back
			WriteValueToMemory("[_orgval]", "dl");
	
		fprintf(fp, "		xor	edx, edx\n");
		FetchNextInstruction(dwOpcode);
	}
	else
	if (MZ80_C == bWhat)
	{
		if (0x06 == dwOpcode)		// RLC
		{
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_ZERO | Z80_FLAG_SIGN | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE | Z80_FLAG_CARRY);\n");
			fprintf(fp, "				bTemp2 = (bTemp >> 7);\n");
			fprintf(fp, "				bTemp = (bTemp << 1) | bTemp2;\n");
			fprintf(fp, "				cpu.z80F |= bTemp2 | bPostORFlags[bTemp];\n");
			WriteValueToMemory("dwAddr", "bTemp");
		}
		else
		if (0x0e == dwOpcode)		// RRC
		{
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_ZERO | Z80_FLAG_SIGN | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE | Z80_FLAG_CARRY);\n");
			fprintf(fp, "				cpu.z80F |= (bTemp & Z80_FLAG_CARRY);\n");
			fprintf(fp, "				bTemp = (bTemp >> 1) | (bTemp << 7);\n");
			fprintf(fp, "				cpu.z80F |= bPostORFlags[bTemp];\n");
			WriteValueToMemory("dwAddr", "bTemp");
		}
		else
		if (0x16 == dwOpcode)		// RL
		{
			fprintf(fp, "				bTemp2 = cpu.z80F & Z80_FLAG_CARRY;\n");
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_ZERO | Z80_FLAG_SIGN | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE | Z80_FLAG_CARRY);\n");
			fprintf(fp, "				cpu.z80F |= (bTemp >> 7);\n");
			fprintf(fp, "				bTemp = (bTemp << 1) | bTemp2;\n");
			fprintf(fp, "				cpu.z80F |= bPostORFlags[bTemp];\n");
			WriteValueToMemory("dwAddr", "bTemp");
		}
		else
		if (0x1e == dwOpcode)		// RR
		{
			fprintf(fp, "				bTemp2 = (cpu.z80F & Z80_FLAG_CARRY) << 7;\n");
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_ZERO | Z80_FLAG_SIGN | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE | Z80_FLAG_CARRY);\n");
			fprintf(fp, "				cpu.z80F |= (bTemp & Z80_FLAG_CARRY);\n");
			fprintf(fp, "				bTemp = (bTemp >> 1) | bTemp2;\n");
			fprintf(fp, "				cpu.z80F |= bPostORFlags[bTemp];\n");
			WriteValueToMemory("dwAddr", "bTemp");
		}
		else
		if (0x26 == dwOpcode)		// SLA
		{
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_ZERO | Z80_FLAG_SIGN | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE | Z80_FLAG_CARRY);\n");
			fprintf(fp, "				cpu.z80F |= (bTemp >> 7);\n");
			fprintf(fp, "				bTemp = (bTemp << 1);\n");
			fprintf(fp, "				cpu.z80F |= bPostORFlags[bTemp];\n");
			WriteValueToMemory("dwAddr", "bTemp");
		}
		else
		if (0x2e == dwOpcode)		// SRA
		{
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_ZERO | Z80_FLAG_SIGN | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE | Z80_FLAG_CARRY);\n");
			fprintf(fp, "				cpu.z80F |= (bTemp & Z80_FLAG_CARRY);\n");
			fprintf(fp, "				bTemp = (bTemp >> 1) | (bTemp & 0x80);\n");
			fprintf(fp, "				cpu.z80F |= bPostORFlags[bTemp];\n");
			WriteValueToMemory("dwAddr", "bTemp");
		}
		else
		if (0x3e == dwOpcode)		// SRL
		{
			fprintf(fp, "				cpu.z80F &= ~(Z80_FLAG_ZERO | Z80_FLAG_SIGN | Z80_FLAG_HALF_CARRY | Z80_FLAG_OVERFLOW_PARITY | Z80_FLAG_NEGATIVE | Z80_FLAG_CARRY);\n");
			fprintf(fp, "				cpu.z80F |= (bTemp & Z80_FLAG_CARRY);\n");
			fprintf(fp, "				bTemp = (bTemp >> 1);\n");
			fprintf(fp, "				cpu.z80F |= bPostORFlags[bTemp];\n");
			WriteValueToMemory("dwAddr", "bTemp");
		}
		else
		if ((dwOpcode & 0xc0) == 0x40)	// BIT
		{
			fprintf(fp, "				cpu.z80F = (cpu.z80F & ~(Z80_FLAG_ZERO | Z80_FLAG_NEGATIVE)) | Z80_FLAG_HALF_CARRY;\n");
			fprintf(fp, "				if (!(bTemp & 0x%.2x))\n", 1 << ((dwOpcode >> 3) & 0x07));
			fprintf(fp, "				{\n");
			fprintf(fp, "					cpu.z80F |= Z80_FLAG_ZERO;\n");
			fprintf(fp, "				}\n");
		}
		else
		if ((dwOpcode & 0xc0) == 0x80)	// RES
		{
			fprintf(fp, "				bTemp &= 0x%.2x;\n", ~(1 << ((dwOpcode >> 3) & 0x07)) & 0xff);
			WriteValueToMemory("dwAddr", "bTemp");
		}
		else
		if ((dwOpcode & 0xc0) == 0xC0)	// SET
		{
			fprintf(fp, "				bTemp |= 0x%.2x;\n", 1 << ((dwOpcode >> 3) & 0x07));
			WriteValueToMemory("dwAddr", "bTemp");
		}
		else
			InvalidInstructionC(4);
	}
	else
		abort();
}

void GetTicksCode(void)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp, "		global	_%sGetElapsedTicks\n", cpubasename);
		fprintf(fp, "		global	%sGetElapsedTicks_\n", cpubasename);
		fprintf(fp, "		global	%sGetElapsedTicks\n", cpubasename);
	
		Alignment();
		sprintf(procname, "%sGetElapsedTicks_", cpubasename);
		ProcBegin(0xffffffff);
		fprintf(fp, "_%sGetElapsedTicks:\n", cpubasename);
		fprintf(fp, "%sGetElapsedTicks:\n", cpubasename);
	
		if (bUseStack)
			fprintf(fp, "		mov	eax, [esp+4]	; Get our context address\n");
	
		fprintf(fp, "		or	eax, eax	; Should we clear it?\n");
		fprintf(fp, "		jz	getTicks\n");
		fprintf(fp, "		xor	eax, eax\n");
		fprintf(fp, "		xchg	eax, [dwElapsedTicks]\n");
		fprintf(fp, "		ret\n");
		fprintf(fp, "getTicks:\n");
		fprintf(fp, "		mov	eax, [dwElapsedTicks]\n");
		fprintf(fp, "		ret\n");
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "/* This will return the elapsed ticks */\n\n");
		fprintf(fp, "UINT32 %sGetElapsedTicks(UINT32 dwClear)\n", cpubasename);
		fprintf(fp, "{\n");
		fprintf(fp, "	UINT32 dwTemp = dwElapsedTicks;\n\n");
		fprintf(fp, "	if (dwClear)\n");
		fprintf(fp, "	{\n");
		fprintf(fp, "		dwElapsedTicks = 0;\n");
		fprintf(fp, "	}\n\n");
		fprintf(fp, "	return(dwTemp);\n");
		fprintf(fp, "}\n\n");
	}
	else
	{
		abort();
	}
}

void ReleaseTimesliceCode(void)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp, "		global	_%sReleaseTimeslice\n", cpubasename);
		fprintf(fp, "		global	%sReleaseTimeslice_\n", cpubasename);
		fprintf(fp, "		global	%sReleaseTimeslice\n", cpubasename);
	
		Alignment();
		sprintf(procname, "%sReleaseTimeslice_", cpubasename);
		ProcBegin(0xffffffff);
		fprintf(fp, "_%sReleaseTimeslice:\n", cpubasename);
		fprintf(fp, "%sReleaseTimeslice:\n", cpubasename);
	
		fprintf(fp, "		mov	eax, [cyclesRemaining]\n");
		fprintf(fp, "		sub	[dwOriginalExec], eax\n");
		fprintf(fp, "		mov	[cyclesRemaining], dword 0\n");
	
		fprintf(fp, "		ret\n\n");
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "/* Releases mz80 from its current timeslice */\n\n");
		fprintf(fp, "void %sReleaseTimeslice(void)\n", cpubasename);
		fprintf(fp, "{\n");
		fprintf(fp, "	dwOriginalCycles -= sdwCyclesRemaining;\n");
		fprintf(fp, "	sdwCyclesRemaining = 0;\n");
		fprintf(fp, "}\n\n");
	}
	else
	{
		abort();
	}
}

void DataSegment(void)
{
	UINT32 dwLoop = 0;
	UINT8 bUsed[256];

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		if (bOS2)
			fprintf(fp, "		section	.DATA32	use32 flat class=data\n");
		else
			fprintf(fp, "		section	.data	use32 flat class=data\n");
	
		Alignment();
		fprintf(fp, "		global	_%scontextBegin\n", cpubasename);
		fprintf(fp, "_%scontextBegin:\n", cpubasename);

		fprintf(fp, "		global	_z80pc\n");
		fprintf(fp, "		global	z80pc_\n");

		if (bPlain)
			fprintf(fp, "		global	z80pc\n");

		fprintf(fp, "		global	_z80nmiAddr\n");
		fprintf(fp, "		global	_z80intAddr\n");
		fprintf(fp, "		global	z80intAddr\n");

			fprintf(fp, "\n");
		fprintf(fp, "; DO NOT CHANGE THE ORDER OF AF, BC, DE, HL and THE PRIME REGISTERS!\n");
		fprintf(fp, "\n");
		fprintf(fp, "_z80Base	dd	0	; Base address for Z80 stuff\n");
		fprintf(fp, "_z80MemRead	dd	0	; Offset of memory read structure array\n");
		fprintf(fp, "_z80MemWrite	dd	0	; Offset of memory write structure array\n");
		fprintf(fp, "_z80IoRead	dd	0	; Base address for I/O reads list\n");
		fprintf(fp, "_z80IoWrite	dd	0	; Base address for I/O write list\n");
		fprintf(fp, "_z80clockticks	dd	0	; # Of clock tips that have elapsed\n");
		fprintf(fp, "_z80iff	dd	0	; Non-zero if we're in an interrupt\n");
		fprintf(fp, "_z80interruptMode dd	0	; Interrupt mode\n");
		fprintf(fp, "_z80halted	dd	0	; 0=Not halted, 1=Halted\n");
#ifdef MZ80_TRAP
		fprintf(fp, "_z80trapList	dd	0	; pointer to trap list\n");
		fprintf(fp, "_z80trapAddr	dw	0	; PC where trap occurred\n");
#endif
		fprintf(fp, "_z80af		dd	0	; A Flag & Flags\n");
		fprintf(fp, "_z80bc		dd	0	; BC\n");
		fprintf(fp, "_z80de		dd	0	; DE\n");
		fprintf(fp, "_z80hl		dd	0	; HL\n");
		fprintf(fp, "_z80afprime	dd	0	; A Flag & Flags prime\n");
		fprintf(fp, "_z80bcprime	dd	0	; BC prime\n");
		fprintf(fp, "_z80deprime	dd	0	; DE prime\n");
		fprintf(fp, "_z80hlprime	dd	0	; HL prime\n");
		fprintf(fp, "\n");
		fprintf(fp, "; The order of the following registers can be changed without adverse\n");
		fprintf(fp, "; effect. Keep the WORD and DWORDs on boundaries of two for faster access\n");
		fprintf(fp, "\n");
		fprintf(fp, "_z80ix		dd	0	; IX\n");
		fprintf(fp, "_z80iy		dd	0	; IY\n");
		fprintf(fp, "_z80sp		dd	0	; Stack pointer\n");
		
		if (bPlain)
			fprintf(fp,"z80pc:\n");
	
		fprintf(fp, "z80pc_:\n");
		fprintf(fp, "_z80pc		dd	0	; PC\n");
		fprintf(fp, "_z80nmiAddr	dd	0	; Address to jump to for NMI\n");
		fprintf(fp, "z80intAddr:\n");
		fprintf(fp, "_z80intAddr	dd	0	; Address to jump to for INT\n");
		fprintf(fp, "_z80rCounter	dd	0	; R Register counter\n");
		fprintf(fp, "_z80i		db	0	; I register\n");
		fprintf(fp, "_z80r		db	0	; R register\n");
		fprintf(fp, "_z80intPending	db	0	; Non-zero if an interrupt is pending\n");
		fprintf(fp, "\n");
		fprintf(fp, "_%scontextEnd:\n", cpubasename);
		Alignment();
		fprintf(fp, "dwElapsedTicks	dd	0	; # Of ticks elapsed\n");
		fprintf(fp, "cyclesRemaining	dd	0	; # Of cycles remaining\n");
		fprintf(fp, "dwOriginalExec	dd	0	; # Of cycles originally executing\n");
		fprintf(fp, "dwLastRSample	dd	0	; Last sample for R computation\n");
		fprintf(fp, "dwEITiming	dd	0	; Used when we cause an interrupt\n");
		fprintf(fp, "_orgval	dw	0	; Scratch area\n");
		fprintf(fp, "_orgval2	dw	0	; Scratch area\n");
		fprintf(fp, "_wordval	dw	0	; Scratch area\n");
		fprintf(fp, "_intData	db	0	; Interrupt data when an interrupt is pending\n");
		fprintf(fp, "bEIExit	db	0	; Are we exiting because of an EI instruction?\n");
		fprintf(fp, "\n");

		// Debugger junk

		fprintf(fp, "RegTextPC	db	'PC',0\n");
		fprintf(fp, "RegTextAF	db	'AF',0\n");
		fprintf(fp, "RegTextBC	db	'BC',0\n");
		fprintf(fp, "RegTextDE	db	'DE',0\n");
		fprintf(fp, "RegTextHL	db	'HL',0\n");
		fprintf(fp, "RegTextAFP	db	'AF',27h,0\n");
		fprintf(fp, "RegTextBCP	db	'BC',27h,0\n");
		fprintf(fp, "RegTextDEP	db	'DE',27h,0\n");
		fprintf(fp, "RegTextHLP	db	'HL',27h,0\n");
		fprintf(fp, "RegTextIX	db	'IX',0\n");
		fprintf(fp, "RegTextIY	db	'IY',0\n");
		fprintf(fp, "RegTextSP	db	'SP',0\n");
		fprintf(fp, "RegTextI	db	'I',0\n");
		fprintf(fp, "RegTextR	db	'R',0\n");

		// 8 Byte textual info

		fprintf(fp, "RegTextA	db	'A',0\n");
		fprintf(fp, "RegTextB	db	'B',0\n");
		fprintf(fp, "RegTextC	db	'C',0\n");
		fprintf(fp, "RegTextD	db	'D',0\n");
		fprintf(fp, "RegTextE	db	'E',0\n");
		fprintf(fp, "RegTextH	db	'H',0\n");
		fprintf(fp, "RegTextL	db	'L',0\n");
		fprintf(fp, "RegTextF	db	'F',0\n");

		// Individual flags

		fprintf(fp, "RegTextCarry	db	'Carry',0\n");
		fprintf(fp, "RegTextNegative	db	'Negative',0\n");
		fprintf(fp, "RegTextParity	db	'Parity',0\n");
		fprintf(fp, "RegTextOverflow	db	'Overflow',0\n");
		fprintf(fp, "RegTextHalfCarry	db	'HalfCarry',0\n");
		fprintf(fp, "RegTextZero	db	'Zero',0\n");
		fprintf(fp, "RegTextSign	db	'Sign',0\n");
		fprintf(fp, "RegTextIFF1	db	'IFF1',0\n");
		fprintf(fp, "RegTextIFF2	db	'IFF2',0\n\n");

		// Timing for interrupt modes

		fprintf(fp, "intModeTStates:\n");
		fprintf(fp, "		db	13	; IM 0 - 13 T-States\n");
		fprintf(fp, "		db	11	; IM 1 - 11 T-States\n");
		fprintf(fp, "		db	11	; IM 2 - 11 T-States\n\n");

		// Now the master reg/flag table

		fprintf(fp, "\n;\n");
		fprintf(fp, "; Info is in: pointer to text, address, shift value, mask value, size of data chunk\n");
		fprintf(fp, ";\n\n");
		fprintf(fp, "RegTable:\n");

		// Pointer to text, address, shift value, mask, size

		fprintf(fp, "		dd	RegTextPC, _z80pc - _%scontextBegin, 0, 0ffffh\n", cpubasename);
		fprintf(fp, "		dd	RegTextSP, _z80sp - _%scontextBegin, 0, 0ffffh\n", cpubasename);
		fprintf(fp, "		dd	RegTextAF, _z80af - _%scontextBegin, 0, 0ffffh\n", cpubasename);
		fprintf(fp, "		dd	RegTextBC, _z80bc - _%scontextBegin, 0, 0ffffh\n", cpubasename);
		fprintf(fp, "		dd	RegTextDE, _z80de - _%scontextBegin, 0, 0ffffh\n", cpubasename);
		fprintf(fp, "		dd	RegTextHL, _z80hl - _%scontextBegin, 0, 0ffffh\n", cpubasename);
		fprintf(fp, "		dd	RegTextAFP, _z80af - _%scontextBegin, 0, 0ffffh\n", cpubasename);
		fprintf(fp, "		dd	RegTextBCP, _z80bc - _%scontextBegin, 0, 0ffffh\n", cpubasename);
		fprintf(fp, "		dd	RegTextDEP, _z80de - _%scontextBegin, 0, 0ffffh\n", cpubasename);
		fprintf(fp, "		dd	RegTextHLP, _z80hl - _%scontextBegin, 0, 0ffffh\n", cpubasename);
		fprintf(fp, "		dd	RegTextIX, _z80ix - _%scontextBegin, 0, 0ffffh\n", cpubasename);
		fprintf(fp, "		dd	RegTextIY, _z80iy - _%scontextBegin, 0, 0ffffh\n", cpubasename);
		fprintf(fp, "		dd	RegTextI, _z80i - _%scontextBegin, 0, 0ffh\n", cpubasename);
		fprintf(fp, "		dd	RegTextR, _z80r - _%scontextBegin, 0, 0ffh\n", cpubasename);

		// Individual regs

		fprintf(fp, "		dd	RegTextA, (_z80af + 1) - _%scontextBegin, 0, 0ffh\n", cpubasename);
		fprintf(fp, "		dd	RegTextF, _z80af - _%scontextBegin, 0, 0ffh\n", cpubasename);
		fprintf(fp, "		dd	RegTextB, (_z80bc + 1) - _%scontextBegin, 0, 0ffh\n", cpubasename);
		fprintf(fp, "		dd	RegTextC, _z80bc - _%scontextBegin, 0, 0ffh\n", cpubasename);
		fprintf(fp, "		dd	RegTextD, (_z80de + 1) - _%scontextBegin, 0, 0ffh\n", cpubasename);
		fprintf(fp, "		dd	RegTextE, _z80de - _%scontextBegin, 0, 0ffh\n", cpubasename);
		fprintf(fp, "		dd	RegTextH, (_z80hl + 1) - _%scontextBegin, 0, 0ffh\n", cpubasename);
		fprintf(fp, "		dd	RegTextL, _z80hl - _%scontextBegin, 0, 0ffh\n", cpubasename);

		// IFF register

		fprintf(fp, "		dd	RegTextIFF1, _z80iff - _%scontextBegin, 0, 01h\n", cpubasename);
		fprintf(fp, "		dd	RegTextIFF2, _z80iff - _%scontextBegin, 1, 01h\n", cpubasename);

		// Individual flags

		fprintf(fp, "		dd	RegTextCarry, _z80af - _%scontextBegin, 0, 01h\n", cpubasename);
		fprintf(fp, "		dd	RegTextNegative, _z80af - _%scontextBegin, 1, 01h\n", cpubasename);
		fprintf(fp, "		dd	RegTextParity, _z80af - _%scontextBegin, 2, 01h\n", cpubasename);
		fprintf(fp, "		dd	RegTextOverflow, _z80af - _%scontextBegin, 2, 01h\n", cpubasename);
		fprintf(fp, "		dd	RegTextHalfCarry, _z80af - _%scontextBegin, 4, 01h\n", cpubasename);
		fprintf(fp, "		dd	RegTextZero, _z80af - _%scontextBegin, 6, 01h\n", cpubasename);
		fprintf(fp, "		dd	RegTextSign, _z80af - _%scontextBegin, 7, 01h\n", cpubasename);

		// Now we write out our tables
	
		Alignment();
	
		for (dwLoop = 0; dwLoop < 256; dwLoop++)
			bUsed[dwLoop] = 0;

	// Now rip through and find out what is and isn't used
	
		dwLoop = 0;
	
		while (StandardOps[dwLoop].Emitter)
		{
			assert(StandardOps[dwLoop].bOpCode < 0x100);
			if (bUsed[StandardOps[dwLoop].bOpCode])
			{
				fprintf(stderr, "Oops! %.2x\n", dwLoop);
				fclose(fp);
				exit(1);
			}
			bUsed[StandardOps[dwLoop].bOpCode] = 1;
			dwLoop++;
		}
	
		// Now that that's taken care of, emit the table
	
		fprintf(fp, "z80regular:\n");
	
		dwLoop = 0;
	
		while (dwLoop < 0x100)
		{
			fprintf(fp, "		dd	");
			if (bUsed[dwLoop])
				fprintf(fp, "RegInst%.2x", dwLoop);
			else
				fprintf(fp, "invalidInsByte");
			fprintf(fp, "\n");
			dwLoop++;
		}
		fprintf(fp, "\n");
	
		// Now rip through and find out what is and isn't used (CB Ops)
	
		for (dwLoop = 0; dwLoop < 0x100; dwLoop++)
			bUsed[dwLoop] = 0;
	
		dwLoop = 0;
	
		while (CBOps[dwLoop].Emitter)
		{
			assert(CBOps[dwLoop].bOpCode < 0x100);
			if (bUsed[CBOps[dwLoop].bOpCode])
			{
				fprintf(stderr, "Oops CB! %.2x\n", dwLoop);
				fclose(fp);
				exit(1);
			}
			bUsed[CBOps[dwLoop].bOpCode] = 1;
			dwLoop++;
		}
	
		dwLoop = 0;
	
		// Let's emit the CB prefixes
	
		fprintf(fp, "z80PrefixCB:\n");
	
		while (dwLoop < 0x100)
		{
			fprintf(fp, "		dd	");
			if (bUsed[dwLoop])
				fprintf(fp, "CBInst%.2x", dwLoop);
			else
				fprintf(fp, "invalidInsWord");
			fprintf(fp, "\n");
			dwLoop++;
		}
		fprintf(fp, "\n");
	
		// Now rip through and find out what is and isn't used (ED Ops)
	
		for (dwLoop = 0; dwLoop < 0x100; dwLoop++)
			bUsed[dwLoop] = 0;
	
		dwLoop = 0;
	
		while (EDOps[dwLoop].Emitter)
		{
			assert(EDOps[dwLoop].bOpCode < 0x100);
			if (bUsed[EDOps[dwLoop].bOpCode])
			{
				fprintf(stderr, "Oops ED! %.2x\n", dwLoop);
				fclose(fp);
				exit(1);
			}
			bUsed[EDOps[dwLoop].bOpCode] = 1;
			dwLoop++;
		}
	
		dwLoop = 0;
	
		// Let's emit the ED prefixes
	
		fprintf(fp, "z80PrefixED:\n");
	
		while (dwLoop < 0x100)
		{
			fprintf(fp, "		dd	");
			if (bUsed[dwLoop])
				fprintf(fp, "EDInst%.2x", dwLoop);
			else
				fprintf(fp, "invalidInsWord");
			fprintf(fp, "\n");
			dwLoop++;
		}
		fprintf(fp, "\n");
	
		// Now rip through and find out what is and isn't used (DD Ops)
	
		for (dwLoop = 0; dwLoop < 0x100; dwLoop++)
			bUsed[dwLoop] = 0;
	
		dwLoop = 0;
	
		while (DDFDOps[dwLoop].Emitter)
		{
			assert(DDFDOps[dwLoop].bOpCode < 0x100);
			if (bUsed[DDFDOps[dwLoop].bOpCode])
			{
				fprintf(stderr, "Oops DD! %.2x\n", bUsed[DDFDOps[dwLoop].bOpCode]);
				fclose(fp);
				exit(1);
			}
			bUsed[DDFDOps[dwLoop].bOpCode] = 1;
			dwLoop++;
		}
	
		dwLoop = 0;
	
		// Let's emit the DD prefixes
	
		fprintf(fp, "z80PrefixDD:\n");
	
		while (dwLoop < 0x100)
		{
			fprintf(fp, "		dd	");
			if (bUsed[dwLoop])
				fprintf(fp, "DDInst%.2x", dwLoop);
			else
				fprintf(fp, "invalidInsWord");
			fprintf(fp, "\n");
			dwLoop++;
		}
		fprintf(fp, "\n");
	
		// Now rip through and find out what is and isn't used (FD Ops)
	
		for (dwLoop = 0; dwLoop < 0x100; dwLoop++)
			bUsed[dwLoop] = 0;
	
		dwLoop = 0;
	
		while (DDFDOps[dwLoop].Emitter)
		{
			assert(DDFDOps[dwLoop].bOpCode < 0x100);
			if (bUsed[DDFDOps[dwLoop].bOpCode])
			{
				fprintf(stderr, "Oops FD! %.2x\n", dwLoop);
				fclose(fp);
				exit(1);
			}
			bUsed[DDFDOps[dwLoop].bOpCode] = 1;
			dwLoop++;
		}
	
		for (dwLoop = 0; dwLoop < 0x100; dwLoop++)
			bUsed[dwLoop] = 0;
	
		// Let's emit the DDFD prefixes
	
		for (dwLoop = 0; dwLoop < 0x100; dwLoop++)
			bUsed[dwLoop] = 0;
	
		dwLoop = 0;
	
		while (DDFDOps[dwLoop].Emitter)
		{
			assert(DDFDOps[dwLoop].bOpCode < 0x100);
			if (bUsed[DDFDOps[dwLoop].bOpCode])
			{
				fprintf(stderr, "Oops FD! %.2x\n", dwLoop);
				exit(1);
			}
			bUsed[DDFDOps[dwLoop].bOpCode] = 1;
			dwLoop++;
		}
	
		dwLoop = 0;
	
		// Let's emit the DDFD prefixes
	
		fprintf(fp, "z80PrefixFD:\n");
	
		while (dwLoop < 0x100)
		{
			fprintf(fp, "		dd	");
			if (bUsed[dwLoop])
				fprintf(fp, "FDInst%.2x", dwLoop);
			else
				fprintf(fp, "invalidInsWord");
			fprintf(fp, "\n");
			dwLoop++;
		}
	
		for (dwLoop = 0; dwLoop < 0x100; dwLoop++)
			bUsed[dwLoop] = 0;
	
		dwLoop = 0;
	
		while (DDFDCBOps[dwLoop].Emitter)
		{
			assert(DDFDCBOps[dwLoop].bOpCode < 0x100);
			if (bUsed[DDFDCBOps[dwLoop].bOpCode])
			{
				fprintf(stderr, "Oops CBFDDD! %.2x\n", bUsed[DDFDCBOps[dwLoop].bOpCode]);
				fclose(fp);
				exit(1);
			}
			bUsed[DDFDCBOps[dwLoop].bOpCode] = 1;
			dwLoop++;
		}
	
		// Let's emit the DDFD prefixes
	
		dwLoop = 0;
	
		fprintf(fp, "z80ddfdcbInstructions:\n");
	
		while (dwLoop < 0x100)
		{
			fprintf(fp, "		dd	");
			if (bUsed[dwLoop])
				fprintf(fp, "DDFDCBInst%.2x", dwLoop);
			else
				fprintf(fp, "invalidInsWord");
			fprintf(fp, "\n");
			dwLoop++;
		}
		fprintf(fp, "\n");
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "/* Modular global variables go here*/\n\n");
		fprintf(fp, "static CONTEXTMZ80 cpu;	/* CPU Context */\n");
		fprintf(fp, "static UINT8 *pbPC;			/* Program counter normalized */\n");
		fprintf(fp, "static UINT8 *pbSP;			/* Stack pointer normalized */\n");
		fprintf(fp, "static struct MemoryReadByte *psMemRead; /* Read memory structure */\n");
		fprintf(fp, "static struct MemoryWriteByte *psMemWrite; /* Write memory structure */\n");
		fprintf(fp, "static struct z80PortRead *psIoRead; /* Read I/O structure */\n");
		fprintf(fp, "static struct z80PortWrite *psIoWrite; /* Write memory structure */\n");
		fprintf(fp, "static INT32 sdwCyclesRemaining; /* Used as a countdown */\n");
		fprintf(fp, "static UINT32 dwReturnCode; /* Return code from exec() */\n");
		fprintf(fp, "static UINT32 dwOriginalCycles; /* How many cycles did we start with? */\n");
		fprintf(fp, "static UINT32 dwElapsedTicks;	/* How many ticks did we elapse? */\n");
		fprintf(fp, "static UINT32 sdwAddr;		/* Temporary address storage */\n");
		fprintf(fp, "static UINT32 dwAddr;		/* Temporary stack address */\n");
		fprintf(fp, "static UINT8 *pbAddAdcTable;	/* Pointer to add/adc flag table */\n");
		fprintf(fp, "static UINT8 *pbSubSbcTable;	/* Pointer to sub/sbc flag table */\n");
		fprintf(fp, "static UINT32 dwTemp;			/* Temporary value */\n\n");
		fprintf(fp, "static UINT8 bTemp;			/* Temporary value */\n\n");
		fprintf(fp, "static UINT8 bTemp2; 		/* Temporary value */\n\n");

		fprintf(fp, "/* Precomputed flag tables */\n\n");

		fprintf(fp, "static UINT8 bPostIncFlags[0x100] = \n");
		fprintf(fp, "{\n");
		fprintf(fp, "	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,\n");
		fprintf(fp, "	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,\n");
		fprintf(fp, "	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,\n");
		fprintf(fp, "	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,\n");
		fprintf(fp, "	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,\n");
		fprintf(fp, "	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,\n");
		fprintf(fp, "	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,\n");
		fprintf(fp, "	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x94,\n");
		fprintf(fp, "	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x90,\n");
		fprintf(fp, "	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x90,\n");
		fprintf(fp, "	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x90,\n");
		fprintf(fp, "	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x90,\n");
		fprintf(fp, "	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x90,\n");
		fprintf(fp, "	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x90,\n");
		fprintf(fp, "	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x90,\n");
		fprintf(fp, "	0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x50\n");
		fprintf(fp, "};\n\n");

		fprintf(fp, "static UINT8 bPostDecFlags[0x100] = \n");
		fprintf(fp, "{\n");
		fprintf(fp, "	0x92,0x42,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,\n");
		fprintf(fp, "	0x12,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,\n");
		fprintf(fp, "	0x12,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,\n");
		fprintf(fp, "	0x12,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,\n");
		fprintf(fp, "	0x12,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,\n");
		fprintf(fp, "	0x12,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,\n");
		fprintf(fp, "	0x12,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,\n");
		fprintf(fp, "	0x12,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02,\n");
		fprintf(fp, "	0x16,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,\n");
		fprintf(fp, "	0x92,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,\n");
		fprintf(fp, "	0x92,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,\n");
		fprintf(fp, "	0x92,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,\n");
		fprintf(fp, "	0x92,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,\n");
		fprintf(fp, "	0x92,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,\n");
		fprintf(fp, "	0x92,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,\n");
		fprintf(fp, "	0x92,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82,0x82\n");
		fprintf(fp, "};\n\n");

		fprintf(fp, "static UINT8 bPostORFlags[0x100] = \n");
		fprintf(fp, "{\n");
		fprintf(fp, "	0x44,0x00,0x00,0x04,0x00,0x04,0x04,0x00,0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,\n");
		fprintf(fp, "	0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,0x04,0x00,0x00,0x04,0x00,0x04,0x04,0x00,\n");
		fprintf(fp, "	0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,0x04,0x00,0x00,0x04,0x00,0x04,0x04,0x00,\n");
		fprintf(fp, "	0x04,0x00,0x00,0x04,0x00,0x04,0x04,0x00,0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,\n");
		fprintf(fp, "	0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,0x04,0x00,0x00,0x04,0x00,0x04,0x04,0x00,\n");
		fprintf(fp, "	0x04,0x00,0x00,0x04,0x00,0x04,0x04,0x00,0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,\n");
		fprintf(fp, "	0x04,0x00,0x00,0x04,0x00,0x04,0x04,0x00,0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,\n");
		fprintf(fp, "	0x00,0x04,0x04,0x00,0x04,0x00,0x00,0x04,0x04,0x00,0x00,0x04,0x00,0x04,0x04,0x00,\n");
		fprintf(fp, "	0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,\n");
		fprintf(fp, "	0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,\n");
		fprintf(fp, "	0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,\n");
		fprintf(fp, "	0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,\n");
		fprintf(fp, "	0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,\n");
		fprintf(fp, "	0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,\n");
		fprintf(fp, "	0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84,0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,\n");
		fprintf(fp, "	0x84,0x80,0x80,0x84,0x80,0x84,0x84,0x80,0x80,0x84,0x84,0x80,0x84,0x80,0x80,0x84\n");
		fprintf(fp, "};\n\n");

		fprintf(fp, "static UINT8 bPostANDFlags[0x100] = \n");
		fprintf(fp, "{\n");
		fprintf(fp, "	0x54,0x10,0x10,0x14,0x10,0x14,0x14,0x10,0x10,0x14,0x14,0x10,0x14,0x10,0x10,0x14,\n");
		fprintf(fp, "	0x10,0x14,0x14,0x10,0x14,0x10,0x10,0x14,0x14,0x10,0x10,0x14,0x10,0x14,0x14,0x10,\n");
		fprintf(fp, "	0x10,0x14,0x14,0x10,0x14,0x10,0x10,0x14,0x14,0x10,0x10,0x14,0x10,0x14,0x14,0x10,\n");
		fprintf(fp, "	0x14,0x10,0x10,0x14,0x10,0x14,0x14,0x10,0x10,0x14,0x14,0x10,0x14,0x10,0x10,0x14,\n");
		fprintf(fp, "	0x10,0x14,0x14,0x10,0x14,0x10,0x10,0x14,0x14,0x10,0x10,0x14,0x10,0x14,0x14,0x10,\n");
		fprintf(fp, "	0x14,0x10,0x10,0x14,0x10,0x14,0x14,0x10,0x10,0x14,0x14,0x10,0x14,0x10,0x10,0x14,\n");
		fprintf(fp, "	0x14,0x10,0x10,0x14,0x10,0x14,0x14,0x10,0x10,0x14,0x14,0x10,0x14,0x10,0x10,0x14,\n");
		fprintf(fp, "	0x10,0x14,0x14,0x10,0x14,0x10,0x10,0x14,0x14,0x10,0x10,0x14,0x10,0x14,0x14,0x10,\n");
		fprintf(fp, "	0x90,0x94,0x94,0x90,0x94,0x90,0x90,0x94,0x94,0x90,0x90,0x94,0x90,0x94,0x94,0x90,\n");
		fprintf(fp, "	0x94,0x90,0x90,0x94,0x90,0x94,0x94,0x90,0x90,0x94,0x94,0x90,0x94,0x90,0x90,0x94,\n");
		fprintf(fp, "	0x94,0x90,0x90,0x94,0x90,0x94,0x94,0x90,0x90,0x94,0x94,0x90,0x94,0x90,0x90,0x94,\n");
		fprintf(fp, "	0x90,0x94,0x94,0x90,0x94,0x90,0x90,0x94,0x94,0x90,0x90,0x94,0x90,0x94,0x94,0x90,\n");
		fprintf(fp, "	0x94,0x90,0x90,0x94,0x90,0x94,0x94,0x90,0x90,0x94,0x94,0x90,0x94,0x90,0x90,0x94,\n");
		fprintf(fp, "	0x90,0x94,0x94,0x90,0x94,0x90,0x90,0x94,0x94,0x90,0x90,0x94,0x90,0x94,0x94,0x90,\n");
		fprintf(fp, "	0x90,0x94,0x94,0x90,0x94,0x90,0x90,0x94,0x94,0x90,0x90,0x94,0x90,0x94,0x94,0x90,\n");
		fprintf(fp, "	0x94,0x90,0x90,0x94,0x90,0x94,0x94,0x90,0x90,0x94,0x94,0x90,0x94,0x90,0x90,0x94\n");
		fprintf(fp, "};\n\n");

		fprintf(fp, "static UINT16 wDAATable[0x800] = \n");
		fprintf(fp, "{\n");
		fprintf(fp, "	0x5400,0x1001,0x1002,0x1403,0x1004,0x1405,0x1406,0x1007,\n");
		fprintf(fp, "	0x1008,0x1409,0x1010,0x1411,0x1412,0x1013,0x1414,0x1015,\n");
		fprintf(fp, "	0x1010,0x1411,0x1412,0x1013,0x1414,0x1015,0x1016,0x1417,\n");
		fprintf(fp, "	0x1418,0x1019,0x1020,0x1421,0x1422,0x1023,0x1424,0x1025,\n");
		fprintf(fp, "	0x1020,0x1421,0x1422,0x1023,0x1424,0x1025,0x1026,0x1427,\n");
		fprintf(fp, "	0x1428,0x1029,0x1430,0x1031,0x1032,0x1433,0x1034,0x1435,\n");
		fprintf(fp, "	0x1430,0x1031,0x1032,0x1433,0x1034,0x1435,0x1436,0x1037,\n");
		fprintf(fp, "	0x1038,0x1439,0x1040,0x1441,0x1442,0x1043,0x1444,0x1045,\n");
		fprintf(fp, "	0x1040,0x1441,0x1442,0x1043,0x1444,0x1045,0x1046,0x1447,\n");
		fprintf(fp, "	0x1448,0x1049,0x1450,0x1051,0x1052,0x1453,0x1054,0x1455,\n");
		fprintf(fp, "	0x1450,0x1051,0x1052,0x1453,0x1054,0x1455,0x1456,0x1057,\n");
		fprintf(fp, "	0x1058,0x1459,0x1460,0x1061,0x1062,0x1463,0x1064,0x1465,\n");
		fprintf(fp, "	0x1460,0x1061,0x1062,0x1463,0x1064,0x1465,0x1466,0x1067,\n");
		fprintf(fp, "	0x1068,0x1469,0x1070,0x1471,0x1472,0x1073,0x1474,0x1075,\n");
		fprintf(fp, "	0x1070,0x1471,0x1472,0x1073,0x1474,0x1075,0x1076,0x1477,\n");
		fprintf(fp, "	0x1478,0x1079,0x9080,0x9481,0x9482,0x9083,0x9484,0x9085,\n");
		fprintf(fp, "	0x9080,0x9481,0x9482,0x9083,0x9484,0x9085,0x9086,0x9487,\n");
		fprintf(fp, "	0x9488,0x9089,0x9490,0x9091,0x9092,0x9493,0x9094,0x9495,\n");
		fprintf(fp, "	0x9490,0x9091,0x9092,0x9493,0x9094,0x9495,0x9496,0x9097,\n");
		fprintf(fp, "	0x9098,0x9499,0x5500,0x1101,0x1102,0x1503,0x1104,0x1505,\n");
		fprintf(fp, "	0x5500,0x1101,0x1102,0x1503,0x1104,0x1505,0x1506,0x1107,\n");
		fprintf(fp, "	0x1108,0x1509,0x1110,0x1511,0x1512,0x1113,0x1514,0x1115,\n");
		fprintf(fp, "	0x1110,0x1511,0x1512,0x1113,0x1514,0x1115,0x1116,0x1517,\n");
		fprintf(fp, "	0x1518,0x1119,0x1120,0x1521,0x1522,0x1123,0x1524,0x1125,\n");
		fprintf(fp, "	0x1120,0x1521,0x1522,0x1123,0x1524,0x1125,0x1126,0x1527,\n");
		fprintf(fp, "	0x1528,0x1129,0x1530,0x1131,0x1132,0x1533,0x1134,0x1535,\n");
		fprintf(fp, "	0x1530,0x1131,0x1132,0x1533,0x1134,0x1535,0x1536,0x1137,\n");
		fprintf(fp, "	0x1138,0x1539,0x1140,0x1541,0x1542,0x1143,0x1544,0x1145,\n");
		fprintf(fp, "	0x1140,0x1541,0x1542,0x1143,0x1544,0x1145,0x1146,0x1547,\n");
		fprintf(fp, "	0x1548,0x1149,0x1550,0x1151,0x1152,0x1553,0x1154,0x1555,\n");
		fprintf(fp, "	0x1550,0x1151,0x1152,0x1553,0x1154,0x1555,0x1556,0x1157,\n");
		fprintf(fp, "	0x1158,0x1559,0x1560,0x1161,0x1162,0x1563,0x1164,0x1565,\n");
		fprintf(fp, "	0x1560,0x1161,0x1162,0x1563,0x1164,0x1565,0x1566,0x1167,\n");
		fprintf(fp, "	0x1168,0x1569,0x1170,0x1571,0x1572,0x1173,0x1574,0x1175,\n");
		fprintf(fp, "	0x1170,0x1571,0x1572,0x1173,0x1574,0x1175,0x1176,0x1577,\n");
		fprintf(fp, "	0x1578,0x1179,0x9180,0x9581,0x9582,0x9183,0x9584,0x9185,\n");
		fprintf(fp, "	0x9180,0x9581,0x9582,0x9183,0x9584,0x9185,0x9186,0x9587,\n");
		fprintf(fp, "	0x9588,0x9189,0x9590,0x9191,0x9192,0x9593,0x9194,0x9595,\n");
		fprintf(fp, "	0x9590,0x9191,0x9192,0x9593,0x9194,0x9595,0x9596,0x9197,\n");
		fprintf(fp, "	0x9198,0x9599,0x95a0,0x91a1,0x91a2,0x95a3,0x91a4,0x95a5,\n");
		fprintf(fp, "	0x95a0,0x91a1,0x91a2,0x95a3,0x91a4,0x95a5,0x95a6,0x91a7,\n");
		fprintf(fp, "	0x91a8,0x95a9,0x91b0,0x95b1,0x95b2,0x91b3,0x95b4,0x91b5,\n");
		fprintf(fp, "	0x91b0,0x95b1,0x95b2,0x91b3,0x95b4,0x91b5,0x91b6,0x95b7,\n");
		fprintf(fp, "	0x95b8,0x91b9,0x95c0,0x91c1,0x91c2,0x95c3,0x91c4,0x95c5,\n");
		fprintf(fp, "	0x95c0,0x91c1,0x91c2,0x95c3,0x91c4,0x95c5,0x95c6,0x91c7,\n");
		fprintf(fp, "	0x91c8,0x95c9,0x91d0,0x95d1,0x95d2,0x91d3,0x95d4,0x91d5,\n");
		fprintf(fp, "	0x91d0,0x95d1,0x95d2,0x91d3,0x95d4,0x91d5,0x91d6,0x95d7,\n");
		fprintf(fp, "	0x95d8,0x91d9,0x91e0,0x95e1,0x95e2,0x91e3,0x95e4,0x91e5,\n");
		fprintf(fp, "	0x91e0,0x95e1,0x95e2,0x91e3,0x95e4,0x91e5,0x91e6,0x95e7,\n");
		fprintf(fp, "	0x95e8,0x91e9,0x95f0,0x91f1,0x91f2,0x95f3,0x91f4,0x95f5,\n");
		fprintf(fp, "	0x95f0,0x91f1,0x91f2,0x95f3,0x91f4,0x95f5,0x95f6,0x91f7,\n");
		fprintf(fp, "	0x91f8,0x95f9,0x5500,0x1101,0x1102,0x1503,0x1104,0x1505,\n");
		fprintf(fp, "	0x5500,0x1101,0x1102,0x1503,0x1104,0x1505,0x1506,0x1107,\n");
		fprintf(fp, "	0x1108,0x1509,0x1110,0x1511,0x1512,0x1113,0x1514,0x1115,\n");
		fprintf(fp, "	0x1110,0x1511,0x1512,0x1113,0x1514,0x1115,0x1116,0x1517,\n");
		fprintf(fp, "	0x1518,0x1119,0x1120,0x1521,0x1522,0x1123,0x1524,0x1125,\n");
		fprintf(fp, "	0x1120,0x1521,0x1522,0x1123,0x1524,0x1125,0x1126,0x1527,\n");
		fprintf(fp, "	0x1528,0x1129,0x1530,0x1131,0x1132,0x1533,0x1134,0x1535,\n");
		fprintf(fp, "	0x1530,0x1131,0x1132,0x1533,0x1134,0x1535,0x1536,0x1137,\n");
		fprintf(fp, "	0x1138,0x1539,0x1140,0x1541,0x1542,0x1143,0x1544,0x1145,\n");
		fprintf(fp, "	0x1140,0x1541,0x1542,0x1143,0x1544,0x1145,0x1146,0x1547,\n");
		fprintf(fp, "	0x1548,0x1149,0x1550,0x1151,0x1152,0x1553,0x1154,0x1555,\n");
		fprintf(fp, "	0x1550,0x1151,0x1152,0x1553,0x1154,0x1555,0x1556,0x1157,\n");
		fprintf(fp, "	0x1158,0x1559,0x1560,0x1161,0x1162,0x1563,0x1164,0x1565,\n");
		fprintf(fp, "	0x1406,0x1007,0x1008,0x1409,0x140a,0x100b,0x140c,0x100d,\n");
		fprintf(fp, "	0x100e,0x140f,0x1010,0x1411,0x1412,0x1013,0x1414,0x1015,\n");
		fprintf(fp, "	0x1016,0x1417,0x1418,0x1019,0x101a,0x141b,0x101c,0x141d,\n");
		fprintf(fp, "	0x141e,0x101f,0x1020,0x1421,0x1422,0x1023,0x1424,0x1025,\n");
		fprintf(fp, "	0x1026,0x1427,0x1428,0x1029,0x102a,0x142b,0x102c,0x142d,\n");
		fprintf(fp, "	0x142e,0x102f,0x1430,0x1031,0x1032,0x1433,0x1034,0x1435,\n");
		fprintf(fp, "	0x1436,0x1037,0x1038,0x1439,0x143a,0x103b,0x143c,0x103d,\n");
		fprintf(fp, "	0x103e,0x143f,0x1040,0x1441,0x1442,0x1043,0x1444,0x1045,\n");
		fprintf(fp, "	0x1046,0x1447,0x1448,0x1049,0x104a,0x144b,0x104c,0x144d,\n");
		fprintf(fp, "	0x144e,0x104f,0x1450,0x1051,0x1052,0x1453,0x1054,0x1455,\n");
		fprintf(fp, "	0x1456,0x1057,0x1058,0x1459,0x145a,0x105b,0x145c,0x105d,\n");
		fprintf(fp, "	0x105e,0x145f,0x1460,0x1061,0x1062,0x1463,0x1064,0x1465,\n");
		fprintf(fp, "	0x1466,0x1067,0x1068,0x1469,0x146a,0x106b,0x146c,0x106d,\n");
		fprintf(fp, "	0x106e,0x146f,0x1070,0x1471,0x1472,0x1073,0x1474,0x1075,\n");
		fprintf(fp, "	0x1076,0x1477,0x1478,0x1079,0x107a,0x147b,0x107c,0x147d,\n");
		fprintf(fp, "	0x147e,0x107f,0x9080,0x9481,0x9482,0x9083,0x9484,0x9085,\n");
		fprintf(fp, "	0x9086,0x9487,0x9488,0x9089,0x908a,0x948b,0x908c,0x948d,\n");
		fprintf(fp, "	0x948e,0x908f,0x9490,0x9091,0x9092,0x9493,0x9094,0x9495,\n");
		fprintf(fp, "	0x9496,0x9097,0x9098,0x9499,0x949a,0x909b,0x949c,0x909d,\n");
		fprintf(fp, "	0x909e,0x949f,0x5500,0x1101,0x1102,0x1503,0x1104,0x1505,\n");
		fprintf(fp, "	0x1506,0x1107,0x1108,0x1509,0x150a,0x110b,0x150c,0x110d,\n");
		fprintf(fp, "	0x110e,0x150f,0x1110,0x1511,0x1512,0x1113,0x1514,0x1115,\n");
		fprintf(fp, "	0x1116,0x1517,0x1518,0x1119,0x111a,0x151b,0x111c,0x151d,\n");
		fprintf(fp, "	0x151e,0x111f,0x1120,0x1521,0x1522,0x1123,0x1524,0x1125,\n");
		fprintf(fp, "	0x1126,0x1527,0x1528,0x1129,0x112a,0x152b,0x112c,0x152d,\n");
		fprintf(fp, "	0x152e,0x112f,0x1530,0x1131,0x1132,0x1533,0x1134,0x1535,\n");
		fprintf(fp, "	0x1536,0x1137,0x1138,0x1539,0x153a,0x113b,0x153c,0x113d,\n");
		fprintf(fp, "	0x113e,0x153f,0x1140,0x1541,0x1542,0x1143,0x1544,0x1145,\n");
		fprintf(fp, "	0x1146,0x1547,0x1548,0x1149,0x114a,0x154b,0x114c,0x154d,\n");
		fprintf(fp, "	0x154e,0x114f,0x1550,0x1151,0x1152,0x1553,0x1154,0x1555,\n");
		fprintf(fp, "	0x1556,0x1157,0x1158,0x1559,0x155a,0x115b,0x155c,0x115d,\n");
		fprintf(fp, "	0x115e,0x155f,0x1560,0x1161,0x1162,0x1563,0x1164,0x1565,\n");
		fprintf(fp, "	0x1566,0x1167,0x1168,0x1569,0x156a,0x116b,0x156c,0x116d,\n");
		fprintf(fp, "	0x116e,0x156f,0x1170,0x1571,0x1572,0x1173,0x1574,0x1175,\n");
		fprintf(fp, "	0x1176,0x1577,0x1578,0x1179,0x117a,0x157b,0x117c,0x157d,\n");
		fprintf(fp, "	0x157e,0x117f,0x9180,0x9581,0x9582,0x9183,0x9584,0x9185,\n");
		fprintf(fp, "	0x9186,0x9587,0x9588,0x9189,0x918a,0x958b,0x918c,0x958d,\n");
		fprintf(fp, "	0x958e,0x918f,0x9590,0x9191,0x9192,0x9593,0x9194,0x9595,\n");
		fprintf(fp, "	0x9596,0x9197,0x9198,0x9599,0x959a,0x919b,0x959c,0x919d,\n");
		fprintf(fp, "	0x919e,0x959f,0x95a0,0x91a1,0x91a2,0x95a3,0x91a4,0x95a5,\n");
		fprintf(fp, "	0x95a6,0x91a7,0x91a8,0x95a9,0x95aa,0x91ab,0x95ac,0x91ad,\n");
		fprintf(fp, "	0x91ae,0x95af,0x91b0,0x95b1,0x95b2,0x91b3,0x95b4,0x91b5,\n");
		fprintf(fp, "	0x91b6,0x95b7,0x95b8,0x91b9,0x91ba,0x95bb,0x91bc,0x95bd,\n");
		fprintf(fp, "	0x95be,0x91bf,0x95c0,0x91c1,0x91c2,0x95c3,0x91c4,0x95c5,\n");
		fprintf(fp, "	0x95c6,0x91c7,0x91c8,0x95c9,0x95ca,0x91cb,0x95cc,0x91cd,\n");
		fprintf(fp, "	0x91ce,0x95cf,0x91d0,0x95d1,0x95d2,0x91d3,0x95d4,0x91d5,\n");
		fprintf(fp, "	0x91d6,0x95d7,0x95d8,0x91d9,0x91da,0x95db,0x91dc,0x95dd,\n");
		fprintf(fp, "	0x95de,0x91df,0x91e0,0x95e1,0x95e2,0x91e3,0x95e4,0x91e5,\n");
		fprintf(fp, "	0x91e6,0x95e7,0x95e8,0x91e9,0x91ea,0x95eb,0x91ec,0x95ed,\n");
		fprintf(fp, "	0x95ee,0x91ef,0x95f0,0x91f1,0x91f2,0x95f3,0x91f4,0x95f5,\n");
		fprintf(fp, "	0x95f6,0x91f7,0x91f8,0x95f9,0x95fa,0x91fb,0x95fc,0x91fd,\n");
		fprintf(fp, "	0x91fe,0x95ff,0x5500,0x1101,0x1102,0x1503,0x1104,0x1505,\n");
		fprintf(fp, "	0x1506,0x1107,0x1108,0x1509,0x150a,0x110b,0x150c,0x110d,\n");
		fprintf(fp, "	0x110e,0x150f,0x1110,0x1511,0x1512,0x1113,0x1514,0x1115,\n");
		fprintf(fp, "	0x1116,0x1517,0x1518,0x1119,0x111a,0x151b,0x111c,0x151d,\n");
		fprintf(fp, "	0x151e,0x111f,0x1120,0x1521,0x1522,0x1123,0x1524,0x1125,\n");
		fprintf(fp, "	0x1126,0x1527,0x1528,0x1129,0x112a,0x152b,0x112c,0x152d,\n");
		fprintf(fp, "	0x152e,0x112f,0x1530,0x1131,0x1132,0x1533,0x1134,0x1535,\n");
		fprintf(fp, "	0x1536,0x1137,0x1138,0x1539,0x153a,0x113b,0x153c,0x113d,\n");
		fprintf(fp, "	0x113e,0x153f,0x1140,0x1541,0x1542,0x1143,0x1544,0x1145,\n");
		fprintf(fp, "	0x1146,0x1547,0x1548,0x1149,0x114a,0x154b,0x114c,0x154d,\n");
		fprintf(fp, "	0x154e,0x114f,0x1550,0x1151,0x1152,0x1553,0x1154,0x1555,\n");
		fprintf(fp, "	0x1556,0x1157,0x1158,0x1559,0x155a,0x115b,0x155c,0x115d,\n");
		fprintf(fp, "	0x115e,0x155f,0x1560,0x1161,0x1162,0x1563,0x1164,0x1565,\n");
		fprintf(fp, "	0x5600,0x1201,0x1202,0x1603,0x1204,0x1605,0x1606,0x1207,\n");
		fprintf(fp, "	0x1208,0x1609,0x1204,0x1605,0x1606,0x1207,0x1208,0x1609,\n");
		fprintf(fp, "	0x1210,0x1611,0x1612,0x1213,0x1614,0x1215,0x1216,0x1617,\n");
		fprintf(fp, "	0x1618,0x1219,0x1614,0x1215,0x1216,0x1617,0x1618,0x1219,\n");
		fprintf(fp, "	0x1220,0x1621,0x1622,0x1223,0x1624,0x1225,0x1226,0x1627,\n");
		fprintf(fp, "	0x1628,0x1229,0x1624,0x1225,0x1226,0x1627,0x1628,0x1229,\n");
		fprintf(fp, "	0x1630,0x1231,0x1232,0x1633,0x1234,0x1635,0x1636,0x1237,\n");
		fprintf(fp, "	0x1238,0x1639,0x1234,0x1635,0x1636,0x1237,0x1238,0x1639,\n");
		fprintf(fp, "	0x1240,0x1641,0x1642,0x1243,0x1644,0x1245,0x1246,0x1647,\n");
		fprintf(fp, "	0x1648,0x1249,0x1644,0x1245,0x1246,0x1647,0x1648,0x1249,\n");
		fprintf(fp, "	0x1650,0x1251,0x1252,0x1653,0x1254,0x1655,0x1656,0x1257,\n");
		fprintf(fp, "	0x1258,0x1659,0x1254,0x1655,0x1656,0x1257,0x1258,0x1659,\n");
		fprintf(fp, "	0x1660,0x1261,0x1262,0x1663,0x1264,0x1665,0x1666,0x1267,\n");
		fprintf(fp, "	0x1268,0x1669,0x1264,0x1665,0x1666,0x1267,0x1268,0x1669,\n");
		fprintf(fp, "	0x1270,0x1671,0x1672,0x1273,0x1674,0x1275,0x1276,0x1677,\n");
		fprintf(fp, "	0x1678,0x1279,0x1674,0x1275,0x1276,0x1677,0x1678,0x1279,\n");
		fprintf(fp, "	0x9280,0x9681,0x9682,0x9283,0x9684,0x9285,0x9286,0x9687,\n");
		fprintf(fp, "	0x9688,0x9289,0x9684,0x9285,0x9286,0x9687,0x9688,0x9289,\n");
		fprintf(fp, "	0x9690,0x9291,0x9292,0x9693,0x9294,0x9695,0x9696,0x9297,\n");
		fprintf(fp, "	0x9298,0x9699,0x1334,0x1735,0x1736,0x1337,0x1338,0x1739,\n");
		fprintf(fp, "	0x1340,0x1741,0x1742,0x1343,0x1744,0x1345,0x1346,0x1747,\n");
		fprintf(fp, "	0x1748,0x1349,0x1744,0x1345,0x1346,0x1747,0x1748,0x1349,\n");
		fprintf(fp, "	0x1750,0x1351,0x1352,0x1753,0x1354,0x1755,0x1756,0x1357,\n");
		fprintf(fp, "	0x1358,0x1759,0x1354,0x1755,0x1756,0x1357,0x1358,0x1759,\n");
		fprintf(fp, "	0x1760,0x1361,0x1362,0x1763,0x1364,0x1765,0x1766,0x1367,\n");
		fprintf(fp, "	0x1368,0x1769,0x1364,0x1765,0x1766,0x1367,0x1368,0x1769,\n");
		fprintf(fp, "	0x1370,0x1771,0x1772,0x1373,0x1774,0x1375,0x1376,0x1777,\n");
		fprintf(fp, "	0x1778,0x1379,0x1774,0x1375,0x1376,0x1777,0x1778,0x1379,\n");
		fprintf(fp, "	0x9380,0x9781,0x9782,0x9383,0x9784,0x9385,0x9386,0x9787,\n");
		fprintf(fp, "	0x9788,0x9389,0x9784,0x9385,0x9386,0x9787,0x9788,0x9389,\n");
		fprintf(fp, "	0x9790,0x9391,0x9392,0x9793,0x9394,0x9795,0x9796,0x9397,\n");
		fprintf(fp, "	0x9398,0x9799,0x9394,0x9795,0x9796,0x9397,0x9398,0x9799,\n");
		fprintf(fp, "	0x97a0,0x93a1,0x93a2,0x97a3,0x93a4,0x97a5,0x97a6,0x93a7,\n");
		fprintf(fp, "	0x93a8,0x97a9,0x93a4,0x97a5,0x97a6,0x93a7,0x93a8,0x97a9,\n");
		fprintf(fp, "	0x93b0,0x97b1,0x97b2,0x93b3,0x97b4,0x93b5,0x93b6,0x97b7,\n");
		fprintf(fp, "	0x97b8,0x93b9,0x97b4,0x93b5,0x93b6,0x97b7,0x97b8,0x93b9,\n");
		fprintf(fp, "	0x97c0,0x93c1,0x93c2,0x97c3,0x93c4,0x97c5,0x97c6,0x93c7,\n");
		fprintf(fp, "	0x93c8,0x97c9,0x93c4,0x97c5,0x97c6,0x93c7,0x93c8,0x97c9,\n");
		fprintf(fp, "	0x93d0,0x97d1,0x97d2,0x93d3,0x97d4,0x93d5,0x93d6,0x97d7,\n");
		fprintf(fp, "	0x97d8,0x93d9,0x97d4,0x93d5,0x93d6,0x97d7,0x97d8,0x93d9,\n");
		fprintf(fp, "	0x93e0,0x97e1,0x97e2,0x93e3,0x97e4,0x93e5,0x93e6,0x97e7,\n");
		fprintf(fp, "	0x97e8,0x93e9,0x97e4,0x93e5,0x93e6,0x97e7,0x97e8,0x93e9,\n");
		fprintf(fp, "	0x97f0,0x93f1,0x93f2,0x97f3,0x93f4,0x97f5,0x97f6,0x93f7,\n");
		fprintf(fp, "	0x93f8,0x97f9,0x93f4,0x97f5,0x97f6,0x93f7,0x93f8,0x97f9,\n");
		fprintf(fp, "	0x5700,0x1301,0x1302,0x1703,0x1304,0x1705,0x1706,0x1307,\n");
		fprintf(fp, "	0x1308,0x1709,0x1304,0x1705,0x1706,0x1307,0x1308,0x1709,\n");
		fprintf(fp, "	0x1310,0x1711,0x1712,0x1313,0x1714,0x1315,0x1316,0x1717,\n");
		fprintf(fp, "	0x1718,0x1319,0x1714,0x1315,0x1316,0x1717,0x1718,0x1319,\n");
		fprintf(fp, "	0x1320,0x1721,0x1722,0x1323,0x1724,0x1325,0x1326,0x1727,\n");
		fprintf(fp, "	0x1728,0x1329,0x1724,0x1325,0x1326,0x1727,0x1728,0x1329,\n");
		fprintf(fp, "	0x1730,0x1331,0x1332,0x1733,0x1334,0x1735,0x1736,0x1337,\n");
		fprintf(fp, "	0x1338,0x1739,0x1334,0x1735,0x1736,0x1337,0x1338,0x1739,\n");
		fprintf(fp, "	0x1340,0x1741,0x1742,0x1343,0x1744,0x1345,0x1346,0x1747,\n");
		fprintf(fp, "	0x1748,0x1349,0x1744,0x1345,0x1346,0x1747,0x1748,0x1349,\n");
		fprintf(fp, "	0x1750,0x1351,0x1352,0x1753,0x1354,0x1755,0x1756,0x1357,\n");
		fprintf(fp, "	0x1358,0x1759,0x1354,0x1755,0x1756,0x1357,0x1358,0x1759,\n");
		fprintf(fp, "	0x1760,0x1361,0x1362,0x1763,0x1364,0x1765,0x1766,0x1367,\n");
		fprintf(fp, "	0x1368,0x1769,0x1364,0x1765,0x1766,0x1367,0x1368,0x1769,\n");
		fprintf(fp, "	0x1370,0x1771,0x1772,0x1373,0x1774,0x1375,0x1376,0x1777,\n");
		fprintf(fp, "	0x1778,0x1379,0x1774,0x1375,0x1376,0x1777,0x1778,0x1379,\n");
		fprintf(fp, "	0x9380,0x9781,0x9782,0x9383,0x9784,0x9385,0x9386,0x9787,\n");
		fprintf(fp, "	0x9788,0x9389,0x9784,0x9385,0x9386,0x9787,0x9788,0x9389,\n");
		fprintf(fp, "	0x9790,0x9391,0x9392,0x9793,0x9394,0x9795,0x9796,0x9397,\n");
		fprintf(fp, "	0x9398,0x9799,0x9394,0x9795,0x9796,0x9397,0x9398,0x9799,\n");
		fprintf(fp, "	0x97fa,0x93fb,0x97fc,0x93fd,0x93fe,0x97ff,0x5600,0x1201,\n");
		fprintf(fp, "	0x1202,0x1603,0x1204,0x1605,0x1606,0x1207,0x1208,0x1609,\n");
		fprintf(fp, "	0x160a,0x120b,0x160c,0x120d,0x120e,0x160f,0x1210,0x1611,\n");
		fprintf(fp, "	0x1612,0x1213,0x1614,0x1215,0x1216,0x1617,0x1618,0x1219,\n");
		fprintf(fp, "	0x121a,0x161b,0x121c,0x161d,0x161e,0x121f,0x1220,0x1621,\n");
		fprintf(fp, "	0x1622,0x1223,0x1624,0x1225,0x1226,0x1627,0x1628,0x1229,\n");
		fprintf(fp, "	0x122a,0x162b,0x122c,0x162d,0x162e,0x122f,0x1630,0x1231,\n");
		fprintf(fp, "	0x1232,0x1633,0x1234,0x1635,0x1636,0x1237,0x1238,0x1639,\n");
		fprintf(fp, "	0x163a,0x123b,0x163c,0x123d,0x123e,0x163f,0x1240,0x1641,\n");
		fprintf(fp, "	0x1642,0x1243,0x1644,0x1245,0x1246,0x1647,0x1648,0x1249,\n");
		fprintf(fp, "	0x124a,0x164b,0x124c,0x164d,0x164e,0x124f,0x1650,0x1251,\n");
		fprintf(fp, "	0x1252,0x1653,0x1254,0x1655,0x1656,0x1257,0x1258,0x1659,\n");
		fprintf(fp, "	0x165a,0x125b,0x165c,0x125d,0x125e,0x165f,0x1660,0x1261,\n");
		fprintf(fp, "	0x1262,0x1663,0x1264,0x1665,0x1666,0x1267,0x1268,0x1669,\n");
		fprintf(fp, "	0x166a,0x126b,0x166c,0x126d,0x126e,0x166f,0x1270,0x1671,\n");
		fprintf(fp, "	0x1672,0x1273,0x1674,0x1275,0x1276,0x1677,0x1678,0x1279,\n");
		fprintf(fp, "	0x127a,0x167b,0x127c,0x167d,0x167e,0x127f,0x9280,0x9681,\n");
		fprintf(fp, "	0x9682,0x9283,0x9684,0x9285,0x9286,0x9687,0x9688,0x9289,\n");
		fprintf(fp, "	0x928a,0x968b,0x928c,0x968d,0x968e,0x928f,0x9690,0x9291,\n");
		fprintf(fp, "	0x9292,0x9693,0x1334,0x1735,0x1736,0x1337,0x1338,0x1739,\n");
		fprintf(fp, "	0x173a,0x133b,0x173c,0x133d,0x133e,0x173f,0x1340,0x1741,\n");
		fprintf(fp, "	0x1742,0x1343,0x1744,0x1345,0x1346,0x1747,0x1748,0x1349,\n");
		fprintf(fp, "	0x134a,0x174b,0x134c,0x174d,0x174e,0x134f,0x1750,0x1351,\n");
		fprintf(fp, "	0x1352,0x1753,0x1354,0x1755,0x1756,0x1357,0x1358,0x1759,\n");
		fprintf(fp, "	0x175a,0x135b,0x175c,0x135d,0x135e,0x175f,0x1760,0x1361,\n");
		fprintf(fp, "	0x1362,0x1763,0x1364,0x1765,0x1766,0x1367,0x1368,0x1769,\n");
		fprintf(fp, "	0x176a,0x136b,0x176c,0x136d,0x136e,0x176f,0x1370,0x1771,\n");
		fprintf(fp, "	0x1772,0x1373,0x1774,0x1375,0x1376,0x1777,0x1778,0x1379,\n");
		fprintf(fp, "	0x137a,0x177b,0x137c,0x177d,0x177e,0x137f,0x9380,0x9781,\n");
		fprintf(fp, "	0x9782,0x9383,0x9784,0x9385,0x9386,0x9787,0x9788,0x9389,\n");
		fprintf(fp, "	0x938a,0x978b,0x938c,0x978d,0x978e,0x938f,0x9790,0x9391,\n");
		fprintf(fp, "	0x9392,0x9793,0x9394,0x9795,0x9796,0x9397,0x9398,0x9799,\n");
		fprintf(fp, "	0x979a,0x939b,0x979c,0x939d,0x939e,0x979f,0x97a0,0x93a1,\n");
		fprintf(fp, "	0x93a2,0x97a3,0x93a4,0x97a5,0x97a6,0x93a7,0x93a8,0x97a9,\n");
		fprintf(fp, "	0x97aa,0x93ab,0x97ac,0x93ad,0x93ae,0x97af,0x93b0,0x97b1,\n");
		fprintf(fp, "	0x97b2,0x93b3,0x97b4,0x93b5,0x93b6,0x97b7,0x97b8,0x93b9,\n");
		fprintf(fp, "	0x93ba,0x97bb,0x93bc,0x97bd,0x97be,0x93bf,0x97c0,0x93c1,\n");
		fprintf(fp, "	0x93c2,0x97c3,0x93c4,0x97c5,0x97c6,0x93c7,0x93c8,0x97c9,\n");
		fprintf(fp, "	0x97ca,0x93cb,0x97cc,0x93cd,0x93ce,0x97cf,0x93d0,0x97d1,\n");
		fprintf(fp, "	0x97d2,0x93d3,0x97d4,0x93d5,0x93d6,0x97d7,0x97d8,0x93d9,\n");
		fprintf(fp, "	0x93da,0x97db,0x93dc,0x97dd,0x97de,0x93df,0x93e0,0x97e1,\n");
		fprintf(fp, "	0x97e2,0x93e3,0x97e4,0x93e5,0x93e6,0x97e7,0x97e8,0x93e9,\n");
		fprintf(fp, "	0x93ea,0x97eb,0x93ec,0x97ed,0x97ee,0x93ef,0x97f0,0x93f1,\n");
		fprintf(fp, "	0x93f2,0x97f3,0x93f4,0x97f5,0x97f6,0x93f7,0x93f8,0x97f9,\n");
		fprintf(fp, "	0x97fa,0x93fb,0x97fc,0x93fd,0x93fe,0x97ff,0x5700,0x1301,\n");
		fprintf(fp, "	0x1302,0x1703,0x1304,0x1705,0x1706,0x1307,0x1308,0x1709,\n");
		fprintf(fp, "	0x170a,0x130b,0x170c,0x130d,0x130e,0x170f,0x1310,0x1711,\n");
		fprintf(fp, "	0x1712,0x1313,0x1714,0x1315,0x1316,0x1717,0x1718,0x1319,\n");
		fprintf(fp, "	0x131a,0x171b,0x131c,0x171d,0x171e,0x131f,0x1320,0x1721,\n");
		fprintf(fp, "	0x1722,0x1323,0x1724,0x1325,0x1326,0x1727,0x1728,0x1329,\n");
		fprintf(fp, "	0x132a,0x172b,0x132c,0x172d,0x172e,0x132f,0x1730,0x1331,\n");
		fprintf(fp, "	0x1332,0x1733,0x1334,0x1735,0x1736,0x1337,0x1338,0x1739,\n");
		fprintf(fp, "	0x173a,0x133b,0x173c,0x133d,0x133e,0x173f,0x1340,0x1741,\n");
		fprintf(fp, "	0x1742,0x1343,0x1744,0x1345,0x1346,0x1747,0x1748,0x1349,\n");
		fprintf(fp, "	0x134a,0x174b,0x134c,0x174d,0x174e,0x134f,0x1750,0x1351,\n");
		fprintf(fp, "	0x1352,0x1753,0x1354,0x1755,0x1756,0x1357,0x1358,0x1759,\n");
		fprintf(fp, "	0x175a,0x135b,0x175c,0x135d,0x135e,0x175f,0x1760,0x1361,\n");
		fprintf(fp, "	0x1362,0x1763,0x1364,0x1765,0x1766,0x1367,0x1368,0x1769,\n");
		fprintf(fp, "	0x176a,0x136b,0x176c,0x136d,0x136e,0x176f,0x1370,0x1771,\n");
		fprintf(fp, "	0x1772,0x1373,0x1774,0x1375,0x1376,0x1777,0x1778,0x1379,\n");
		fprintf(fp, "	0x137a,0x177b,0x137c,0x177d,0x177e,0x137f,0x9380,0x9781,\n");
		fprintf(fp, "	0x9782,0x9383,0x9784,0x9385,0x9386,0x9787,0x9788,0x9389,\n");
		fprintf(fp, "	0x938a,0x978b,0x938c,0x978d,0x978e,0x938f,0x9790,0x9391,\n");
		fprintf(fp, "	0x9392,0x9793,0x9394,0x9795,0x9796,0x9397,0x9398,0x9799 \n");
		fprintf(fp, "};\n\n");

		fprintf(fp, "void DDFDCBHandler(UINT32 dwWhich);\n\n");

		fprintf(fp, "\n");
	}
	else
	{
		abort();
	}
}
	
void CodeSegmentBegin(void)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp, "		section	.text use32 flat class=code\n");
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "static void InvalidInstruction(UINT32 dwCount)\n");
		fprintf(fp, "{\n");

		fprintf(fp, "	pbPC -= dwCount; /* Invalid instruction - back up */\n");
		fprintf(fp, "	dwReturnCode = (UINTPTR) pbPC - (UINTPTR) cpu.z80Base;\n");
		fprintf(fp, "	dwOriginalCycles -= sdwCyclesRemaining;\n");
		fprintf(fp, "	sdwCyclesRemaining = 0;\n");

		fprintf(fp, "}\n\n");
	}
	else
	{
		abort();
	}
}

void CodeSegmentEnd(void)
{
}

void ProgramEnd(void)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp, "\n");
		fprintf(fp, "%%ifdef NASM_STACK_NOEXEC\n");
		fprintf(fp, "section .note.GNU-stack noalloc noexec nowrite progbits\n");
		fprintf(fp, "%%endif\n");
	}
	else
	if (MZ80_C == bWhat)
	{
	}
	else
	{
		abort();
	}
}

void EmitRegularInstructions(void)
{
	UINT32 dwLoop = 0;
	UINT32 dwLoop2 = 0;

	bCurrentMode = TIMING_REGULAR;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		while (dwLoop < 0x100)
		{
			dwLoop2 = 0;
			sprintf(procname, "RegInst%.2x", dwLoop);

			while (StandardOps[dwLoop2].bOpCode != dwLoop && StandardOps[dwLoop2].bOpCode != 0xffffffff)
				dwLoop2++;

			assert(dwLoop2 < 0x100);
			if (StandardOps[dwLoop2].Emitter
				&& StandardOps[dwLoop2].bOpCode != 0xffffffff)
				StandardOps[dwLoop2].Emitter((UINT32) dwLoop);

			dwLoop++;
		}
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "/* Main execution entry point */\n\n");

		fprintf(fp, "UINT32 %sexec(UINT32 dwCycles)\n", cpubasename);
		fprintf(fp, "{\n");
		fprintf(fp, "	UINT8 bOpcode;\n\n");

		fprintf(fp, "	dwReturnCode = 0x80000000; /* Assume it'll work */\n");

		fprintf(fp, "	sdwCyclesRemaining = dwCycles;\n");
		fprintf(fp, "	dwOriginalCycles = dwCycles;\n");

		fprintf(fp, "		if (cpu.z80halted)\n");
		fprintf(fp, "		{\n");

		fprintf(fp, "		dwElapsedTicks += dwCycles;\n");
		fprintf(fp, "		return(0x80000000);\n");

		fprintf(fp, "		}\n\n");
		

		fprintf(fp, "	pbPC = cpu.z80Base + cpu.z80pc;\n\n");

		fprintf(fp, "	while (sdwCyclesRemaining > 0)\n");

		fprintf(fp, "	{\n");
		fprintf(fp, "		bOpcode = *pbPC++;\n");
		fprintf(fp, "		switch (bOpcode)\n");
		fprintf(fp, "		{\n");

		while (dwLoop < 0x100)
		{
			dwLoop2 = 0;

			fprintf(fp, "			case 0x%.2x:\n", dwLoop);
			fprintf(fp, "			{\n");

			while (StandardOps[dwLoop2].bOpCode != dwLoop && StandardOps[dwLoop2].bOpCode != 0xffffffff)
				dwLoop2++;

			if (bTimingRegular[dwLoop])
			{
				fprintf(fp, "				sdwCyclesRemaining -= %" PRIu32 ";\n", bTimingRegular[dwLoop]);
			}

			if (StandardOps[dwLoop2].Emitter)
			{
				StandardOps[dwLoop2].Emitter(dwLoop);
			}

			fprintf(fp, "				break;\n");
			fprintf(fp, "			}\n");
			++dwLoop;
		}

		fprintf(fp, "		}\n");
		fprintf(fp, "	}\n\n");

		fprintf(fp, "	dwElapsedTicks += (dwOriginalCycles - sdwCyclesRemaining);\n\n");

		fprintf(fp, "	cpu.z80pc = (UINTPTR) pbPC - (UINTPTR) cpu.z80Base;\n");

		fprintf(fp, "	return(dwReturnCode); /* Indicate success */\n");
		fprintf(fp, "}\n\n");
	}
	else
	{
		abort();
	}
}

void EmitCBInstructions(void)
{
	UINT32 dwLoop = 0;
	UINT32 dwLoop2 = 0;

	bCurrentMode = TIMING_CB;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		while (dwLoop < 0x100)
		{
			sprintf(procname, "CBInst%.2x", dwLoop);
			dwLoop2 = 0;

			while (CBOps[dwLoop2].bOpCode != dwLoop && CBOps[dwLoop2].bOpCode != 0xffffffff)
				dwLoop2++;

			assert(dwLoop2 < 0x100);
			if (CBOps[dwLoop2].Emitter && CBOps[dwLoop2].bOpCode != 0xffffffff)
				CBOps[dwLoop2].Emitter((UINT32) dwLoop);

			dwLoop++;
		}
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "void CBHandler(void)\n");
		fprintf(fp, "{\n");
		fprintf(fp, "	switch (*pbPC++)\n");
		fprintf(fp, "	{\n");

		while (dwLoop < 0x100)
		{
			dwLoop2 = 0;

			fprintf(fp, " 		case 0x%.2x:\n", dwLoop);
			fprintf(fp, " 		{\n");

			while (CBOps[dwLoop2].bOpCode != dwLoop && CBOps[dwLoop2].bOpCode != 0xffffffff)
				dwLoop2++;

			if (bTimingCB[dwLoop])
			{
				fprintf(fp, "			sdwCyclesRemaining -= %" PRIu32 ";\n", bTimingCB[dwLoop]);
			}

			if (CBOps[dwLoop2].Emitter)
			{
				CBOps[dwLoop2].Emitter(dwLoop);
			}
			else
			{
				InvalidInstructionC(2);
			}

			fprintf(fp, "			break;\n");
			fprintf(fp, "		}\n");
			++dwLoop;
		}

		fprintf(fp, "	}\n");
		fprintf(fp, "}\n");
	}
	else
	{
		abort();
	}
}

void EmitEDInstructions(void)
{
	UINT32 dwLoop = 0;
	UINT32 dwLoop2 = 0;

	bCurrentMode = TIMING_ED;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		while (dwLoop < 0x100)
		{
			sprintf(procname, "EDInst%.2x", dwLoop);
			dwLoop2 = 0;

			while (EDOps[dwLoop2].bOpCode != dwLoop && EDOps[dwLoop2].bOpCode != 0xffffffff)
				dwLoop2++;

			assert(dwLoop2 < 0x100);
			if (EDOps[dwLoop2].Emitter && EDOps[dwLoop2].bOpCode != 0xffffffff)
				EDOps[dwLoop2].Emitter((UINT32) dwLoop);

			dwLoop++;
		}
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "void EDHandler(void)\n");
		fprintf(fp, "{\n");
		fprintf(fp, "	switch (*pbPC++)\n");
		fprintf(fp, "	{\n");

		while (dwLoop < 0x100)
		{
			dwLoop2 = 0;

			fprintf(fp, " 		case 0x%.2x:\n", dwLoop);
			fprintf(fp, " 		{\n");

			while (EDOps[dwLoop2].bOpCode != dwLoop && EDOps[dwLoop2].bOpCode != 0xffffffff)
				dwLoop2++;

			if (bTimingED[dwLoop])
			{
				fprintf(fp, "			sdwCyclesRemaining -= %" PRIu32 ";\n", bTimingED[dwLoop]);
			}

			if (EDOps[dwLoop2].Emitter)
			{
				EDOps[dwLoop2].Emitter(dwLoop);
			}
			else
			{
				InvalidInstructionC(2);
			}

			fprintf(fp, "			break;\n");
			fprintf(fp, "		}\n");
			++dwLoop;
		}

		fprintf(fp, "	}\n");
		fprintf(fp, "}\n");
	}
	else
	{
		abort();
	}

	fprintf(fp, "\n");
}

void EmitDDInstructions(void)
{
	UINT32 dwLoop = 0;
	UINT32 dwLoop2 = 0;

	bCurrentMode = TIMING_DDFD;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		while (dwLoop < 0x100)
		{
			sprintf(procname, "DDInst%.2x", dwLoop);
			dwLoop2 = 0;
	
			while (DDFDOps[dwLoop2].bOpCode != dwLoop && DDFDOps[dwLoop2].bOpCode != 0xffffffff)
				dwLoop2++;
	
			assert(dwLoop2 < 0x100);
			if (DDFDOps[dwLoop2].Emitter && DDFDOps[dwLoop2].bOpCode != 0xffffffff)
				DDFDOps[dwLoop2].Emitter((UINT32) dwLoop);
	
			dwLoop++;
		}

		bCurrentMode = TIMING_XXCB;

		dwLoop = 0;

		while (dwLoop < 0x100)
		{
			sprintf(procname, "DDFDCBInst%.2x", dwLoop);
			dwLoop2 = 0;

			while (DDFDCBOps[dwLoop2].bOpCode != dwLoop && DDFDCBOps[dwLoop2].bOpCode != 0xffffffff)
				dwLoop2++;

			assert(dwLoop2 < 0x100);
			if (DDFDCBOps[dwLoop2].Emitter && DDFDCBOps[dwLoop2].bOpCode != 0xffffffff)
				DDFDCBOps[dwLoop2].Emitter((UINT32) dwLoop);

			dwLoop++;
		}
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "void DDHandler(void)\n");
		fprintf(fp, "{\n");
		fprintf(fp, "	switch (*pbPC++)\n");
		fprintf(fp, "	{\n");

		while (dwLoop < 0x100)
		{
			dwLoop2 = 0;

			fprintf(fp, " 		case 0x%.2x:\n", dwLoop);
			fprintf(fp, " 		{\n");

			while (DDFDOps[dwLoop2].bOpCode != dwLoop && DDFDOps[dwLoop2].bOpCode != 0xffffffff)
				dwLoop2++;

			if (bTimingDDFD[dwLoop])
			{
				fprintf(fp, "			sdwCyclesRemaining -= %" PRIu32 ";\n", bTimingDDFD[dwLoop]);
			}

			if (DDFDOps[dwLoop2].Emitter)
			{
				DDFDOps[dwLoop2].Emitter(dwLoop);
			}
			else
			{
				InvalidInstructionC(2);
			}

			fprintf(fp, "			break;\n");
			fprintf(fp, "		}\n");
			++dwLoop;
		}

		fprintf(fp, "	}\n");
		fprintf(fp, "}\n");

		// DDFD Handler

		bCurrentMode = TIMING_XXCB;

		dwLoop = 0;

		fprintf(fp, "void DDFDCBHandler(UINT32 dwWhich)\n");
		fprintf(fp, "{\n");
		fprintf(fp, "	if (dwWhich)\n");
		fprintf(fp, "	{\n");
		fprintf(fp, "		dwAddr = (UINT32) ((INT32) cpu.z80IY + ((INT32) *pbPC++)) & 0xffff;\n");
		fprintf(fp, "	}\n");
		fprintf(fp, "	else\n");
		fprintf(fp, "	{\n");
		fprintf(fp, "		dwAddr = (UINT32) ((INT32) cpu.z80IX + ((INT32) *pbPC++)) & 0xffff;\n");
		fprintf(fp, "	}\n\n");

		ReadValueFromMemory("dwAddr", "bTemp");

		fprintf(fp, "	switch (*pbPC++)\n");
		fprintf(fp, "	{\n");

		while (dwLoop < 0x100)
		{
			dwLoop2 = 0;

			fprintf(fp, " 		case 0x%.2x:\n", dwLoop);
			fprintf(fp, " 		{\n");

			while (DDFDCBOps[dwLoop2].bOpCode != dwLoop && DDFDCBOps[dwLoop2].bOpCode != 0xffffffff)
				dwLoop2++;

			if (bTimingXXCB[dwLoop])
			{
				fprintf(fp, "			sdwCyclesRemaining -= %" PRIu32 ";\n", bTimingXXCB[dwLoop]);
			}

			if (DDFDCBOps[dwLoop2].Emitter)
			{
				DDFDCBOps[dwLoop2].Emitter(dwLoop);
			}
			else
			{
				InvalidInstructionC(4);
			}

			fprintf(fp, "			break;\n");
			fprintf(fp, "		}\n");
			++dwLoop;
		}

		fprintf(fp, "	}\n");
		fprintf(fp, "}\n");
	}
	else
	{
		abort();
	}
}

void EmitFDInstructions(void)
{
	UINT32 dwLoop = 0;
	UINT32 dwLoop2 = 0;

	bCurrentMode = TIMING_DDFD;

	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		while (dwLoop < 0x100)
		{
			sprintf(procname, "FDInst%.2x", dwLoop);
			dwLoop2 = 0;

			while (DDFDOps[dwLoop2].bOpCode != dwLoop && DDFDOps[dwLoop2].bOpCode != 0xffffffff)
				dwLoop2++;

			assert(dwLoop2 < 0x100);
			if (DDFDOps[dwLoop2].Emitter && DDFDOps[dwLoop2].bOpCode != 0xffffffff)
				DDFDOps[dwLoop2].Emitter((UINT32) dwLoop);

			dwLoop++;
		}
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "void FDHandler(void)\n");
		fprintf(fp, "{\n");
		fprintf(fp, "	switch (*pbPC++)\n");
		fprintf(fp, "	{\n");

		while (dwLoop < 0x100)
		{
			dwLoop2 = 0;

			fprintf(fp, " 		case 0x%.2x:\n", dwLoop);
			fprintf(fp, " 		{\n");

			while (DDFDOps[dwLoop2].bOpCode != dwLoop && DDFDOps[dwLoop2].bOpCode != 0xffffffff)
				dwLoop2++;

			if (bTimingDDFD[dwLoop])
			{
				fprintf(fp, "			sdwCyclesRemaining -= %" PRIu32 ";\n", bTimingDDFD[dwLoop]);
			}

			if (DDFDOps[dwLoop2].Emitter)
			{
				DDFDOps[dwLoop2].Emitter(dwLoop);
			}
			else
			{
				InvalidInstructionC(2);
			}

			fprintf(fp, "			break;\n");
			fprintf(fp, "		}\n");
			++dwLoop;
		}

		fprintf(fp, "	}\n");
		fprintf(fp, "}\n");
	}
	else
	{
		abort();
	}
}

/* These are the meta routines */

void ReadMemoryByteHandler()
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		Alignment();
		fprintf(fp, "; This is a generic read memory byte handler when a foreign\n");
		fprintf(fp, "; handler is to be called\n\n");
		fprintf(fp, "; EDI=Handler address, EDX=Address\n");
		fprintf(fp, "; On return, EDX & EDI are undisturbed and AL=Byte read\n\n");
		fprintf(fp, "ReadMemoryByte:\n");
	
		fprintf(fp, "		mov	[_z80af], ax	; Save AF\n");
		fprintf(fp, "		cmp	[edi+8], dword 0 ; Null handler?\n");
		fprintf(fp, "		je	directReadHandler	; Yep! It's a direct read!\n\n");

		fprintf(fp, "		mov	[_z80hl], bx	; Save HL\n");
		fprintf(fp, "		mov	[_z80bc], cx	; Save BC\n");

		fprintf(fp, "		sub	esi, ebp	; Our program counter\n");
		fprintf(fp, "		mov	[_z80pc], si	; Save our program counter\n");

		// Now adjust the proper timing

		fprintf(fp, "		mov	esi, [dwOriginalExec]	\n");
		fprintf(fp, "		sub	esi, [cyclesRemaining]\n");
		fprintf(fp, "		add	[dwElapsedTicks], esi\n");
		fprintf(fp, "		add	[_z80rCounter], esi\n");
		fprintf(fp, "		sub	[dwOriginalExec], esi\n");
	
		fprintf(fp, "		push	edi	; Save our structure address\n");
		fprintf(fp, "		push	edx	; And our desired address\n");

		if (FALSE == bUseStack)
		{
			fprintf(fp, "		mov	eax, edx	; Get our desired address reg\n");
			fprintf(fp, "		mov	edx, edi	; Pointer to the structure\n");
		}
	
		fprintf(fp, "		call	dword [edi + 8]	; Go call our handler\n");
	
		fprintf(fp, "		pop	edx	; Restore our address\n");
		fprintf(fp, "		pop	edi	; Restore our handler's address\n");
	
		fprintf(fp, "		xor	ebx, ebx	; Zero our future HL\n");
		fprintf(fp, "		xor	esi, esi	; Zero it!\n");
		fprintf(fp, "		mov	ebp, [_z80Base] ; Base pointer comes back\n");
		fprintf(fp, "		mov	si, [_z80pc]	; Get our program counter back\n");
		fprintf(fp, "		xor	ecx, ecx	; Zero our future BC\n");
		fprintf(fp, "		add	esi, ebp	; Rebase it properly\n");
	
		fprintf(fp, "		mov	bx, [_z80hl]	; Get HL back\n");
		fprintf(fp, "		mov	cx, [_z80bc]	; Get BC back\n");
	
	// Note: the callee must restore AF!

		fprintf(fp, "		ret\n\n");
		fprintf(fp, "directReadHandler:\n");
		fprintf(fp, "		mov	eax, [edi+12]	; Get our base address\n");
		fprintf(fp, "		sub	edx, [edi]	; Subtract our base (low) address\n");
		fprintf(fp, "		mov	al, [edx+eax]	; Get our data byte\n");
		fprintf(fp, "		and	eax, 0ffh	; Only the lower byte matters!\n");
		fprintf(fp, "		add	edx, [edi]	; Add our base back\n");
		fprintf(fp, "		ret		; Return to caller!\n\n");
	}
	else
	if (MZ80_C == bWhat)
	{
	}
	else
	{
		abort();
	}
}

void WriteMemoryByteHandler()
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		Alignment();
		fprintf(fp, "; This is a generic read memory byte handler when a foreign\n");
		fprintf(fp, "; handler is to be called.\n");
		fprintf(fp, "; EDI=Handler address, AL=Byte to write, EDX=Address\n");
		fprintf(fp, "; EDI and EDX Are undisturbed on exit\n\n");
		fprintf(fp, "WriteMemoryByte:\n");

		fprintf(fp, "		cmp	[edi+8], dword 0	; Null handler?\n");
		fprintf(fp, "		je	directWriteHandler\n\n");
		
		
		fprintf(fp, "		mov	[_z80hl], bx	; Save HL\n");
		fprintf(fp, "		mov	[_z80bc], cx	; Save BX\n");
	
		fprintf(fp, "		sub	esi, ebp	; Our program counter\n");
		fprintf(fp, "		mov	[_z80pc], si	; Save our program counter\n");
	
		// Now adjust the proper timing
	
		fprintf(fp, "		mov	esi, [dwOriginalExec]	\n");
		fprintf(fp, "		sub	esi, [cyclesRemaining]\n");
		fprintf(fp, "		add	[dwElapsedTicks], esi\n");
		fprintf(fp, "		add	[_z80rCounter], esi\n");
		fprintf(fp, "		sub	[dwOriginalExec], esi\n");
	
		fprintf(fp, "		push	edi	; Save our structure address\n");
	
		if (bUseStack)
			fprintf(fp, "		push	eax	; Data to write\n");
	
		fprintf(fp, "		push	edx	; And our desired address\n");
	
		if (FALSE == bUseStack)
		{
			fprintf(fp, "		xchg	eax, edx ; Swap address/data around\n");
			fprintf(fp, "		mov	ebx, edi	; Our MemoryWriteByte structure address\n");
		}
	
		fprintf(fp, "		call	dword [edi + 8]	; Go call our handler\n");
	
		fprintf(fp, "		pop	edx	; Restore our address\n");
		
		if (bUseStack)
			fprintf(fp, "		pop	eax	; Restore our data written\n");
		
		fprintf(fp, "		pop	edi	; Save our structure address\n");
	
		fprintf(fp, "		xor	ebx, ebx	; Zero our future HL\n");
		fprintf(fp, "		xor	ecx, ecx	; Zero our future BC\n");
		fprintf(fp, "		mov	bx, [_z80hl]	; Get HL back\n");
		fprintf(fp, "		mov	cx, [_z80bc]	; Get BC back\n");
		fprintf(fp, "		mov	ax, [_z80af]	; Get AF back\n");
		fprintf(fp, "		xor	esi, esi	; Zero it!\n");
		fprintf(fp, "		mov	si, [_z80pc]	; Get our program counter back\n");
		fprintf(fp, "		mov	ebp, [_z80Base] ; Base pointer comes back\n");
		fprintf(fp, "		add	esi, ebp	; Rebase it properly\n");

		fprintf(fp, "		ret\n\n");

		fprintf(fp, "directWriteHandler:\n");
		fprintf(fp, "		sub	edx, [edi]	; Subtract our offset\n");
		fprintf(fp, "		add	edx, [edi+12]	; Add in the base address\n");
		fprintf(fp, "		mov	[edx], al	; Store our byte\n");
		fprintf(fp, "		sub	edx, [edi+12]	; Restore our base address\n");
		fprintf(fp, "		add	edx, [edi]	; And put our offset back\n");
		fprintf(fp, "		ret\n\n");
	}
	else
	if (MZ80_C == bWhat)
	{
	}
	else
	{
		abort();
	}
}

void PushWordHandler()
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		Alignment();
	
		fprintf(fp, ";\n");
		fprintf(fp, "; DX=Top of SP, [_wordval]=word value to push\n");
		fprintf(fp, ";\n\n");
		fprintf(fp, "PushWord:\n");
		fprintf(fp, "		mov	dx, [_z80sp]\n");
		fprintf(fp, "		dec	dx\n");
		WriteValueToMemory("dx", "byte [_wordval+1]");
		fprintf(fp, "		dec	dx\n");
		WriteValueToMemory("dx", "byte [_wordval]");
		fprintf(fp, "		sub	[_z80sp], word 2\n");
		fprintf(fp, "		xor	edx, edx\n");
		fprintf(fp, "		ret\n\n");
	}
}

void PopWordHandler()
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		Alignment();
		
		fprintf(fp, ";\n");
		fprintf(fp, "; [_z80sp]=Top of SP, DX=Word value read\n");
		fprintf(fp, ";\n\n");
		fprintf(fp, "PopWord:\n");
		fprintf(fp, "		mov	dx, [_z80sp]\n");
		
		ReadWordFromMemory("dx", "dx");
		fprintf(fp, "		ret\n\n");
	}
}

void ReadIoHandler()
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		Alignment();

		fprintf(fp, "; This is a generic I/O read byte handler for when a foreign\n");
		fprintf(fp, "; handler is to be called\n");

		fprintf(fp, "; EDI=Handler address, EDX=I/O Address\n");
		fprintf(fp, "; On return, EDX & EDI are undisturbed and AL=Byte read\n\n");
		fprintf(fp, "ReadIOByte:\n");
	
		fprintf(fp, "		mov	[_z80af], ax	; Save AF\n");
		fprintf(fp, "		mov	[_z80hl], bx	; Save HL\n");
		fprintf(fp, "		mov	[_z80bc], cx	; Save BC\n");

		fprintf(fp, "		sub	esi, ebp	; Our program counter\n");
		fprintf(fp, "		mov	[_z80pc], si	; Save our program counter\n");

		// Now adjust the proper timing

		fprintf(fp, "		mov	esi, [dwOriginalExec]	\n");
		fprintf(fp, "		sub	esi, [cyclesRemaining]\n");
		fprintf(fp, "		add	[dwElapsedTicks], esi\n");
		fprintf(fp, "		add	[_z80rCounter], esi\n");
		fprintf(fp, "		sub	[dwOriginalExec], esi\n");

		fprintf(fp, "		push	edi	; Save our structure address\n");
		fprintf(fp, "		push	edx	; And our desired I/O port\n");

		if (FALSE == bUseStack)
		{
			fprintf(fp, "		mov	eax, edx	; Get our desired address reg\n");
			fprintf(fp, "		mov	edx, edi	; Pointer to the structure\n");
		}

		fprintf(fp, "		call	dword [edi + 4]	; Go call our handler\n");

		fprintf(fp, "		pop	edx	; Restore our address\n");
		fprintf(fp, "		pop	edi	; Restore our handler's address\n");

		fprintf(fp, "		xor	ebx, ebx	; Zero our future HL\n");
		fprintf(fp, "		xor	ecx, ecx	; Zero our future BC\n");
		fprintf(fp, "		xor	esi, esi	; Zero it!\n");
		fprintf(fp, "		mov	si, [_z80pc]	; Get our program counter back\n");
		fprintf(fp, "		mov	ebp, [_z80Base] ; Base pointer comes back\n");
		fprintf(fp, "		add	esi, ebp	; Rebase it properly\n");

		fprintf(fp, "		mov	bx, [_z80hl]	; Get HL back\n");
		fprintf(fp, "		mov	cx, [_z80bc]	; Get BC back\n");

	// Note: the callee must restore AF!

		fprintf(fp, "		ret\n\n");
	}
	else
	if (MZ80_C == bWhat)
	{
	}
	else
	{
		abort();
	}
}

void WriteIoHandler()
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		Alignment();

		fprintf(fp, "; This is a generic write I/O byte handler when a foreign handler is to\n");
		fprintf(fp, "; be called\n");
		fprintf(fp, "; EDI=Handler address, AL=Byte to write, EDX=I/O Address\n");
		fprintf(fp, "; EDI and EDX Are undisturbed on exit\n\n");
		fprintf(fp, "WriteIOByte:\n");
	
		fprintf(fp, "		mov	[_z80hl], bx	; Save HL\n");
		fprintf(fp, "		mov	[_z80bc], cx	; Save BX\n");

		fprintf(fp, "		sub	esi, ebp	; Our program counter\n");
		fprintf(fp, "		mov	[_z80pc], si	; Save our program counter\n");

		// Now adjust the proper timing

		fprintf(fp, "		mov	esi, [dwOriginalExec]	\n");
		fprintf(fp, "		sub	esi, [cyclesRemaining]\n");
		fprintf(fp, "		add	[dwElapsedTicks], esi\n");
		fprintf(fp, "		add	[_z80rCounter], esi\n");
		fprintf(fp, "		sub	[dwOriginalExec], esi\n");

		fprintf(fp, "		push	edi	; Save our structure address\n");

		if (bUseStack)
			fprintf(fp, "		push	eax	; Data to write\n");

		fprintf(fp, "		push	edx	; And our desired I/O address\n");

		if (FALSE == bUseStack)
		{
			fprintf(fp, "		xchg	eax, edx ; Swap address/data around\n");
			fprintf(fp, "		mov	ebx, edi	; Our z80IoWrite structure address\n");
		}

		fprintf(fp, "		call	dword [edi + 4]	; Go call our handler\n");

		fprintf(fp, "		pop	edx	; Restore our address\n");
	
		if (bUseStack)
			fprintf(fp, "		pop	eax	; Restore our data written\n");
		
		fprintf(fp, "		pop	edi	; Save our structure address\n");

		fprintf(fp, "		xor	ebx, ebx	; Zero our future HL\n");
		fprintf(fp, "		xor	ecx, ecx	; Zero our future BC\n");
		fprintf(fp, "		mov	bx, [_z80hl]	; Get HL back\n");
		fprintf(fp, "		mov	cx, [_z80bc]	; Get BC back\n");
		fprintf(fp, "		mov	ax, [_z80af]	; Get AF back\n");
		fprintf(fp, "		xor	esi, esi	; Zero it!\n");
		fprintf(fp, "		mov	si, [_z80pc]	; Get our program counter back\n");
		fprintf(fp, "		mov	ebp, [_z80Base] ; Base pointer comes back\n");
		fprintf(fp, "		add	esi, ebp	; Rebase it properly\n");

		fprintf(fp, "		ret\n\n");
	}
	else
	if (MZ80_C == bWhat)
	{
	}
	else
	{
		abort();
	}
}

void ExecCode(void)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp, "		global	_%sexec\n", cpubasename);
		fprintf(fp, "		global	%sexec_\n", cpubasename);
	
		if (bPlain)
			fprintf(fp, "		global	%sexec\n", cpubasename);
	
		sprintf(procname, "%sexec_", cpubasename);
		ProcBegin(0xffffffff);
	
		fprintf(fp, "_%sexec:\n", cpubasename);
	
		if (bPlain)
			fprintf(fp, "%sexec:\n", cpubasename);
	
		if (bUseStack)
			fprintf(fp, "		mov	eax, [esp+4]	; Get our execution cycle count\n");
	
		fprintf(fp, "		push	ebx			; Save all registers we use\n");
		fprintf(fp, "		push	ecx\n");
		fprintf(fp, "		push	edx\n");
		fprintf(fp, "		push	ebp\n");
		fprintf(fp, "		push	esi\n");
		fprintf(fp, "		push	edi\n");
		fprintf(fp, "\n");

		fprintf(fp, "		mov	edi, eax\n");	
		fprintf(fp, "		mov	dword [cyclesRemaining], eax	; Store # of instructions to\n");
		fprintf(fp, "		mov	[dwLastRSample], eax\n");
		fprintf(fp, "		mov	[dwOriginalExec], eax	; Store this!\n");

		fprintf(fp, "		cmp	dword [_z80halted], 0\n");
		fprintf(fp, "		je	goCpu\n");
		fprintf(fp, "		add	[_z80rCounter], eax\n");
	
		if (FALSE == bNoTiming)
		{
			fprintf(fp, "		add	dword [dwElapsedTicks], eax\n");
		}
	
		fprintf(fp, "		mov	dword [cyclesRemaining], 0	; Nothing left!\n");
		fprintf(fp, "		mov	eax, 80000000h	; Successful exection\n");
		fprintf(fp, "		jmp	popReg\n");
		fprintf(fp, "goCpu:\n");
		fprintf(fp, "		cld				; Go forward!\n");
		fprintf(fp, "\n");
		fprintf(fp, "		xor	eax, eax		; Zero EAX 'cause we use it!\n");
		fprintf(fp, "		xor	ebx, ebx		; Zero EBX, too\n");
		fprintf(fp, "		xor	ecx, ecx		; Zero ECX\n");
		fprintf(fp, "		xor	edx, edx		; And EDX\n");
		fprintf(fp, "		xor	esi, esi		; Zero our source address\n");
		fprintf(fp, "\n");
		fprintf(fp, "		mov	ax, [_z80af]		; Accumulator & flags\n");
		fprintf(fp, "		xchg	ah, al		; Swap these for later\n");
		fprintf(fp, "		mov	bx, [_z80hl]		; Get our HL value\n");
		fprintf(fp, "		mov	cx, [_z80bc]		; And our BC value\n");
		fprintf(fp, "		mov	ebp, [_z80Base]		; Get the base address\n");
		fprintf(fp, "		mov	si, [_z80pc]		; Get our program counter\n");
		fprintf(fp, "		add	esi, ebp		; Add in our base address\n");

		fprintf(fp, "		cmp	[_z80intPending], byte 0	; Interrupt pending?\n");
		fprintf(fp, "		jz		masterExecTarget\n\n");
		fprintf(fp, "		call	causeInternalInterrupt\n\n");
		fprintf(fp, "masterExecTarget:\n");
		fprintf(fp, "		mov	dl, [esi]\n");
		fprintf(fp, "		inc	esi\n");
		fprintf(fp, "		jmp	dword [z80regular+edx*4]\n\n");
		fprintf(fp, "; We get to invalidInsWord if it's a double byte invalid opcode\n");
		fprintf(fp, "\n");
		fprintf(fp, "invalidInsWord:\n");
	
		fprintf(fp, "		dec	esi\n");
		fprintf(fp, "\n");
		fprintf(fp, "; We get to invalidInsByte if it's a single byte invalid opcode\n");
		fprintf(fp, "\n");
	
		fprintf(fp, "invalidInsByte:\n");
		fprintf(fp, "		xchg	ah, al		; Swap them back so they look good\n");
		fprintf(fp, "		mov	[_z80af], ax		; Store A & flags\n");
		fprintf(fp, "		dec	esi			; Back up one instruction...\n");
		fprintf(fp, "		mov	edx, esi		; Get our address in EAX\n");
		fprintf(fp, "		sub	edx, ebp		; And subtract our base for\n");
		fprintf(fp, "						; an invalid instruction\n");
		fprintf(fp, "		jmp	short emulateEnd\n");
		fprintf(fp, "\n");
		fprintf(fp, "noMoreExec:\n");
		fprintf(fp, "		cmp	[bEIExit], byte 0	; Are we exiting because of an EI?\n");
		fprintf(fp, "		jne	checkEI\n");
		fprintf(fp, "noMoreExecNoEI:\n");
		fprintf(fp, "		xchg	ah, al		; Swap these for later\n");
		fprintf(fp, "		mov	[_z80af], ax		; Store A & flags\n");
	
		fprintf(fp, "		mov	edx, [dwOriginalExec]	; Original exec time\n");
		fprintf(fp, "		sub	edx, edi		; Subtract # of cycles remaining\n");
		fprintf(fp, "		add	[_z80rCounter], edx\n");
		fprintf(fp, "		add	[dwElapsedTicks], edx	; Add our executed time\n");
	
		fprintf(fp, "		mov	edx, 80000000h		; Indicate successful exec\n");
		fprintf(fp, "		jmp	short emulateEnd	; All finished!\n");
		fprintf(fp, "\n");
		fprintf(fp, "; Now let's tuck away the virtual registers for next time\n");
		fprintf(fp, "\n");
		fprintf(fp, "storeFlags:\n");
		fprintf(fp, "		xchg	ah, al		; Swap these for later\n");
		fprintf(fp, "		mov	[_z80af], ax		; Store A & flags\n");
		fprintf(fp, "emulateEnd:\n");
		fprintf(fp, "		mov	[_z80hl], bx		; Store HL\n");
		fprintf(fp, "		mov	[_z80bc], cx		; Store BC\n");
		fprintf(fp, "		sub	esi, [_z80Base]		; Knock off physical address\n");
		fprintf(fp, "		mov	[_z80pc], si		; And store virtual address\n");
		fprintf(fp, "		mov	eax, edx		; Result code return\n");
		fprintf(fp, "\n");
		fprintf(fp, "popReg:\n");
		fprintf(fp, "		pop	edi			; Restore registers\n");
		fprintf(fp, "		pop	esi\n");
		fprintf(fp, "		pop	ebp\n");
		fprintf(fp, "		pop	edx\n");
		fprintf(fp, "		pop	ecx\n");
		fprintf(fp, "		pop	ebx\n");
		fprintf(fp, "\n");
		fprintf(fp, "		ret\n");
		fprintf(fp, "\n");
		Alignment();
		fprintf(fp, "checkEI:\n");
		fprintf(fp, "		xor	edx, edx\n");
		fprintf(fp, "		mov	[bEIExit], byte 0\n");
		fprintf(fp, "		sub	edx, edi	; Find out how much time has passed\n");
		fprintf(fp, "		mov	edi, [dwEITiming]\n");
		fprintf(fp, "		sub	edi, edx\n");
		fprintf(fp, "		js		noMoreExecNoEI\n");
		fprintf(fp, "		xor	edx, edx\n");

		fprintf(fp, "		cmp	[_z80intPending], byte 0\n");
		fprintf(fp, "		je	near masterExecTarget\n");
		fprintf(fp, "		call	causeInternalInterrupt\n");
		fprintf(fp, "		jmp	masterExecTarget\n\n");

		Alignment();
		fprintf(fp, "causeInternalInterrupt:\n");
		fprintf(fp, "		mov	dword [_z80halted], 0	; We're not halted anymore!\n");
		fprintf(fp, "		test	[_z80iff], byte IFF1	; Interrupt enabled yet?\n");
		fprintf(fp, "		jz		near internalInterruptsDisabled\n");

		fprintf(fp, "\n; Interrupts enabled. Clear IFF1 and IFF2\n\n");

		fprintf(fp, "		mov	[_z80intPending], byte 0\n");

		fprintf(fp, "\n; Save off our active register sets\n\n");

		fprintf(fp, "		xchg	ah, al		; Swap these for later\n");
		fprintf(fp, "		mov	[_z80af], ax		; Store A & flags\n");
		fprintf(fp, "		mov	[_z80hl], bx		; Store HL\n");
		fprintf(fp, "		mov	[_z80bc], cx		; Store BC\n");
		fprintf(fp, "		sub	esi, ebp			; Knock off physical address\n");
		fprintf(fp, "		mov	[_z80pc], si		; And store virtual address\n");

		fprintf(fp, "		xor	eax, eax\n");
		fprintf(fp, "		mov	al, [_intData]\n\n");

		fprintf(fp, "\n");
		fprintf(fp, "		push	edi\n");
		fprintf(fp, "\n");
	
		if (bThroughCallHandler)
		{
			fprintf(fp, "       pushad\n" );
			fprintf(fp, "		xor edx, edx\n" );
			fprintf(fp, "		mov	ax, [_z80pc]\n");
			fprintf(fp, "		mov	[_wordval], ax\n");
			fprintf(fp, "		push	ecx\n");
			fprintf(fp, "		push	ebx\n");
			fprintf(fp, "		push	esi\n");
	
			fprintf(fp, "       mov ax, [_z80af]\n");	// Get AF
			fprintf(fp, "       mov	bx, [_z80hl]\n");	// Get HL
			fprintf(fp, "       mov	cx, [_z80bc]\n");	// Get BC
			fprintf(fp, "		call	PushWord\n");
	
			fprintf(fp, "		pop	esi\n");
			fprintf(fp, "		pop	ebx\n");
			fprintf(fp, "		pop	ecx\n");
			fprintf(fp, "       popad\n" );
		}
		else
		{
			fprintf(fp, "		mov	dx, [_z80pc]\n");
			fprintf(fp, "		xor	edi, edi\n");
			fprintf(fp, "		mov	di, word [_z80sp]\n");
			fprintf(fp, "		sub	di, 2\n");
			fprintf(fp, "		mov	word [_z80sp], di\n");
			fprintf(fp, "		mov	[ebp+edi], dx\n");
		}
	
		fprintf(fp, "		cmp	dword [_z80interruptMode], 2 ; Are we lower than mode 2?\n");
		fprintf(fp, "		jb		internalJustModeTwo\n");
		fprintf(fp, "		mov	ah, [_z80i]	; Get our high address here\n");
		fprintf(fp, "		and	eax, 0ffffh ; Only the lower part\n");
		fprintf(fp, "		mov	ax, [eax+ebp] ; Get our vector\n");
		fprintf(fp, "		jmp	short internalSetNewVector ; Go set it!\n");
		fprintf(fp, "internalJustModeTwo:\n");
		fprintf(fp, "		mov	ax, word [_z80intAddr]\n");
		fprintf(fp, "internalSetNewVector:\n");
		fprintf(fp, "		mov	[_z80pc], ax\n");
		fprintf(fp, "\n");
		fprintf(fp, "		pop	edi\n");
		fprintf(fp, "\n");
		fprintf(fp, "		xor	eax, eax	; Zero this so we can use it as an index\n");

		fprintf(fp, "		mov	al, [_z80interruptMode]\n");
		fprintf(fp, "		mov	al, [intModeTStates+eax]\n");
		fprintf(fp, "		sub	edi, eax\n");
		fprintf(fp, "		add	[_z80rCounter], eax\n");

		fprintf(fp, "\n; Restore all the registers and whatnot\n\n");

		fprintf(fp, "		mov	ax, [_z80af]		; Accumulator & flags\n");
		fprintf(fp, "		xchg	ah, al		; Swap these for later\n");
		fprintf(fp, "		mov	bx, [_z80hl]		; Get our HL value\n");
		fprintf(fp, "		mov	cx, [_z80bc]		; And our BC value\n");
		fprintf(fp, "		mov	ebp, [_z80Base]		; Get the base address\n");
		fprintf(fp, "		mov	si, [_z80pc]		; Get our program counter\n");
		fprintf(fp, "		add	esi, ebp		; Add in our base address\n");

		fprintf(fp, "internalInterruptsDisabled:\n");
		fprintf(fp, "		xor	edx, edx\n");
		fprintf(fp, "		ret\n");
	}
	else
	if (MZ80_C == bWhat)
	{
	}
	else
	{
		abort();
	}
}

void NmiCode(void)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp, "		global	_%snmi\n", cpubasename);
		fprintf(fp, "		global	%snmi_\n", cpubasename);

		if (bPlain)
			fprintf(fp, "		global	%snmi\n", cpubasename);
	
		sprintf(procname, "%snmi_", cpubasename);
		ProcBegin(0xffffffff);
		fprintf(fp, "_%snmi:\n", cpubasename);
	
		if (bPlain)
			fprintf(fp, "%snmi:\n", cpubasename);

		fprintf(fp, "		mov	dword [_z80halted], 0	; We're not halted anymore!\n");
		fprintf(fp, "		mov	al, [_z80iff]	; Get our IFF setting\n");
		fprintf(fp, "		and	al, IFF1	; Just IFF 1\n");
		fprintf(fp, "		shl	al, 1	; Makes IFF1->IFF2 and zeros IFF1\n");
		fprintf(fp, "		mov	[_z80iff], al	; Store it back to the interrupt state!\n");
		fprintf(fp, "\n");
		fprintf(fp, "		push	ebp\n");
		fprintf(fp, "		push	edi\n");
		fprintf(fp, "		mov	ebp, [_z80Base]\n");
		fprintf(fp, "\n");

		fprintf(fp, "		xor	eax, eax\n");
		fprintf(fp, "		mov	ax, [_z80pc]\n");

		if (bThroughCallHandler)
		{
			fprintf(fp, "		push	esi\n");
			fprintf(fp, "		push	ebx\n");
			fprintf(fp, "		push	ecx\n");

			fprintf(fp, "		mov	[_wordval], ax\n");
			fprintf(fp, "		mov	esi, ebp\n");
			fprintf(fp, "		add	esi, eax\n");
			fprintf(fp, "       mov ax, [_z80af]\n");	// Get AF
			fprintf(fp, "       mov	bx, [_z80hl]\n");	// Get HL
			fprintf(fp, "       mov	cx, [_z80bc]\n");	// Get BC
			fprintf(fp, "		push	ebx\n");
			fprintf(fp, "		push	ecx\n");
			fprintf(fp, "		push	edx\n");
			fprintf(fp, "		push	esi\n");
			fprintf(fp, "		push	eax\n");
			fprintf(fp, "		call	PushWord\n");
			fprintf(fp, "		pop	eax\n");
			fprintf(fp, "		pop	esi\n");
			fprintf(fp, "		pop	edx\n");
			fprintf(fp, "		pop	ecx\n");
			fprintf(fp,	"		pop	ebx\n");

			fprintf(fp, "		pop	ecx\n");
			fprintf(fp, "		pop	ebx\n");
			fprintf(fp, "		pop	esi\n");
		}	  
		else   
		{ 
			fprintf(fp, "		xor	edi, edi\n");
			fprintf(fp, "		mov	di, word [_z80sp]\n");
			fprintf(fp, "		sub	di, 2\n");
			fprintf(fp, "		mov	word [_z80sp], di\n");
			fprintf(fp, "		mov	[ebp+edi], ax\n");
		}

		fprintf(fp, "		mov	ax, [_z80nmiAddr]\n");
		fprintf(fp, "		mov	[_z80pc], ax\n");
		fprintf(fp, "\n");
		fprintf(fp, "		add	[dwElapsedTicks], dword 11	; 11 T-States for NMI\n");
		fprintf(fp, "		add	[_z80rCounter], dword 11\n");
		fprintf(fp, "		pop	edi\n");
		fprintf(fp, "		pop	ebp\n");
		fprintf(fp, "\n");
		fprintf(fp, "		xor	eax, eax	; Indicate we took the interrupt\n");
		fprintf(fp, "		ret\n");
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "/* NMI Handler */\n\n");
		fprintf(fp, "UINT32 %snmi(void)\n", cpubasename);
		fprintf(fp, "{\n");

		fprintf(fp, "	cpu.z80halted = 0;\n");
		fprintf(fp, "	pbSP = (cpu.z80Base + cpu.z80sp - 1);	/* Normalize the stack pointer */\n");
		fprintf(fp, "	*pbSP-- = cpu.z80pc >> 8;	/* LSB */\n");
		fprintf(fp, "	*pbSP = (UINT8) cpu.z80pc;	/* MSB */\n");
		fprintf(fp, "	cpu.z80sp -= 2;	/* Back our stack up */\n");
		fprintf(fp, "	cpu.z80pc = cpu.z80nmiAddr;	/* Our NMI */\n");

		fprintf(fp, "	return(0);\n");
		fprintf(fp, "}\n\n");
	}
	else
	{
		abort();
	}
}

void IntCode(void)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp, "		global	_%sint\n", cpubasename);
		fprintf(fp, "		global	%sint_\n", cpubasename);

		if (bPlain)
			fprintf(fp, "		global	%sint\n", cpubasename);

		sprintf(procname, "%sint_", cpubasename);
		ProcBegin(0xffffffff);
		fprintf(fp, "_%sint:\n", cpubasename);

		if (bPlain)
			fprintf(fp, "%sint:\n", cpubasename);

		if (bUseStack)
			fprintf(fp, "		mov	eax, [esp+4]	; Get our (potential) lower interrupt address\n");
	
		fprintf(fp, "		mov	dword [_z80halted], 0	; We're not halted anymore!\n");

		fprintf(fp, "		mov	ah, IFF1	; Is IFF1 enabled?\n");
		fprintf(fp, "		and	ah, [_z80iff]	; Well, is it?\n");
		fprintf(fp, "		jz		near interruptsDisabled\n");

		fprintf(fp, "\n; Interrupts enabled. Clear IFF1 and IFF2\n\n");

		fprintf(fp, "		and	dword [_z80iff], ~(IFF1 | IFF2);\n\n");
		fprintf(fp, "		mov	[_z80intPending], byte 0\n");

		fprintf(fp, "\n");
		fprintf(fp, "		push	ebp\n");
		fprintf(fp, "		push	edi\n");
		fprintf(fp, "		push	edx\n");
		fprintf(fp, "		mov	ebp, [_z80Base]\n");
		fprintf(fp, "\n");
	
	
		if (bThroughCallHandler)
		{
			fprintf(fp, "       pushad\n" );
			fprintf(fp, "		xor edx, edx\n" );
			fprintf(fp, "		mov	ax, [_z80pc]\n");
			fprintf(fp, "		mov	[_wordval], ax\n");
			fprintf(fp, "		push	ecx\n");
			fprintf(fp, "		push	ebx\n");
			fprintf(fp, "		push	esi\n");
	
			fprintf(fp, "       mov ax, [_z80af]\n");	// Get AF
			fprintf(fp, "       mov	bx, [_z80hl]\n");	// Get HL
			fprintf(fp, "       mov	cx, [_z80bc]\n");	// Get BC
			fprintf(fp, "		call	PushWord\n");
	
			fprintf(fp, "		pop	esi\n");
			fprintf(fp, "		pop	ebx\n");
			fprintf(fp, "		pop	ecx\n");
			fprintf(fp, "       popad\n" );
		}
		else
		{
			fprintf(fp, "		mov	dx, [_z80pc]\n");
			fprintf(fp, "		xor	edi, edi\n");
			fprintf(fp, "		mov	di, word [_z80sp]\n");
			fprintf(fp, "		sub	di, 2\n");
			fprintf(fp, "		mov	word [_z80sp], di\n");
			fprintf(fp, "		mov	[ebp+edi], dx\n");
		}
	
		fprintf(fp, "		cmp	dword [_z80interruptMode], 2 ; Are we lower than mode 2?\n");
		fprintf(fp, "		jb		justModeTwo\n");
		fprintf(fp, "		mov	ah, [_z80i]	; Get our high address here\n");
		fprintf(fp, "		and	eax, 0ffffh ; Only the lower part\n");
		fprintf(fp, "		mov	ax, [eax+ebp] ; Get our vector\n");
		fprintf(fp, "		jmp	short setNewVector ; Go set it!\n");
		fprintf(fp, "justModeTwo:\n");
		fprintf(fp, "		mov	ax, word [_z80intAddr]\n");
		fprintf(fp, "setNewVector:\n");
		fprintf(fp, "		mov	[_z80pc], ax\n");
		fprintf(fp, "\n");
		fprintf(fp, "		pop	edx\n");
		fprintf(fp, "		pop	edi\n");
		fprintf(fp, "		pop	ebp\n");
		fprintf(fp, "\n");
		fprintf(fp, "		xor	eax, eax	; Zero this so we can use it as an index\n");

		fprintf(fp, "		mov	al, [_z80interruptMode]\n");
		fprintf(fp, "		mov	al, [intModeTStates+eax]\n");
		fprintf(fp, "		add	[dwElapsedTicks], eax\n");
		fprintf(fp, "		add	[_z80rCounter], eax\n");
		fprintf(fp, "		xor	eax, eax	; Indicate we took the interrupt\n");

		fprintf(fp, "		jmp	short z80intExit\n");
		fprintf(fp, "\n");
		fprintf(fp, "interruptsDisabled:\n");
		fprintf(fp, "		mov	[_z80intPending], byte 1\n");
		fprintf(fp, "		mov	[_intData], al	; Save this info for later\n");
		fprintf(fp, "		mov	eax, 0ffffffffh		; Indicate we didn't take it\n");
		fprintf(fp, "\n");
		fprintf(fp, "z80intExit:\n");
		fprintf(fp, "		ret\n\n");


		fprintf(fp, "		global	_%sClearPendingInterrupt\n", cpubasename);
		fprintf(fp, "		global	%sClearPendingInterrupt_\n", cpubasename);

		if (bPlain)
			fprintf(fp, "		global	%sClearPendingInterrupt\n", cpubasename);

		sprintf(procname, "%sClearPendingInterrupt_", cpubasename);
		ProcBegin(0xffffffff);
		fprintf(fp, "_%sClearPendingInterrupt:\n", cpubasename);

		if (bPlain)
			fprintf(fp, "%sClearPendingInterrupt:\n", cpubasename);

		fprintf(fp, "		mov	[_z80intPending], byte 0\n");
		fprintf(fp, "		ret\n\n");
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "/* Interrupt handler */\n\n");
		fprintf(fp, "UINT32 %sint(UINT32 dwLowAddr)\n", cpubasename);
		fprintf(fp, "{\n");
		fprintf(fp, "	cpu.z80halted = 0;\n");

		fprintf(fp, "	if (0 == (cpu.z80iff & IFF1))\n");
		fprintf(fp, "		return(0xffffffff);\n");

		fprintf(fp, "	cpu.z80iff &= ~(IFF1 | IFF2);\n");
		fprintf(fp, "	pbSP = (cpu.z80Base + cpu.z80sp - 1);	/* Normalize the stack pointer */\n");
		fprintf(fp, "	*pbSP-- = cpu.z80pc >> 8;	/* LSB */\n");
		fprintf(fp, "	*pbSP = (UINT8) cpu.z80pc;	/* MSB */\n");
		fprintf(fp, "	cpu.z80sp -= 2;	/* Back our stack up */\n");

		fprintf(fp, "	if (2 == cpu.z80interruptMode)\n");
		fprintf(fp, "	{\n");
		fprintf(fp, "		cpu.z80pc = ((UINT16) cpu.z80i << 8) | (dwLowAddr & 0xff);\n");
		fprintf(fp, "		cpu.z80pc = ((UINT16) cpu.z80Base[cpu.z80pc + 1] << 8) | (cpu.z80Base[cpu.z80pc]);\n");
		fprintf(fp, "	}\n");
		fprintf(fp, "	else\n");
		fprintf(fp, "	{\n");
		fprintf(fp, "		cpu.z80pc = cpu.z80intAddr;\n");
		fprintf(fp, "	}\n");

		fprintf(fp, "	pbPC = cpu.z80Base + cpu.z80pc;	/* Normalize the address */\n");

		fprintf(fp, "	return(0);\n");
		fprintf(fp, "}\n\n");
		fprintf(fp, "void %sClearPendingInterrupt(void)\n", cpubasename);
		fprintf(fp, "{\n");
		fprintf(fp, "	cpu.z80intPending = 0;\n");
		fprintf(fp, "}\n\n");
	}
	else
	{
		abort();
	}
}

void ResetCode(void)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp, "		global	_%sreset\n", cpubasename);
		fprintf(fp, "		global	%sreset_\n", cpubasename);

		if (bPlain)
			fprintf(fp, "		global	%sreset\n", cpubasename);
		sprintf(procname, "%sreset_", cpubasename);
		ProcBegin(0xffffffff);

		fprintf(fp, "_%sreset:\n", cpubasename);

		if (bPlain)
			fprintf(fp, "%sreset:\n", cpubasename);

		fprintf(fp, "		xor	eax, eax 	; Zero AX\n");
		fprintf(fp, "\n");
		fprintf(fp, "		mov	dword [_z80halted], eax	; We're not halted anymore!\n");
		fprintf(fp, "		mov	word [_z80af], 0040h	; Zero A & flags - zero flag set\n");
		fprintf(fp, "		mov	word [_z80bc], ax	; Zero BC\n");
		fprintf(fp, "		mov	word [_z80de],	ax	; Zero DE\n");
		fprintf(fp, "		mov	word [_z80hl], ax	; Zero HL\n");
		fprintf(fp, "		mov	word [_z80afprime], ax	; Zero AF Prime\n");
		fprintf(fp, "		mov	word [_z80bcprime], ax	; Zero BC prime\n");
		fprintf(fp, "		mov	word [_z80deprime], ax ; Zero DE prime\n");
		fprintf(fp, "		mov	word [_z80hlprime], ax ; Zero HL prime\n");
		fprintf(fp, "		mov	byte [_z80i], al	; Zero Interrupt register\n");
		fprintf(fp, "		mov	byte [_z80r], al	; Zero refresh register\n");
		fprintf(fp, "		mov	word [_z80ix], 0ffffh	; Default mz80Index register\n");
		fprintf(fp, "		mov	word [_z80iy], 0ffffh	; Default mz80Index register\n");
		fprintf(fp, "		mov	word [_z80pc], ax	; Zero program counter\n");
		fprintf(fp, "		mov	word [_z80sp], ax	; And the stack pointer\n");
		fprintf(fp, "		mov	dword [_z80iff], eax ; IFF1/IFF2 disabled!\n");
		fprintf(fp, "		mov	dword [_z80interruptMode], eax ; Clear our interrupt mode (0)\n");
		fprintf(fp, "		mov	word [_z80intAddr], 38h ; Set default interrupt address\n");
		fprintf(fp, "		mov	word [_z80nmiAddr], 66h ; Set default nmi addr\n");
		fprintf(fp, "\n");
		fprintf(fp, "		ret\n");
		fprintf(fp, "\n");
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "/* This routine is mz80's reset handler */\n\n");
		fprintf(fp, "void %sreset(void)\n", cpubasename);
		fprintf(fp, "{\n");
		fprintf(fp, "	cpu.z80halted = 0;\n");
		fprintf(fp, "	cpu.z80AF = 0;\n");
		fprintf(fp, "	cpu.z80F = Z80_FLAG_ZERO;\n");
		fprintf(fp, "	cpu.z80BC = 0;\n");
		fprintf(fp, "	cpu.z80DE = 0;\n");
		fprintf(fp, "	cpu.z80HL = 0;\n");
		fprintf(fp, "	cpu.z80afprime = 0;\n");
		fprintf(fp, "	cpu.z80bcprime = 0;\n");
		fprintf(fp, "	cpu.z80deprime = 0;\n");
		fprintf(fp, "	cpu.z80hlprime = 0;\n");
		fprintf(fp, "	cpu.z80i = 0;\n");
		fprintf(fp, "	cpu.z80r = 0;\n");
		fprintf(fp, "	cpu.z80IX = 0xffff; /* Yes, this is intentional */\n");
		fprintf(fp, "	cpu.z80IY = 0xffff; /* Yes, this is intentional */\n");
		fprintf(fp, "	cpu.z80pc = 0;\n");
		fprintf(fp, "	cpu.z80sp = 0xffff; /* Prevent segfault on reset */\n");
		fprintf(fp, "	cpu.z80interruptMode = 0;\n");
		fprintf(fp, "	cpu.z80intAddr = 0x38;\n");
		fprintf(fp, "	cpu.z80nmiAddr = 0x66;\n");
		fprintf(fp, "}\n\n");
	}
	else
	{
		abort();
	}
}

void SetContextCode(void)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp, "		global	_%sSetContext\n", cpubasename);
		fprintf(fp, "		global	%sSetContext_\n", cpubasename);
	
		if (bPlain)
			fprintf(fp, "		global	%sSetContext\n", cpubasename);
	
		sprintf(procname, "%sSetContext_", cpubasename);
		ProcBegin(0xffffffff);
		fprintf(fp, "_%sSetContext:\n", cpubasename);
	
		if (bPlain)
			fprintf(fp, "%sSetContext:\n", cpubasename);
	
		if (bUseStack)
			fprintf(fp, "		mov	eax, [esp+4]	; Get our context address\n");
	
		fprintf(fp, "		push	esi		; Save registers we use\n");
		fprintf(fp, "		push	edi\n");
		fprintf(fp, "		push	ecx\n");
		fprintf(fp, "		push	es\n");
		fprintf(fp, "		mov	di, ds\n");
		fprintf(fp, "		mov	es, di\n");
		fprintf(fp, "		mov	edi, _%scontextBegin\n", cpubasename);
		fprintf(fp, "		mov	esi, eax	; Source address in ESI\n");
		fprintf(fp, "		mov     ecx, (_%scontextEnd - _%scontextBegin) >> 2\n", cpubasename, cpubasename);
		fprintf(fp, "		rep	movsd\n");
		fprintf(fp, "		mov     ecx, (_%scontextEnd - _%scontextBegin) & 0x03\n", cpubasename, cpubasename);
		fprintf(fp, "		rep	movsb\n");
		fprintf(fp, "		pop	es\n");
		fprintf(fp, "		pop	ecx\n");
		fprintf(fp, "		pop	edi\n");
		fprintf(fp, "		pop	esi\n");
		fprintf(fp, "		ret			; No return code\n");
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "/* Set mz80's context */\n\n");
		fprintf(fp, "void %sSetContext(void *pData)\n", cpubasename);
		fprintf(fp, "{\n");
		fprintf(fp, "	memcpy(&cpu, pData, sizeof(CONTEXTMZ80));\n");
		fprintf(fp, "}\n\n");
	}
	else
	{
		abort();
	}
}

void GetContextCode(void)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp, "		global	_%sGetContext\n", cpubasename);
		fprintf(fp, "		global	%sGetContext_\n", cpubasename);

		if (bPlain)
			fprintf(fp, "		global	%sGetContext\n", cpubasename);
	
		sprintf(procname, "%sGetContext_", cpubasename);
		ProcBegin(0xffffffff);
		fprintf(fp, "_%sGetContext:\n", cpubasename);

		if (bPlain)
			fprintf(fp, "%sGetContext:\n", cpubasename);

		if (bUseStack)
			fprintf(fp, "		mov	eax, [esp+4]	; Get our context address\n");

		fprintf(fp, "		push	esi		; Save registers we use\n");
		fprintf(fp, "		push	edi\n");
		fprintf(fp, "		push	ecx\n");
		fprintf(fp, "		push	es\n");
		fprintf(fp, "		mov	di, ds\n");
		fprintf(fp, "		mov	es, di\n");

		fprintf(fp, "		mov	esi, _%scontextBegin\n", cpubasename);
		fprintf(fp, "		mov	edi, eax	; Source address in ESI\n");

		fprintf(fp, "		mov     ecx, (_%scontextEnd - _%scontextBegin) >> 2\n", cpubasename, cpubasename);
		fprintf(fp, "		rep	movsd\n");
		fprintf(fp, "		mov     ecx, (_%scontextEnd - _%scontextBegin) & 0x03\n", cpubasename, cpubasename);
		fprintf(fp, "		rep	movsb\n");

		fprintf(fp, "		pop	es\n");
		fprintf(fp, "		pop	ecx\n");
		fprintf(fp, "		pop	edi\n");
		fprintf(fp, "		pop	esi\n");
		fprintf(fp, "		ret			; No return code\n");
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "/* Get mz80's context */\n\n");
		fprintf(fp, "void %sGetContext(void *pData)\n", cpubasename);
		fprintf(fp, "{\n");
		fprintf(fp, "	memcpy(pData, &cpu, sizeof(CONTEXTMZ80));\n");
		fprintf(fp, "}\n\n");
	}
	else
	{
		abort();
	}
}

void GetContextSizeCode(void)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp, "		global	_%sGetContextSize\n", cpubasename);
		fprintf(fp, "		global	%sGetContextSize_\n", cpubasename);

		if (bPlain)
			fprintf(fp, "		global	%sGetContextSize\n", cpubasename);

		sprintf(procname, "%sGetContextSize_", cpubasename);
		ProcBegin(0xffffffff);

		fprintf(fp, "_%sGetContextSize:\n", cpubasename);
		
		if (bPlain)
			fprintf(fp, "%sGetContextSize:\n", cpubasename);

		fprintf(fp, "		mov     eax, _%scontextEnd - _%scontextBegin\n", cpubasename, cpubasename);
		fprintf(fp, "		ret\n\n");
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "/* Get mz80's context size */\n\n");
		fprintf(fp, "UINT32 %sGetContextSize(void)\n", cpubasename);
		fprintf(fp, "{\n");
		fprintf(fp, "	return(sizeof(CONTEXTMZ80));\n");
		fprintf(fp, "}\n\n");
	}
	else
	{
		abort();
	}
}

void InitCode(void)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp, "		global	_%sinit\n", cpubasename);
		fprintf(fp, "		global	%sinit_\n", cpubasename);

		if (bPlain)
			fprintf(fp, "		global	%sinit\n", cpubasename);

		sprintf(procname, "%sinit_", cpubasename);
		ProcBegin(0xffffffff);

		fprintf(fp, "_%sinit:\n", cpubasename);
		
		if (bPlain)
			fprintf(fp, "%sinit:\n", cpubasename);

		fprintf(fp, "		ret\n\n");
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "/* Initialize MZ80 for action */\n\n");
		fprintf(fp, "void %sinit(void)\n", cpubasename);
		fprintf(fp, "{\n");

		fprintf(fp, "	UINT32 dwLoop;\n");
		fprintf(fp, "	UINT8 *pbTempPtr;\n");
		fprintf(fp, "	UINT8 *pbTempPtr2;\n");
		fprintf(fp, "	UINT8 bNewAdd;\n");
		fprintf(fp, "	UINT8 bNewSub;\n");
		fprintf(fp, "	UINT8 bFlag;\n");
		fprintf(fp, "	UINT8 bLow;\n");
		fprintf(fp, "	UINT8 bHigh;\n");
		fprintf(fp, "	UINT8 bCarry;\n");
		fprintf(fp, "\n");
		fprintf(fp, "	if (NULL == pbAddAdcTable)\n");
		fprintf(fp, "	{\n");
		fprintf(fp, "           static UINT8 paat[256 * 256 * 2];");
		fprintf(fp, "           static UINT8 psst[256 * 256 * 2];");
		fprintf(fp, "\n");
		fprintf(fp, "		pbAddAdcTable = paat;\n");
		fprintf(fp, "\n");
		fprintf(fp, "		pbTempPtr = pbAddAdcTable;\n\n");
		fprintf(fp, "		pbSubSbcTable = psst;\n");
		fprintf(fp, "\n");
		fprintf(fp, "		pbTempPtr2 = pbSubSbcTable;\n");
		fprintf(fp, "\n");
		fprintf(fp, "		for (dwLoop = 0; dwLoop < (256*256*2); dwLoop++)\n");
		fprintf(fp, "		{\n");
		fprintf(fp, "			bLow = dwLoop & 0xff;\n");
		fprintf(fp, "			bHigh = (dwLoop >> 8) & 0xff;\n");
		fprintf(fp, "			bCarry = (dwLoop >> 16);\n");
		fprintf(fp, "\n");
		fprintf(fp, "			bFlag = 0;\n");
		fprintf(fp, "			bNewAdd = bHigh + bLow + bCarry;\n");
		fprintf(fp, "\n");
		fprintf(fp, "			if (0 == bNewAdd)\n");
		fprintf(fp, "			{\n");
		fprintf(fp, "				bFlag |= Z80_FLAG_ZERO;\n");
		fprintf(fp, "			}\n");
		fprintf(fp, "			else\n");
		fprintf(fp, "			{\n");
		fprintf(fp, "				bFlag = bNewAdd & 0x80; /* Sign flag */\n");
		fprintf(fp, "			}\n");
		fprintf(fp, "\n");
		fprintf(fp, "			if (((UINT32) bLow + (UINT32) bHigh + (UINT32) bCarry) >= 0x100)\n");
		fprintf(fp, "			{\n");
		fprintf(fp, "				bFlag |= Z80_FLAG_CARRY;\n");
		fprintf(fp, "			}\n");
		fprintf(fp, "\n");
		fprintf(fp, "			if ( ((bLow ^ bHigh ^ 0x80) & (bLow ^ (bNewAdd & 0x80))) & 0x80)\n");
		fprintf(fp, "			{\n");
		fprintf(fp, "				bFlag |= Z80_FLAG_OVERFLOW_PARITY;\n");
		fprintf(fp, "			}\n");
		fprintf(fp, "\n");
		fprintf(fp, "			if (((bLow & 0x0f) + (bHigh & 0x0f) + bCarry) >= 0x10)\n");
		fprintf(fp, "			{\n");
		fprintf(fp, "				bFlag |= Z80_FLAG_HALF_CARRY;\n");
		fprintf(fp, "			}\n");
		fprintf(fp, "\n");
		fprintf(fp, "			*pbTempPtr++ = bFlag;	/* Store our new flag */\n\n");

		fprintf(fp, "			// Now do subtract - Zero\n");
		fprintf(fp, "\n");
		fprintf(fp, "			bFlag = Z80_FLAG_NEGATIVE;\n");
		fprintf(fp, "			bNewSub = bHigh - bLow - bCarry;\n");
		fprintf(fp, "\n");
		fprintf(fp, "			if (0 == bNewSub)\n");
		fprintf(fp, "			{\n");
		fprintf(fp, "				bFlag |= Z80_FLAG_ZERO;\n");
		fprintf(fp, "			}\n");
		fprintf(fp, "			else\n");
		fprintf(fp, "			{\n");
		fprintf(fp, "				bFlag |= bNewSub & 0x80; /* Sign flag */\n");
		fprintf(fp, "			}\n");
		fprintf(fp, "\n");
		fprintf(fp, "			if ( ((INT32) bHigh - (INT32) bLow - (INT32) bCarry) < 0)\n");
		fprintf(fp, "			{\n");
		fprintf(fp, "				bFlag |= Z80_FLAG_CARRY;\n");
		fprintf(fp, "			}\n");
		fprintf(fp, "\n");
		fprintf(fp, "			if ( ((INT32) (bHigh & 0xf) - (INT32) (bLow & 0x0f) - (INT32) bCarry) < 0)\n");
		fprintf(fp, "			{\n");
		fprintf(fp, "				bFlag |= Z80_FLAG_HALF_CARRY;\n");
		fprintf(fp, "			}\n");
		fprintf(fp, "\n");
		fprintf(fp, "			if ( ((bLow ^ bHigh) & (bHigh ^ bNewSub) & 0x80) )\n");
		fprintf(fp, "			{\n");
		fprintf(fp, "				bFlag |= Z80_FLAG_OVERFLOW_PARITY;\n");
		fprintf(fp, "			}\n");
		fprintf(fp, "\n");
		fprintf(fp, "			*pbTempPtr2++ = bFlag;	/* Store our sub flag */\n");
		fprintf(fp, "\n");
		fprintf(fp, "		}\n");
		fprintf(fp, "	}\n");
		fprintf(fp, "}\n");
	}
	else
	{
		abort();
	}
}

void ShutdownCode(void)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		fprintf(fp, "		global	_%sshutdown\n", cpubasename);
		fprintf(fp, "		global	%sshutdown_\n", cpubasename);

		if (bPlain)
			fprintf(fp, "		global	%sshutdown\n", cpubasename);

		sprintf(procname, "%sshutdown_", cpubasename);
		ProcBegin(0xffffffff);

		fprintf(fp, "_%sshutdown:\n", cpubasename);
		
		if (bPlain)
			fprintf(fp, "%sshutdown:\n", cpubasename);

		fprintf(fp, "		ret\n\n");
	}
	else
	if (MZ80_C == bWhat)
	{
		fprintf(fp, "/* Shut down MZ80 */\n\n");
		fprintf(fp, "void %sshutdown(void)\n", cpubasename);
		fprintf(fp, "{\n");

		fprintf(fp, "}\n\n");
	}
	else
	{
		abort();
	}
}

void DebuggerCode(void)
{
	if (MZ80_ASSEMBLY_X86 == bWhat)
	{
		Alignment();

		fprintf(fp, ";\n");
		fprintf(fp, "; In : EAX=Reg #, ESI=Context address\n");
		fprintf(fp, "; Out: EAX=Value of register\n");
		fprintf(fp, ";\n");

		fprintf(fp, "getRegValueInternal:\n");

		fprintf(fp, "		push	ecx\n");
		fprintf(fp, "		push	edx\n\n");

		fprintf(fp, "		cmp	eax, CPUREG_MAXINDEX\n");
		fprintf(fp, "		jae	badIndex2\n\n");

		fprintf(fp, "		shl	eax, 4	; Times 16 for table entry size\n");
		fprintf(fp, "		add	eax, RegTable	; Now it's the memory location\n");

		fprintf(fp, "		mov	edx, [eax+4]	; Get the offset of the register\n");
		fprintf(fp, "		mov	edx, [edx + esi]	; Get our value\n");

		fprintf(fp, "		mov	ecx, [eax+8]	; Get our shift value\n");
		fprintf(fp, "		shr	edx, cl			; Shift it right by a value\n");

		fprintf(fp, "		and	edx, [eax+12]	; Mask off any unneeded bits\n");
		fprintf(fp, "		mov	eax, edx			; Put our value in EAX\n");
		fprintf(fp, "		jmp	short indexExit	; Index's exit!\n");

		fprintf(fp, "badIndex2:\n");
		fprintf(fp, "		mov	eax, 0ffffffffh\n\n");
		fprintf(fp, "indexExit:\n");
		fprintf(fp, "		pop	edx\n");
		fprintf(fp, "		pop	ecx\n");
		fprintf(fp, "		ret\n\n");

		Alignment();

		fprintf(fp, ";\n");
		fprintf(fp, "; In : EAX=Value, EDX=Reg #, ESI=Context address\n");
		fprintf(fp, "; Out: EAX=Value of register\n");
		fprintf(fp, ";\n");

		fprintf(fp, "convertValueToText:\n");

		fprintf(fp, "		push	ecx\n");
		fprintf(fp, "		push	edx\n\n");

		fprintf(fp, "		cmp	edx, CPUREG_MAXINDEX\n");
		fprintf(fp, "		jae	badIndex3\n\n");

		fprintf(fp, "		shl	edx, 4	; Times 16 for table entry size\n");
		fprintf(fp, "		add	edx, RegTable	; Now it's the memory location\n");
		fprintf(fp, "		mov	edx, [edx + 12] ; Shift mask\n");
		fprintf(fp, "		xor	ecx, ecx	; Zero our shift\n");

		fprintf(fp, "shiftLoop:\n");
		fprintf(fp, "		test	edx, 0f0000000h	; High nibble nonzero yet?\n");
		fprintf(fp, "		jnz	convertLoop		; Yup!\n");
		fprintf(fp, "		shl	edx, 4			; Move over, bacon\n");
		fprintf(fp, "		shl	eax, 4		; Move the value over, too\n");
		fprintf(fp, "		jmp	short shiftLoop	; Keep shiftin'\n\n");

		fprintf(fp, "convertLoop:\n");
		fprintf(fp, "		mov	ecx, eax			; Get our value\n");
		fprintf(fp, "		shr	ecx, 28			; Only the top nibble\n");
		fprintf(fp, "		add	cl, '0'			; Convert to ASCII\n");
		fprintf(fp, "		cmp	cl, '9'			; Greater than 9?\n");
		fprintf(fp, "		jbe	noAdd				; Nope! Don't add it\n");
		fprintf(fp, "		add	cl, 32+7			; Convert from lowercase a-f\n");
		fprintf(fp, "noAdd:\n");
		fprintf(fp, "		mov	[edi], cl		; New value storage\n");
		fprintf(fp, "		inc	edi			; Next byte, please\n");
		fprintf(fp, "		shl	eax, 4			; Move the mask over\n");
		fprintf(fp, "		shl	edx, 4			; Move the mask over\n");
		fprintf(fp, "		jnz	convertLoop		; Keep convertin'\n\n");


		fprintf(fp, "badIndex3:\n");
		fprintf(fp, "		mov	[edi], byte 0	; Null terminate the sucker!\n");
		fprintf(fp, "		pop	edx\n");
		fprintf(fp, "		pop	ecx\n");
		fprintf(fp, "		ret\n\n");

		fprintf(fp, "		global	_%sSetRegisterValue\n", cpubasename);
		fprintf(fp, "		global	%sSetRegisterValue_\n", cpubasename);

		if (bPlain)
			fprintf(fp, "		global	%sSetRegisterValue\n", cpubasename);

		sprintf(procname, "%sSetRegisterValue_", cpubasename);
		ProcBegin(0xffffffff);

		fprintf(fp, "_%sSetRegisterValue:\n", cpubasename);
		if (bPlain)
			fprintf(fp, "%sSetRegisterValue:\n", cpubasename);

		fprintf(fp, "		push	esi\n");
		fprintf(fp, "		push	edi\n");
		fprintf(fp, "		push	edx\n");
		fprintf(fp, "		push	ecx\n");

		if (bUseStack)
		{
			fprintf(fp, "		mov	eax, [esp+20]	; Get our register #\n");
			fprintf(fp, "		mov	esi, [esp+24]	; Get our context address\n");
			fprintf(fp, "		mov	edi, [esp+28]	; Value to assign\n");
		}
		else
		{
			fprintf(fp, "		mov	esi, eax	; Get context\n");
			fprintf(fp, "		mov	eax, edx	; Get register # in EAX\n");
			fprintf(fp, "		mov	edi, ebx	; Get value to assign\n");
		}

		fprintf(fp, "		or	esi, esi	; Are we NULL?\n");
		fprintf(fp, "		jnz	userDefined\n");

		fprintf(fp, "		mov	esi, _%scontextBegin\n", cpubasename);
		fprintf(fp, "userDefined:\n\n");
		fprintf(fp, "		shl	eax, 4	; Times 16 for reg entry size\n");
		fprintf(fp, "		add	eax, RegTable\n");
		fprintf(fp, "		mov	edx, [eax+12] ; Our mask\n");
		fprintf(fp, "		not	edx	; Invert EDX!\n");
		fprintf(fp, "		test	edi, edx	; Did we set any invalid bits?\n");
		fprintf(fp, "		jnz	rangeViolation\n\n");

		fprintf(fp, "		not	edx	; Toggle it back to normal\n");
		fprintf(fp, "		mov	ecx, [eax+8]	; Get our shift value\n");
		fprintf(fp, "		shl	edx, cl	; Shift our mask\n");
		fprintf(fp, "		shl	eax, cl	; And our value to OR in\n");
	
		fprintf(fp, "		not	edx	; Make it the inverse of what we want\n");
		fprintf(fp, "		mov	eax, [eax+4]	; Get our offset into the context\n");
		fprintf(fp, "		and	[esi+eax], edx	; Mask off the bits we're changin\n");
		fprintf(fp, "		or	[esi+eax], edi	; Or in our new value\n\n");
		fprintf(fp, "		xor	eax, eax\n");
		fprintf(fp, "		jmp	short  setExit\n\n");

		fprintf(fp, "rangeViolation:\n");
		fprintf(fp, "		mov	eax, 0ffffffffh\n\n");

		fprintf(fp, "setExit:\n");
		fprintf(fp, "		pop	ecx\n");
		fprintf(fp, "		pop	edx\n");
		fprintf(fp, "		pop	edi\n");
		fprintf(fp, "		pop	esi\n\n");

		fprintf(fp, "		ret\n\n");

		Alignment();

		fprintf(fp, "		global	_%sGetRegisterValue\n", cpubasename);
		fprintf(fp, "		global	%sGetRegisterValue_\n", cpubasename);

		if (bPlain)
			fprintf(fp, "		global	%sGetRegisterValue\n", cpubasename);

		sprintf(procname, "%sGetRegisterValue_", cpubasename);
		ProcBegin(0xffffffff);

		fprintf(fp, "_%sGetRegisterValue:\n", cpubasename);
		if (bPlain)
			fprintf(fp, "%sGetRegisterValue:\n", cpubasename);

		fprintf(fp, "		push	esi\n");

		if (bUseStack)
		{
			fprintf(fp, "		mov	eax, [esp+8]	; Get our register #\n");
			fprintf(fp, "		mov	esi, [esp+12]	; Get our context address\n");
		}
		else
		{
			fprintf(fp, "		mov	esi, eax	; Get context\n");
			fprintf(fp, "		mov	eax, edx	; Get register # in EAX\n");
		}

		fprintf(fp, "		or	esi, esi	; Is context NULL?\n");
		fprintf(fp, "		jnz	getVal	; Nope - use it!\n");
		fprintf(fp, "		mov	esi, _%scontextBegin\n\n", cpubasename);

		fprintf(fp, "getVal:\n");
		fprintf(fp, "		call	getRegValueInternal\n\n");

		fprintf(fp, "		pop	esi\n");

		fprintf(fp, "		ret\n\n");

		Alignment();

		fprintf(fp, "		global	_%sGetRegisterName\n", cpubasename);
		fprintf(fp, "		global	%sGetRegisterName_\n", cpubasename);

		if (bPlain)
			fprintf(fp, "		global	%sGetRegisterName\n", cpubasename);

		sprintf(procname, "%sGetRegisterName_", cpubasename);
		ProcBegin(0xffffffff);

		fprintf(fp, "_%sGetRegisterName:\n", cpubasename);
		if (bPlain)
			fprintf(fp, "%sGetRegisterName:\n", cpubasename);

		if (bUseStack)
		{
			fprintf(fp, "		mov	eax, [esp+4]	; Get our register #\n");
		}

		fprintf(fp, "		cmp	eax, CPUREG_MAXINDEX\n");
		fprintf(fp, "		jae	badIndex\n");

		fprintf(fp, "		shl	eax, 4	; Times 16 bytes for each entry\n");
		fprintf(fp, "		mov	eax, [eax+RegTable]\n");
		fprintf(fp, "		jmp	nameExit\n\n");

		fprintf(fp, "badIndex:\n");
		fprintf(fp, "		xor	eax, eax\n\n");

		fprintf(fp, "nameExit:\n");
		fprintf(fp, "		ret\n\n");

		Alignment();

		fprintf(fp, "		global	_%sGetRegisterTextValue\n", cpubasename);
		fprintf(fp, "		global	%sGetRegisterTextValue_\n", cpubasename);

		if (bPlain)
			fprintf(fp, "		global	%sGetRegisterTextValue\n", cpubasename);

		sprintf(procname, "%sGetRegisterTextValue_", cpubasename);
		ProcBegin(0xffffffff);

		fprintf(fp, "_%sGetRegisterTextValue:\n", cpubasename);
		if (bPlain)
			fprintf(fp, "%sGetRegisterTextValue:\n", cpubasename);

		fprintf(fp, "		push	esi\n");
		fprintf(fp, "		push	edi\n");
		fprintf(fp, "		push	edx\n");

		if (bUseStack)
		{
			fprintf(fp, "		mov	eax, [esp+16]	; Get our register #\n");
			fprintf(fp, "		mov	esi, [esp+20]	; Get our context address\n");
			fprintf(fp, "		mov	edi, [esp+24]	; Address to place text\n");
		}
		else
		{
			fprintf(fp, "		mov	esi, eax	; Get context\n");
			fprintf(fp, "		mov	eax, edx	; Get register # in EAX\n");
			fprintf(fp, "		mov	edi, ebx	; Address to place text\n");
		}

		fprintf(fp, "		or	esi, esi	; Is context NULL?\n");
		fprintf(fp, "		jnz	getVal2	; Nope - use it!\n");
		fprintf(fp, "		mov	esi, _%scontextBegin\n\n", cpubasename);

		fprintf(fp, "getVal2:\n");
		fprintf(fp, "		mov	edx, eax	; Save off our index for later\n");
		fprintf(fp, "		call	getRegValueInternal\n\n");

		fprintf(fp, "; EAX Holds the value, EDX=Register #, and EDI=Destination!\n\n");

		fprintf(fp, "		call	convertValueToText\n\n");

		fprintf(fp, "		pop	edx\n");
		fprintf(fp, "		pop	esi\n");
		fprintf(fp, "		pop	edi\n");

		fprintf(fp, "		ret\n\n");

		Alignment();

		fprintf(fp, "		global	_%sWriteValue\n", cpubasename);
		fprintf(fp, "		global	%sWriteValue_\n", cpubasename);

		if (bPlain)
			fprintf(fp, "		global	%sWriteValue\n", cpubasename);

		sprintf(procname, "%sWriteValue_", cpubasename);
		ProcBegin(0xffffffff);

		fprintf(fp, "_%sWriteValue:\n", cpubasename);
		if (bPlain)
			fprintf(fp, "%sWriteValue:\n", cpubasename);

		fprintf(fp, "		push	esi\n");
		fprintf(fp, "		push	edi\n");
		fprintf(fp, "		push	edx\n");
		fprintf(fp, "		push	ebx\n");
		fprintf(fp, "		push	ecx\n");
		fprintf(fp, "		push	ebp\n");

		if (bUseStack)
		{
			fprintf(fp, "		mov	eax, [esp+28]	; What kind of write is this?\n");
			fprintf(fp, "		mov	ebx, [esp+32]	; Address\n");
			fprintf(fp, "		mov	edx, [esp+36]	; Value\n");
		}
		else
		{
			fprintf(fp, "		xchg	edx, ebx	; Addr=EBX, value=EDX\n");
		}

		fprintf(fp, "		cmp	eax, 1	; Is it a word write?\n");
		fprintf(fp, "		je	near invalidWrite	; Yep - it's not valid\n");
		fprintf(fp, "		cmp	eax, 2	; Is it a dword write?\n");
		fprintf(fp, "		je	near invalidWrite	; Yep - it's not valid\n\n");
		fprintf(fp, "		or	eax, eax	; Is it a byte write?\n");
		fprintf(fp, "		jnz	itsIoDummy	; Nope... it's an I/O write\n\n");

		// Here we do a write memory byte

		fprintf(fp, "		mov	ebp, [_z80Base] ; Base pointer comes back\n");
		fprintf(fp, "		mov	edi, [_z80MemWrite]	; Point to the write array\n");

		fprintf(fp, "checkLoop:\n");
		fprintf(fp, "		cmp	[edi], word 0ffffh ; End of our list?\n");
		fprintf(fp, "		je	memoryWrite	; Yes - go write it!\n");
		fprintf(fp, "		cmp	bx, [edi]	; Are we smaller?\n");
		fprintf(fp, "		jb	nextAddr	; Yes... go to the next addr\n");
		fprintf(fp, "		cmp	bx, [edi+4]	; Are we smaller?\n");
		fprintf(fp, "		jbe	callRoutine	; If not, go call it!\n");

		fprintf(fp, "nextAddr:\n");
		fprintf(fp, "		add	edi, 10h		; Next structure, please\n");
		fprintf(fp, "		jmp	short checkLoop\n");

		fprintf(fp, "callRoutine:\n");

		fprintf(fp, "\n;\n; EBX=Address to target, DL=Byte to write \n;\n\n");

		fprintf(fp, "		cmp	[edi+8], dword 0	; Null handler?\n");
		fprintf(fp, "		je	directWriteHandler2\n\n");
		
		if (FALSE == bUseStack)
		{
			fprintf(fp, "		mov	eax, ebx	; Address\n");
			fprintf(fp, "		mov	ebx, edi	; Pointer to struct (EDX Already has the byte to write)\n");
		}
		else
		{
			fprintf(fp, "		push	edi	; Handler\n");
			fprintf(fp, "		push	edx	; Byte\n");
			fprintf(fp, "		push	ebx	; Address\n");
		}
	
		fprintf(fp, "		call	dword [edi + 8]	; Go call our handler\n");

		if (bUseStack)
		{
			fprintf(fp, "		add	esp, 12\n");
		}

		fprintf(fp, "		jmp	short itsGood\n");

		fprintf(fp, "directWriteHandler2:\n");
		fprintf(fp, "		sub	ebx, [edi]	; Subtract our offset\n");
		fprintf(fp, "		add	ebx, [edi+12]	; Add in the base address\n");
		fprintf(fp, "		mov	[ebx], dl	; Store our byte\n");
		fprintf(fp, "		jmp	short itsGood\n");
		fprintf(fp, "memoryWrite:\n");
		fprintf(fp, "		mov	[ebp + ebx], dl\n\n");
		fprintf(fp, "		jmp	short itsGood\n");

		// Here we do an "out"

		fprintf(fp, "itsIoDummy:\n");

		fprintf(fp, "		mov	edi, [_z80IoWrite]	; Point to the I/O write array\n");

		fprintf(fp, "IOCheck:\n");
		fprintf(fp, "		cmp	[edi], word 0ffffh ; End of our list?\n");
		fprintf(fp, "		je	itsGood	; Yes - ignore it!\n");
		fprintf(fp, "		cmp	bx, [edi]	; Are we smaller?\n");
		fprintf(fp, "		jb	nextIOAddr	; Yes... go to the next addr\n");
		fprintf(fp, "		cmp	bx, [edi+2]	; Are we bigger?\n");
		fprintf(fp, "		jbe	callIOHandler	; If not, go call it!\n");

		fprintf(fp, "nextIOAddr:\n");
		fprintf(fp, "		add	edi, 0ch		; Next structure, please\n");
		fprintf(fp, "		jmp	short IOCheck\n");

		fprintf(fp, "callIOHandler:\n");

		if (FALSE == bUseStack)
		{
			fprintf(fp, "		mov	eax, ebx	; Address\n");
			fprintf(fp, "		mov	ebx, edi	; Pointer to struct (EDX Already has the byte to write)\n");
		}
		else
		{
			fprintf(fp, "		push	edi	; Handler\n");
			fprintf(fp, "		push	edx	; Byte\n");
			fprintf(fp, "		push	ebx	; Address\n");
		}

		fprintf(fp, "		call	dword [edi+4]	; Call the handler!\n");

		if (bUseStack)
			fprintf(fp, "		add	esp, 12\n");

		fprintf(fp, "		jmp	short itsGood\n\n");

		// Errors and whatnot

		fprintf(fp, "invalidWrite:\n");
		fprintf(fp, "		mov	eax, 0ffffffffh\n");
		fprintf(fp, "		jmp	short writeValueExit\n\n");

		fprintf(fp, "itsGood:\n");
		fprintf(fp, "		xor	eax, eax\n\n");

		fprintf(fp, "writeValueExit:\n");

		fprintf(fp, "		pop	ebp\n");
		fprintf(fp, "		pop	ecx\n");
		fprintf(fp, "		pop	ebx\n");
		fprintf(fp, "		pop	edx\n");
		fprintf(fp, "		pop	esi\n");
		fprintf(fp, "		pop	edi\n");

		fprintf(fp, "		ret\n\n");

		Alignment();

		fprintf(fp, "		global	_%sReadValue\n", cpubasename);
		fprintf(fp, "		global	%sReadValue_\n", cpubasename);

		if (bPlain)
			fprintf(fp, "		global	%sReadValue\n", cpubasename);

		sprintf(procname, "%sReadValue_", cpubasename);
		ProcBegin(0xffffffff);

		fprintf(fp, "_%sReadValue:\n", cpubasename);
		if (bPlain)
			fprintf(fp, "%sReadValue:\n", cpubasename);

		fprintf(fp, "		push	esi\n");
		fprintf(fp, "		push	edi\n");
		fprintf(fp, "		push	edx\n");
		fprintf(fp, "		push	ebx\n");
		fprintf(fp, "		push	ecx\n");
		fprintf(fp, "		push	ebp\n");

		if (bUseStack)
		{
			fprintf(fp, "		mov	eax, [esp+28]	; What kind of read is this?\n");
			fprintf(fp, "		mov	ebx, [esp+32]	; Address\n");
		}
		else
		{
			fprintf(fp, "		xchg	edx, ebx	; Addr=EBX\n");
		}

		fprintf(fp, "		cmp	eax, 1	; Is it a word read?\n");
		fprintf(fp, "		je	near invalidRead	; Yep - it's not valid\n");
		fprintf(fp, "		cmp	eax, 2	; Is it a dword read?\n");
		fprintf(fp, "		je	near invalidRead	; Yep - it's not valid\n\n");
		fprintf(fp, "		or	eax, eax	; Is it a byte read?\n");
		fprintf(fp, "		jnz	itsIoDummyRead	; Nope... it's an I/O read\n\n");

		// Here we do a read memory byte

		fprintf(fp, "		mov	ebp, [_z80Base] ; Base pointer comes back\n");
		fprintf(fp, "		mov	edi, [_z80MemRead]	; Point to the read array\n");

		fprintf(fp, "checkLoopRead:\n");
		fprintf(fp, "		cmp	[edi], word 0ffffh ; End of our list?\n");
		fprintf(fp, "		je	memoryRead	; Yes - go read it!\n");
		fprintf(fp, "		cmp	bx, [edi]	; Are we smaller?\n");
		fprintf(fp, "		jb	nextAddrRead	; Yes... go to the next addr\n");
		fprintf(fp, "		cmp	bx, [edi+4]	; Are we smaller?\n");
		fprintf(fp, "		jbe	callRoutineRead	; If not, go call it!\n");

		fprintf(fp, "nextAddrRead:\n");
		fprintf(fp, "		add	edi, 10h		; Next structure, please\n");
		fprintf(fp, "		jmp	short checkLoopRead\n");

		fprintf(fp, "callRoutineRead:\n");

		fprintf(fp, "\n;\n; EBX=Address to target\n;\n\n");

		fprintf(fp, "		cmp	[edi+8], dword 0 ; NULL HAndler?\n");
		fprintf(fp, "		je	handleSharedRead\n\n");

		if (FALSE == bUseStack)
		{
			fprintf(fp, "		mov	eax, ebx	; Address\n");
			fprintf(fp, "		mov	edx, edi	; Pointer to struct\n");
		}
		else
		{
			fprintf(fp, "		push	edi	; Handler\n");
			fprintf(fp, "		push	ebx	; Address\n");
		}
	
		fprintf(fp, "		call	dword [edi + 8]	; Go call our handler\n");
		fprintf(fp, "		mov	dl, al	; Get our byte read\n");

		if (bUseStack)
		{
			fprintf(fp, "		add	esp, 8\n");
		}

		fprintf(fp, "		jmp	short itsGoodRead\n\n");

		fprintf(fp, "memoryRead:\n");
		fprintf(fp, "		mov	dl, [ebp+ebx]\n\n");
		fprintf(fp, "		jmp	short itsGoodRead\n\n");

		fprintf(fp, "handleSharedRead:\n");
		fprintf(fp, "		sub	ebx, [edi]\n");
		fprintf(fp, "		add	ebx, [edi+12]\n");
		fprintf(fp, "		mov	dl, [ebx]\n");
		fprintf(fp, "		jmp	short itsGoodRead\n\n");

		// Here we do an "out"

		fprintf(fp, "itsIoDummyRead:\n");

		fprintf(fp, "		mov	edi, [_z80IoRead]	; Point to the I/O read array\n");
		fprintf(fp, "		mov	dl, 0ffh	; Assume no handler\n");

		fprintf(fp, "IOCheckRead:\n");
		fprintf(fp, "		cmp	[edi], word 0ffffh ; End of our list?\n");
		fprintf(fp, "		je	itsGoodRead	; Yes - ignore it!\n");
		fprintf(fp, "		cmp	bx, [edi]	; Are we smaller?\n");
		fprintf(fp, "		jb	nextIOAddrRead	; Yes... go to the next addr\n");
		fprintf(fp, "		cmp	bx, [edi+2]	; Are we bigger?\n");
		fprintf(fp, "		jbe	callIOHandlerRead	; If not, go call it!\n");

		fprintf(fp, "nextIOAddrRead:\n");
		fprintf(fp, "		add	edi, 0ch		; Next structure, please\n");
		fprintf(fp, "		jmp	short IOCheckRead\n");

		fprintf(fp, "callIOHandlerRead:\n");

		if (FALSE == bUseStack)
		{
			fprintf(fp, "		mov	eax, ebx	; Address\n");
			fprintf(fp, "		mov	edx, edi	; Pointer to struct (EDX Already has the byte to write)\n");
		}
		else
		{
			fprintf(fp, "		push	edi	; Handler\n");
			fprintf(fp, "		push	ebx	; Address\n");
		}

		fprintf(fp, "		call	dword [edi+4]	; Call the handler!\n");
		fprintf(fp, "		mov	dl, al	; Get our byte read\n");

		if (bUseStack)
			fprintf(fp, "		add	esp, 8\n");

		fprintf(fp, "		jmp	short itsGoodRead\n\n");

		// Errors and whatnot

		fprintf(fp, "invalidRead:\n");
		fprintf(fp, "		mov	eax, 0ffffffffh\n");
		fprintf(fp, "		jmp	short ReadValueExit\n\n");

		fprintf(fp, "itsGoodRead:\n");
		fprintf(fp, "		xor	eax, eax\n");
		fprintf(fp, "		mov	al, dl\n\n");

		fprintf(fp, "ReadValueExit:\n");

		fprintf(fp, "		pop	ebp\n");
		fprintf(fp, "		pop	ecx\n");
		fprintf(fp, "		pop	ebx\n");
		fprintf(fp, "		pop	edx\n");
		fprintf(fp, "		pop	esi\n");
		fprintf(fp, "		pop	edi\n");

		fprintf(fp, "		ret\n\n");



	}
	else
	if (MZ80_C == bWhat)
	{
	}
}


void EmitCode(void)
{
	CodeSegmentBegin();
	EmitCBInstructions();
	EmitEDInstructions();

	if (MZ80_ASSEMBLY_X86 == bWhat)
		strcpy(mz80Index, "ix");
		
	else
	{
		strcpy(mz80Index, "cpu.z80IX");
		strcpy(mz80IndexHalfHigh, "cpu.z80XH");
		strcpy(mz80IndexHalfLow, "cpu.z80XL");
	}

	strcpy(majorOp, "DD");
	EmitDDInstructions();

	if (MZ80_ASSEMBLY_X86 == bWhat)
		strcpy(mz80Index, "iy");
	else
	{
		strcpy(mz80Index, "cpu.z80IY");
		strcpy(mz80IndexHalfHigh, "cpu.z80YH");
		strcpy(mz80IndexHalfLow, "cpu.z80YL");
	}

	strcpy(majorOp, "FD");
	EmitFDInstructions();
	majorOp[0] = '\0';
	EmitRegularInstructions();
	ReadMemoryByteHandler();
	WriteMemoryByteHandler();

	if (bThroughCallHandler)
	{
		PushWordHandler();
		PopWordHandler();
	}

	ReadIoHandler();
	WriteIoHandler();
	GetContextCode();
	SetContextCode();
	GetContextSizeCode();
	GetTicksCode();
	ReleaseTimesliceCode();
	ResetCode();
	IntCode();
	NmiCode();
	ExecCode();
	InitCode();
	ShutdownCode();
	DebuggerCode();
	CodeSegmentEnd();
}

int main(int argc, char **argv)
{
	int dwLoop = 0;

	printf("MakeZ80 - V%s - Copyright 1996-2000 Neil Bradley (neil@synthcom.com)\n", MZ80_VERSION);

	if (argc < 2)
	{
		printf("Usage: %s outfile [option1] [option2] ....\n", argv[0]);
		printf("\n   -s   - Stack calling conventions (DJGPP, MSVC, Borland)\n");
		printf("   -x86 - Emit an assembly version of mz80\n");
		printf("   -c   - Emit a C version of mz80\n");
		printf("   -cs  - All stack operations go through handlers\n");
		printf("   -16  - Treat all I/O input and output as 16 bit (BC) instead of (C)\n");
		printf("   -l   - Create 'plain' labels - ones without leading or trailing _'s\n");
		printf("   -nt  - No timing additions occur\n");
		printf("   -os2 - Emit OS/2 compatible segmentation pragmas\n");
		exit(1);
	}

	dwLoop = 1;

	while (dwLoop < argc)
	{
		if (strcmp("-x86", argv[dwLoop]) == 0 || strcmp("-X86", argv[dwLoop]) == 0)
			bWhat = MZ80_ASSEMBLY_X86;
		if (strcmp("-c", argv[dwLoop]) == 0 || strcmp("-C", argv[dwLoop]) == 0)
			bWhat = MZ80_C;
		if (strcmp("-cs", argv[dwLoop]) == 0 || strcmp("-cs", argv[dwLoop]) == 0)
			bThroughCallHandler = TRUE;
		if (strcmp("-s", argv[dwLoop]) == 0 || strcmp("-S", argv[dwLoop]) == 0)
			bUseStack = 1;
		if (strcmp("-l", argv[dwLoop]) == 0 || strcmp("-L", argv[dwLoop]) == 0)
			bPlain = TRUE;
		if (strcmp("-16", argv[dwLoop]) == 0)
			b16BitIo = TRUE;
		if (strcmp("-os2", argv[dwLoop]) == 0 || strcmp("-OS2", argv[dwLoop]) == 0)
			bOS2 = TRUE;
		if (strcmp("-nt", argv[dwLoop]) == 0)
		{
			bNoTiming = TRUE;
		}

		dwLoop++;
	}

	if (bWhat == MZ80_UNKNOWN)
	{
		fprintf(stderr, "Need emitted type qualifier\n");
		exit(1);
	}

	for (dwLoop = 1; dwLoop < argc; dwLoop++)
		if (argv[dwLoop][0] != '-')
		{
			fp = fopen(argv[dwLoop], "w");
			break;
		}

	if (NULL == fp)
	{
		fprintf(stderr, "Can't open %s for writing\n", argv[1]);
		exit(1);
	}

	strcpy(cpubasename, "mz80");

	StandardHeader();
	DataSegment();
	EmitCode();
	ProgramEnd();

	fclose(fp);
}
