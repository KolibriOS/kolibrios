#include "tok.h"

#include "table.h"

#include <stdarg.h>

#define _DISASM_



/* Percent tokens in strings:

   First char after '%':

	A - direct address

	C - reg of r/m picks control register

	D - reg of r/m picks debug register

	E - r/m picks operand

	F - second operand for mmx instruction reg/mem

	G - reg of r/m picks general register

	I - immediate data

	J - relative IP offset

+      K - call/jmp distance

	L - first operand for mmx instruction

	M - r/m picks memory

	O - no r/m, offset only

	R - mod of r/m picks register only

	S - reg of r/m picks segment register

	T - reg of r/m picks test register

	X - DS:ESI

	Y - ES:EDI

	2 - prefix of two-byte opcode

+       e - put in 'e' if use32 (second char is part of reg name)

+           put in 'w' for use16 or 'd' for use32 (second char is 'w')

+       j - put in 'e' in jcxz if prefix==0x66

	f - floating point (second char is esc value)

	g - do r/m group 'n', n==0..7

	p - prefix

	s - size override (second char is a,o)

+       d - put d if double arg, nothing otherwise (pushfd, popfd &c)

+       w - put w if word, d if double arg, nothing otherwise (lodsw/lodsd)

+       P - simple prefix



   Second char after '%':

	a - two words in memory (BOUND)

	b - byte

	c - byte or word

	d - dword

+       f - far call/jmp

+       n - near call/jmp

        p - 32 or 48 bit pointer

+       q - byte/word thingy

	s - six byte pseudo-descriptor

	v - word or dword

        w - word

+       x - sign extended byte

	y - qword

	F - use floating regs in mod/rm

	M - use MMX regs

	1-8 - group number, esc value, etc

	m - no size memory operand

*/



/* watch out for aad && aam with odd operands */

char *opmap1[256]={

/* 0 */

  "add %Eb,%Rb",      "add %Ev,%Rv",     "add %Rb,%Eb",    "add %Rv,%Ev",

  "add al,%Ib",       "add %eax,%Iv",    "push es",        "pop es",

  "or %Eb,%Rb",       "or %Ev,%Rv",      "or %Rb,%Eb",     "or %Rv,%Ev",

  "or al,%Ib",        "or %eax,%Iv",     "push cs",        "%2 ",

/* 1 */

  "adc %Eb,%Rb",      "adc %Ev,%Rv",     "adc %Rb,%Eb",    "adc %Rv,%Ev",

  "adc al,%Ib",       "adc %eax,%Iv",    "push ss",        "pop ss",

  "sbb %Eb,%Rb",      "sbb %Ev,%Rv",     "sbb %Rb,%Eb",    "sbb %Rv,%Ev",

  "sbb al,%Ib",       "sbb %eax,%Iv",    "push ds",        "pop ds",

/* 2 */

  "and %Eb,%Rb",      "and %Ev,%Rv",     "and %Rb,%Eb",    "and %Rv,%Ev",

  "and al,%Ib",       "and %eax,%Iv",    "%pe",            "daa",

  "sub %Eb,%Rb",      "sub %Ev,%Rv",     "sub %Rb,%Eb",    "sub %Rv,%Ev",

  "sub al,%Ib",       "sub %eax,%Iv",    "%pc",            "das",

/* 3 */

  "xor %Eb,%Rb",      "xor %Ev,%Rv",     "xor %Rb,%Eb",    "xor %Rv,%Ev",

  "xor al,%Ib",       "xor %eax,%Iv",    "%ps",            "aaa",

  "cmp %Eb,%Rb",      "cmp %Ev,%Rv",     "cmp %Rb,%Eb",    "cmp %Rv,%Ev",

  "cmp al,%Ib",       "cmp %eax,%Iv",    "%pd",            "aas",

/* 4 */

  "inc %eax",         "inc %ecx",        "inc %edx",       "inc %ebx",

  "inc %esp",         "inc %ebp",        "inc %esi",       "inc %edi",

  "dec %eax",         "dec %ecx",        "dec %edx",       "dec %ebx",

  "dec %esp",         "dec %ebp",        "dec %esi",       "dec %edi",

/* 5 */

  "push %eax",        "push %ecx",       "push %edx",      "push %ebx",

  "push %esp",        "push %ebp",       "push %esi",      "push %edi",

  "pop %eax",         "pop %ecx",        "pop %edx",       "pop %ebx",

  "pop %esp",         "pop %ebp",        "pop %esi",       "pop %edi",

/* 6 */

  "pusha%d ",         "popa%d ",         "bound %Rv,%Ma",  "arpl %Ew,%Rw",

  "%pf",              "%pg",             "%so",            "%sa",

  "push %Iv",         "imul %Rv,%Ev,%Iv","push %Ix",       "imul %Rv,%Ev,%Ib",

  "insb",             "ins%ew",          "outsb",          "outs%ew",

/* 7 */

  "jo %Jb",           "jno %Jb",         "jc %Jb",         "jnc %Jb",

  "je %Jb",           "jne %Jb",         "jbe %Jb",        "ja %Jb",

  "js %Jb",           "jns %Jb",         "jpe %Jb",        "jpo %Jb",

  "jl %Jb",           "jge %Jb",         "jle %Jb",        "jg %Jb",

/* 8 */

/*  "%g0 %Eb,%Ib",      "%g0 %Ev,%Iv",     "%g0 %Ev,%Ib",    "%g0 %Ev,%Ib", */

  "%g0 %Eb,%Ib",      "%g0 %Ev,%Iv",     "%g0 %Ev,%Ix",    "%g0 %Ev,%Ix",

  "test %Eb,%Rb",     "test %Ev,%Rv",    "xchg %Eb,%Rb",   "xchg %Ev,%Rv",

  "mov %Eb,%Rb",      "mov %Ev,%Rv",     "mov %Rb,%Eb",    "mov %Rv,%Ev",

  "mov %Ew,%Sw",      "lea %Rv,%M ",     "mov %Sw,%Ew",    "pop %Ev",

/* 9 */

  "nop",              "xchg %ecx,%eax",  "xchg %edx,%eax", "xchg %ebx,%eax",

  "xchg %esp,%eax",   "xchg %ebp,%eax",  "xchg %esi,%eax", "xchg %edi,%eax",

  "cbw",              "cwd",             "call %Ap",       "fwait",

  "pushf%d ",         "popf%d ",         "sahf",           "lahf",

/* a */

  "mov al,%Oc",       "mov %eax,%Ov",    "mov %Oc,al",     "mov %Ov,%eax",

  "%P movsb",         "%P movs%w",       "%P cmpsb",       "%P cmps%w ",

  "test al,%Ib",      "test %eax,%Iv",   "%P stosb",       "%P stos%w ",

  "%P lodsb",         "%P lods%w ",      "%P scasb",       "%P scas%w ",

/* b */

  "mov al,%Ib",       "mov cl,%Ib",      "mov dl,%Ib",     "mov bl,%Ib",

  "mov ah,%Ib",       "mov ch,%Ib",      "mov dh,%Ib",     "mov bh,%Ib",

  "mov %eax,%Iv",     "mov %ecx,%Iv",    "mov %edx,%Iv",   "mov %ebx,%Iv",

  "mov %esp,%Iv",     "mov %ebp,%Iv",    "mov %esi,%Iv",   "mov %edi,%Iv",

/* c */

  "%g1 %Eb,%Ib",      "%g1 %Ev,%Ib",     "ret %Iw",        "ret",

  "les %Rv,%Mp",      "lds %Rv,%Mp",     "mov %Eb,%Ib",    "mov %Ev,%Iv",

  "enter %Iw,%Ib",    "leave",           "retf %Iw",       "retf",

  "int 03",           "int %Ib",         "into",           "iret",

/* d */

  "%g1 %Eb,1",        "%g1 %Ev,1",       "%g1 %Eb,cl",     "%g1 %Ev,cl",

  "aam ; %Ib",        "aad ; %Ib",       "setalc",         "xlat",

/*#if 0

  "esc 0,%Ib",        "esc 1,%Ib",       "esc 2,%Ib",      "esc 3,%Ib",

  "esc 4,%Ib",        "esc 5,%Ib",       "esc 6,%Ib",      "esc 7,%Ib",

#else  */

  "%f0",              "%f1",             "%f2",            "%f3",

  "%f4",              "%f5",             "%f6",            "%f7",

//#endif

/* e */

  "loopne %Jb",       "loope %Jb",       "loop %Jb",       "j%j cxz %Jb",

  "in al,%Ib",        "in %eax,%Ib",     "out %Ib,al",     "out %Ib,%eax",

  "call %Jv",         "jmp %Jv",         "jmp %Ap",        "jmp %Ks%Jb",

  "in al,dx",         "in %eax,dx",      "out dx,al",      "out dx,%eax",

/* f */

  "lock %p ",         0,                 "repne %p ",      "repe %p ",

  "hlt",              "cmc",             "%g2",            "%g2",

  "clc",              "stc",             "cli",            "sti",

  "cld",              "std",             "%g3",            "%g4"

};



