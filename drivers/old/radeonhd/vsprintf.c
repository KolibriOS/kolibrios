/*
 * vsprintf - print formatted output without ellipsis on an array
 */
/* $Header$ */

//#include  "stdio.h"
//#include  <stdarg.h>

typedef unsigned size_t;

typedef struct
{
  char *_ptr;
  unsigned _count;
}__str;


#define va_start(v,l) __builtin_va_start(v,l)
#define va_end(v)	__builtin_va_end(v)
#define va_arg(v,l)	__builtin_va_arg(v,l)
#if !defined(__STRICT_ANSI__) || __STDC_VERSION__ + 0 >= 199900L
#define va_copy(d,s)	__builtin_va_copy(d,s)
#endif
#define __va_copy(d,s)	__builtin_va_copy(d,s)

typedef __builtin_va_list __gnuc_va_list;
typedef __gnuc_va_list va_list;

#define arg(x) va_arg (ap, u32)

#define	io_testflag(p,x)	((p)->_flags & (x))

char *_i_compute(unsigned long val, int base, char *s, int nrdigits);
char *_f_print(va_list *ap, int flags, char *s, char c, int precision);
void __cleanup(void);


void *calloc( size_t num, size_t size );
int memcmp(const void *s1, const void *s2, size_t n);
void * memcpy(void * _dest, const void *_src, size_t _n);
char * strcpy(char *to, const char *from);
char * strcat(char *s, const char *append);
int strcmp(const char *s1, const char *s2);
size_t strlen(const char *str);
char * strdup(const char *_s);
char * strchr(const char *s, int c);


#define	FL_LJUST	0x0001		/* left-justify field */
#define	FL_SIGN		0x0002		/* sign in signed conversions */
#define	FL_SPACE	0x0004		/* space in signed conversions */
#define	FL_ALT		0x0008		/* alternate form */
#define	FL_ZEROFILL	0x0010		/* fill with zero's */
#define	FL_SHORT	0x0020		/* optional h */
#define	FL_LONG		0x0040		/* optional l */
#define	FL_LONGDOUBLE	0x0080		/* optional L */
#define	FL_WIDTHSPEC	0x0100		/* field width is specified */
#define	FL_PRECSPEC	0x0200		/* precision is specified */
#define FL_SIGNEDCONV	0x0400		/* may contain a sign */
#define	FL_NOASSIGN	0x0800		/* do not assign (in scanf) */
#define FL_NOMORE 0x1000    /* all flags collected */

#define	_IOFBF		0x000
#define	_IOREAD		0x001
#define	_IOWRITE	0x002
#define	_IONBF		0x004
#define	_IOMYBUF	0x008
#define	_IOEOF		0x010
#define	_IOERR		0x020
#define	_IOLBF		0x040
#define	_IOREADING	0x080
#define	_IOWRITING	0x100
#define	_IOAPPEND	0x200
#define _IOFIFO		0x400

#define SGL_MAX      254  /*  standard definition */
#define	SGL_MIN		     1	/*	standard definition	*/
#define	DBL_MAX		  2046	/*	standard definition	*/
#define	DBL_MIN		     1	/*	standard definition	*/
#define EXT_MAX		 16383	/*	standard minimum	*/
#define EXT_MIN   -16382  /*  standard minimum  */
#include	<limits.h>

static char *cvt();
#define NDIGITS	128

unsigned char __dj_ctype_toupper[] = {
  0x00,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
  0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
  0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
  0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
  0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
  0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
  0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
  0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
  0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
  0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
  0x58, 0x59, 0x5a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
  0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
  0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
  0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
  0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
  0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
  0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
  0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
  0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
  0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
  0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
  0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
  0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
  0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
  0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
  0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
  0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};
#define toupper(c) (__dj_ctype_toupper[(int)(c)+1])

int (toupper)(int c)
{
  return toupper(c);
}

char *
_ecvt(value, ndigit, decpt, sign)
	double value;
	int ndigit, *decpt, *sign;
{
	return cvt(value, ndigit, decpt, sign, 1);
}

char *
_fcvt(value, ndigit, decpt, sign)
	double value;
	int ndigit, *decpt, *sign;
{
	return cvt(value, ndigit, decpt, sign, 0);
}

