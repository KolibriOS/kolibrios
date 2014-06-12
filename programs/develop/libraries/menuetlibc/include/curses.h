/*
***************************************************************************
* This file comprises part of PDCurses. PDCurses is Public Domain software.
* You may use this code for whatever purposes you desire. This software
* is provided AS IS with NO WARRANTY whatsoever.
* Should this software be used in another application, an acknowledgement
* that PDCurses code is used would be appreciated, but is not mandatory.
*
* Any changes which you make to this software which may improve or enhance
* it, should be forwarded to the current maintainer for the benefit of 
* other users.
*
* The only restriction placed on this code is that no distribution of
* modified PDCurses code be made under the PDCurses name, by anyone
* other than the current maintainer.
* 
* See the file maintain.er for details of the current maintainer.
***************************************************************************
*/
/* 
$Id$
*/
/*
*----------------------------------------------------------------------
*				PDCurses
*----------------------------------------------------------------------
* MH
*	950126	2.2	Added more System V R4 functions
*
*			Added beta Xwindows port
*
*			Changed chtype to long
*
*			Incorporated panels library
*
*			Support for more and newer versions of compilers
*
* MH
*	930531	2.1	Added support for djgpp
*
*			Added beta Unix version
*
*			Added OS/2 DLL support.
*
*			Changed behaviour of overlay(), overwrite() and typeahead()
*
*	921120	2.0	Changed #if to #ifdef/#if defined to make it
*			easier to add new platforms/compilers.
*
*			Added System V colour support.
*
*			Added OS/2 port.
*-------
* Frotz
*	911221	2.0 pre-beta	Changed back from short to int. (int is the
*			correct size for the default platform.  Short
*			might be too short on some platforms.  This
*			is more portable.  I, also, made this mistake.)
*
*			Many functions are now macros.  If you want
*			the real thing, #undef the macro. (X/Open
*			requirement.)
*
*			Merged many sources into current release.
*
*			Added many X/Open routines (not quite all yet).
*
*			Added internal documentation to all routines.
*
*			Added a HISTORY file to the environment.
*
*			Added a CONTRIB file to the environment.
*-------
* bl	900114	1.4	Window origin mod in overlay() and overwrite(), on
*			public (and very reasonable) request. Swapped
*			#define'd values of OK and ERR; OK now 1, and
*			ERR is 0/NULL. Conforms better to UNIX
*			versions.  borderchars[] removed from WINDOW
*			struct since the border() functions were
*			redefined. Use of short wherever possible.
*			Portability improvements, mispelled name of
*			[w]setscrreg().
*
*	881005	1.3	All modules lint-checked with MSC '-W3' and
*			turbo'C' '-w -w-pro' switches. Support for
*			border(), wborder() functions.
*
*	881002	1.2	Rcsid[] string in all modules, for maintenance.
*
*	880306	1.1	'Raw' output routines, revision info in curses.h.
*
*	870515	1.0	Initial Release.
*
*----------------------------------------------------------------------
*/

#ifndef  __PDCURSES__
#define	__PDCURSES__ 1

/*man-start*********************************************************************

All defines are "defined" here.  All compiler and environment
specific definitions are defined into generic class defines.
These defines are to be given values so that the code can
rely on #if, rather than a complicated set of #if defined() or
#ifdefs...

PDCurses definitions list:  (Only define those needed)

	REGISTERWINDOWS True for auto window update registery.
	FAST_VIDEO      True if display is memory mapped, or
	                we can utilize the fast video update routines.
	DOS             True if compiling for DOS.
	OS2             True if compiling for OS/2.
	FLEXOS          True if compiling for Flexos.
	HC              True if using a Metaware compiler.
	TC              True if using a Borland compiler.
	MSC             True if using a Microsoft compiler.
	ANSI            True if the compiler supports ANSI C and
	                (full or mixed) prototypes.
	CPLUSPLUS       True if the compiler supports C++.

PDCurses portable platform definitions list:

	PDCurses        Enables access to PDCurses-only routines.
	XOPEN           Always true.
	SYSV            True if you are compiling for SYSV portability.
	BSD             True if you are compiling for BSD portability.
	INTERNAL        Enables access to internal PDCurses routines.
	PDCURSES_WCLR   Makes behaviour of wclrtoeol() and wclrtoeof()
	                unique to PDCurses. By default Unix behavior is set.
	                See notes in wclrtoeol() and wclrtoeof().
**man-end**********************************************************************/

#define	PDCURSES	1	/* PDCurses-only routines	*/
#define	XOPEN		1	/* X/Open Curses routines	*/
#define	SYSV		1	/* System V Curses routines	*/
#define	BSD		1	/* BSD Curses routines		*/
#define	INTERNAL	1	/* PDCurses Internal routines	*/


#ifdef __MENUET_CURSES__

#undef DOS
#undef LINUX
#undef UNIX
#define MENUETOS	1
#define ANSI		1
#define CURSES__32BIT__
#ifdef _cplusplus
#define CPLUSPLUS 1
#endif
#define NO_VSSCANF 1
#endif

/*---------------------------------------------------------------------*/
#include <stdio.h>		/* Required by X/Open usage below	*/
/*----------------------------------------------------------------------
 *
 *	PDCurses Manifest Constants
 *
 */
#ifndef FALSE			/* booleans		 */
#  define	FALSE	0
#endif
#ifndef	TRUE			/* booleans		 */
#  define	TRUE	!FALSE
#endif
#ifndef	NULL
#  define NULL	((void*)0)	/* Null pointer		 */
#endif
#ifndef	ERR
#  define	 ERR	0		/* general error flag	 */
#endif
#ifndef	OK
#  define	 OK	1		/* general OK flag	 */
#endif




/*----------------------------------------------------------------------
 *
 *	PDCurses Type Declarations
 *
 */
typedef unsigned char bool;	/* PDCurses Boolean type	*/
typedef unsigned short chtype;	/* 8-bit attr + 8-bit char	*/



