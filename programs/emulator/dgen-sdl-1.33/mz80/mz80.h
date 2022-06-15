/* Multi-Z80 32 Bit emulator */

/* Copyright 1996, Neil Bradley, All rights reserved
 *
 * License agreement:
 *
 * The mZ80 emulator may be distributed in unmodified form to any medium.
 *
 * mZ80 May not be sold, or sold as a part of a commercial package without
 * the express written permission of Neil Bradley (neil@synthcom.com). This
 * includes shareware.
 *
 * Modified versions of mZ80 may not be publicly redistributed without author
 * approval (neil@synthcom.com). This includes distributing via a publicly
 * accessible LAN. You may make your own source modifications and distribute
 * mZ80 in object only form.
 *
 * mZ80 Licensing for commercial applications is available. Please email
 * neil@synthcom.com for details.
 *
 * Synthcom Systems, Inc, and Neil Bradley will not be held responsible for
 * any damage done by the use of mZ80. It is purely "as-is".
 *
 * If you use mZ80 in a freeware application, credit in the following text:
 *
 * "Multi-Z80 CPU emulator by Neil Bradley (neil@synthcom.com)"
 *
 * must accompany the freeware application within the application itself or
 * in the documentation.
 *
 * Legal stuff aside:
 *
 * If you find problems with mZ80, please email the author so they can get
 * resolved. If you find a bug and fix it, please also email the author so
 * that those bug fixes can be propogated to the installed base of mZ80
 * users. If you find performance improvements or problems with mZ80, please
 * email the author with your changes/suggestions and they will be rolled in
 * with subsequent releases of mZ80.
 *
 * The whole idea of this emulator is to have the fastest available 32 bit
 * Multi-z80 emulator for the PC, giving maximum performance. 
 */ 

/* General z80 based defines */

#ifndef	_MZ80_H_
#define	_MZ80_H_

#include <stdint.h>

#ifndef UINT32
#define UINT32  uint32_t
#endif

#ifndef UINT16
#define UINT16  uint16_t
#endif

#ifndef UINT8
#define UINT8   uint8_t
#endif

#ifndef INT32
#define INT32  int32_t
#endif

#ifndef INT16
#define INT16  int16_t
#endif

#ifndef INT8
#define INT8   int8_t
#endif

#ifndef UINTPTR
#define UINTPTR uintptr_t
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _MEMORYREADWRITEBYTE_
#define _MEMORYREADWRITEBYTE_

struct MemoryWriteByte
{
	UINT32 lowAddr;
	UINT32 highAddr;
	void (*memoryCall)(UINT32, UINT8, struct MemoryWriteByte *);
	void *pUserArea;
};      

struct MemoryReadByte
{
	UINT32 lowAddr;
	UINT32 highAddr;
	UINT8 (*memoryCall)(UINT32, struct MemoryReadByte *);
	void *pUserArea;
};      

#endif // _MEMORYREADWRITEBYTE_

struct z80PortWrite
{
	UINT16 lowIoAddr;
	UINT16 highIoAddr;
	void (*IOCall)(UINT16, UINT8, struct z80PortWrite *);
	void *pUserArea;
};

struct z80PortRead
{
	UINT16 lowIoAddr;
	UINT16 highIoAddr;
	UINT16 (*IOCall)(UINT16, struct z80PortRead *);
	void *pUserArea;
};	

struct z80TrapRec
{
  	UINT16 trapAddr;
	UINT8  skipCnt;
	UINT8  origIns;
};

typedef union
{
	UINT32 af;

	struct
	{
#ifdef WORDS_BIGENDIAN
		UINT16 wFiller;
		UINT8 a;
		UINT8 f;
#else
		UINT8 f;
		UINT8 a;
		UINT16 wFiller;
#endif
	} half;
} reg_af;

#define	z80AF	z80af.af
#define	z80A	z80af.half.a
#define	z80F	z80af.half.f

typedef union
{
	UINT32 bc;

	struct
	{
#ifdef WORDS_BIGENDIAN
		UINT16 wFiller;
		UINT8 b;
		UINT8 c;
#else
		UINT8 c;
		UINT8 b;
		UINT16 wFiller;
#endif
	} half;
} reg_bc;

#define	z80BC	z80bc.bc
#define	z80B	z80bc.half.b
#define	z80C	z80bc.half.c

typedef union
{
	UINT32 de;

	struct
	{
#ifdef WORDS_BIGENDIAN
		UINT16 wFiller;
		UINT8 d;
		UINT8 e;
#else
		UINT8 e;
		UINT8 d;
		UINT16 wFiller;
#endif
	} half;
} reg_de;

#define	z80DE	z80de.de
#define	z80D	z80de.half.d
#define	z80E	z80de.half.e

typedef union
{
	UINT32 hl;

	struct
	{
#ifdef WORDS_BIGENDIAN
		UINT16 wFiller;
		UINT8 h;
		UINT8 l;
#else
		UINT8 l;
		UINT8 h;
		UINT16 wFiller;
#endif
	} half;
} reg_hl;

#define	z80HL	z80hl.hl
#define	z80H	z80hl.half.h
#define	z80L	z80hl.half.l

#define	z80SP	z80sp.sp

typedef union
{
	UINT32 ix;

	struct
	{
#ifdef WORDS_BIGENDIAN
		UINT16 wFiller;
		UINT8 xh;
		UINT8 xl;
#else
		UINT8 xl;
		UINT8 xh;
		UINT16 wFiller;
#endif
	} half;
} reg_ix;