char *second[] = {

/* 0 */

  "%g5",              "%g6",             "lar %Rv,%Ew",    "lsl %Rv,%Ew",

  0,                  "loadall",         "clts",           "loadall",

  "invd",             "wbinvd",          0,                "ud2",

  0,                  0,                 0,                0,

/* 1 */

  "movups %RX,%EX",   "movups %Md,%RX",  "%x0",             "movlps %Md,%RX",

  "unpcklps %RX,%EX", "unpckhps %RX,%EX","%x1",             "movhps %Md,%RX",

  "%g7",              0,                 0,                0,

  0,                  0,                 0,                0,

/* 2 */

  "mov %Ed,%Cd",      "mov %Ed,%Dd",     "mov %Cd,%Ed",     "mov %Dd,%Ed",

  "mov %Ed,%Td",      0,                 "mov %Td,%Ed",     0,

  "movaps %RX,%EX",   "movaps %Md,%RX",  "cvtpi2ps %RX,%EM","movntps %Md,%RX",

  "cvttps2pi %RM,%EX","cvtps2pi %RM,%EX","ucomiss %RX,%EX", "comiss %RX,%EX",

/* 3 */

  "wrmsr", "rdtsc", "rdmsr", "rdpmc", "sysenter", "sysexit", 0, 0,

  0, 0, 0, 0, 0, 0, 0, 0,

/* 4 */

  "cmovo %Rv,%Ev",    "cmovno %Rv,%Ev",  "cmovc %Rv,%Ev",  "cmovnc %Rv,%Ev",

  "cmovz %Rv,%Ev",    "cmovnz %Rv,%Ev",  "cmovbe %Rv,%Ev", "cmovnbe %Rv,%Ev",

  "cmovs %Rv,%Ev",    "cmovns %Rv,%Ev",  "cmovp %Rv,%Ev",  "cmovnp %Rv,%Ev",

  "cmovl %Rv,%Ev",    "cmovge %Rv,%Ev",  "cmovle %Rv,%Ev", "cmovg %Rv,%Ev",

/* 5 */

  "movmskps %Rd,%GX", "sqrtps %RX,%EX", "rsqrtps %RX,%EX", "rcpps %RX,%EX",

	"andps %RX,%EX",    "andnps %RX,%EX", "orps %RX,%EX",    "xorps %RX,%EX",

  "addps %RX,%EX",    "mulps %RX,%EX",  "cvtps2pd %RX,%EX","cvtdq2ps %RX,%EX",

	"subps %RX,%EX",    "minps %RX,%EX",  "divps %RX,%EX",   "maxps %RX,%EX",

/* 6 */

  "punpcklbw %RM,%EM", "punpcklwd %RM,%EM", "punpckldq %RM,%EM", "packsswb %RM,%EM",

	"pcmpgtb %RM,%EM",   "pcmpgtw %RM,%EM",   "pcmpgtd %RM,%EM",   "packuswb %RM,%EM",

  "punpckhbw %RM,%EM", "punpckhwd %RM,%EM", "punpckhdq %RM,%EM", "packssdw %RM,%EM",

	0,                   0,                   "movd %RM,%Md",      "movq %RM,%EM",

/* 7 */

  "pshufw %LM,%FM,%Ib","%g3w %EM,%Ib",    "%g3d %EM,%Ib",    "%g3q %EM,%Ib",

	"pcmpeqb %RM,%EM",  "pcmpeqw %RM,%EM", "pcmpeqd %RM,%EM", "emms",

  0,                  0,                 0,                 0,

	0,                  0,                 "movd %Md,%RM",    "movq %Md,%RM",

/* 8 */

  "jo %Jv",           "jno %Jv",         "jb %Jv",         "jnb %Jv",

  "jz %Jv",           "jnz %Jv",         "jbe %Jv",        "ja %Jv",

  "js %Jv",           "jns %Jv",         "jp %Jv",         "jnp %Jv",

  "jl %Jv",           "jge %Jv",         "jle %Jv",        "jg %Jv",

/* 9 */

  "seto %Eb",         "setno %Eb",       "setc %Eb",       "setnc %Eb",

  "setz %Eb",         "setnz %Eb",       "setbe %Eb",      "setnbe %Eb",

  "sets %Eb",         "setns %Eb",       "setp %Eb",       "setnp %Eb",

  "setl %Eb",         "setge %Eb",       "setle %Eb",      "setg %Eb",

/* a */

  "push fs",          "pop fs",          "cpuid",          "bt %Ev,%Rv",

  "shld %Ev,%Rv,%Ib", "shld %Ev,%Rv,cl", 0,                0,

  "push gs",          "pop gs",          "rsm",            "bts %Ev,%Rv",

  "shrd %Ev,%Rv,%Ib", "shrd %Ev,%Rv,cl", "%g8",            "imul %Rv,%Ev",

/* b */

  "cmpxchg %Eb,%Rb",  "cmpxchg %Ev,%Rv", "lss %Rv,%Mp",    "btr %Ev,%Rv",

  "lfs %Rv,%Mp",      "lgs %Rv,%Mp",     "movzx %Rv,%Eb",  "movzx %Rv,%Ew",

  0,                  0,                 "%g7 %Ev,%Ib",    "btc %Ev,%Rv",

  "bsf %Rv,%Ev",      "bsr %Rv,%Ev",     "movsx %Rv,%Eb",  "movsx %Rv,%Ew",

/* c */

  "xadd %Eb,%Rb",     "xadd %Ev,%Rv",    "cmpps %RX,%EX,%Ib","movnti %Md,%Rd",

  "pinsrw %LM,%Fd,%Ib","pextrw %Gd,%RM,%Ib","shufps %RX,%EX,%Ib","cmpxchg8b %Myv",

  "bswap eax",        "bswap ecx",       "bswap edx",      "bswap ebx",

  "bswap esp",        "bswap ebp",       "bswap esi",      "bswap edi",

/* d */

  0,                  "psrlw %RM,%EM",   "psrld %RM,%EM",  "psrlq %RM,%EM",

	"paddq %RM,%EM",    "pmullw %RM,%EM",  0,                "pmovmskb %Rd,%GM",

  "psubusb %RM,%EM",  "psubusw %RM,%EM", "pminub %RM,%EM", "pand %RM,%EM",

	"paddusb %RM,%EM",  "paddusw %RM,%EM", "pmaxub %RM,%EM", "pandn %RM,%EM",

/* e */

  "pavgb %RM,%EM",    "psraw %RM,%EM",   "psrad %RM,%EM",  "pavgw %RM,%EM",

	"pmulhuw %RM,%EM",  "pmulhw %RM,%EM",  0,                "movntq %Myv,%RM",

  "psubsb %RM,%EM",   "psubsw %RM,%EM",  "pminsw %RM,%EM", "por %RM,%EM",

	"paddsb %RM,%EM",   "paddsw %RM,%EM",  "pmaxsw %RM,%EM", "pxor %RM,%EM",

/* f */

  0,                  "psllw %RM,%EM",   "pslld %RM,%EM",  "psllq %RM,%EM",

	"pmuludq %RM,%EM",  "pmaddwd %RM,%EM", "psadbw %RM,%EM", "maskmovq %GM,%RM",

  "psubb %RM,%EM",    "psubw %RM,%EM",   "psubd %RM,%EM",  "psubq %RM,%EM",

	"paddb %RM,%EM",    "paddw %RM,%EM",   "paddd %RM,%EM",  0

};