/*----------------------------------------------------------------------
 *
 *	PDCurses Structure Definitions:
 *
 */
typedef struct _win		/* definition of a window.	   */
{
	int	_cury;		/* current pseudo-cursor	   */
	int	_curx;
	int	_maxy;		/* max window coordinates	   */
	int	_maxx;
	int	_pmaxy;		/* max physical size		   */
	int	_pmaxx;
	int	_begy;		/* origin on screen		   */
	int	_begx;
	int	_lastpy;	/* last y coordinate of upper left pad display area */
	int	_lastpx;	/* last x coordinate of upper left pad display area */
	int	_lastsy1;	/* last upper y coordinate of screen window for pad */
	int	_lastsx1;	/* last upper x coordinate of screen window for pad */
	int	_lastsy2;	/* last lower y coordinate of screen window for pad */
	int	_lastsx2;	/* last lower x coordinate of screen window for pad */
	int	_flags;		/* window properties		   */
	chtype	_attrs;		/* standard A_STANDOUT attributes and colors  */
	chtype	_bkgd;		/* wrs(4/6/93) background, normally blank */
	int	_tabsize;	/* tab character size		   */
	bool	_clear;		/* causes clear at next refresh	   */
	bool	_leave;		/* leaves cursor as it happens	   */
	bool	_scroll;	/* allows window scrolling	   */
	bool	_nodelay;	/* input character wait flag	   */
	bool	_immed;	/* immediate update flag	   */
	bool	_use_keypad;	/* flags keypad key mode active	   */
	bool	_use_idl;	/* True if Ins/Del line can be used*/
	bool	_use_idc;	/* True if Ins/Del character can be used*/
	chtype**_y;		/* pointer to line pointer array   */
	int*	_firstch;	/* first changed character in line */
	int*	_lastch;	/* last changed character in line  */
	int	_tmarg;	/* top of scrolling region	   */
	int	_bmarg;	/* bottom of scrolling region	   */
	char*	_title;		/* window title			   */
	char	_title_ofs;	/* window title offset from left   */
	chtype	_title_attr;	/* window title attributes	   */
	chtype	_blank;		/* window's blank character	   */
	int	_parx, _pary;	/* coords relative to parent (0,0) */
struct	_win*	_parent;	/* subwin's pointer to parent win  */
}	WINDOW;



/*----------------------------------------------------------------------
*
*	Private structures that are necessary for correct
*	macro construction.
*
*/

#ifdef	REGISTERWINDOWS
typedef struct _ref		/* Refresh Window Structure	 */
{
	WINDOW*	win;
struct	_ref*	next;
struct	_ref*	tail;
}	ACTIVE;

typedef struct _wins
{
	WINDOW*		w;	/* pointer to a visible window	    */
	struct _wins*	next;	/* Next visible window pointer	    */
	struct _wins*	prev;	/* Next visible window pointer	    */
	struct _wins*	tail;	/* Last visible window pointer	    */
				/* Only head window (stdscr) has    */
				/* a valid tail pointer.	    */
}	WINDS;
#endif




typedef struct
{
	bool	alive;		/* TRUE if already opened.	    */
	bool	autocr;		/* if lf -> crlf		    */
	bool	cbreak;		/* if terminal unbuffered	    */
	bool	echo;		/* if terminal echo		    */
	bool	raw_inp;	/* raw input mode (v. cooked input) */
	bool	raw_out;	/* raw output mode (7 v. 8 bits)    */
	bool	refrbrk;	/* if premature refresh brk allowed */
	bool	orgcbr;		/* original MSDOS ^-BREAK setting   */
	bool	visible_cursor; /* TRUE if cursor is visible	    */
	bool	audible;	/* FALSE if the bell is visual	    */
	bool	full_redraw;	/* TRUE for bad performance	    */
	bool	direct_video;	/* Allow Direct Screen Memory writes*/
	bool	mono;		/* TRUE if current screen is mono.  */
	bool	sizeable;	/* TRUE if adapter is resizeable.   */
	bool	bogus_adapter;	/* TRUE if adapter has insane values*/
	bool	shell;		/* TRUE if reset_prog_mode() needs  */
				/*	to be called.		    */
	chtype	blank;		/* Background character		    */
	chtype	orig_attr;	/* Original screen attributes	    */
	int	cursrow;	/* position of physical cursor	    */
	int	curscol;	/* position of physical cursor	    */
	int	cursor;		/* Current Cursor definition	    */
	int	visibility;		/* Visibility of cursor	*/
	int	video_page;	/* Current PC video page	    */
	int	orig_emulation; /* Original cursor emulation value  */
	int	orig_cursor;	/* Original cursor size		    */
	int	font;		/* default font size		    */
	int	orig_font;	/* Original font size		    */
	int	lines;		/* New value for LINES		    */
	int	cols;		/* New value for COLS		    */
	int	emalloc;	/* 0x0C0C if initscr() is to reset  */
				/*     this value to TRUE;	    */
				/* TRUE only if emalloc()/ecalloc() */
				/*     are is to be used;	    */
				/* FALSE if malloc()/calloc() are   */
				/*     to be used.		    */
	int tahead; 			/* Type-ahead value */
	int adapter;			/* Screen type	*/
#ifdef UNIX
	int	adapter;	/* Screen type			    */
	int number_keys;	/* number of function keys */
	char *key_seq[200];	/* key sequence ptr for function keys */
	int key_num[200];	/* key numbers for function keys */
#endif

}	SCREEN;





/* external variables */
extern	int	LINES;		/* terminal height		*/
extern	int	COLS;		/* terminal width		*/
extern	WINDOW*	stdscr;		/* the default screen window	*/
extern	SCREEN	_cursvar;	/* curses variables		*/

#if	defined (INTERNAL) | defined (CURSES_LIBRARY)
extern	WINDOW*	curscr;		/* the current screen image	*/
extern	int	_default_lines;	/* For presetting maximum lines	*/
#endif