#define	z80IX	z80ix.ix
#define	z80XH	z80ix.half.xh
#define	z80XL	z80ix.half.xl

typedef union
{
	UINT32 iy;

	struct
	{
#ifdef WORDS_BIGENDIAN
		UINT16 wFiller;
		UINT8 yh;
		UINT8 yl;
#else
		UINT8 yl;
		UINT8 yh;
		UINT16 wFiller;
#endif
	} half;
} reg_iy;

#define	z80IY	z80iy.iy
#define	z80YH	z80iy.half.yh
#define	z80YL	z80iy.half.yl

struct mz80context
{
	UINT8 *z80Base;
	struct MemoryReadByte *z80MemRead;
	struct MemoryWriteByte *z80MemWrite;
	struct z80PortRead *z80IoRead;
	struct z80PortWrite *z80IoWrite;
	UINT32 z80clockticks;
	UINT32 z80iff;
	UINT32 z80interruptMode;
	UINT32 z80halted;

	reg_af z80af;
	reg_bc z80bc;
	reg_de z80de;
	reg_hl z80hl;
	UINT32 z80afprime;
	UINT32 z80bcprime;
	UINT32 z80deprime;
	UINT32 z80hlprime;
	reg_ix z80ix;
	reg_iy z80iy;
	UINT32 z80sp;
	UINT32 z80pc;
	UINT32 z80nmiAddr;
	UINT32 z80intAddr;
	UINT32 z80rCounter;
	UINT8 z80i;
	UINT8 z80r;
	UINT8 z80intPending;
}; 

// These are the enumerations used for register access. DO NOT ALTER THEIR
// ORDER! It must match the same order as in the mz80.c/mz80.asm files!

enum
{
#ifndef CPUREG_PC
	CPUREG_PC = 0,
#endif
	CPUREG_Z80_AF = 1,
	CPUREG_Z80_BC,
	CPUREG_Z80_DE,
	CPUREG_Z80_HL,
	CPUREG_Z80_AFPRIME,
	CPUREG_Z80_BCPRIME,
	CPUREG_Z80_DEPRIME,
	CPUREG_Z80_HLPRIME,
	CPUREG_Z80_IX,
	CPUREG_Z80_IY,
	CPUREG_Z80_SP,
	CPUREG_Z80_I,
	CPUREG_Z80_R,
	CPUREG_Z80_A,
	CPUREG_Z80_B,
	CPUREG_Z80_C,
	CPUREG_Z80_D,
	CPUREG_Z80_E,
	CPUREG_Z80_H,
	CPUREG_Z80_L,
	CPUREG_Z80_F,
	CPUREG_Z80_CARRY,
	CPUREG_Z80_NEGATIVE,
	CPUREG_Z80_PARITY,
	CPUREG_Z80_OVERFLOW,
	CPUREG_Z80_HALFCARRY,
	CPUREG_Z80_ZERO,
	CPUREG_Z80_SIGN,
	CPUREG_Z80_IFF1,
	CPUREG_Z80_IFF2,

	// Leave this here!

	CPUREG_Z80_MAX_INDEX
};

extern UINT32 mz80exec(UINT32);
extern UINT32 mz80GetContextSize(void);
extern UINT32 mz80GetElapsedTicks(UINT32);
extern void mz80ReleaseTimeslice(void);
extern void mz80GetContext(void *);
extern void mz80SetContext(void *);
extern void mz80reset(void);
extern void mz80ClearPendingInterrupt(void);
extern UINT32 mz80int(UINT32);
extern UINT32 mz80nmi(void);
extern void mz80init(void);
extern UINT32 z80intAddr;
extern UINT32 z80nmiAddr;

// Debugger useful routines

extern UINT8 mz80SetRegisterValue(void *, UINT32, UINT32);
extern UINT32 mz80GetRegisterValue(void *, UINT32);
extern UINT32 mz80GetRegisterTextValue(void *, UINT32, UINT8 *);
extern UINT8 *mz80GetRegisterName(UINT32);

// Memory/IO read/write commands

#ifndef VALUE_BYTE
#define	VALUE_BYTE	0
#endif

#ifndef VALUE_WORD
#define	VALUE_WORD	1
#endif

#ifndef VALUE_DWORD
#define	VALUE_DWORD	2
#endif

#ifndef VALUE_IO
#define	VALUE_IO	3
#endif

extern void mz80WriteValue(UINT8 bWhat, UINT32 dwAddr, UINT32 dwData);
extern UINT32 mz80ReadValue(UINT8 bWhat, UINT32 dwAddr);

// Flag definitions

#define	Z80_FLAG_CARRY					0x01
#define	Z80_FLAG_NEGATIVE				0x02
#define	Z80_FLAG_OVERFLOW_PARITY	0x04
#define	Z80_FLAG_UNDEFINED1			0x08
#define	Z80_FLAG_HALF_CARRY			0x10
#define	Z80_FLAG_UNDEFINED2			0x20
#define	Z80_FLAG_ZERO					0x40
#define	Z80_FLAG_SIGN					0x80

#define	IFF1			0x01
#define	IFF2			0x02

typedef struct mz80context CONTEXTMZ80;

#ifdef __cplusplus
}
#endif

#endif	// _MZ80_H_