char *second_f30f[]={

// 0

	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,

// 1

	"movss %RX,%EX","movss %Md,%RX","movsldup %RX,%MX",0,

	0,0,"movshdup %RX,%MX",0,

	0,0,0,0,0,0,0,0,

// 2

	0,0,0,0,

	0,0,0,0,

	0,                  0,                 "cvtsi2ss %RX,%Ed",0,

	"cvttss2si %Rd,%EX","cvtss2si %Rd,%EX",0,                 0,

// 3

	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,

// 4

	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,

// 5

	0,              "sqrtss %RX,%EX","rsqrtss %RX,%EX", "rcpss %RX,%EX",

	0,              0,               0,                 0,

	"addss %RX,%EX","mulss %RX,%EX", "cvtss2sd %RX,%EX","cvttps2dq %RX,%EX",

	"subss %RX,%EX","minss %RX,%EX", "divss %RX,%EX",   "maxss %RX,%EX",

// 6

	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,"movdqu %RX,%EX",

// 7

	"pshufhw %RX,%EX,%Ib",0,0,0,

	0,0,0,0,

	0,0,0,0,

	0,0,"movq %RX,%EX","movdqu %Md,%RX",

// 8

	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,

// 9

	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,

// a

	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,

// b

	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,

// c

	0,0,"cmpss %RX,%EX,%Ib",0,

	0,0,0,                  0,

	0,0,0,                  0,

	0,0,0,                  0,

// d

	0,0,0,0,

	0,0,"movq2dq RX,RM",0,

	0,0,0,0,0,0,0,0,

// e

	0,0,0,0,

	0,0,"cvtdq2pd %RX,%EX",0,

	0,0,0,0,0,0,0,0,

// f

	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0

};



char *second_f20f[]={

// 0

	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,

// 1

	"movsd %RX,%Md","movsd %Md,%RX","movddup %RX,%MX",0,

	0,0,0,0,

	0,0,0,0,0,0,0,0,

// 2

	0,0,0,0,

	0,0,0,0,

	0,0,"cvtsi2sd %RX,%Md",0,

	"cvttsd2si %Rd,%EX","cvtsd2si %Rd,%EX",0,0,

// 3

	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,

// 4

	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,

// 5

	0,"sqrtsd %RX,%EX",0,0,

	0,0,0,0,

	"addsd %RX,%EX","mulsd %RX,%MX","cvtsd2ss %RX,%EX",0,

	"subsd %RX,%EX","minsd %RX,%EX","divsd %RX,%EX","maxsd %RX,%EX",

// 6

	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,

// 7

	"pshuflw %RX,%EX,%Ib",0,0,0,

	0,0,0,0,

	0,0,0,0,

	"haddps %RX,%EX","hsubps %RX,%EX",0,0,

// 8

	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,

// 9

	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,

// a

	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,

// b

	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,

// c

	0,0,"cmpsd %RX,%EX,%Ib",0,

	0,0,0,0,

	0,0,0,0,

	0,0,0,0,

// d

	"addsubps %RX,%EX",0,0,0,

	0,0,"movdq2q %RM,%RX",0,

	0,0,0,0,

	0,0,0,0,

// e

	0,0,0,0,

	0,0,"cvtpd2dq %RX,%EX",0,

	0,0,0,0,0,0,0,0,

// f

	"lddqu %Md,%RX",0,0,0,

	0,0,0,0,

	0,0,0,0,

	0,0,0,0

};