#ifdef	REGISTERWINDOWS
extern	ACTIVE*	CurWins;	/* Currently Visible Windows	*/
#endif




/*man-start*********************************************************************

PDCurses Text Attributes:

To include colour in PDCurses, a number of things had to be sacrificed
from the strict Unix and System V support.
The main problem is fitting all character attributes and colour into
an unsigned char (all 8 bits!). On System V, chtype is a long on
PDCurses it is a short int.

The following is the structure of a win->_attrs chtype:

-------------------------------------------------
|15|14|13|12|11|10| 9| 8| 7| 6| 5| 4| 3| 2| 1| 0|
-------------------------------------------------
 colour number |  attrs |   character eg 'a'

the high order char is an index into an array of physical colours
(defined in INITPAIR.c). 32 (5 bits) foreground/background colour
combinations combined with 8 (3 bits) attribute modifiers are
available.

The available attribute enhancers are bold, reverse and blink.
All other Unix attributes have no effect as attributes. This
limitation may be overcome in future releases by expanding chtype
to a long.

**man-end**********************************************************************/

/* Video attribute definitions. */
#define A_NORMAL	(chtype)0x0000		/* SysV */
#define A_ALTCHARSET	(chtype)0x0000		/* X/Open	*/
#define A_BLINK		(chtype)0x0400		/* X/Open	*/
#define A_BLANK		(chtype)0x0000		/* X/Open	*/
#define A_BOLD		(chtype)0x0100		/* X/Open	*/
#define A_DIM		(chtype)0x0000		/* X/Open	*/
#define A_PROTECT	(chtype)0x0000		/* X/Open	*/
#define A_REVERSE	(chtype)0x0200		/* X/Open	*/
#define A_STANDOUT	((chtype)(A_REVERSE | A_BOLD))		/* X/Open	*/
#define A_UNDERLINE	(chtype)0x0000		/* X/Open	*/
#define A_COLOR	(chtype)0xF800		/*System V	*/
#define A_CHARTEXT	(chtype)(0xFF)			/* X/Open	*/
#define A_ATTRIBUTES	(chtype)(~A_CHARTEXT)			/* X/Open	*/

#define CHR_MSK		A_CHARTEXT		/* Obsolete	*/
#define ATR_MSK		A_ATTRIBUTES	/* Obsolete	*/
#define ATR_NRM		A_NORMAL			/* Obsolete	*/

#define ACS_ULCORNER	(chtype)0xda			/* SysV		*/
#define ACS_LLCORNER	(chtype)0xc0			/* SysV		*/
#define ACS_URCORNER	(chtype)0xbf			/* SysV		*/
#define ACS_LRCORNER	(chtype)0xd9			/* SysV		*/
#define ACS_RTEE	(chtype)0xb4			/* SysV		*/
#define ACS_LTEE	(chtype)0xc3			/* SysV		*/
#define ACS_BTEE	(chtype)0xc1			/* SysV		*/
#define ACS_TTEE	(chtype)0xc2			/* SysV		*/
#define ACS_HLINE	(chtype)0xc4			/* SysV		*/
#define ACS_VLINE	(chtype)0xb3			/* SysV		*/
#define ACS_PLUS	(chtype)0xc5			/* SysV		*/
#define ACS_S1	(chtype)0x2d			/* SysV		*/
#define ACS_S9	(chtype)0x5f			/* SysV		*/
#define ACS_DIAMOND	(chtype)0xc5			/* SysV		*/
#define ACS_CKBOARD	(chtype)0xb2			/* SysV		*/
#define ACS_DEGREE	(chtype)0xf8			/* SysV		*/
#define ACS_PLMINUS	(chtype)0xf1			/* SysV		*/
#define ACS_BULLET	(chtype)0xf9			/* SysV		*/
#define ACS_LARROW	(chtype)0x3c			/* SysV		*/
#define ACS_RARROW	(chtype)0x3e			/* SysV		*/
#define ACS_DARROW	(chtype)0x76			/* SysV		*/
#define ACS_UARROW	(chtype)0x5e			/* SysV		*/
#define ACS_BOARD	(chtype)0x23			/* SysV		*/
#define ACS_LANTERN	(chtype)0x23			/* SysV		*/
#define ACS_BLOCK	(chtype)0x23			/* SysV		*/

#  define COLOR_BLACK		0
#  define COLOR_BLUE		1
#  define COLOR_GREEN		2
#  define COLOR_CYAN		3
#  define COLOR_RED		4
#  define COLOR_MAGENTA		5
#  define COLOR_YELLOW		6
#  define COLOR_WHITE		7

#define COLOR_PAIR(n)  (((n) << 11) & A_ATTRIBUTES)
#define PAIR_NUMBER(n) (((n) & A_COLOR) >> 11)

extern int COLORS,COLOR_PAIRS;
/*----------------------------------------------------------------------
 *
 *	Function and Keypad Key Definitions.
 *	Many are just for compatibility.
 *
 */
