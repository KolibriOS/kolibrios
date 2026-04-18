/****************************************************************************
*
*                            Open Watcom Project
*
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  C/C++ run-time library floating-point definitions.
*
****************************************************************************/


#ifndef _XFLOAT_H_INCLUDED
#define _XFLOAT_H_INCLUDED

#include <stddef.h>     // for wchar_t
#include <float.h>      // for LDBL_DIG

#ifdef __cplusplus
extern "C" {
#endif

#if (defined(__386__) || defined(M_I86)) && defined(__WATCOMC__)
 #define _LONG_DOUBLE_
#endif

typedef struct {                // This layout matches Intel 8087
  #ifdef _LONG_DOUBLE_
    unsigned long low_word;     // - low word of fraction
    unsigned long high_word;    // - high word of fraction
    unsigned short exponent;    // - exponent and sign
  #else                         // use this for all other 32-bit RISC
    union {
        double          value;  // - double value
        unsigned long   word[2];// - so we can access bits
    };
  #endif
} long_double;

typedef struct {                // Layout of IEEE 754 double (FD)
    union {
        double          value;  // - double value
        unsigned long   word[2];// - so we can access bits
    };
} float_double;

typedef struct {                // Layout of IEEE 754 single (FS)
    union {
        float           value;  // - double value
        unsigned long   word;   // - so we can access bits
    };
} float_single;

/* NB: The following values *must* match FP_ macros in math.h! */
enum    ld_classification {
    __ZERO      = 0,
    __DENORMAL  = 1,
    __NONZERO   = 2,
    __NAN       = 3,
    __INFINITY  = 4
};

enum    ldcvt_flags {
    E_FMT       = 0x0001,       // 'E' format
    F_FMT       = 0x0002,       // 'F' format
    G_FMT       = 0x0004,       // 'G' format
    F_CVT       = 0x0008,       // __cvt routine format rules
    F_DOT       = 0x0010,       // always put '.' in result
    LONG_DOUBLE = 0x0020,       // number is true long double
    NO_TRUNC    = 0x0040,       // always provide ndigits in buffer
    IN_CAPS     = 0x0080,       // 'inf'/'nan' is uppercased
};

typedef struct cvt_info {
      int       ndigits;        // INPUT: number of digits
      int       scale;          // INPUT: FORTRAN scale factor
      int       flags;          // INPUT: flags (see ldcvt_flags)
      int       expchar;        // INPUT: exponent character to use
      int       expwidth;       // INPUT/OUTPUT: number of exponent digits
      int       sign;           // OUTPUT: 0 => +ve; otherwise -ve
      int       decimal_place;  // OUTPUT: position of '.'
      int       n1;             // OUTPUT: number of leading characters
      int       nz1;            // OUTPUT: followed by this many '0's
      int       n2;             // OUTPUT: followed by these characters
      int       nz2;            // OUTPUT: followed by this many '0's
} CVT_INFO;

_WMRTLINK extern void __LDcvt(
                         long_double *pld,      // pointer to long_double
                         CVT_INFO  *cvt,        // conversion info
                         char      *buf );      // buffer
#if defined( __WATCOMC__ )
_WMRTLINK extern int __Strtold(
                        const char *bufptr,
                        long_double *pld,
                        char **endptr );
#endif
extern  int     __LDClass( long_double * );
extern  void    __ZBuf2LD(char _WCNEAR *, long_double _WCNEAR *);
extern  void    _LDScale10x(long_double _WCNEAR *,int);
_WMRTLINK extern void  __cnvd2ld( double _WCNEAR *src, long_double _WCNEAR *dst );
_WMRTLINK extern void  __cnvs2d( char *buf, double *value );
_WMRTLINK extern int   __cnvd2f( double *src, float *tgt );
#ifdef _LONG_DOUBLE_
extern  void    __iLDFD(long_double _WCNEAR *, double _WCNEAR *);
extern  void    __iLDFS(long_double _WCNEAR *, float _WCNEAR *);
extern  void    __iFDLD(double _WCNEAR *,long_double _WCNEAR *);
extern  void    __iFSLD(float _WCNEAR *,long_double _WCNEAR *);
extern  long    __LDI4(long_double _WCNEAR *);
extern  void    __I4LD(long,long_double _WCNEAR *);
extern  void    __U4LD(unsigned long,long_double _WCNEAR *);
extern void __FLDA(long_double _WCNEAR *,long_double _WCNEAR *,long_double _WCNEAR *);
extern void __FLDS(long_double _WCNEAR *,long_double _WCNEAR *,long_double _WCNEAR *);
extern void __FLDM(long_double _WCNEAR *,long_double _WCNEAR *,long_double _WCNEAR *);
extern void __FLDD(long_double _WCNEAR *,long_double _WCNEAR *,long_double _WCNEAR *);
extern int  __FLDC(long_double _WCNEAR *,long_double _WCNEAR *);
#endif

#ifdef __WATCOMC__
#if defined(__386__)
 #pragma aux    __ZBuf2LD       "*"  parm caller [eax] [edx];
 #if defined(__FPI__)
  extern unsigned __Get87CW(void);
  extern void __Set87CW(unsigned short);
  #pragma aux   __Get87CW = \
                "push 0"\
        float   "fstcw [esp]"\
        float   "fwait"\
                "pop eax"\
                value [eax];
  #pragma aux   __Set87CW = \
                "push eax"\
        float   "fldcw [esp]"\
                "pop eax"\
                parm caller [eax];
  #pragma aux   __FLDA = \
        float   "fld tbyte ptr [eax]"\
        float   "fld tbyte ptr [edx]"\
        float   "fadd"\
        float   "fstp tbyte ptr [ebx]"\
                parm caller [eax] [edx] [ebx];
  #pragma aux   __FLDS = \
        float   "fld tbyte ptr [eax]"\
        float   "fld tbyte ptr [edx]"\
        float   "fsub"\
        float   "fstp tbyte ptr [ebx]"\
                parm caller [eax] [edx] [ebx];
  #pragma aux   __FLDM = \
        float   "fld tbyte ptr [eax]"\
        float   "fld tbyte ptr [edx]"\
        float   "fmul"\
        float   "fstp tbyte ptr [ebx]"\
                parm caller [eax] [edx] [ebx];
  #pragma aux   __FLDD = \
        float   "fld tbyte ptr [eax]"\
        float   "fld tbyte ptr [edx]"\
        float   "fdiv"\
        float   "fstp tbyte ptr [ebx]"\
                parm caller [eax] [edx] [ebx];
  #pragma aux   __FLDC = \
                /* ST(1) */\
        float   "fld tbyte ptr [edx]"\
                /* ST(0) */\
        float   "fld tbyte ptr [eax]"\
                /* compare ST(0) with ST(1) */\
        float   "fcompp"\
        float   "fstsw  ax"\
                "sahf"\
                "sbb  edx,edx"\
                "shl  edx,1"\
                "shl  ah,2"\
                "cmc"\
                "adc  edx,0"\
                /* edx will be -1,0,+1 if [eax] <, ==, > [edx] */\
                parm caller [eax] [edx] value [edx];
  #pragma aux   __LDI4 = \
        float   "fld tbyte ptr [eax]"\
                "push  eax"\
                "push  eax"\
        float   "fstcw [esp]"\
        float   "fwait"\
                "pop eax"\
                "push eax"\
                "or ah,0x0c"\
                "push eax"\
        float   "fldcw [esp]"\
                "pop eax"\
        float   "fistp dword ptr 4[esp]"\
        float   "fldcw [esp]"\
                "pop   eax"\
                "pop   eax"\
                parm caller [eax] value [eax];
  #pragma aux   __I4LD = \
                "push  eax"\
        float   "fild  dword ptr [esp]"\
                "pop   eax"\
        float   "fstp tbyte ptr [edx]"\
                parm caller [eax] [edx];
  #pragma aux   __U4LD = \
                "push  0"\
                "push  eax"\
        float   "fild  qword ptr [esp]"\
                "pop   eax"\
                "pop   eax"\
        float   "fstp tbyte ptr [edx]"\
                parm caller [eax] [edx];
  #pragma aux   __iFDLD = \
        float   "fld  qword ptr [eax]"\
        float   "fstp tbyte ptr [edx]"\
                parm caller [eax] [edx];
  #pragma aux   __iFSLD = \
        float   "fld  dword ptr [eax]"\
        float   "fstp tbyte ptr [edx]"\
                parm caller [eax] [edx];
  #pragma aux   __iLDFD = \
        float   "fld  tbyte ptr [eax]"\
        float   "fstp qword ptr [edx]"\
                parm caller [eax] [edx];
  #pragma aux   __iLDFS = \
        float   "fld  tbyte ptr [eax]"\
        float   "fstp dword ptr [edx]"\
                parm caller [eax] [edx];
 #else  // floating-point calls
  #pragma aux   __FLDA  "*"  parm caller [eax] [edx] [ebx];
  #pragma aux   __FLDS  "*"  parm caller [eax] [edx] [ebx];
  #pragma aux   __FLDM  "*"  parm caller [eax] [edx] [ebx];
  #pragma aux   __FLDD  "*"  parm caller [eax] [edx] [ebx];
  #pragma aux   __LDI4  "*"  parm caller [eax] value [eax];
  #pragma aux   __I4LD  "*"  parm caller [eax] [edx];
  #pragma aux   __U4LD  "*"  parm caller [eax] [edx];
  #pragma aux   __iFDLD "*"  parm caller [eax] [edx];
  #pragma aux   __iFSLD "*"  parm caller [eax] [edx];
  #pragma aux   __iLDFD "*"  parm caller [eax] [edx];
  #pragma aux   __iLDFS "*"  parm caller [eax] [edx];
  #pragma aux   __FLDC  "*"  parm caller [eax] [edx] value [eax];
 #endif
#elif defined(M_I86)            // 16-bit pragmas
 #pragma aux     __ZBuf2LD      "*"  parm caller [ax] [dx];
 #if defined(__FPI__)
  extern unsigned __Get87CW(void);
  extern void __Set87CW(unsigned short);
  #pragma aux   __Get87CW = \
                "push ax"\
                "push bp"\
                "mov  bp,sp"\
        float   "fstcw 2[bp]"\
        float   "fwait"\
                "pop bp"\
                "pop ax"\
                value [ax];
  #pragma aux   __Set87CW = \
                "push ax"\
                "push bp"\
                "mov  bp,sp"\
        float   "fldcw 2[bp]"\
                "pop bp"\
                "pop ax"\
                parm caller [ax];
  #pragma aux   __FLDA = \
                "push bp"\
                "mov  bp,ax"\
        float   "fld  tbyte ptr [bp]"\
                "mov  bp,dx"\
        float   "fld  tbyte ptr [bp]"\
        float   "fadd"\
                "mov  bp,bx"\
        float   "fstp tbyte ptr [bp]"\
                "pop  bp"\
                parm caller [ax] [dx] [bx];
  #pragma aux   __FLDS = \
                "push bp"\
                "mov  bp,ax"\
        float   "fld  tbyte ptr [bp]"\
                "mov  bp,dx"\
        float   "fld  tbyte ptr [bp]"\
        float   "fsub"\
                "mov  bp,bx"\
        float   "fstp tbyte ptr [bp]"\
                "pop  bp"\
                parm caller [ax] [dx] [bx];
  #pragma aux   __FLDM = \
                "push bp"\
                "mov  bp,ax"\
        float   "fld  tbyte ptr [bp]"\
                "mov  bp,dx"\
        float   "fld  tbyte ptr [bp]"\
        float   "fmul"\
                "mov  bp,bx"\
        float   "fstp tbyte ptr [bp]"\
                "pop  bp"\
                parm caller [ax] [dx] [bx];
  #pragma aux   __FLDD = \
                "push bp"\
                "mov  bp,ax"\
        float   "fld  tbyte ptr [bp]"\
                "mov  bp,dx"\
        float   "fld  tbyte ptr [bp]"\
        float   "fdiv"\
                "mov  bp,bx"\
        float   "fstp tbyte ptr [bp]"\
                "pop  bp"\
                parm caller [ax] [dx] [bx];
  #pragma aux   __FLDC = \
                "push bp"\
                "mov  bp,dx"\
                /* ST(1) */\
        float   "fld  tbyte ptr [bp]"\
                "mov  bp,ax"\
                /* ST(0) */\
        float   "fld  tbyte ptr [bp]"\
                /* compare ST(0) with ST(1) */\
        float   "fcompp"\
                "push ax"\
                "mov  bp,sp"\
        float   "fstsw 0[bp]"\
        float   "fwait"\
                "pop  ax"\
                "sahf"\
                "sbb  dx,dx"\
                "shl  dx,1"\
                "shl  ah,1"\
                "shl  ah,1"\
                "cmc"\
                "adc  dx,0"\
                "pop  bp"\
                parm caller [ax] [dx] value [dx];
  #pragma aux   __LDI4 = \
                "push bp"\
                "mov  bp,ax"\
        float   "fld  tbyte ptr [bp]"\
                "push dx"\
                "push ax"\
                "push ax"\
                "mov  bp,sp"\
        float   "fstcw [bp]"\
        float   "fwait"\
                "pop  ax"\
                "push ax"\
                "or   ah,0x0c"\
                "mov  2[bp],ax"\
        float   "fldcw 2[bp]"\
        float   "fistp dword ptr 2[bp]"\
        float   "fldcw [bp]"\
                "pop   ax"\
                "pop   ax"\
                "pop   dx"\
                "pop   bp"\
                parm caller [ax] value [dx ax];
  #pragma aux   __I4LD = \
                "push  bp"\
                "push  dx"\
                "push  ax"\
                "mov   bp,sp"\
        float   "fild  dword ptr [bp]"\
                "pop   ax"\
                "pop   dx"\
                "mov   bp,bx"\
        float   "fstp  tbyte ptr [bp]"\
                "pop   bp"\
                parm caller [dx ax] [bx];
  #pragma aux   __U4LD = \
                "push  bp"\
                "push  ax"\
                "push  ax"\
                "push  dx"\
                "push  ax"\
                "mov   bp,sp"\
                "sub   ax,ax"\
                "mov   4[bp],ax"\
                "mov   6[bp],ax"\
        float   "fild  qword ptr 2[bp]"\
                "pop   ax"\
                "pop   dx"\
                "pop   ax"\
                "pop   ax"\
                "mov   bp,bx"\
        float   "fstp  tbyte ptr [bp]"\
                "pop   bp"\
                parm caller [dx ax] [bx];
  #pragma aux   __iFDLD = \
                "push  bp"\
                "mov   bp,ax"\
        float   "fld   qword ptr [bp]"\
                "mov   bp,dx"\
        float   "fstp  tbyte ptr [bp]"\
                "pop   bp"\
                parm caller [ax] [dx];
  #pragma aux   __iFSLD = \
                "push  bp"\
                "mov   bp,ax"\
        float   "fld   dword ptr [bp]"\
                "mov   bp,dx"\
        float   "fstp  tbyte ptr [bp]"\
                "pop   bp"\
                parm caller [ax] [dx];
  #pragma aux   __iLDFD = \
                "push  bp"\
                "mov   bp,ax"\
        float   "fld   tbyte ptr [bp]"\
                "mov   bp,dx"\
        float   "fstp  qword ptr [bp]"\
                "pop   bp"\
                parm caller [ax] [dx];
  #pragma aux   __iLDFS = \
                "push  bp"\
                "mov   bp,ax"\
        float   "fld   tbyte ptr [bp]"\
                "mov   bp,dx"\
        float   "fstp  dword ptr [bp]"\
                "pop   bp"\
                parm caller [ax] [dx];
 #else  // floating-point calls
  #pragma aux   __FLDA  "*"  parm caller [ax] [dx] [bx];
  #pragma aux   __FLDS  "*"  parm caller [ax] [dx] [bx];
  #pragma aux   __FLDM  "*"  parm caller [ax] [dx] [bx];
  #pragma aux   __FLDD  "*"  parm caller [ax] [dx] [bx];
  #pragma aux   __LDI4  "*"  parm caller [ax] value [dx ax];
  #pragma aux   __I4LD  "*"  parm caller [dx ax] [bx];
  #pragma aux   __U4LD  "*"  parm caller [dx ax] [bx];
  #pragma aux   __iFDLD "*"  parm caller [ax] [dx];
  #pragma aux   __iFSLD "*"  parm caller [ax] [dx];
  #pragma aux   __iLDFD "*"  parm caller [ax] [dx];
  #pragma aux   __iLDFS "*"  parm caller [ax] [dx];
  #pragma aux   __FLDC  "*"  parm caller [ax] [dx] value [ax];
 #endif
#endif
#endif

#ifdef _LONG_DOUBLE_
  // macros to allow old source code names to still work
  #define __FDLD __iFDLD
  #define __FSLD __iFSLD
  #define __LDFD __iLDFD
  #define __LDFS __iLDFS
#endif

// define number of significant digits for long double numbers (80-bit)
// it will be defined in float.h as soon as OW support long double
// used in mathlib/c/ldcvt.c

#ifdef _LONG_DOUBLE_
#if LDBL_DIG == 15
#undef LDBL_DIG
#define LDBL_DIG        19
#else
#error LDBL_DIG has changed from 15
#endif
#endif

// floating point conversion buffer length definition
// used by various floating point conversion routines
// used in clib/startup/c/cvtbuf.c and lib_misc/h/thread.h
// it must be equal maximum FP precision ( LDBL_DIG )

#define __FPCVT_BUFFERLEN  19

#ifdef __cplusplus
};
#endif
#endif