char *second_660f[]={

// 0

	0,0,0,0,

	0,0,0,0,

	0,0,0,0,

	0,0,0,0,

// 1

	"movupd %RX,%Md","movupd %Md,%RX","movlpd %RX,%Md","movlpd %Md,%RX",

	"unpcklpd %RX,%EX","unpckhpd %RX,%EX","movhpd %RX,%Md","movhpd %Md,%RX",

	0,0,0,0,

	0,0,0,0,

// 2

	0,0,0,0,

	0,0,0,0,

	"movapd %RX,%EX",   "movapd %Md,%RX",  "cvtpi2pd %RX,%EM","movntpd %Md,%RX",

	"cvttpd2pi %RM,%EX","cvtpd2pi %RM,%EX","ucomisd %RX,%EX","comisd %RX,%EX",

// 3

	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,

// 4

	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,

// 5

	"movmskpd %Rd,%GX","sqrtpd %RX,%EX",0,0,

	"andpd %RX,%EX","andnpd %RX,%EX","orpd %RX,%MX","xorpd %RX,%EX",

	"addpd %RX,%EX","mulpd %RX,%MX","cvtpd2ps %RX,%EX","cvtps2dq %RX,%EX",

	"subpd %RX,%EX","minpd %RX,%EX","divpd %RX,%EX","maxpd %RX,%EX",

// 6

	"punpcklbw %RX,%EX", "punpcklwd %RX,%EX", "punpckldq %RX,%EX","packsswb %RX,%EX",

	"pcmpgtb %RX,%EX",   "pcmpgtw %RX,%EX",   "pcmpgtd %RX,%EX",  "packuswb %RX,%EX",

	"punpckhbw %RX,%EX", "punpckhwd %RX,%EX", "punpckhdq %RX,%EX","packssdw %RX,%EX",

	"punpcklqdq %RX,%EX","punpckhqdq %RX,%EX","movd %RX,%Md",     "movdqa %RX,%EX",

// 7

	"pshufd %RX,%EX,%Ib","%g3w %EX,%Ib",    "%g3d %EX,%Ib","%g9",

	"pcmpeqb %RX,%EX",  "pcmpeqw %RX,%EX", "pcmpeqd %RX,%EX",0,

	0,0,0,0,

	"haddpd %RX,%EX","hsubpd %RX,%EX","movd %Md,%RX","movdqa %Md,%RX",

// 8

	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,

// 9

	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,

// a

	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,

// b

	0,0,0,0,0,0,0,0,

	0,0,0,0,0,0,0,0,

// c

	0,0,"cmppd %RX,%EX,%Ib",0,

	"pinsrw %LX,%Fd,%Ib","pextrw %Gd,%RX,%Ib","shufpd %RX,%EX,%Ib",0,

	0,0,0,0,

	0,0,0,0,

// d

	"addsubpd %RX,%EX","psrlw %RX,%EX",  "psrld %RX,%EX", "psrlq %RX,%EX",

	"paddq %RX,%EX",   "pmullw %RX,%EX", "movq %Md,%RX",  "pmovmskb %Rd,%GX",

	"psubusb %RX,%EX", "psubusw %RX,%EX","pminub %RX,%EX","pand %RX,%EX",

	"paddusb %RX,%EX", "paddusw %RX,%EX","pmaxub %RX,%EX","pandn %RX,%EX",

// e

	"pavgb %RX,%EX",   "psraw %RX,%EX", "psrad %RX,%EX",    "pavgw %RX,%EX",

	"pmulhuw %RX,%EX", "pmulhw %RX,%EX","cvttpd2dq %RX,%EX","movntdq %Md,%RX",

	"psubsb %RX,%EX",  "psubsw %RX,%EX","pminsw %RX,%EX",   "por %RX,%EX",

	"paddsb %RX,%EX",  "paddsw %RX,%EX","pmaxsw %RX,%EX",   "pxor %RX,%EX",

// f

	0,                 "psllw %RX,%EX",  "pslld %RX,%EX", "psllq %RX,%EX",

	"pmuludq %RX,%EX", "pmaddwd %RX,%EX","psadbw %RX,%EX","maskmovdqu %RX,%RX",

	"psubb %RX,%EX",   "psubw %RX,%EX",  "psubd %RX,%EX", "psubq %RX,%EX",

	"paddb %RX,%EX",   "paddw %RX,%EX",  "paddd %RX,%EX", 0

};



char *groups[][8] = {   /* group 0 is group 3 for %Ev set */

/* 0 */

  { "add",            "or",              "adc",            "sbb",

    "and",            "sub",             "xor",            "cmp"           },

/* 1 */

  { "rol",            "ror",             "rcl",            "rcr",

    "shl",            "shr",             "shl",            "sar"           },

/* 2 */  /* v   v*/

  { "test %Eq,%Iq",   0/*"test %Eq,%Iq"*/,    "not %Ec",        "neg %Ec",

    "mul %Ec",        "imul %Ec",        "div %Ec",        "idiv %Ec" },

/* 3 */

  { "inc %Eb",        "dec %Eb",         "psrl",           0,

    "psra",           0,                 "psll",           0          },

/* 4 */

  { "inc %Ev",        "dec %Ev",         "call %Kn%Ev",  "call %Kf%Ep",

    "jmp %Kn%Ev",     "jmp %Kf%Ep",      "push %Ev",       0               },

/* 5 */

  { "sldt %Ew",       "str %Ew",         "lldt %Ew",       "ltr %Ew",

    "verr %Ew",       "verw %Ew",        0,                0               },

/* 6 */

  { "sgdt %Ms",       "sidt %Ms",        "lgdt %Ms",       "lidt %Ms",

    "smsw %Ew",       0,                 "lmsw %Ew",       "invlpg %Em"    },

/* 7 */

  { "prefetchnta %Em","prefetcht0 %Em",  "prefetcht1 %Em", "prefetcht2 %Em",

    "bt",             "bts",             "btr",            "btc"           },

/* 8 */

	{ "fxsave %Em",    "fxrstor %Em",      "ldmxcsr %Em",    "stmxcsr %Em",

		0,               "lfence",           "mfence",         "%x2"        },

/* 9 */

	{

		0,0,"psrlq %RX,%Ib","psrldq %RX,%Ib",

		0,0,"psllq %RX,%Ib","pslldq %RX,%Ib"

	}



};



/* zero here means invalid.  If first entry starts with '*', use st(i) */

/* no assumed %EFs here.  Indexed by RM(modrm())                       */

char *f0[]     = { 0, 0, 0, 0, 0, 0, 0, 0};

char *fop_9[]  = { "*fxch st,%GF",0,0,0,0,0,0,0 };

char *fop_10[] = { "fnop", 0, 0, 0, 0, 0, 0, 0 };

char *fop_12[] = { "fchs", "fabs", 0, 0, "ftst", "fxam", 0, 0 };

char *fop_13[] = { "fld1", "fldl2t", "fldl2e", "fldpi",

                   "fldlg2", "fldln2", "fldz", 0 };

char *fop_14[] = { "f2xm1", "fyl2x", "fptan", "fpatan",

                   "fxtract", "fprem1", "fdecstp", "fincstp" };

char *fop_15[] = { "fprem", "fyl2xp1", "fsqrt", "fsincos",

                   "frndint", "fscale", "fsin", "fcos" };

char *fop_21[] = { 0, "fucompp", 0, 0, 0, 0, 0, 0 };

char *fop_28[] = { "fneni", "fndisi", "fnclex", "fninit", "fnsetpm", 0, 0, 0 };

char *fop_32[] = { "*fadd %GF,st",0,0,0,0,0,0,0 };

char *fop_33[] = { "*fmul %GF,st",0,0,0,0,0,0,0 };

char *fop_36[] = { "*fsubr %GF,st",0,0,0,0,0,0,0 };

char *fop_37[] = { "*fsub %GF,st",0,0,0,0,0,0,0 };

char *fop_38[] = { "*fdivr %GF,st",0,0,0,0,0,0,0 };

char *fop_39[] = { "*fdiv %GF,st",0,0,0,0,0,0,0 };

char *fop_40[] = { "*ffree %GF",0,0,0,0,0,0,0 };

char *fop_42[] = { "*fst %GF",0,0,0,0,0,0,0 };

char *fop_43[] = { "*fstp %GF",0,0,0,0,0,0,0 };

char *fop_44[] = { "*fucom %GF",0,0,0,0,0,0,0 };

char *fop_45[] = { "*fucomp %GF",0,0,0,0,0,0,0 };

char *fop_48[] = { "*faddp %GF,st",0,0,0,0,0,0,0 };

char *fop_49[] = { "*fmulp %GF,st",0,0,0,0,0,0,0 };

char *fop_51[] = { 0, "fcompp", 0, 0, 0, 0, 0, 0 };

char *fop_52[] = { "*fsubrp %GF,st",0,0,0,0,0,0,0 };