#define KEY_MIN         0x101   /* Minimum curses key value      */
#define KEY_BREAK       0x101   /* Not on PC KBD                 */
#define KEY_DOWN        0x102   /* Down arrow key                */
#define KEY_UP          0x103   /* Up arrow key                  */
#define KEY_LEFT        0x104   /* Left arrow key                */
#define KEY_RIGHT       0x105   /* Right arrow key               */
#define KEY_HOME        0x106   /* home key                      */
#define KEY_BACKSPACE   0x107   /* not on pc                     */
#define KEY_F0          0x108   /* function keys. space for      */
#define KEY_F(n)    (KEY_F0+(n))/* 64 keys are reserved.         */
#define KEY_DL          0x148   /* not on pc                     */
#define KEY_IL          0x149   /* insert line                   */
#define KEY_DC          0x14a   /* delete character              */
#define KEY_IC          0x14b   /* insert char or enter ins mode */
#define KEY_EIC         0x14c   /* exit insert char mode         */
#define KEY_CLEAR       0x14d   /* clear screen                  */
#define KEY_EOS         0x14e   /* clear to end of screen        */
#define KEY_EOL         0x14f   /* clear to end of line          */
#define KEY_SF          0x150   /* scroll 1 line forward         */
#define KEY_SR          0x151   /* scroll 1 line back (reverse)  */
#define KEY_NPAGE       0x152   /* next page                     */
#define KEY_PPAGE       0x153   /* previous page                 */
#define KEY_STAB        0x154   /* set tab                       */
#define KEY_CTAB        0x155   /* clear tab                     */
#define KEY_CATAB       0x156   /* clear all tabs                */
#define KEY_ENTER       0x157   /* enter or send (unreliable)    */
#define KEY_SRESET      0x158   /* soft/reset (partial/unreliable)*/
#define KEY_RESET       0x159   /* reset/hard reset (unreliable) */
#define KEY_PRINT       0x15a   /* print/copy                    */
#define KEY_LL          0x15b   /* home down/bottom (lower left) */
#define KEY_ABORT       0x15c   /* abort/terminate key (any)     */
#define KEY_SHELP       0x15d   /* short help                    */
#define KEY_LHELP       0x15e   /* long help                     */
#define KEY_BTAB        0x15f   /* Back tab key                  */
#define KEY_BEG         0x160   /* beg(inning) key               */
#define KEY_CANCEL      0x161   /* cancel key                    */
#define KEY_CLOSE       0x162   /* close key                     */
#define KEY_COMMAND     0x163   /* cmd (command) key             */
#define KEY_COPY        0x164   /* copy key                      */
#define KEY_CREATE      0x165   /* create key                    */
#define KEY_END         0x166   /* end key                       */
#define KEY_EXIT        0x167   /* exit key                      */
#define KEY_FIND        0x168   /* find key                      */
#define KEY_HELP        0x169   /* help key                      */
#define KEY_MARK        0x16a   /* mark key                      */
#define KEY_MESSAGE     0x16b   /* message key                   */
#define KEY_MOVE        0x16c   /* move key                      */
#define KEY_NEXT        0x16d   /* next object key               */
#define KEY_OPEN        0x16e   /* open key                      */
#define KEY_OPTIONS     0x16f   /* options key                   */
#define KEY_PREVIOUS    0x170   /* previous object key           */
#define KEY_REDO        0x171   /* redo key                      */
#define KEY_REFERENCE   0x172   /* ref(erence) key               */
#define KEY_REFRESH     0x173   /* refresh key                   */
#define KEY_REPLACE     0x174   /* replace key                   */
#define KEY_RESTART     0x175   /* restart key                   */
#define KEY_RESUME      0x176   /* resume key                    */
#define KEY_SAVE        0x177   /* save key                      */
#define KEY_SBEG        0x178   /* shifted beginning key         */
#define KEY_SCANCEL     0x179   /* shifted cancel key            */
#define KEY_SCOMMAND    0x17a   /* shifted command key           */
#define KEY_SCOPY       0x17b   /* shifted copy key              */
#define KEY_SCREATE     0x17c   /* shifted create key            */
#define KEY_SDC         0x17d   /* shifted delete char key       */
#define KEY_SDL         0x17e   /* shifted delete line key       */
#define KEY_SELECT      0x17f   /* select key                    */
#define KEY_SEND        0x180   /* shifted end key               */
#define KEY_SEOL        0x181   /* shifted clear line key        */
#define KEY_SEXIT       0x182   /* shifted exit key              */
#define KEY_SFIND       0x183   /* shifted find key              */
#define KEY_SHOME       0x184   /* shifted home key              */
#define KEY_SIC         0x185   /* shifted input key             */
#define KEY_SLEFT       0x187   /* shifted left arrow key        */
#define KEY_SMESSAGE    0x188   /* shifted message key           */
#define KEY_SMOVE       0x189   /* shifted move key              */
#define KEY_SNEXT       0x18a   /* shifted next key              */
#define KEY_SOPTIONS    0x18b   /* shifted options key           */
#define KEY_SPREVIOUS   0x18c   /* shifted prev key              */
#define KEY_SPRINT      0x18d   /* shifted print key             */
#define KEY_SREDO       0x18e   /* shifted redo key              */
#define KEY_SREPLACE    0x18f   /* shifted replace key           */
#define KEY_SRIGHT      0x190   /* shifted right arrow           */
#define KEY_SRSUME      0x191   /* shifted resume key            */
#define KEY_SSAVE       0x192   /* shifted save key              */
#define KEY_SSUSPEND    0x193   /* shifted suspend key           */
#define KEY_SUNDO       0x194   /* shifted undo key              */
#define KEY_SUSPEND     0x195   /* suspend key                   */
#define KEY_UNDO        0x196   /* undo key                      */

/* PDCurses specific key definitions */

