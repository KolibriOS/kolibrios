/* The <ctype.h> header file defines some macros used to identify characters.
 * It works by using a table stored in chartab.c. When a character is presented
 * to one of these macros, the character is used as an index into the table
 * (__ctype) to retrieve a byte.  The relevant bit is then extracted.
 */

#ifndef _CTYPE_H
#define _CTYPE_H


extern char	__ctype[];	/* property array defined in chartab.c */

#define _U		0x01	/* this bit is for upper-case letters [A-Z] */
#define _L		0x02	/* this bit is for lower-case letters [a-z] */
#define _N		0x04	/* this bit is for numbers [0-9] */
#define _S		0x08	/* this bit is for white space \t \n \f etc */
#define _P		0x10	/* this bit is for punctuation characters */
#define _C		0x20	/* this bit is for control characters */
#define _X		0x40	/* this bit is for hex digits [a-f] and [A-F]*/


/* Macros for identifying character classes. */
#define isalnum(c)	((__ctype+1)[c]&(_U|_L|_N))
#define isalpha(c)	((__ctype+1)[c]&(_U|_L))
#define iscntrl(c)	((__ctype+1)[c]&_C)
#define isgraph(c)	((__ctype+1)[c]&(_P|_U|_L|_N))
#define ispunct(c)	((__ctype+1)[c]&_P)
#define isspace(c)	((__ctype+1)[c]&_S)
#define isxdigit(c)	((__ctype+1)[c]&(_N|_X))

#define isdigit(c)	((unsigned) ((c)-'0') < 10)
#define islower(c)	((unsigned) ((c)-'a') < 26)
#define isupper(c)	((unsigned) ((c)-'A') < 26)
#define isprint(c)	((unsigned) ((c)-' ') < 95)
#define isascii(c)	((unsigned) (c) < 128)

#define toascii(c)	((c) & 0x7f)

static inline int toupper(int c)
{
    return islower(c) ? c - 'a' + 'A' : c ;
}

#endif /* _CTYPE_H */