char *fop_53[] = { "*fsubp %GF,st",0,0,0,0,0,0,0 };

char *fop_54[] = { "*fdivrp %GF,st",0,0,0,0,0,0,0 };

char *fop_55[] = { "*fdivp %GF,st",0,0,0,0,0,0,0 };

char *fop_60[] = { "fnstsw ax", 0, 0, 0, 0, 0, 0, 0 };

char *fop_16[]={"*fcmovb st,%GF",0,0,0,0,0,0,0};

char *fop_17[]={"*fcmove st,%GF",0,0,0,0,0,0,0};

char *fop_18[]={"*fcmovbe st,%GF",0,0,0,0,0,0,0};

char *fop_19[]={"*fcmovu st,%GF",0,0,0,0,0,0,0};

char *fop_24[]={"*fcmovnb st,%GF",0,0,0,0,0,0,0};

char *fop_25[]={"*fcmovne st,%GF",0,0,0,0,0,0,0};

char *fop_26[]={"*fcmovnbe st,%GF",0,0,0,0,0,0,0};

char *fop_27[]={"*fcmovnu st,%GF",0,0,0,0,0,0,0};

char *fop_29[]={"*fucomi st,%GF",0,0,0,0,0,0,0};

char *fop_30[]={"*fcomi st,%GF",0,0,0,0,0,0,0};

char *fop_61[]={"*fucomip st,%GF",0,0,0,0,0,0,0};

char *fop_62[]={"*fcomip st,%GF",0,0,0,0,0,0,0};



char **fspecial[] = { /* 0=use st(i), 1=undefined 0 in fop_* means undefined */

  0, 0, 0, 0, 0, 0, 0, 0,

  0, fop_9, fop_10, 0, fop_12, fop_13, fop_14, fop_15,

  fop_16, fop_17, fop_18, fop_19, f0, fop_21, f0, f0,

  fop_24, fop_25, fop_26, fop_27, fop_28, fop_29, fop_30, f0,

  fop_32, fop_33, f0, f0, fop_36, fop_37, fop_38, fop_39,

  fop_40, f0, fop_42, fop_43, fop_44, fop_45, f0, f0,

  fop_48, fop_49, f0, fop_51, fop_52, fop_53, fop_54, fop_55,

  f0, f0, f0, f0, fop_60, fop_61, fop_62, f0

};



char *floatops[] = { /* assumed " %EF" at end of each.  mod != 3 only */

/*00*/ "fadd", "fmul", "fcom", "fcomp",

       "fsub", "fsubr", "fdiv", "fdivr",

/*08*/ "fld", 0, "fst", "fstp",

       "fldenv", "fldcw", "fnstenv", "fnstcw",

/*16*/ "fiadd", "fimul", "ficomw", "ficompw",

       "fisub", "fisubr", "fidiv", "fidivr",

/*24*/ "fild", 0, "fist", "fistp",

       "frstor", "fldt", 0, "fstpt",

/*32*/ "faddq", "fmulq", "fcomq", "fcompq",

       "fsubq", "fsubrq", "fdivq", "fdivrq",

/*40*/ "fldq", 0, "fstq", "fstpq",

       0, 0, "fnsave", "fnstsw",

/*48*/ "fiaddw", "fimulw", "ficomw", "ficompw",

       "fisubw", "fisubrw", "fidivw", "fidivr",

/*56*/ "fildw", 0, "fistw", "fistpw",

       "fbldt", "fildq", "fbstpt", "fistpq"

};



/* variables controlled by command line flags */

unsigned char seg_size=16;   /* default size is 16 */

unsigned char must_do_size;



unsigned int wordop,qwordop;           /* dealing with word or byte operand */

unsigned long instruction_offset;

unsigned short done_space; /* for opcodes with > one space */



char ubuf[100],*ubufp;

int col;               /* output column */

unsigned int prefix;            /* segment override prefix byte */

unsigned int modrmv;            /* flag for getting modrm byte */

unsigned int sibv;              /* flag for getting sib byte   */

unsigned int opsize;            /* just like it says ...       */

unsigned int addrsize;



void printbyte(unsigned char c)

{

	if(c<10)uprintf("%u",(unsigned char)c);

	else if(c<16||c>0x9F)uprintf("0%Xh",(unsigned char)c);

	else uprintf("%Xh",(unsigned char)c);

}



void printword(unsigned short c)

{

	if(c<256)printbyte(c);

	else if(c<0xA00||(c>0xFFF&&c<0xA000))uprintf("%Xh",c);

	else uprintf("0%Xh",c);

}



void printdword(unsigned int c)

{

	if(c<65536)printword((unsigned short)c);

	else if(c<0xA0000L||(c>0xFFFFFL&&c<0xA00000L)||(c>0xFFFFFFL&&c<0xA000000L)||

			(c>0xFFFFFFFL&&c<0xA0000000L))uprintf("%lXh",c);

	else uprintf("0%lXh",c);

}



void addr_to_hex(long addr, unsigned char splitup)

{

static char buffer[11];

WORD32 adr;

  adr.dword=addr;

  if(splitup){

    if(adr.w.seg==0/*||adr.w.seg==0xffff*/)printword(adr.w.ofs);//sprintf(buffer,"%04Xh",adr.w.ofs);

    else{

			sprintf(buffer,"%04Xh:%04Xh",adr.w.seg,adr.w.ofs);

			uprintf("%s",buffer);

		}

  }

	else{

    if(adr.w.seg==0/*||adr.w.seg==0xffff*/)printword(adr.w.ofs);//sprintf(buffer,"%04Xh",adr.w.ofs);

    else printdword(addr);//sprintf(buffer, "%08lXh",addr);

  }

}



unsigned char getbyte(void)

{

short c;

  c=output[outptr++];

  fprintf(hout,"%02X", c);   /* print out byte */

  col+=2;

  instruction_offset++;

  return c;

}



int modrm()

{

  if (modrmv == -1) modrmv = getbyte();

  return modrmv;

}



int sib()

{

  if (sibv == -1) sibv = getbyte();

  return sibv;

}



/*------------------------------------------------------------------------*/

void uprintf(char *s, ...)

{

va_list argptr;

	va_start(argptr,s);

	vsprintf(ubufp,s,argptr);

	va_end(argptr);

  while (*ubufp) ubufp++;

}



void uputchar(char c)

{

  if (c == '\t') {

    if(done_space)uputchar(' ');

		else {

      done_space=1;

      do {

        *ubufp++ = ' ';

      } while ((ubufp-ubuf) % 8);

    }

  }

	else *ubufp++ = c;

  *ubufp = 0;

}



/*------------------------------------------------------------------------*/

int bytes(char c)

{

  switch (c){

  case 'b': return 1;

  case 'w': return 2;

  case 'd': return 4;

  case 'v':

       if (opsize == 32) return 4;

       else return 2;

  }

  return 0;

}



/*------------------------------------------------------------------------*/

void outhex(char subtype, int extend, int optional, int defsize, int sign)