static struct powers_of_10 {
	double pval;
	double rpval;
	int exp;
} p10[] = {
	1.0e32, 1.0e-32, 32,
	1.0e16, 1.0e-16, 16,
	1.0e8, 1.0e-8, 8,
	1.0e4, 1.0e-4, 4,
	1.0e2, 1.0e-2, 2,
	1.0e1, 1.0e-1, 1,
	1.0e0, 1.0e0, 0
};

static char *
cvt(value, ndigit, decpt, sign, ecvtflag)
	double value;
	int ndigit, *decpt, *sign;
{
	static char buf[NDIGITS+1];
	register char *p = buf;
	register char *pe;

	if (ndigit < 0) ndigit = 0;
	if (ndigit > NDIGITS) ndigit = NDIGITS;
	pe = &buf[ndigit];
	buf[0] = '\0';

	*sign = 0;
	if (value < 0) {
		*sign = 1;
		value = -value;
	}

	*decpt = 0;
	if (value >= DBL_MAX) {
		value = DBL_MAX;
	}
	if (value != 0.0) {
		register struct powers_of_10 *pp = &p10[0];

		if (value >= 10.0) do {
			while (value >= pp->pval) {
				value *= pp->rpval;
				*decpt += pp->exp;
			}
		} while ((++pp)->exp > 0);

		pp = &p10[0];
		if (value < 1.0) do {
			while (value * pp->pval < 10.0) {
				value *= pp->pval;
				*decpt -= pp->exp;
			}
		} while ((++pp)->exp > 0);

		(*decpt)++;	/* because now value in [1.0, 10.0) */
	}
	if (! ecvtflag) {
		/* for fcvt() we need ndigit digits behind the dot */
		pe += *decpt;
		if (pe > &buf[NDIGITS]) pe = &buf[NDIGITS];
	}
	while (p <= pe) {
		*p++ = (int)value + '0';
		value = 10.0 * (value - (int)value);
	}
	if (pe >= buf) {
		p = pe;
		*p += 5;	/* round of at the end */
		while (*p > '9') {
			*p = '0';
			if (p > buf) ++*--p;
			else {
				*p = '1';
				++*decpt;
				if (! ecvtflag) {
					/* maybe add another digit at the end,
					   because the point was shifted right
					*/
					if (pe > buf) *pe = '0';
					pe++;
				}
			}
		}
		*pe = '\0';
	}
	return buf;
}


int _fp_hook = 1;

static char *
_pfloat(double r, register char *s, int n, int flags)
{
	register char *s1;
	int sign, dp;
	register int i;

	s1 = _fcvt(r, n, &dp, &sign);
	if (sign)
		*s++ = '-';
	else if (flags & FL_SIGN)
		*s++ = '+';
	else if (flags & FL_SPACE)
		*s++ = ' ';

	if (dp<=0)
		*s++ = '0';
	for (i=dp; i>0; i--)
		if (*s1) *s++ = *s1++;
		else *s++ = '0';
	if (((i=n) > 0) || (flags & FL_ALT))
		*s++ = '.';
	while (++dp <= 0) {
		if (--i<0)
			break;
		*s++ = '0';
	}
	while (--i >= 0)
		if (*s1) *s++ = *s1++;
		else *s++ = '0';
	return s;
}

static char *
_pscien(double r, register char *s, int n, int flags)
{
	int sign, dp;
	register char *s1;

	s1 = _ecvt(r, n + 1, &dp, &sign);
	if (sign)
		*s++ = '-';
	else if (flags & FL_SIGN)
		*s++ = '+';
	else if (flags & FL_SPACE)
		*s++ = ' ';

	*s++ = *s1++;
	if ((n > 0) || (flags & FL_ALT))
		*s++ = '.';
	while (--n >= 0)
		if (*s1) *s++ = *s1++;
		else *s++ = '0';
	*s++ = 'e';
	if ( r != 0 ) --dp ;
	if ( dp<0 ) {
		*s++ = '-' ; dp= -dp ;
	} else {
		*s++ = '+' ;
	}
	if (dp >= 100) {
		*s++ = '0' + (dp / 100);
		dp %= 100;
	}
	*s++ = '0' + (dp/10);
	*s++ = '0' + (dp%10);
	return s;
}

