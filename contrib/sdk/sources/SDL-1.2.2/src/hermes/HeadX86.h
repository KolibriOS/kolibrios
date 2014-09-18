/*
   Header definitions for the x86 routines for the HERMES library
   Copyright (c) 1998 Christian Nentwich (brn@eleet.mcb.at)
   This source code is licensed under the GNU LGPL
  
   Please refer to the file COPYING.LIB contained in the distribution for
   licensing conditions
*/

#ifndef __HERMES_HEAD_X86__
#define __HERMES_HEAD_X86__


#ifdef X86_ASSEMBLER

/* If you can't stand IFDEFS, then close your eyes now, please :) */

/* Ok, we start with normal function definitions */
#ifdef __cplusplus
extern "C" {
#endif


void STACKCALL ConvertX86(HermesConverterInterface *);
void STACKCALL ClearX86_32(HermesClearInterface *);
void STACKCALL ClearX86_24(HermesClearInterface *);
void STACKCALL ClearX86_16(HermesClearInterface *);
void STACKCALL ClearX86_8(HermesClearInterface *);

int STACKCALL Hermes_X86_CPU();

void ConvertX86p32_32BGR888();
void ConvertX86p32_32RGBA888();
void ConvertX86p32_32BGRA888();
void ConvertX86p32_24RGB888();
void ConvertX86p32_24BGR888();
void ConvertX86p32_16RGB565();
void ConvertX86p32_16BGR565();
void ConvertX86p32_16RGB555();
void ConvertX86p32_16BGR555();
void ConvertX86p32_8RGB332();

void ConvertX86p16_32RGB888();
void ConvertX86p16_32BGR888();
void ConvertX86p16_32RGBA888();
void ConvertX86p16_32BGRA888();
void ConvertX86p16_24RGB888();
void ConvertX86p16_24BGR888();
void ConvertX86p16_16BGR565();
void ConvertX86p16_16RGB555();
void ConvertX86p16_16BGR555();
void ConvertX86p16_8RGB332();

void CopyX86p_4byte();
void CopyX86p_3byte();
void CopyX86p_2byte();
void CopyX86p_1byte();

void ConvertX86pI8_32();
void ConvertX86pI8_24();
void ConvertX86pI8_16();

extern int32 ConvertX86p16_32RGB888_LUT_X86[512];
extern int32 ConvertX86p16_32BGR888_LUT_X86[512];
extern int32 ConvertX86p16_32RGBA888_LUT_X86[512];
extern int32 ConvertX86p16_32BGRA888_LUT_X86[512];
  
#ifdef __cplusplus
}
#endif




/* Now fix up the ELF underscore problem */

#if defined(__ELF__) && defined(__GNUC__)
  #ifdef __cplusplus
  extern "C" {
  #endif

  int Hermes_X86_CPU() __attribute__ ((alias ("_Hermes_X86_CPU")));

  void ConvertX86(HermesConverterInterface *) __attribute__ ((alias ("_ConvertX86")));

#if 0
  void ClearX86_32(HermesClearInterface *) __attribute__ ((alias ("_ClearX86_32")));
  void ClearX86_24(HermesClearInterface *)  __attribute__ ((alias ("_ClearX86_24")));
  void ClearX86_16(HermesClearInterface *)  __attribute__ ((alias ("_ClearX86_16")));
  void ClearX86_8(HermesClearInterface *)  __attribute__ ((alias ("_ClearX86_8")));
#endif

  void ConvertX86p32_32BGR888() __attribute__ ((alias ("_ConvertX86p32_32BGR888")));
  void ConvertX86p32_32RGBA888() __attribute__ ((alias ("_ConvertX86p32_32RGBA888")));
  void ConvertX86p32_32BGRA888() __attribute__ ((alias ("_ConvertX86p32_32BGRA888")));
  void ConvertX86p32_24RGB888() __attribute__ ((alias ("_ConvertX86p32_24RGB888")));
  void ConvertX86p32_24BGR888() __attribute__ ((alias ("_ConvertX86p32_24BGR888")));
  void ConvertX86p32_16RGB565() __attribute__ ((alias ("_ConvertX86p32_16RGB565")));
  void ConvertX86p32_16BGR565() __attribute__ ((alias ("_ConvertX86p32_16BGR565")));
  void ConvertX86p32_16RGB555() __attribute__ ((alias ("_ConvertX86p32_16RGB555")));
  void ConvertX86p32_16BGR555() __attribute__ ((alias ("_ConvertX86p32_16BGR555")));
  void ConvertX86p32_8RGB332() __attribute__ ((alias ("_ConvertX86p32_8RGB332")));

#if 0
  void ConvertX86p16_32RGB888() __attribute__ ((alias ("_ConvertX86p16_32RGB888")));
  void ConvertX86p16_32BGR888() __attribute__ ((alias ("_ConvertX86p16_32BGR888")));
  void ConvertX86p16_32RGBA888() __attribute__ ((alias ("_ConvertX86p16_32RGBA888")));
  void ConvertX86p16_32BGRA888() __attribute__ ((alias ("_ConvertX86p16_32BGRA888")));
  void ConvertX86p16_24RGB888() __attribute__ ((alias ("_ConvertX86p16_24RGB888")));
  void ConvertX86p16_24BGR888() __attribute__ ((alias ("_ConvertX86p16_24BGR888")));
#endif
  void ConvertX86p16_16BGR565() __attribute__ ((alias ("_ConvertX86p16_16BGR565")));
  void ConvertX86p16_16RGB555() __attribute__ ((alias ("_ConvertX86p16_16RGB555")));
  void ConvertX86p16_16BGR555() __attribute__ ((alias ("_ConvertX86p16_16BGR555")));
  void ConvertX86p16_8RGB332() __attribute__ ((alias ("_ConvertX86p16_8RGB332")));

#if 0
  void CopyX86p_4byte() __attribute__ ((alias ("_CopyX86p_4byte")));
  void CopyX86p_3byte() __attribute__ ((alias ("_CopyX86p_3byte")));
  void CopyX86p_2byte() __attribute__ ((alias ("_CopyX86p_2byte")));
  void CopyX86p_1byte() __attribute__ ((alias ("_CopyX86p_1byte")));

  void ConvertX86pI8_32() __attribute__ ((alias ("_ConvertX86pI8_32")));
  void ConvertX86pI8_24() __attribute__ ((alias ("_ConvertX86pI8_24")));
  void ConvertX86pI8_16() __attribute__ ((alias ("_ConvertX86pI8_16")));

  extern int32 ConvertX86p16_32RGB888_LUT_X86[512] __attribute__ ((alias ("_ConvertX86p16_32RGB888_LUT_X86")));
  extern int32 ConvertX86p16_32BGR888_LUT_X86[512] __attribute__ ((alias ("_ConvertX86p16_32BGR888_LUT_X86")));
  extern int32 ConvertX86p16_32RGBA888_LUT_X86[512] __attribute__ ((alias ("_ConvertX86p16_32RGBA888_LUT_X86")));
  extern int32 ConvertX86p16_32BGRA888_LUT_X86[512] __attribute__ ((alias ("_ConvertX86p16_32BGRA888_LUT_X86")));
#endif

  #ifdef __cplusplus
  }
  #endif

