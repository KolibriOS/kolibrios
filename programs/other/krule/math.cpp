#include <math.h>
#include "kosSyst.h"
extern "C" int _fltused = 0;
double acos(double x)
{
	__asm {
		fld	qword ptr [esp+4]
		fld1
		fadd	st, st(1)
		fld1
		fsub	st, st(2)
		fmulp	st(1), st
		fsqrt
		fxch	st(1)
		fpatan
	}
}
double asin(double x)
{
	__asm {
		fld	qword ptr [esp+4]
		fld1
		fadd	st, st(1)
		fld1
		fsub	st, st(2)
		fmulp	st(1), st
		fsqrt
		fpatan
		ret
	}
}
#if _MSC_VER <= 1200
extern "C" double _ftol(double x)
{
	__asm {
		fld	qword ptr [esp+4]
		push	1F3Fh
		fstcw	word ptr [esp+2]
		fldcw	word ptr [esp]
		frndint
		fldcw	word ptr [esp+2]
		add	esp, 4
	}
}
#endif
double ceil(double x)
{
	__asm {
		fld	qword ptr [esp+4]
		push	1B3Fh
		fstcw	word ptr [esp+2]
		fldcw	word ptr [esp]
		frndint
		fldcw	word ptr [esp+2]
		add	esp, 4
	}
}

double floor(double x)
{
	__asm {
		fld	qword ptr [esp+4]
		push	173Fh
		fstcw	word ptr [esp+2]
		fldcw	word ptr [esp]
		frndint
		fldcw	word ptr [esp+2]
		add	esp, 4
	}
}

double round(double x)
{
	__asm {
		fld	qword ptr [esp+4]
		push	133Fh
		fstcw	word ptr [esp+2]
		fldcw	word ptr [esp]
		frndint
		fldcw	word ptr [esp+2]
		add	esp, 4
	}
}