#define	NDIGINEXP(exp)		(((exp) >= 100 || (exp) <= -100) ? 3 : 2)
#define	LOW_EXP			-4
#define	USE_EXP(exp, ndigits)	(((exp) < LOW_EXP + 1) || (exp >= ndigits + 1))

static char *
_gcvt(double value, int ndigit, char *s, int flags)
{
	int sign, dp;
	register char *s1, *s2;
	register int i;
	register int nndigit = ndigit;

	s1 = _ecvt(value, ndigit, &dp, &sign);
	s2 = s;
	if (sign) *s2++ = '-';
	else if (flags & FL_SIGN)
		*s2++ = '+';
	else if (flags & FL_SPACE)
		*s2++ = ' ';

	if (!(flags & FL_ALT))
		for (i = nndigit - 1; i > 0 && s1[i] == '0'; i--)
			nndigit--;

	if (USE_EXP(dp,ndigit))	{
		/* Use E format */
		dp--;
		*s2++ = *s1++;
		if ((nndigit > 1) || (flags & FL_ALT)) *s2++ = '.';
		while (--nndigit > 0) *s2++ = *s1++;
		*s2++ = 'e';
		if (dp < 0) {
			*s2++ = '-';
			dp = -dp;
		}
		else	 *s2++ = '+';
		s2 += NDIGINEXP(dp);
		*s2 = 0;
		for (i = NDIGINEXP(dp); i > 0; i--) {
			*--s2 = dp % 10 + '0';
			dp /= 10;
		}
		return s;
	}
	/* Use f format */
	if (dp <= 0) {
		if (*s1 != '0')	{
			/* otherwise the whole number is 0 */
			*s2++ = '0';
			*s2++ = '.';
		}
		while (dp < 0) {
			dp++;
			*s2++ = '0';
		}
	}
	for (i = 1; i <= nndigit; i++) {
		*s2++ = *s1++;
		if (i == dp) *s2++ = '.';
	}
	if (i <= dp) {
		while (i++ <= dp) *s2++ = '0';
		*s2++ = '.';
	}
	if ((s2[-1]=='.') && !(flags & FL_ALT)) s2--;
	*s2 = '\0';
	return s;
}

char *
_f_print(va_list *ap, int flags, char *s, char c, int precision)
{
	register char *old_s = s;
  double ld_val;

 // if (flags & FL_LONGDOUBLE) ld_val = va_arg(*ap, double);
//  else
   ld_val = (double) va_arg(*ap, double);

	switch(c) {
	case 'f':
		s = _pfloat(ld_val, s, precision, flags);
		break;
	case 'e':
	case 'E':
		s = _pscien(ld_val, s, precision , flags);
		break;
	case 'g':
	case 'G':
		s = _gcvt(ld_val, precision, s, flags);
		s += strlen(s);
		break;
	}
	if ( c == 'E' || c == 'G') {
		while (*old_s && *old_s != 'e') old_s++;
		if (*old_s == 'e') *old_s = 'E';
	}
	return s;
}

//#endif  /* NOFLOAT */
/* $Header$ */

//#include <stdlib.h>
//#include "../ansi/ext_fmt.h"

//void _str_ext_cvt(const char *s, char **ss, struct EXTEND *e);
//double _ext_dbl_cvt(struct EXTEND *e);

//double
//strtod(const char *p, char **pp)
//{
//  struct EXTEND e;

//  _str_ext_cvt(p, pp, &e);
//  return _ext_dbl_cvt(&e);
//}

#define BUFSIZ    4096
#define	NULL		((void *)0)
#define	EOF		(-1)


/* gnum() is used to get the width and precision fields of a format. */
static const char *
gnum(register const char *f, int *ip, va_list *app)
{
	register int	i, c;

	if (*f == '*') {
		*ip = va_arg((*app), int);
		f++;
	} else {
		i = 0;
		while ((c = *f - '0') >= 0 && c <= 9) {
			i = i*10 + c;
			f++;
		}
		*ip = i;
	}
	return f;
}

#if	_EM_WSIZE == _EM_PSIZE
#define set_pointer(flags)				/* nothing */
#elif	_EM_LSIZE == _EM_PSIZE
#define set_pointer(flags)	(flags |= FL_LONG)
#else
#error garbage pointer size
#define set_pointer(flags)		/* compilation might continue */
#endif