#endif /* ELF & GNU */



/* Make it run with WATCOM C */
#ifdef __WATCOMC__
#pragma warning 601 9

#pragma aux Hermes_X86_CPU "_*"

#pragma aux ConvertX86 "_*" modify [EAX EBX ECX EDX ESI EDI]
#pragma aux ClearX86_32 "_*" modify [EAX EBX ECX EDX ESI EDI]
#pragma aux ClearX86_24 "_*" modify [EAX EBX ECX EDX ESI EDI]
#pragma aux ClearX86_16 "_*" modify [EAX EBX ECX EDX ESI EDI]
#pragma aux ClearX86_8 "_*" modify [EAX EBX ECX EDX ESI EDI]

#pragma aux ConvertX86p32_32BGR888 "_*"
#pragma aux ConvertX86p32_32RGBA888 "_*"
#pragma aux ConvertX86p32_32BGRA888 "_*"
#pragma aux ConvertX86p32_24RGB888 "_*"
#pragma aux ConvertX86p32_24BGR888 "_*"
#pragma aux ConvertX86p32_16RGB565 "_*"
#pragma aux ConvertX86p32_16BGR565 "_*"
#pragma aux ConvertX86p32_16RGB555 "_*"
#pragma aux ConvertX86p32_16BGR555 "_*"
#pragma aux ConvertX86p32_8RGB332 "_*"

#pragma aux ConvertX86p16_32RGB888 "_*"
#pragma aux ConvertX86p16_32BGR888 "_*"
#pragma aux ConvertX86p16_32RGBA888 "_*"
#pragma aux ConvertX86p16_32BGRA888 "_*"
#pragma aux ConvertX86p16_24RGB888 "_*"
#pragma aux ConvertX86p16_24BGR888 "_*"
#pragma aux ConvertX86p16_16BGR565 "_*"
#pragma aux ConvertX86p16_16RGB555 "_*"
#pragma aux ConvertX86p16_16BGR555 "_*"
#pragma aux ConvertX86p16_8RGB332 "_*"

#pragma aux CopyX86p_4byte "_*"
#pragma aux CopyX86p_3byte "_*"
#pragma aux CopyX86p_2byte "_*"
#pragma aux CopyX86p_1byte "_*"

#pragma aux ConvertX86pI8_32 "_*"
#pragma aux ConvertX86pI8_24 "_*"
#pragma aux ConvertX86pI8_16 "_*"

#pragma aux ConvertX86p16_32RGB888_LUT_X86 "_*"
#pragma aux ConvertX86p16_32BGR888_LUT_X86 "_*"
#pragma aux ConvertX86p16_32RGBA888_LUT_X86 "_*"
#pragma aux ConvertX86p16_32BGRA888_LUT_X86 "_*"

#endif /* __WATCOMC__ */


#endif /* X86_ASSEMBLER */


#endif 