#define ALT_0           0x197   /* Alt-0                PC only  */
#define ALT_1           0x198   /* Alt-1                PC only  */
#define ALT_2           0x199   /* Alt-2                PC only  */
#define ALT_3           0x19a   /* Alt-3                PC only  */
#define ALT_4           0x19b   /* Alt-4                PC only  */
#define ALT_5           0x19c   /* Alt-5                PC only  */
#define ALT_6           0x19d   /* Alt-6                PC only  */
#define ALT_7           0x19e   /* Alt-7                PC only  */
#define ALT_8           0x19f   /* Alt-8                PC only  */
#define ALT_9           0x1a0   /* Alt-9                PC only  */
#define ALT_A           0x1a1   /* Alt-A                PC only  */
#define ALT_B           0x1a2   /* Alt-B                PC only  */
#define ALT_C           0x1a3   /* Alt-C                PC only  */
#define ALT_D           0x1a4   /* Alt-D                PC only  */
#define ALT_E           0x1a5   /* Alt-E                PC only  */
#define ALT_F           0x1a6   /* Alt-F                PC only  */
#define ALT_G           0x1a7   /* Alt-G                PC only  */
#define ALT_H           0x1a8   /* Alt-H                PC only  */
#define ALT_I           0x1a9   /* Alt-I                PC only  */
#define ALT_J           0x1aa   /* Alt-J                PC only  */
#define ALT_K           0x1ab   /* Alt-K                PC only  */
#define ALT_L           0x1ac   /* Alt-L                PC only  */
#define ALT_M           0x1ad   /* Alt-M                PC only  */
#define ALT_N           0x1ae   /* Alt-N                PC only  */
#define ALT_O           0x1af   /* Alt-O                PC only  */
#define ALT_P           0x1b0   /* Alt-P                PC only  */
#define ALT_Q           0x1b1   /* Alt-Q                PC only  */
#define ALT_R           0x1b2   /* Alt-R                PC only  */
#define ALT_S           0x1b3   /* Alt-S                PC only  */
#define ALT_T           0x1b4   /* Alt-T                PC only  */
#define ALT_U           0x1b5   /* Alt-U                PC only  */
#define ALT_V           0x1b6   /* Alt-V                PC only  */
#define ALT_W           0x1b7   /* Alt-W                PC only  */
#define ALT_X           0x1b8   /* Alt-X                PC only  */
#define ALT_Y           0x1b9   /* Alt-Y                PC only  */
#define ALT_Z           0x1ba   /* Alt-Z                PC only  */
#define CTL_LEFT        0x1bb   /* Control-Left-Arrow   PC only  */
#define CTL_RIGHT       0x1bc   /* Control-Right-Arrow  PC only  */
#define CTL_PGUP        0x1bd   /* Control-PgUp         PC only  */
#define CTL_PGDN        0x1be   /* Control-PgDn         PC only  */
#define CTL_HOME        0x1bf   /* Control-Home         PC only  */
#define CTL_END         0x1c0   /* Control-End          PC only  */
#define KEY_BACKTAB     0x1c1   /* Back-tab             PC only  */

#define KEY_A1          0x1c2   /* upper left on Virtual keypad  */
#define KEY_A2          0x1c3   /* upper middle on Virt. keypad  */
#define KEY_A3          0x1c4   /* upper right on Vir. keypad    */
#define KEY_B1          0x1c5   /* middle left on Virt. keypad   */
#define KEY_B2          0x1c6   /* center on Virt. keypad        */
#define KEY_B3          0x1c7   /* middle right on Vir. keypad   */
#define KEY_C1          0x1c8   /* lower left on Virt. keypad    */
#define KEY_C2          0x1c9   /* lower middle on Virt. keypad  */
#define KEY_C3          0x1ca   /* lower right on Vir. keypad    */
#define PADSLASH        0x1cb   /* slash on keypad               */
#define PADENTER        0x1cc   /* enter on keypad               */
#define CTL_PADENTER    0x1cd   /* ctl-enter on keypad           */
#define ALT_PADENTER    0x1ce   /* alt-enter on keypad           */
#define SHF_PADSTOP     0x1cf   /* shift-stop on keypad          */
#define PADSTAR         0x1d0   /* star on keypad                */
#define PADMINUS        0x1d1   /* minus on keypad               */
#define PADPLUS         0x1d2   /* plus on keypad                */
#define CTL_PADSTOP     0x1d3   /* ctl-stop on keypad            */
#define CTL_PADCENTER   0x1d4   /* ctl-enter on keypad           */
#define CTL_PADPLUS     0x1d5   /* ctl-plus on keypad            */
#define CTL_PADMINUS    0x1d6   /* ctl-minus on keypad           */
#define CTL_PADSLASH    0x1d7   /* ctl-slash on keypad           */
#define CTL_PADSTAR     0x1d8   /* ctl-star on keypad            */
#define ALT_PADPLUS     0x1d9   /* alt-plus on keypad            */
#define ALT_PADMINUS    0x1da   /* alt-minus on keypad           */
#define ALT_PADSLASH    0x1db   /* alt-slash on keypad           */
#define ALT_PADSTAR     0x1dc   /* alt-star on keypad            */
#define CTL_INS         0x1dd   /* ctl-insert                    */
#define ALT_DEL         0x1de   /* alt-delete                    */
#define ALT_INS         0x1df   /* alt-insert                    */
#define CTL_UP          0x1e0   /* ctl-up arrow                  */
#define CTL_DOWN        0x1e1   /* ctl-down arrow                */
#define CTL_TAB         0x1e2   /* ctl-tab                       */
#define ALT_TAB         0x1e3   /* alt-tab                       */
#define ALT_MINUS       0x1e4   /* alt-minus                     */
#define ALT_EQUAL       0x1e5   /* alt-equal                     */
#define ALT_HOME        0x1e6   /* alt-home                      */
#define ALT_PGUP        0x1e7   /* alt-pgup                      */
#define ALT_PGDN        0x1e8   /* alt-pgdn                      */
#define ALT_END         0x1e9   /* alt-end                       */
#define ALT_UP          0x1ea   /* alt-up arrow                  */
#define ALT_DOWN        0x1eb   /* alt-down arrow                */
#define ALT_RIGHT       0x1ec   /* alt-right arrow               */
#define ALT_LEFT        0x1ed   /* alt-left arrow                */
#define ALT_ENTER       0x1ee   /* alt-enter                     */
#define ALT_ESC         0x1ef   /* alt-escape                    */
#define ALT_BQUOTE      0x1f0   /* alt-back quote                */
#define ALT_LBRACKET    0x1f1   /* alt-left bracket              */
#define ALT_RBRACKET    0x1f2   /* alt-right bracket             */
#define ALT_SEMICOLON   0x1f3   /* alt-semi-colon                */
#define ALT_FQUOTE      0x1f4   /* alt-forward quote             */
#define ALT_COMMA       0x1f5   /* alt-comma                     */
#define ALT_STOP        0x1f6   /* alt-stop                      */
#define ALT_FSLASH      0x1f7   /* alt-forward slash             */
#define ALT_BKSP        0x1f8   /* alt-backspace                 */
#define CTL_BKSP        0x1f9   /* ctl-backspace                 */
#define CTL_PAD0        0x1fa   /* ctl-keypad 0                  */
#define CTL_PAD1        0x1fb   /* ctl-keypad 1                  */
#define CTL_PAD2        0x1fc   /* ctl-keypad 2                  */
#define CTL_PAD3        0x1fd   /* ctl-keypad 3                  */
#define CTL_PAD4        0x1fe   /* ctl-keypad 4                  */
#define CTL_PAD5        0x1ff   /* ctl-keypad 5                  */
#define CTL_PAD6        0x200   /* ctl-keypad 6                  */
#define CTL_PAD7        0x201   /* ctl-keypad 7                  */
#define CTL_PAD8        0x202   /* ctl-keypad 8                  */
#define CTL_PAD9        0x203   /* ctl-keypad 9                  */
#define CTL_DEL         0x204   /* clt-delete                    */
#define ALT_BSLASH      0x205   /* alt-back slash                */
#define CTL_ENTER       0x206   /* ctl-enter                     */
#define KEY_MOUSE       0x207 /* "mouse" key  */
#define KEY_MAX         KEY_MOUSE  /* Maximum curses key         */