{

int n=0, s=0, i;

long delta;

unsigned char buff[6];

//char *name;

char  signchar;

  switch (subtype) {

  case 'q':

       if (wordop) {

         if (opsize==16) n = 2;

				 else n = 4;

       }

			 else n = 1;

       break;



  case 'a': break;

  case 'x':

       extend = defsize/8;//2;

       n = 1;

       break;

  case 'b':

       n = 1;

       break;

  case 'w':

       n = 2;

       break;

  case 'd':

       n = 4;

       break;

  case 's':

       n = 6;

       break;

  case 'c':

  case 'v':

       if (defsize == 32) n = 4;

       else n = 2;

       break;

  case 'p':

       if (defsize == 32) n = 6;

       else n = 4;

       s = 1;

       break;

  }

  for (i=0; i<n; i++) buff[i] = getbyte();

  for(;i<extend;i++)buff[i]=(buff[i-1]&0x80)?(unsigned char)0xff:(unsigned char)0;

  if (s) {

    uprintf("%02X%02X:", buff[n-1], buff[n-2]);

    n -= 2;

  }

  switch (n) {

  case 1:

       delta = *(signed char *)buff;

       break;

  case 2:

       delta = *(signed short *)buff;

       break;

  case 4:

       delta = *(signed long *)buff;

       break;

  }

  if (extend > n) {

    if (subtype!='x') {

      if ((long)delta<0) {

        delta = -delta;

        signchar = '-';

      }

			else signchar = '+';

      if (delta || !optional){

				uprintf("%c",signchar);

				printdword(delta);

//        uprintf("%c%0*lXh", signchar, extend+1, delta);

			}

    }

		else {

      if (extend==2)

        delta = (unsigned short) delta;

			printdword(delta);

//     uprintf("%0.*lXh", 2*extend+1, delta);

    }

    return;

  }

  if ((n == 4) && !sign) {

    addr_to_hex(delta, 0);

    return;

  }

  switch (n) {

  case 1:

       if (sign && (char)delta<0) {

         delta = -delta;

         signchar = '-';

       }

			 else signchar = '+';

			if(sign)uprintf("%c",signchar);

			printbyte((unsigned char)delta);

//       if (sign)uprintf("%c%03Xh",signchar,(unsigned char)delta);

//       else uprintf("%03Xh", (unsigned char)delta);

       break;

  case 2:

       if (sign && (int)delta<0) {

         signchar = '-';

         delta = -delta;

       }

			 else signchar = '+';

			if(sign)uprintf("%c",signchar);

			printword((unsigned short)delta);

//       if (sign) uprintf("%c%05Xh", signchar,(int)delta);

//       else uprintf("%05Xh", (unsigned int)delta);

       break;

  case 4:

       if (sign && (long)delta<0) {

         delta = -delta;

         signchar = '-';

       }

			 else signchar = '+';

			if(sign)uprintf("%c",signchar);

			printdword(delta);

//       if (sign)uprintf("%c%09lXh", signchar, (unsigned long)delta);

//       else uprintf("%09lXh", (unsigned long)delta);

       break;

  }

}



/*------------------------------------------------------------------------*/

void reg_name(int regnum, char size)

{

  if (size == 'F') { /* floating point register? */

    uprintf("st(%d)", regnum);

    return;

  }

  if (size == 'M') { /* multimedia register? */

    uprintf("mm%d", regnum);

    return;

  }

  if(size=='X'){ /* xmm register? */

    uprintf("xmm%d",regnum);

    return;

  }

  if (((size == 'v') && (opsize == 32)) ||

	     (size == 'd')||

			 ((size=='c'||size=='q')&&wordop&&opsize==32)

			 ) uputchar('e');

//	printf("size=%c wordop=%d opsize=%d\n",size,wordop,opsize);

  if(size=='b'||((size=='q'||size=='c')&&!wordop)){

    uputchar("acdbacdb"[regnum]);

    uputchar("llllhhhh"[regnum]);

  }

	else {

    uputchar("acdbsbsd"[regnum]);

    uputchar("xxxxppii"[regnum]);

  }

}



/*------------------------------------------------------------------------*/

void do_sib(int m)

{

int s, i, b;

  s = SCALE(sib());

  i = INDEX(sib());

  b = BASE(sib());

  switch (b) {     /* pick base */

  case 0: ua_str("%p:[eax"); break;

  case 1: ua_str("%p:[ecx"); break;

  case 2: ua_str("%p:[edx"); break;

  case 3: ua_str("%p:[ebx"); break;

  case 4: ua_str("%p:[esp"); break;

  case 5:

       if (m == 0) {

         ua_str("%p:[");

         outhex('d', 4, 0, addrsize, 0);

       }

			 else ua_str("%p:[ebp");

       break;

  case 6: ua_str("%p:[esi"); break;

  case 7: ua_str("%p:[edi"); break;

  }

  switch (i) {     /* and index */

  case 0: uprintf("+eax"); break;

  case 1: uprintf("+ecx"); break;

  case 2: uprintf("+edx"); break;

  case 3: uprintf("+ebx"); break;

  case 4: break;

  case 5: uprintf("+ebp"); break;

  case 6: uprintf("+esi"); break;

  case 7: uprintf("+edi"); break;

  }

  if (i != 4) {

    switch (s) {    /* and scale */

      case 0: uprintf(""); break;

      case 1: uprintf("*2"); break;

      case 2: uprintf("*4"); break;

      case 3: uprintf("*8"); break;

    }

  }

}



/*------------------------------------------------------------------------*/

void do_modrm(char subtype)

{

int mod = MOD(modrm());

int rm = RM(modrm());

int extend = (addrsize == 32) ? 4 : 2;

  if (mod == 3) { /* specifies two registers */

    reg_name(rm, subtype);

    return;

  }

  if (must_do_size) {

		if(qwordop)ua_str("qword ptr ");

		else{

	    if (wordop) {

  	    if (/*addrsize==32 ||*/ opsize==32) {       /* then must specify size */

    	    ua_str("dword ptr ");

      	}

				else ua_str("word ptr ");

  	  }

			else ua_str("byte ptr ");

		}

  }

  if ((mod == 0) && (rm == 5) && (addrsize == 32)) {/* mem operand with 32 bit ofs */

    ua_str("%p:[");

    outhex('d', extend, 0, addrsize, 0);

    uputchar(']');

    return;

  }

  if ((mod == 0) && (rm == 6) && (addrsize == 16)) { /* 16 bit dsplcmnt */

    ua_str("%p:[");

    outhex('w', extend, 0, addrsize, 0);

    uputchar(']');

    return;

  }

  if ((addrsize != 32) || (rm != 4)) ua_str("%p:[");

  if (addrsize == 16) {

    switch (rm) {

    case 0: uprintf("bx+si"); break;

    case 1: uprintf("bx+di"); break;

    case 2: uprintf("bp+si"); break;

    case 3: uprintf("bp+di"); break;

    case 4: uprintf("si"); break;

    case 5: uprintf("di"); break;

    case 6: uprintf("bp"); break;

    case 7: uprintf("bx"); break;

    }

  }

	else {

    switch (rm) {

    case 0: uprintf("eax"); break;

    case 1: uprintf("ecx"); break;

    case 2: uprintf("edx"); break;

    case 3: uprintf("ebx"); break;

    case 4: do_sib(mod); break;

    case 5: uprintf("ebp"); break;

    case 6: uprintf("esi"); break;

    case 7: uprintf("edi"); break;

    }

  }

  switch (mod) {

  case 1:

       outhex('b', extend, 1, addrsize, 0);

       break;

  case 2:

       outhex('v', extend, 1, addrsize, 1);

       break;

  }

  uputchar(']');

}



