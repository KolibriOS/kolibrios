/* NOTE:  This file defines both strftime() and wcsftime().  Take care when
 * making changes.  See also wcsftime.c, and note the (small) overlap in the
 * manual description, taking care to edit both as needed.  */
/*
 * strftime.c
 * Original Author:	G. Haley
 * Additions from:	Eric Blake
 * Changes to allow dual use as wcstime, also:	Craig Howland
 *
 * Places characters into the array pointed to by s as controlled by the string
 * pointed to by format. If the total number of resulting characters including
 * the terminating null character is not more than maxsize, returns the number
 * of characters placed into the array pointed to by s (not including the
 * terminating null character); otherwise zero is returned and the contents of
 * the array indeterminate.
 */

/*
FUNCTION
<<strftime>>---convert date and time to a formatted string

INDEX
	strftime

ANSI_SYNOPSIS
	#include <time.h>
	size_t strftime(char *<[s]>, size_t <[maxsize]>,
			const char *<[format]>, const struct tm *<[timp]>);

TRAD_SYNOPSIS
	#include <time.h>
	size_t strftime(<[s]>, <[maxsize]>, <[format]>, <[timp]>)
	char *<[s]>;
	size_t <[maxsize]>;
	char *<[format]>;
	struct tm *<[timp]>;

DESCRIPTION
<<strftime>> converts a <<struct tm>> representation of the time (at
<[timp]>) into a null-terminated string, starting at <[s]> and occupying
no more than <[maxsize]> characters.

You control the format of the output using the string at <[format]>.
<<*<[format]>>> can contain two kinds of specifications: text to be
copied literally into the formatted string, and time conversion
specifications.  Time conversion specifications are two- and
three-character sequences beginning with `<<%>>' (use `<<%%>>' to
include a percent sign in the output).  Each defined conversion
specification selects only the specified field(s) of calendar time
data from <<*<[timp]>>>, and converts it to a string in one of the
following ways:

o+
o %a
The abbreviated weekday name according to the current locale. [tm_wday]

o %A
The full weekday name according to the current locale.
In the default "C" locale, one of `<<Sunday>>', `<<Monday>>', `<<Tuesday>>',
`<<Wednesday>>', `<<Thursday>>', `<<Friday>>', `<<Saturday>>'. [tm_wday]

o %b
The abbreviated month name according to the current locale. [tm_mon]

o %B
The full month name according to the current locale.
In the default "C" locale, one of `<<January>>', `<<February>>',
`<<March>>', `<<April>>', `<<May>>', `<<June>>', `<<July>>',
`<<August>>', `<<September>>', `<<October>>', `<<November>>',
`<<December>>'. [tm_mon]

o %c
The preferred date and time representation for the current locale.
[tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_wday]

o %C
The century, that is, the year divided by 100 then truncated.  For
4-digit years, the result is zero-padded and exactly two characters;
but for other years, there may a negative sign or more digits.  In
this way, `<<%C%y>>' is equivalent to `<<%Y>>'. [tm_year]

o %d
The day of the month, formatted with two digits (from `<<01>>' to
`<<31>>'). [tm_mday]

o %D
A string representing the date, in the form `<<"%m/%d/%y">>'.
[tm_mday, tm_mon, tm_year]

o %e
The day of the month, formatted with leading space if single digit
(from `<<1>>' to `<<31>>'). [tm_mday]

o %E<<x>>
In some locales, the E modifier selects alternative representations of
certain modifiers <<x>>.  In newlib, it is ignored, and treated as %<<x>>.

o %F
A string representing the ISO 8601:2000 date format, in the form
`<<"%Y-%m-%d">>'. [tm_mday, tm_mon, tm_year]

o %g
The last two digits of the week-based year, see specifier %G (from
`<<00>>' to `<<99>>'). [tm_year, tm_wday, tm_yday]

o %G
The week-based year. In the ISO 8601:2000 calendar, week 1 of the year
includes January 4th, and begin on Mondays. Therefore, if January 1st,
2nd, or 3rd falls on a Sunday, that day and earlier belong to the last
week of the previous year; and if December 29th, 30th, or 31st falls
on Monday, that day and later belong to week 1 of the next year.  For
consistency with %Y, it always has at least four characters.
Example: "%G" for Saturday 2nd January 1999 gives "1998", and for
Tuesday 30th December 1997 gives "1998". [tm_year, tm_wday, tm_yday]

o %h
Synonym for "%b". [tm_mon]

o %H
The hour (on a 24-hour clock), formatted with two digits (from
`<<00>>' to `<<23>>'). [tm_hour]

o %I
The hour (on a 12-hour clock), formatted with two digits (from
`<<01>>' to `<<12>>'). [tm_hour]

o %j
The count of days in the year, formatted with three digits
(from `<<001>>' to `<<366>>'). [tm_yday]

o %k
The hour (on a 24-hour clock), formatted with leading space if single
digit (from `<<0>>' to `<<23>>'). Non-POSIX extension (c.p. %I). [tm_hour]

o %l
The hour (on a 12-hour clock), formatted with leading space if single
digit (from `<<1>>' to `<<12>>'). Non-POSIX extension (c.p. %H). [tm_hour]

o %m
The month number, formatted with two digits (from `<<01>>' to `<<12>>').
[tm_mon]

o %M
The minute, formatted with two digits (from `<<00>>' to `<<59>>'). [tm_min]

o %n
A newline character (`<<\n>>').

o %O<<x>>
In some locales, the O modifier selects alternative digit characters
for certain modifiers <<x>>.  In newlib, it is ignored, and treated as %<<x>>.

o %p
Either `<<AM>>' or `<<PM>>' as appropriate, or the corresponding strings for
the current locale. [tm_hour]

o %P
Same as '<<%p>>', but in lowercase.  This is a GNU extension. [tm_hour]

o %r
Replaced by the time in a.m. and p.m. notation.  In the "C" locale this
is equivalent to "%I:%M:%S %p".  In locales which don't define a.m./p.m.
notations, the result is an empty string. [tm_sec, tm_min, tm_hour]

o %R
The 24-hour time, to the minute.  Equivalent to "%H:%M". [tm_min, tm_hour]

o %S
The second, formatted with two digits (from `<<00>>' to `<<60>>').  The
value 60 accounts for the occasional leap second. [tm_sec]

o %t
A tab character (`<<\t>>').

o %T
The 24-hour time, to the second.  Equivalent to "%H:%M:%S". [tm_sec,
tm_min, tm_hour]

o %u
The weekday as a number, 1-based from Monday (from `<<1>>' to
`<<7>>'). [tm_wday]

o %U
The week number, where weeks start on Sunday, week 1 contains the first
Sunday in a year, and earlier days are in week 0.  Formatted with two
digits (from `<<00>>' to `<<53>>').  See also <<%W>>. [tm_wday, tm_yday]

o %V
The week number, where weeks start on Monday, week 1 contains January 4th,
and earlier days are in the previous year.  Formatted with two digits
(from `<<01>>' to `<<53>>').  See also <<%G>>. [tm_year, tm_wday, tm_yday]

o %w
The weekday as a number, 0-based from Sunday (from `<<0>>' to `<<6>>').
[tm_wday]

o %W
The week number, where weeks start on Monday, week 1 contains the first
Monday in a year, and earlier days are in week 0.  Formatted with two
digits (from `<<00>>' to `<<53>>'). [tm_wday, tm_yday]

o %x
Replaced by the preferred date representation in the current locale.
In the "C" locale this is equivalent to "%m/%d/%y".
[tm_mon, tm_mday, tm_year]

o %X
Replaced by the preferred time representation in the current locale.
In the "C" locale this is equivalent to "%H:%M:%S". [tm_sec, tm_min, tm_hour]

o %y
The last two digits of the year (from `<<00>>' to `<<99>>'). [tm_year]
(Implementation interpretation:  always positive, even for negative years.)

o %Y
The full year, equivalent to <<%C%y>>.  It will always have at least four
characters, but may have more.  The year is accurate even when tm_year
added to the offset of 1900 overflows an int. [tm_year]

o %z
The offset from UTC.  The format consists of a sign (negative is west of
Greewich), two characters for hour, then two characters for minutes
(-hhmm or +hhmm).  If tm_isdst is negative, the offset is unknown and no
output is generated; if it is zero, the offset is the standard offset for
the current time zone; and if it is positive, the offset is the daylight
savings offset for the current timezone. The offset is determined from
the TZ environment variable, as if by calling tzset(). [tm_isdst]

o %Z
The time zone name.  If tm_isdst is negative, no output is generated.
Otherwise, the time zone name is based on the TZ environment variable,
as if by calling tzset(). [tm_isdst]

o %%
A single character, `<<%>>'.
o-

RETURNS
When the formatted time takes up no more than <[maxsize]> characters,
the result is the length of the formatted string.  Otherwise, if the
formatting operation was abandoned due to lack of room, the result is
<<0>>, and the string starting at <[s]> corresponds to just those
parts of <<*<[format]>>> that could be completely filled in within the
<[maxsize]> limit.

PORTABILITY
ANSI C requires <<strftime>>, but does not specify the contents of
<<*<[s]>>> when the formatted string would require more than
<[maxsize]> characters.  Unrecognized specifiers and fields of
<<timp>> that are out of range cause undefined results.  Since some
formats expand to 0 bytes, it is wise to set <<*<[s]>>> to a nonzero
value beforehand to distinguish between failure and an empty string.
This implementation does not support <<s>> being NULL, nor overlapping
<<s>> and <<format>>.

<<strftime>> requires no supporting OS subroutines.

BUGS
<<strftime>> ignores the LC_TIME category of the current locale, hard-coding
the "C" locale settings.
*/