/*----------------------------------------------------------------------
*       PDCurses function declarations
*/
#ifdef ANSI
#  ifdef   CPLUSPLUS
     extern "C" {
#  endif
int     addchnstr( chtype *, int );
int     baudrate( void );
int     beep( void );
int     border( chtype, chtype, chtype, chtype, chtype, chtype, chtype, chtype );
char    breakchar( void );
int     can_change_color ( void );
int     clearok( WINDOW*, bool );
int     color_content( int, short*, short*, short* );
int     copywin( WINDOW*, WINDOW*, int, int, int, int, int, int, int );
int     curs_set( int );
int     cursoff( void );
int     curson( void );
int     def_prog_mode( void );
int     def_shell_mode( void );
int     delay_output(  int  );
int     delwin( WINDOW* );
WINDOW* derwin( WINDOW*, int, int, int, int );
int     doupdate( void );
WINDOW* dupwin( WINDOW* );
int     endwin( void );
char    erasechar( void );
int     fixterm( void );
int     flash( void );
int     flushinp( void );
int     gettmode( void );
bool    has_colors( void );
int     hline( chtype, int );
int     inchnstr( chtype *, int );
int     init_color( short, short, short, short );
int     init_pair( short, short, short );
WINDOW* initscr( void );
int     intrflush(  WINDOW*, bool  );
int     is_linetouched(WINDOW *,int);
int     is_wintouched(WINDOW *);
char*   keyname(  int  );
char    killchar( void );
char*   longname( void );
int     meta( WINDOW*, bool );
int     mvaddrawch( int, int, chtype );
int     mvaddrawstr( int, int, char* );
int     mvcur( int, int, int, int );
int     mvderwin( WINDOW*, int, int );
int     mvinsrawch( int, int, chtype );
int     mvprintw( int, int, char*,... );
int     mvscanw( int, int, char*,... );
int     mvwin( WINDOW*, int, int );
int     mvwinsrawch( WINDOW*, int, int, chtype );
int     mvwprintw( WINDOW*, int, int, char*,... );
int     mvwscanw( WINDOW*, int, int, char*,... );
WINDOW* newpad( int, int );
SCREEN* newterm( char*, FILE*, FILE* );
WINDOW* newwin( int, int, int, int );
int     noraw( void );
int     overlay( WINDOW*, WINDOW* );
int     overwrite( WINDOW*, WINDOW* );
int     pair_content( int, short*, short* );
int     pnoutrefresh( WINDOW*, int, int, int, int, int, int );
int     prefresh( WINDOW*, int, int, int, int, int, int );
int     printw( char*,... );
int     raw( void );
int     refresh( void );
int     reset_prog_mode( void );
int     reset_shell_mode( void );
int     resetterm( void );
int     resetty( void );
int     saveoldterm( void );
int     saveterm( void );
int     savetty( void );
int     scanw( char*,... );
int     scroll( WINDOW* );
SCREEN* set_term( SCREEN* );
int     start_color( void );
WINDOW* subpad( WINDOW*, int, int, int, int );
WINDOW* subwin( WINDOW*, int, int, int, int );
int     tabsize( int );
chtype  termattrs( void );
char*   termname( void );
int     touchline( WINDOW*, int ,int );
int     touchwin( WINDOW* );
int     typeahead( int );
char*   unctrl( chtype );
int     vline( chtype, int );
int     waddchnstr( WINDOW*, chtype*, int );
int     waddnstr( WINDOW*, char*, int );
int     waddrawstr( WINDOW*, char* );
int     waddstr( WINDOW*, char* );
int     wattroff( WINDOW*, chtype );
int     wattron( WINDOW*, chtype );
int     wattrset( WINDOW*, chtype );
int     wbkgd(WINDOW*, chtype);
void    wbkgdset(WINDOW*, chtype);
int     wborder( WINDOW*, chtype, chtype, chtype, chtype, chtype, chtype, chtype, chtype );
int     wclear( WINDOW* );
int     wclrtobot( WINDOW* );
int     wclrtoeol( WINDOW* );
int     wdelch( WINDOW* );
int     wdeleteln( WINDOW* );
int     werase( WINDOW* );
int     wgetch( WINDOW* );
int     wgetnstr( WINDOW*, char*, int );
int     wgetstr( WINDOW*, char* );
int     whline( WINDOW*, chtype, int );
int     winchnstr( WINDOW*, chtype*, int );
int     winnstr( WINDOW*, char*, int );
int     winsch( WINDOW*, chtype );
int     winsdelln( WINDOW*, int );
int     winsertln( WINDOW* );
int     winsnstr( WINDOW*, char*, int );
int     wmove( WINDOW*, int, int );
int     wnoutrefresh( WINDOW* );
char    wordchar( void );
int     wprintw( WINDOW*, char*,... );
int     wredrawln( WINDOW*, int ,int );
int     wrefresh( WINDOW* );
int     wscanw( WINDOW*, char*,... );
int     wscrl( WINDOW*, int );
int     wsetscrreg( WINDOW*, int, int );
int     wtabsize( WINDOW*, int );
int     wtouchln(WINDOW *, int, int, int);
int     ungetch( int );
int     wvline( WINDOW*, chtype, int );

#ifdef     PDCURSES
int     raw_output( bool );
int     resize_screen( int );
WINDOW* resize_window( WINDOW*, int, int );
int     win_print( WINDOW*, int );
#endif

/*
*       Keep the compiler happy with our macros below...
*/
int     PDC_chadd( WINDOW*, chtype, bool, bool );
int     PDC_chins( WINDOW*, chtype, bool );

#  ifdef   CPLUSPLUS
     }