/*------------------------------------------------------------------------*/

void floating_point(int e1)

{

int esc = e1*8 + REG(modrm());

  if(MOD(modrm())==3){	//2-‰ €‰’>C0

    if (fspecial[esc]) {

      if (fspecial[esc][0]!=NULL&&fspecial[esc][0][0] == '*')ua_str(fspecial[esc][0]+1);

      else ua_str(fspecial[esc][RM(modrm())]);

    }

		else {

      ua_str(floatops[esc]);

      ua_str(" %EF");

    }

  }

	else {

    ua_str(floatops[esc]);

    ua_str(" %EF");

  }

}



/*------------------------------------------------------------------------*/

/* Main table driver                                                      */

void percent(char type, char subtype)

{

long vofs;

int extend =(addrsize==32)?4:2;

unsigned char c;

	switch (type) {

	case 'A':                          /* direct address */

		outhex(subtype,extend,0,addrsize,0);

		break;

	case 'C':                          /* reg(r/m) picks control reg */

		uprintf("CR%d",REG(modrm()));

		must_do_size=0;

		break;

  case 'D':                          /* reg(r/m) picks debug reg */

       uprintf("DR%d", REG(modrm()));

       must_do_size = 0;

       break;

  case 'E':                          /* r/m picks operand */

			 if(subtype=='m')must_do_size=0;

       do_modrm(subtype);

       break;

	case 'F':

			 if(MOD(modrm())!=3)do_modrm(subtype);

			 else reg_name(REG(modrm()),subtype);

			 break;

  case 'G':                          /* reg(r/m) picks register */

/*       if (subtype == 'F'||subtype == 'M')

         reg_name(RM(modrm()), subtype);

       else reg_name(REG(modrm()), subtype);*/

       reg_name(RM(modrm()),subtype);

       must_do_size = 0;

       break;

  case 'I':                            /* immed data */

       outhex(subtype, 0, 0, opsize, 0);

       break;

  case 'J':                            /* relative IP offset */

       switch(bytes(subtype)) {              /* sizeof offset value */

       case 1:

						vofs=(signed char)getbyte();

            break;

       case 2:

            vofs = getbyte();

            vofs += getbyte()<<8;

            vofs = (short)vofs;

            break;

       case 4:

            vofs = (unsigned long)getbyte();           /* yuk! */

            vofs |= (unsigned long)getbyte() << 8;

            vofs |= (unsigned long)getbyte() << 16;

            vofs |= (unsigned long)getbyte() << 24;

            break;

       }

       addr_to_hex(vofs+instruction_offset,seg_size==16?(unsigned char)1:(unsigned char)0);

       break;

  case 'K':

			 if(seg_size==16){

	       switch(subtype){

  	     case 'f':

    	        ua_str("far ");

      	      break;

	       case 'n':

  	          ua_str("near ");

    	        break;

      	 case 's':

        	    ua_str("short ");

          	  break;

	       }

			 }

			 else if(subtype=='s')ua_str("short ");

       break;

	case 'L':

			 if(MOD(modrm())!=3)reg_name(REG(modrm()),subtype);

			 else reg_name(RM(modrm()),subtype);

			 break;

  case 'M':                            /* r/m picks memory */

       do_modrm(subtype);

       break;

  case 'O':                            /* offset only */

       ua_str("%p:[");

       outhex(subtype, extend, 0, addrsize, 0);

       uputchar(']');

       break;

  case 'P':                            /* prefix byte (rh) */

       ua_str("%p:");

       break;

  case 'R':                            /* mod(r/m) picks register */

       reg_name(REG(modrm()), subtype);      /* rh */

       must_do_size = 0;

       break;

  case 'S':                            /* reg(r/m) picks segment reg */

       uputchar("ecsdfg"[REG(modrm())]);

       uputchar('s');

       must_do_size = 0;

       break;

  case 'T':                            /* reg(r/m) picks T reg */

       uprintf("tr%d", REG(modrm()));

       must_do_size = 0;

       break;

  case 'X':                            /* ds:si type operator */

       uprintf("ds:[");

       if (addrsize == 32) uputchar('e');

       uprintf("si]");

       break;

  case 'Y':                            /* es:di type operator */

       uprintf("es:[");

       if (addrsize == 32) uputchar('e');

       uprintf("di]");

       break;

  case '2':

			 c=getbyte();          /* old [pop cs]! now indexes */

			wordop = c & 1;

       ua_str(second[c]);      /* instructions in 386/486   */

       break;

  case 'd':                             /* sizeof operand==dword? */

       if (opsize == 32) uputchar('d');

       uputchar(subtype);

       break;

  case 'e':                         /* extended reg name */

       if (opsize == 32) {

         if (subtype == 'w')uputchar('d');

         else {

           uputchar('e');

           uputchar(subtype);

         }

       }

			 else uputchar(subtype);

       break;

  case 'f':                    /* '87 opcode */

       floating_point(subtype-'0');

       break;

  case 'j':

       if (/*addrsize==32 ||*/ opsize==32) /* both of them?! */

         uputchar('e');

       break;

  case 'g':                            /* modrm group `subtype' (0--7) */

			 switch(subtype){

				case '9':

					vofs=REG(modrm());

					modrmv=(modrmv&0xC7)|((modrmv&7)<<3);

					ua_str(groups[9][vofs]);

					break;

				case '6':

					 if(MOD(modrm())!=3)goto defg;

					 switch(modrmv){

						case 0xc8:

							ua_str("monitor");

							break;

						case 0xc9:

							ua_str("mwait");

							break;

						default:

					    uprintf("<invalid>");

							break;

					 }

					break;

				case '5':

					opsize=16;

				case '7':

				 	wordop=1;

				default:

defg:

	       ua_str(groups[subtype-'0'][REG(modrm())]);

				 break;

			 }

       break;

  case 'p':                    /* prefix byte */

       switch (subtype)  {

       case 'c':

       case 'd':

       case 'e':

       case 'f':

       case 'g':

       case 's':

            prefix = subtype;

            c = getbyte();

            wordop = c & 1;

            ua_str(opmap1[c]);

            break;

       case ':':

            if (prefix) uprintf("%cs:", prefix);

            break;

       case ' ':

            c = getbyte();

            wordop = c & 1;

            ua_str(opmap1[c]);

            break;

       }

       break;

  case 's':                           /* size override */

       switch (subtype) {

       case 'a':

            addrsize = 48 - addrsize;

            c = getbyte();

            wordop = c & 1;

            ua_str(opmap1[c]);

            break;

       case 'o':

            opsize = 48 - opsize;

            c = getbyte();

            wordop = c & 1;

            ua_str(opmap1[c]);

            break;

       }

       break;

  case 'w':                             /* insert explicit size specifier */

       if (opsize == 32)uputchar('d');

       else uputchar('w');

       uputchar(subtype);

       break;

  case 'x':

			 if(MOD(modrm())==3){

				switch(subtype-'0'){

					case 0:

						ua_str("movhlps %RX,%GX");

						break;

					case 1:

						ua_str("movlhps %RX,%GX");

						break;

					case 2:

						ua_str("sfence");

						break;

				}

			 }

			 else{

				switch(subtype-'0'){

					case 0:

						ua_str("movlps %RX,%Md");

						break;

					case 1:

						ua_str("movhps %RX,%Md");

						break;

					case 2:

						ua_str("clflush %Em");

						break;

				}

			 }

       break;

	default:

			 preerror("uncnown operand for dizassemler");

			 break;

   }

}



