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
*
*                          CURSPRIV.H
*
* Header file for definitions and declarations for the
* PDCurses package. These definitions should not be generally
* accessible to programmers, but are provided if the applications
* programmer decides to make the decision in favor of speed on a
* PC over portability.
*
* Revision History:
* Frotz 1.5Beta 900714  Added many levels of compiler support.
*                       Added mixed prototypes for all "internal" routines.
*                       Removed all assembly language.  Added EGA/VGA
*                       support.  Converted all #ifdef to #if in all
*                       modules except CURSES.H and CURSPRIV.H.
*                       Always include ASSERT.H.  Added support for an
*                       external malloc(), calloc() and free().
*                       Added support for FAST_VIDEO (direct-memory writes).
*                       Added various memory model support (for FAST_VIDEO).
*                       Added much of the December 1988 X/Open Curses
*                       specification.
* bl    1.3     881005  All modules lint-checked with MSC '-W3' and turbo'C'
*                       '-w -w-pro' switches.
* bl    1.2     881002  Support (by #ifdef UCMASM) for uppercase-only
*                       assembly routine names. If UCMASM if defined,
*                       all assembler names are #defined as upper case.
*                       Not needed if you do "MASM /MX. Also missing
*                       declaration of cursesscroll(). Fixes thanks to
*                       N.D. Pentcheff
* bl    1.1     880306  Add _chadd() for raw output routines.
* bl    1.0     870515  Release.
*
*/

#ifndef __CURSES_INTERNALS__
#define __CURSES_INTERNALS__

/* Always include... */
#include <assert.h>



/*----------------------------------------------------------------------
*       MEMORY MODEL SUPPORT:
*
*       MODELS
*               TINY            cs,ds,ss all in 1 segment (not enough memory!)
*               SMALL           cs:1 segment,           ds:1 segment
*               MEDIUM          cs:many segments        ds:1 segment
*               COMPACT         cs:1 segment,           ds:many segments
*               LARGE           cs:many segments        ds:many segments
*               HUGE            cs:many segments        ds:segments > 64K
*/
#ifdef  __TINY__
#  define SMALL 1
#endif
#ifdef  __SMALL__
#  define SMALL 1
#endif
#ifdef  __MEDIUM__
#  define MEDIUM 1
#endif
#ifdef  __COMPACT__
#  define COMPACT 1
#endif
#ifdef  __LARGE__
#  define LARGE 1
#endif
#ifdef  __HUGE__
#  define HUGE 1
#endif


/*----------------------------------------------------------------------
*       OPERATING SYSTEM SUPPORT:
*
*               DOS             The one we all know and love:-}
*               OS/2            The new kid on the block.
*               FLEXOS          A Real-time, protected-mode OS from
*                               Digital Research, Inc.
*				(AKA, the 4680 from IBM...)
*/

/*----------------------------------------*/


/*----------------------------------------------------------------------
*       MALLOC DEBUGGING SUPPORT:
*
*       Set EMALLOC and EMALLOC_MAGIC in order to use your private
*       versions of malloc(), calloc(), and free().  This can help,
*       but not solve, your malloc problems when debugging...
*
*/
#ifndef	INTERNAL
#  define EMALLOC 0             /* Disable External Malloc	        */
#else
#  define EMALLOC 0             /* Enable/Disable External Malloc       */
#  define EMALLOC_MAGIC  0x0C0C /* Our magic indicator that we should   */
                                /* use our external malloc rather than  */
                                /* the runtime's malloc.                */
#endif


/*----------------------------------------------------------------------*/
/* window properties */
#define _SUBWIN         0x01    /* window is a subwindow            */
#define _ENDLINE        0x02    /* last winline is last screen line */
#define _FULLWIN        0x04    /* window fills screen              */
#define _SCROLLWIN      0x08    /* window lwr rgt is screen lwr rgt */
#define _PAD            0x10    /* X/Open Pad.                      */
#define _SUBPAD         0x20    /* X/Open subpad.                   */




/*----------------------------------------------------------------------*/
/* Miscellaneous */
#define _INBUFSIZ       512     /* size of terminal input buffer */
#define _NO_CHANGE      -1      /* flags line edge unchanged     */




/* @@@ THESE SHOULD BE INDIVIDUAL FUNCTIONS, NOT MACROS! */
#define _BCHAR          0x03    /* Break char	    (^C)         */
#define _ECHAR          0x08    /* Erase char	    (^H)         */
#define _DWCHAR         0x17    /* Delete Word char (^W)         */
#define _DLCHAR         0x15    /* Delete Line char (^U)         */
#define _GOCHAR         0x11    /* ^Q character                  */
#define _PRINTCHAR      0x10    /* ^P character                  */
#define _STOPCHAR       0x13    /* ^S character                  */
#define  NUNGETCH       20      /* max # chars to ungetch()      */




/* Setmode stuff */
struct cttyset
{
	bool	been_set;
	SCREEN	saved;
};

extern struct cttyset c_sh_tty;         /* tty modes for shell_mode */
extern struct cttyset c_pr_tty;         /* tty modes for prog_mode  */
extern struct cttyset c_save_tty;
extern struct cttyset c_save_trm;

/* Printscan stuff */
extern char c_printscanbuf[];           /* buffer used during I/O */

/* tracing flag */
extern bool trace_on;

/* Strget stuff */
extern char*    c_strbeg;

/* doupdate stuff */
extern WINDOW*  twin;                   /* used by many routines */

/* Monitor (terminal) type information */
#define _NONE           0x00
#define _MDA            0x01
#define _CGA            0x02
#define _EGACOLOR       0x04
#define _EGAMONO        0x05
#define _VGACOLOR       0x07
#define _VGAMONO        0x08
#define _MCGACOLOR      0x0a
#define _MCGAMONO       0x0b
#define _FLEXOS         0x20            /* A Flexos console */
#define _MDS_GENIUS     0x30
#define _UNIX_COLOR     0x40
#define _UNIX_MONO      0x41

/* Text-mode font size information */
#define _FONT8  8
#define _FONT14 14
#define _FONT15 15              /* GENIUS */
#define _FONT16 16


/*----------------------------------------------------------------------
*       ANSI C prototypes.  Be sure that your compiler conditional
*       compilation definitions above define ANSI to be non-zero
*       if you compiler supports prototypes.
*/
#ifdef     ANSI
#  ifdef  CPLUSPLUS
     extern "C" {
#  endif
int             PDC_backchar( WINDOW*, char*, int* );
bool            PDC_breakout( void );
int             PDC_chadd( WINDOW*, chtype, bool, bool );
bool            PDC_check_bios_key( void );
int             PDC_chg_attr( WINDOW*, chtype, int, int, int, int );
int             PDC_chins( WINDOW*, chtype, bool );
int             PDC_clr_scrn( WINDOW* );
int             PDC_clr_update( WINDOW* );
int             PDC_copy_win( WINDOW *,WINDOW *,int,int,int,int,int,int,int,int,bool );
int             PDC_cursor_off( void );
int             PDC_cursor_on( void );
int             PDC_fix_cursor( int );
int             PDC_gattr( void );
int             PDC_get_bios_key( void );
int             PDC_get_columns( void );
bool            PDC_get_ctrl_break( void );
int             PDC_get_cur_col( void );
int             PDC_get_cur_row( void );
int             PDC_get_cursor_pos( int*, int* );
int             PDC_get_cursor_mode( void );
int             PDC_get_font( void );
int             PDC_get_rows( void );
int             PDC_gotoxy( int, int );
int             PDC_init_atrtab(void);
WINDOW*         PDC_makenew( int, int, int, int );
int             PDC_newline( WINDOW*, int );
int             PDC_print( int, int, int );
int             PDC_putc( chtype, chtype );
int             PDC_putchar( chtype );
int             PDC_putctty( chtype, chtype );
int             PDC_rawgetch( void );
int             PDC_sanity_check( int );
int             PDC_scr_close( void );
int             PDC_scr_open( SCREEN*, bool );
int             PDC_scroll( int, int, int, int, int, chtype );
int             PDC_set_80x25( void );
int             PDC_set_ctrl_break( bool );
int             PDC_set_cursor_mode( int, int );
int             PDC_set_font( int );
int             PDC_set_rows( int );
int             PDC_split_plane( WINDOW*, char*, char*, int, int, int, int );
int             PDC_sysgetch( void );
bool            PDC_transform_line( int );
void            PDC_usleep( long );
int             PDC_validchar( int );

#if defined( OS2 ) && !defined( EMXVIDEO )
VIOCONFIGINFO   PDC_query_adapter_type( void );
VIOMODEINFO     PDC_get_scrn_mode( void );
int             PDC_set_scrn_mode( VIOMODEINFO );
bool            PDC_scrn_modes_equal (VIOMODEINFO, VIOMODEINFO);
#else
int             PDC_query_adapter_type( void );
int             PDC_get_scrn_mode( void );
int             PDC_set_scrn_mode( int );
bool            PDC_scrn_modes_equal (int, int);
#endif

#ifdef  FLEXOS
int             PDC_flexos_8bitmode( void );
int             PDC_flexos_16bitmode( void );
char*           PDC_flexos_gname( void );
#endif

#ifdef UNIX
int             PDC_kbhit(void);
int             PDC_setup_keys(void);
#endif

#if defined (XCURSES)
int            XCurses_redraw_curscr(void);
int            XCurses_display_cursor(int,int ,chtype ,int ,int ,chtype );
int            XCurses_rawgetch(void);
bool           XCurses_kbhit(void);
int            XCurses_instruct(int);
int            XCurses_transform_line(long *, int , int , int );
int            Xinitscr(void);
int            Xendwin(void);
#endif

#ifdef PDCDEBUG
void            PDC_debug( char*,... );
#endif

#ifdef  REGISTERWINDOWS
bool            PDC_inswin( WINDOW*, WINDOW* );
int             PDC_addtail( WINDOW* );
int             PDC_addwin( WINDOW*, WINDOW* );
int             PDC_rmwin( WINDOW* );
WINDS*          PDC_findwin( WINDOW* );
#endif
#  ifdef  CPLUSPLUS
     }
#  endif
#endif

#define PDC_COLOR_PAIRS  64
#define PDC_OFFSET        8
#define MAX_ATRTAB      272
#define chtype_attr(ch)  ((atrtab[((ch >> 8) & 0xFF)] << 8) & A_ATTRIBUTES)

#include<menuet/os.h>
#include<menuet/textcon.h>

#endif /* __CURSES_INTERNALS__*/