#  endif
#endif


#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

/*
*       Functions defined as macros
*/

#define addch( c )              waddch( stdscr, c )
#define addchstr( c )           addchnstr( c, -1 )
#define addstr(str)             waddstr( stdscr, str )
#define addnstr(str, n)         waddnstr( stdscr, str, n )
#define attroff(attr)           wattroff( stdscr, attr )
#define attron(attr)            wattron( stdscr, attr )
#define attrset(attr)           wattrset( stdscr, attr )
#define bkgd(c)                 wbkgd(stdscr,c)
#define bkgdset(c)              wbkgdset(stdscr,c)
#define border(ls,rs,ts,bs,tl,tr,bl,br)  wborder(stdscr,ls,rs,ts,bs,tl,tr,bl,br)
#define box( w, v, h )          wborder( w, v, v, h, h, 0, 0, 0, 0 )
#define clear()                 (clearok( stdscr, TRUE )==ERR?ERR:wclear( stdscr ))
#define clrtobot()              wclrtobot( stdscr )
#define clrtoeol()              wclrtoeol( stdscr )
#define delch()                 wdelch( stdscr )
#define deleteln()              wdeleteln( stdscr )
#define derwin(w,nl,nc,by,bx)   subwin((w),(nl),(nc),(by+(w)->_begy),(bx+(w)->_begx))
#define echochar(c)             (addch((chtype)c)==ERR?ERR:refresh())
#define erase()                 werase( stdscr )
#define getbegx(w)              (w)->_begx
#define getbegy(w)              (w)->_begy
#define getbegyx(w,y,x)         ( y = (w)->_begy, x = (w)->_begx )
#define getch()                 wgetch(stdscr)
#define getmaxx(w)              (w)->_maxx
#define getmaxy(w)              (w)->_maxy
#define getmaxyx(w,y,x)         ( y = (w)->_maxy, x = (w)->_maxx )
#define getparx(w)              (w)->_parx
#define getpary(w)              (w)->_pary
#define getparyx(w,y,x)         ( y = (w)->_pary, x = (w)->_parx )
#define getstr(str)             wgetstr( stdscr, str )
#define getyx(w,y,x)            ( y = (w)->_cury, x = (w)->_curx )
#define has_colors()            ((_cursvar.mono) ? FALSE : TRUE)
#define idcok(w,flag)           OK
#define idlok(w,flag)           OK
#define inch()                  (stdscr->_y[stdscr->_cury][stdscr->_curx])
#define inchstr( c )            inchnstr( c, stdscr->_maxx-stdscr->_curx )
#define innstr(str,n)           winnstr(stdscr,(str),(n))
#define insch( c )              winsch( stdscr, c )
#define insdelln(n)             winsdelln(stdscr,n)
#define insertln()              winsertln( stdscr )
#define insnstr(s,n)            winsnstr(stdscr,s,n)
#define insstr(s)               winsnstr(stdscr,s,(-1))
#define instr(str)              winnstr(stdscr,(str),stdscr->_maxx)
#define isendwin()              ((_cursvar.alive) ? FALSE : TRUE)
#define keypad(w,flag)          (w->_use_keypad  = flag)
#define leaveok(w,flag)         (w->_leave   = flag)
#define move(y,x)               wmove( stdscr, y, x )
#define mvaddch(y,x,c)          (move( y, x )==ERR?ERR:addch( c ))
#define mvaddchstr(y,x,c)       (move( y, x )==ERR?ERR:addchnstr( c, -1 ))
#define mvaddchnstr(y,x,c,n)    (move( y, x )==ERR?ERR:addchnstr( c, n ))
#define mvaddstr(y,x,str)       (move( y, x )==ERR?ERR:addstr( str ))
#define mvdelch(y,x)            (move( y, x )==ERR?ERR:wdelch( stdscr ))
#define mvgetch(y,x)            (move( y, x )==ERR?ERR:wgetch(stdscr))
#define mvgetstr(y,x,str)       (move( y, x )==ERR?ERR:wgetstr( stdscr, str ))
#define mvinch(y,x)             (move( y, x )==ERR?ERR:(stdscr->_y[y][x]))
#define mvinchstr(y,x,c)        (move( y, x )==ERR?ERR:inchnstr( c, stdscr->_maxx-stdscr->_curx ))
#define mvinchnstr(y,x,c,n)     (move( y, x )==ERR?ERR:inchnstr( c, n ))
#define mvinsch(y,x,c)          (move( y, x )==ERR?ERR:winsch( stdscr, c ))
#define mvinsnstr(y,x,s,n)      (move( y, x )==ERR?ERR:winsnstr(stdscr,s,n))
#define mvinsstr(y,x,s)         (move( y, x )==ERR?ERR:winsnstr(stdscr,s,(-1)))
#define mvinstr(y,x,str)        (move( y, x )==ERR?ERR:winnstr(stdscr,(str),stdscr->_maxx))
#define mvinnstr(y,x,str,n)     (move( y, x )==ERR?ERR:winnstr(stdscr,(str),(n)))
#define mvwaddch(w,y,x,c)       (wmove( w, y, x )==ERR?ERR:waddch( w, c ))
#define mvwaddchstr(w,y,x,c)    (wmove( w, y, x )==ERR?ERR:waddchnstr( w, c, -1 ))
#define mvwaddchnstr(w,y,x,c,n) (wmove( w, y, x )==ERR?ERR:waddchnstr( w, c, n ))
#define mvwaddrawch(w,y,x,c)    (wmove( w, y, x )==ERR?ERR:waddrawch( w, c ))
#define mvwaddrawstr(w,y,x,str) (wmove( w, y, x )==ERR?ERR:waddrawstr( w, str ))
#define mvwaddstr(w,y,x,str)    (wmove( w, y, x )==ERR?ERR:waddstr( w, str ))
#define mvwdelch(w,y,x)         (wmove( w, y, x )==ERR?ERR:wdelch( w ))
#define mvwgetch(w,y,x)         (wmove( w, y, x )==ERR?ERR:wgetch( w ))
#define mvwgetstr(w,y,x,str)    (wmove( w, y, x )==ERR?ERR:wgetstr( w, str ))
#define mvwinch(w,y,x)          (wmove( w, y, x )==ERR?ERR:((w)->_y[y][x]))
#define mvwinchstr(w,y,x,c)     (wmove( w, y, x )==ERR?ERR:winchnstr( w, c, (w)->_maxx-(w)->_curx ))
#define mvwinchnstr(w,y,x,c,n)  (wmove( w, y, x )==ERR?ERR:winchnstr( w, c, n ))
#define mvwinsch(w,y,x,c)       (wmove( w, y, x )==ERR?ERR:winsch( w, c ))
#define mvwinstr(w,y,x,str)     (wmove( w, y, x )==ERR?ERR:winnstr(w,str,(w)->_maxx))
#define mvwinnstr(w,y,x,str,n)  (wmove( w, y, x )==ERR?ERR:winnstr(w,str,n))
#define mvwinsnstr(w,y,x,s,n)   (wmove( w, y, x )==ERR?ERR:winsnstr(w,s,n))
#define mvwinsstr(w,y,x,s)      (wmove( w, y, x )==ERR?ERR:winsnstr(w,s,(-1)))
#define napms(ms)               delay_output(ms)
#define nl()                    (_cursvar.autocr = TRUE)
#define nonl()                  (_cursvar.autocr = FALSE)
#define notimeout(w,flag)       (OK)
#define pechochar(w,c)          (waddch(w,(chtype)c)==ERR?ERR:prefresh(w))
#define redrawwin(w)            wredrawln((w),0,(win)->_maxy)
#define refrbrk(flag)           (_cursvar.refrbrk = flag)
#define refresh()               wrefresh( stdscr )
#define scrl(n)                 wscrl(stdscr,n)
#define scroll(w)               wscrl((w),1)
#define scrollok(w,flag)        ((w)->_scroll  = flag)
#define setscrreg(top, bot)     wsetscrreg( stdscr, top, bot )
#define standend()              wattrset(stdscr, A_NORMAL)
#define standout()              wattrset(stdscr, A_STANDOUT)
#define touchline(w,y,n)        wtouchln((w),(y),(n),TRUE)
#define touchwin(w)             wtouchln((w),0,(w)->_maxy,TRUE)
#define traceoff()              {trace_on = FALSE;}
#define traceon()               {trace_on = TRUE;}
#define untouchwin(w)           wtouchln((w),0,((w)->_maxy),FALSE)
#define waddch(w, c)            PDC_chadd( w, (chtype)c, (bool)!(_cursvar.raw_out), TRUE )
#define waddchstr(w, c)         (waddchnstr( w, c, -1 ) )
#define wclear(w)               ( werase( w )==ERR?ERR:((w)->_clear = TRUE))
#define wechochar(w,c)          (waddch(w,(chtype)c)==ERR?ERR:wrefresh(w))
#define winch(w)                ((w)->_y[(w)->_cury][(w)->_curx])
#define winchstr(w, c)          (winchnstr( w, c, (w)->_maxx-(w)->_curx ) )
#define winsstr(w,str)          winsnstr((w),(str),(-1))
#define winstr(w,str)           winnstr((w),str,(w)->_maxx)
#define wstandend(w)            wattrset(w, A_NORMAL)
#define wstandout(w)            wattrset(w, A_STANDOUT)