void ua_str(char *str)

{

char c;

  if (str == NULL) {

    uprintf("<invalid>");

    return;

  }

  if (strpbrk(str, "CDFGRST")) /* specifiers for registers=>no size 2b specified */

    must_do_size = 0;

  while ((c = *str++) != 0) {

    if (c == '%') {

      c = *str++;

			if(*str=='y'){

				qwordop=1;

				str++;

			}

      percent(c,*str++);

    }

		else {

      if (c == ' ') uputchar('\t');

			else uputchar(c);

    }

  }

}



void unassemble(unsigned long ofs)

{

char *str;

int c;

  fprintf(hout,seg_size==16?"%04X ":"%08lX ",ofs);

  prefix = 0;

  modrmv = sibv = 0xFFFFFFFF;     /* set modrm and sib flags */

  opsize = addrsize = seg_size;

  col = 0;

  ubufp = ubuf;

  done_space = 0;

  c = getbyte();

	if(c==0x9B){

		switch(*(short *)&output[outptr]){

			case 0xE0DB:

			case 0xE1DB:

			case 0xE2DB:

			case 0xE3DB:

			case 0xE4DB:

				getbyte();

			  c=getbyte();

				switch(c){

					case 0xE0:

						ua_str("feni");

						break;

					case 0xE1:

						ua_str("fdisi");

						break;

					case 0xE2:

						ua_str("fclex");

						break;

					case 0xE3:

						ua_str("finit");

						break;

					case 0xE4:

						ua_str("fsetpm");

						break;

				}

				goto endp;

		}

	}

	else if(c==0x98){

		if(am32){

			ua_str("cwde");

			goto endp;

		}

	}

	else if(c==0x99){

		if(am32){

			ua_str("cdq");

			goto endp;

		}

	}

	else if(c==0x66&&am32==0){

		if(output[outptr]==0x98){

			ua_str("cwde");

			getbyte();

			goto endp;

		}

		if(output[outptr]==0x99){

			ua_str("cdq");

			getbyte();

			goto endp;

		}

	}

	else if(c==0xF3&&output[outptr]==0x90){

		ua_str("pause");

		getbyte();

		goto endp;

	}

	if(output[outptr]==0x0f){

		int c2=output[outptr+1];

		switch(c){

			case 0x66:

				if(second_660f[c2]){

					getbyte();

					ua_str(second_660f[getbyte()]);

					goto endp;

				}

				break;

			case 0xf2:

				if(second_f20f[c2]){

					getbyte();

					ua_str(second_f20f[getbyte()]);

					goto endp;

				}

				break;

			case 0xf3:

				if(second_f30f[c2]){

					getbyte();

					ua_str(second_f30f[getbyte()]);

					goto endp;

				}

				break;

		}

	}

  wordop = c & 1;

	qwordop=0;

  must_do_size = 1;

  if ((str = opmap1[c])==NULL) {    /* invalid instruction? */

    uputchar('d');                  /* then output byte defines */

    uputchar('b');

    uputchar('\t');

    uprintf("%02Xh",c);

  }

	else ua_str(str);                      /* valid instruction */

endp:

  fprintf(hout,"%*s", 25-col, " ");

  fprintf(hout,"%s\n", ubuf);

}



#define MAXSTR 12



void undata(unsigned ofs,unsigned long len,unsigned int type)

{

unsigned int sizet,c,i,j;

unsigned long total,d;

unsigned char data[MAXSTR];

	if(type==3)sizet=1;

	else sizet=type;

	for(total=0;total<len;){

		col=0;

		ubufp=ubuf;

	  fprintf(hout,seg_size==16?"%04X ":"%08lX ",ofs);

		if((total+sizet)>len){

			type=sizet=1;

		}

		for(i=0;i<MAXSTR;){

			switch(sizet){

				case 1:

		  		data[i++]=getbyte();

					break;

				case 2:

		  		data[i++]=getbyte();

		  		data[i++]=getbyte();

					total++;

					break;

				case 4:

		  		data[i++]=getbyte();

		  		data[i++]=getbyte();

		  		data[i++]=getbyte();

		  		data[i++]=getbyte();

					total+=3;

					break;

				case 8:

		  		data[i++]=getbyte();

		  		data[i++]=getbyte();

		  		data[i++]=getbyte();

		  		data[i++]=getbyte();

		  		data[i++]=getbyte();

		  		data[i++]=getbyte();

		  		data[i++]=getbyte();

		  		data[i++]=getbyte();

					total+=7;

					break;

			}

			total++;

			if((total+sizet)>len)break;

			if(sizet==8)break;

		}

    uputchar(sizet==8?'q':'d');

		switch(sizet){

			case 1:

		    uputchar('b');

				break;

			case 2:

			case 8:

		    uputchar('w');

				break;

			case 4:

		    uputchar('d');

				break;

		}

	  done_space=0;

    uputchar('\t');

		switch(type){

			case 1:

				for(j=0;j<i;j++){

					printbyte(data[j]);

					if((j+1)!=i)uputchar(',');

				}

				break;

			case 2:

				for(j=0;j<i;){

					d=data[j++];

					d+=data[j++]<<8;

					printword((unsigned short)d);

//					uprintf("%05Xh",(unsigned int)d);

					if(j!=i)uputchar(',');

				}

				break;

			case 3:

				c=0;

				for(j=0;j<i;j++){

					if(data[j]>=0x20){

						if(c==0){

							c++;

							if(j!=0)uputchar(',');

							uputchar(0x27);

						}

						uputchar(data[j]);

					}

					else{

						if(c!=0){

							c=0;

							uputchar(0x27);

						}

						if(j!=0)uputchar(',');

						printbyte(data[j]);

					}

				}

				if(c)uputchar(0x27);

				break;

			case 4:

				for(j=0;j<i;){

					d=data[j++];

					d+=data[j++]<<8;

					d+=data[j++]<<16;

					d+=data[j++]<<24;

					printdword((unsigned int)d);

//					uprintf("%09lXh",(unsigned int)d);

					if(j!=i)uputchar(',');

				}

				break;

			case 8:

				for(j=0;j<i;){

					j+=4;

					d=data[j++];

					d+=data[j++]<<8;

					d+=data[j++]<<16;

					d+=data[j++]<<24;

					if(d)uprintf("%lX",d);

					unsigned long d2;

					d2=d;

					j-=8;

					d=data[j++];

					d+=data[j++]<<8;

					d+=data[j++]<<16;

					d+=data[j++]<<24;

					if(d2==0)

						uprintf("%lXh",(char)d);

					else

						uprintf("%08lXh",(char)d);

					j+=4;

					if(j!=i)uputchar(',');

				}

				break;

		}

	  fprintf(hout,"%*s",25-col," ");

  	fprintf(hout,"%s\n", ubuf);

		ofs+=i;

	}

}