#include <newlib.h>
#include <sys/config.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <wctype.h>

/* Defines to make the file dual use for either strftime() or wcsftime().
 * To get wcsftime, define MAKE_WCSFTIME.
 * To get strftime, do not define MAKE_WCSFTIME.
 * Names are kept friendly to strftime() usage.  The biggest ugliness is the
 * use of the CQ() macro to make either regular character constants and
 * string literals or wide-character constants and wide-character-string
 * literals, as appropriate.  */
#if !defined(MAKE_WCSFTIME)
#  define CHAR      char        /* string type basis */
#  define CQ(a)     a       /* character constant qualifier */
#  define SFLG              /* %s flag (null for normal char) */
#  define _ctloc(x) (ctloclen = strlen (ctloc = _CurrentTimeLocale->x), ctloc)
#  define TOLOWER(c)    tolower((int)(unsigned char)(c))
#  define STRTOUL(c,p,b) strtoul((c),(p),(b))
#  define STRCPY(a,b)   strcpy((a),(b))
#  define STRCHR(a,b)   strchr((a),(b))
#  define STRLEN(a) strlen(a)
# else
#  define strftime  wcsftime    /* Alternate function name */
#  define CHAR      wchar_t     /* string type basis */
#  define CQ(a)     L##a        /* character constant qualifier */
#  define snprintf  swprintf    /* wide-char equivalent function name */
#  define strncmp   wcsncmp     /* wide-char equivalent function name */
#  define TOLOWER(c)    towlower((wint_t)(c))
#  define STRTOUL(c,p,b) wcstoul((c),(p),(b))
#  define STRCPY(a,b)   wcscpy((a),(b))
#  define STRCHR(a,b)   wcschr((a),(b))
#  define STRLEN(a) wcslen(a)
#  define SFLG      "l"     /* %s flag (l for wide char) */
#  ifdef __HAVE_LOCALE_INFO_EXTENDED__
#   define _ctloc(x) (ctloclen = wcslen (ctloc = _CurrentTimeLocale->w##x), \
              ctloc)
#  else
#   define CTLOCBUFLEN   256        /* Arbitrary big buffer size */
    const wchar_t *
    __ctloc (wchar_t *buf, const char *elem, size_t *len_ret)
    {
      buf[CTLOCBUFLEN - 1] = L'\0';
      *len_ret = mbstowcs (buf, elem, CTLOCBUFLEN - 1);
      if (*len_ret == (size_t) -1 )
    *len_ret = 0;
      return buf;
    }
#   define _ctloc(x) (ctloc = __ctloc (ctlocbuf, _CurrentTimeLocale->x, \
              &ctloclen))
#  endif
#endif  /* MAKE_WCSFTIME */


size_t _DEFUN (strftime, (s, maxsize, format, tim_p),
	CHAR *s _AND
	size_t maxsize _AND
	_CONST CHAR *format _AND
	_CONST struct tm *tim_p)
{

  return 0;
}