#ifndef UNIX
#define cbreak()                (_cursvar.cbreak = TRUE)
#define nocbreak()              (_cursvar.cbreak = FALSE)
#define crmode()                (_cursvar.cbreak = TRUE)
#define nocrmode()              (_cursvar.cbreak = FALSE)
#define echo()                  (_cursvar.echo = TRUE)
#define noecho()                (_cursvar.echo = FALSE)
#define nodelay(w,flag)         (w->_nodelay = flag)
#endif

#ifdef     PDCURSES
#define addrawch( c )           waddrawch( stdscr, c )
#define addrawstr(str)          waddrawstr( stdscr, str )
#define insrawch( c )           winsrawch( stdscr, c )
#define waddrawch(w, c)         PDC_chadd( w, (chtype)c, FALSE, TRUE )
#define winsrawch(w, c)         PDC_chins( w, (chtype)c, FALSE )

/*
 *      FYI: Need to document these functions...
 */
#define title(s,a)              wtitle( stdscr, s, (chtype)a )
#define titleofs(ofs)           wtitleofs( stdscr, ofs )
#define wtitle(w,s,a)           (w->_title = s, w->_title_attr = (chtype)a)
#define wtitleofs(w,ofs)        (w->_title_ofs = ofs)
#endif

/*
 *      Load up curspriv.h.     This should be in the same place as
 *      stdlib.h.  We allow anyone who defines CURSES_LIBRARY to have
 *      access to our internal routines.  This provides quick
 *      PC applications at the expense of portability.
 */
#if defined     (CURSES_LIBRARY) | defined( INTERNAL)
#  include <curspriv.h>
#  include <stdlib.h>
#endif

#endif  /* __PDCURSES__ */
