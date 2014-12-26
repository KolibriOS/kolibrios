/*
** All character classification functions except isascii().
** Integer argument (c) must be in ASCII range (0-127) for
** dependable answers.
*/

#define ALNUM     1
#define ALPHA     2
#define CNTRL     4
#define DIGIT     8
#define GRAPH    16
#define LOWER    32
#define PRINT    64
#define PUNCT   128
#define BLANK   256
#define UPPER   512
#define XDIGIT 1024

extern char _is[128];

#define isalnum(c)(_is[c] & ALNUM ) /* 'a'-'z', 'A'-'Z', '0'-'9' */
#define isalpha(c)(_is[c] & ALPHA ) /* 'a'-'z', 'A'-'Z' */
#define iscntrl(c)(_is[c] & CNTRL ) /* 0-31, 127 */
#define isdigit(c)(_is[c] & DIGIT ) /* '0'-'9' */
#define isgraph(c)(_is[c] & GRAPH ) /* '!'-'~' */
#define islower(c)(_is[c] & LOWER ) /* 'a'-'z' */
#define isprint(c)(_is[c] & PRINT ) /* ' '-'~' */
#define ispunct(c)(_is[c] & PUNCT ) /* !alnum && !cntrl && !space */
#define isspace(c)(_is[c] & BLANK ) /* HT, LF, VT, FF, CR, ' ' */
#define isupper(c)(_is[c] & UPPER ) /* 'A'-'Z' */
#define isxdigit(c)(_is[c] & XDIGIT) /* '0'-'9', 'a'-'f', 'A'-'F' */

#define isascii(c) (!((c)&(~0x7f)))
#define toascii(c) ((c)&0x7f)