/* print an ordinal number */
static char *
o_print(va_list *ap, int flags, char *s, char c, int precision, int is_signed)
{
	long signed_val;
	unsigned long unsigned_val;
	char *old_s = s;
	int base;

	switch (flags & (FL_SHORT | FL_LONG)) {
	case FL_SHORT:
		if (is_signed) {
			signed_val = (short) va_arg(*ap, int);
		} else {
			unsigned_val = (unsigned short) va_arg(*ap, unsigned);
		}
		break;
	case FL_LONG:
		if (is_signed) {
			signed_val = va_arg(*ap, long);
		} else {
			unsigned_val = va_arg(*ap, unsigned long);
		}
		break;
	default:
		if (is_signed) {
			signed_val = va_arg(*ap, int);
		} else {
			unsigned_val = va_arg(*ap, unsigned int);
		}
		break;
	}

	if (is_signed) {
		if (signed_val < 0) {
			*s++ = '-';
			signed_val = -signed_val;
		} else if (flags & FL_SIGN) *s++ = '+';
		else if (flags & FL_SPACE) *s++ = ' ';
		unsigned_val = signed_val;
	}
	if ((flags & FL_ALT) && (c == 'o')) *s++ = '0';
	if (!unsigned_val && c != 'p') {
		 if (!precision)
			return s;
	} else if (((flags & FL_ALT) && (c == 'x' || c == 'X'))
		    || c == 'p') {
		*s++ = '0';
		*s++ = (c == 'X' ? 'X' : 'x');
	}

	switch (c) {
	case 'b':	base = 2;	break;
	case 'o':	base = 8;	break;
	case 'd':
	case 'i':
	case 'u':	base = 10;	break;
	case 'x':
	case 'X':
	case 'p':	base = 16;	break;
	}

	s = _i_compute(unsigned_val, base, s, precision);

	if (c == 'X')
		while (old_s != s) {
			*old_s = toupper(*old_s);
			old_s++;
		}

	return s;
}


#define putc(c, p)  (--(p)->_count >= 0 ?  (int) (*(p)->_ptr++ = (c)) : EOF)

int
_doprnt(register const char *fmt, va_list ap, __str *stream)
{
	register char	*s;
	register int	j;
	int		i, c, width, precision, zfill, flags, between_fill;
	int		nrchars=0;
	const char	*oldfmt;
  char    *s1, buf[512];

  while (c = *fmt++)
  {
    if (c != '%')
    {
      if (c == '\n')
      {
				if (putc('\r', stream) == EOF)
					return nrchars ? -nrchars : -1;
				nrchars++;
      }
			if (putc(c, stream) == EOF)
				return nrchars ? -nrchars : -1;
			nrchars++;
			continue;
		}
		flags = 0;
		do {
			switch(*fmt) {
			case '-':	flags |= FL_LJUST;	break;
			case '+':	flags |= FL_SIGN;	break;
			case ' ':	flags |= FL_SPACE;	break;
			case '#':	flags |= FL_ALT;	break;
			case '0':	flags |= FL_ZEROFILL;	break;
			default:	flags |= FL_NOMORE;	continue;
			}
			fmt++;
		} while(!(flags & FL_NOMORE));

		oldfmt = fmt;
		fmt = gnum(fmt, &width, &ap);
		if (fmt != oldfmt) flags |= FL_WIDTHSPEC;

		if (*fmt == '.') {
			fmt++; oldfmt = fmt;
			fmt = gnum(fmt, &precision, &ap);
			if (precision >= 0) flags |= FL_PRECSPEC;
		}

		if ((flags & FL_WIDTHSPEC) && width < 0) {
			width = -width;
			flags |= FL_LJUST;
		}
		if (!(flags & FL_WIDTHSPEC)) width = 0;

		if (flags & FL_SIGN) flags &= ~FL_SPACE;

		if (flags & FL_LJUST) flags &= ~FL_ZEROFILL;


		s = s1 = buf;

		switch (*fmt) {
		case 'h':	flags |= FL_SHORT; fmt++; break;
		case 'l':	flags |= FL_LONG; fmt++; break;
		case 'L':	flags |= FL_LONGDOUBLE; fmt++; break;
		}

		switch (c = *fmt++) {
		default:
      if (c == '\n') {
				if (putc('\r', stream) == EOF)
					return nrchars ? -nrchars : -1;
				nrchars++;
			}
			if (putc(c, stream) == EOF)
				return nrchars ? -nrchars : -1;
			nrchars++;
			continue;
		case 'n':
			if (flags & FL_SHORT)
				*va_arg(ap, short *) = (short) nrchars;
			else if (flags & FL_LONG)
				*va_arg(ap, long *) = (long) nrchars;
			else
				*va_arg(ap, int *) = (int) nrchars;
			continue;
		case 's':
			s1 = va_arg(ap, char *);
			if (s1 == NULL)
				s1 = "(null)";
			s = s1;
			while (precision || !(flags & FL_PRECSPEC)) {
				if (*s == '\0')
					break;
				s++;
				precision--;
			}
			break;
		case 'p':
			set_pointer(flags);
			/* fallthrough */
		case 'b':
		case 'o':
		case 'u':
		case 'x':
		case 'X':
			if (!(flags & FL_PRECSPEC)) precision = 1;
			else if (c != 'p') flags &= ~FL_ZEROFILL;
			s = o_print(&ap, flags, s, c, precision, 0);
			break;
		case 'd':
		case 'i':
			flags |= FL_SIGNEDCONV;
			if (!(flags & FL_PRECSPEC)) precision = 1;
			else flags &= ~FL_ZEROFILL;
			s = o_print(&ap, flags, s, c, precision, 1);
			break;
		case 'c':
			*s++ = va_arg(ap, int);
			break;

		case 'G':
		case 'g':
			if ((flags & FL_PRECSPEC) && (precision == 0))
				precision = 1;
		case 'f':
		case 'E':
		case 'e':
			if (!(flags & FL_PRECSPEC))
				precision = 6;

			if (precision >= sizeof(buf))
				precision = sizeof(buf) - 1;

			flags |= FL_SIGNEDCONV;
			s = _f_print(&ap, flags, s, c, precision);
			break;
    case 'r':
			ap = va_arg(ap, va_list);
			fmt = va_arg(ap, char *);
			continue;
		}
		zfill = ' ';
		if (flags & FL_ZEROFILL) zfill = '0';
		j = s - s1;

		/* between_fill is true under the following conditions:
		 * 1- the fill character is '0'
		 * and
		 * 2a- the number is of the form 0x... or 0X...
		 * or
		 * 2b- the number contains a sign or space
		 */
		between_fill = 0;
		if ((flags & FL_ZEROFILL)
		    && (((c == 'x' || c == 'X') && (flags & FL_ALT) && j > 1)
			|| (c == 'p')
			|| ((flags & FL_SIGNEDCONV)
			    && ( *s1 == '+' || *s1 == '-' || *s1 == ' '))))
			between_fill++;

		if ((i = width - j) > 0)
			if (!(flags & FL_LJUST)) {	/* right justify */
				nrchars += i;
				if (between_fill) {
				    if (flags & FL_SIGNEDCONV) {
					j--; nrchars++;
					if (putc(*s1++, stream) == EOF)
						return nrchars ? -nrchars : -1;
				    } else {
					j -= 2; nrchars += 2;
					if ((putc(*s1++, stream) == EOF)
					    || (putc(*s1++, stream) == EOF))
						return nrchars ? -nrchars : -1;
				    }
				}
				do {
					if (putc(zfill, stream) == EOF)
						return nrchars ? -nrchars : -1;
				} while (--i);
			}

		nrchars += j;
		while (--j >= 0) {
			if (putc(*s1++, stream) == EOF)
				return nrchars ? -nrchars : -1;
		}

		if (i > 0) nrchars += i;
		while (--i >= 0)
			if (putc(zfill, stream) == EOF)
				return nrchars ? -nrchars : -1;
	}
	return nrchars;
}

int
vsnprintf(char *s, size_t n, const char *format, va_list arg)
{
	int retval;
  __str tmp_stream;

  //tmp_stream._buf    = (unsigned char *) s;
	tmp_stream._ptr    = (unsigned char *) s;
	tmp_stream._count  = n-1;

	retval = _doprnt(format, arg, &tmp_stream);
	tmp_stream._count  = 1;
	putc('\0',&tmp_stream);

	return retval;
}

int
vsprintf(char *s, const char *format, va_list arg)
{
	return vsnprintf(s, INT_MAX, format, arg);
}






















