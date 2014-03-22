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
#ifndef CURSOS2_INCL
#define CURSOS2_INCL 1

#ifdef CURSES__32BIT__

#if (NOVIO)
#define KbdSetStatus Kbd32SetStatus
#define KbdGetStatus Kbd32GetStatus
#define KbdCharIn Kbd32CharIn
#define KbdPeek   Kbd32Peek
#define KbdFlushBuffer Kbd32FlushBuffer

#define VioGetMode Vio32GetMode
#define VioSetMode Vio32SetMode
#define VioGetCurPos Vio32GetCurPos
#define VioSetCurPos Vio32SetCurPos
#define VioGetCurType Vio32GetCurType
#define VioSetCurType Vio32SetCurType
#define VioScrollDn Vio32ScrollDn
#define VioScrollUp Vio32ScrollUp
#define VioGetConfig Vio32GetConfig
#define VioWrtTTY Vio32WrtTTY
#define VioReadCellStr Vio32ReadCellStr
#define VioWrtCellStr Vio32WrtCellStr
#define VioWrtNAttr Vio32WrtNAttr
#endif   /*  NOVIO */

#define FARKeyword

#define CURS_INCL_VIO_KBD


#ifdef __EMX__
#  ifndef USE_OS2_H
#   ifdef EMXVIDEO		/* Define to use emx dos compatible video */
#    include <stdlib.h>
#    include <sys/video.h>
#    ifdef USE_OS2_H
#     undef USE_OS2_H	/* And we can use the extra compile speed... */
#    endif
#   else
#    define USE_OS2_H
#   endif
#  endif
#  define APIENTRY
#else
#  define APIRET ULONG
#endif

#else


#   define FARKeyword far
#   define APIRET USHORT

#ifdef USE_OS2_H
#   define INCL_VIO
#   define INCL_KBD
#else
#   define CURS_INCL_VIO_KBD
#endif


#endif   /* __32BIT__ */

#ifndef EMXVIDEO

/* if USE_OS2_H is defined then use the os2.h that comes with your compiler ...*/

#ifdef USE_OS2_H
#  include <os2.h>
#else

/* ... otherwise use these definitions */

#  include <os2def.h>

#endif

#ifdef CURS_INCL_VIO_KBD

typedef SHANDLE         HKBD;
typedef HKBD    FARKeyword *   PHKBD;


typedef SHANDLE         HVIO;
typedef HVIO    FARKeyword *   PHVIO;



typedef struct _KBDINFO {
        USHORT cb;
        USHORT fsMask;
        USHORT chTurnAround;
        USHORT fsInterim;
        USHORT fsState;
        }KBDINFO;
typedef KBDINFO FARKeyword *PKBDINFO;


USHORT APIENTRY KbdSetStatus(
        PKBDINFO    pkbdinfo,
        HKBD        hkbd );


USHORT APIENTRY KbdGetStatus(
        PKBDINFO    pkbdinfo,
        HKBD        hdbd  );


typedef struct _KBDKEYINFO {
        UCHAR    chChar;    /* ASCII character code                     */
        UCHAR    chScan;    /* Scan Code                                */
        UCHAR    fbStatus;
        UCHAR    bNlsShift;
        USHORT   fsState;
        ULONG    time;
        }KBDKEYINFO;
typedef KBDKEYINFO FARKeyword *PKBDKEYINFO;

#define IO_WAIT     0
#define IO_NOWAIT   1

USHORT APIENTRY KbdCharIn(
        PKBDKEYINFO pkbci,
        USHORT      fWait,      /* IO_WAIT, IO_NOWAIT     */
        HKBD        hkbd);

USHORT APIENTRY KbdPeek(
        PKBDKEYINFO  pkbci,
        HKBD         hkbd );

USHORT APIENTRY KbdFlushBuffer(
        HKBD hkbd);


typedef struct _VIOMODEINFO {
        USHORT cb;
        UCHAR  fbType;
        UCHAR  color;
        USHORT col;     /* number of text columns                       */
        USHORT row;     /* number of text rows                          */
        USHORT hres;    /* horizontal resolution                        */
        USHORT vres;    /* vertical resolution                          */
        UCHAR  fmt_ID;
        UCHAR  attrib;  /* number of attributes                         */
        ULONG  buf_addr;
        ULONG  buf_length;
        ULONG  full_length;
        ULONG  partial_length;
        PCH    ext_data_addr;
        } VIOMODEINFO;
typedef VIOMODEINFO FARKeyword *PVIOMODEINFO;


USHORT APIENTRY VioGetMode(
        PVIOMODEINFO  pvioModeInfo,
        HVIO          hvio);


USHORT APIENTRY VioSetMode(
        PVIOMODEINFO  pvioModeInfo,
        HVIO          hvio);


USHORT APIENTRY VioGetCurPos(
        PUSHORT    pusRow,
        PUSHORT    pusColumn,
        HVIO       hvio );


USHORT APIENTRY VioSetCurPos(
        USHORT  usRow,
        USHORT  usColumn,
        HVIO    hvio);

typedef struct _VIOCURSORINFO {
        USHORT   yStart;
        USHORT   cEnd;
        USHORT   cx;
        USHORT   attr;   /* -1=hidden cursor, any other=normal cursor   */
        } VIOCURSORINFO;
typedef VIOCURSORINFO FARKeyword *PVIOCURSORINFO;


USHORT APIENTRY VioGetCurType(
       PVIOCURSORINFO pvioCursorInfo,
       HVIO           hvio );


USHORT APIENTRY VioSetCurType(
        PVIOCURSORINFO pvioCursorInfo,
        HVIO           hvio );

USHORT APIENTRY VioScrollDn(
        USHORT  usTopRow,
        USHORT  usLeftCol,
        USHORT  usBotRow,
        USHORT  usRightCol,
        USHORT  cbLines,
        PBYTE   pCell,
        HVIO    hvio );



USHORT APIENTRY VioScrollUp(
        USHORT  usTopRow,
        USHORT  usLeftCol,
        USHORT  usBotRow,
        USHORT  usRightCol,
        USHORT  cbLines,
        PBYTE   pCell,
        HVIO    hvio );


   /* VIOCONFIGINFO.adapter constants */

   #define DISPLAY_MONOCHROME      0x0000
   #define DISPLAY_CGA             0x0001
   #define DISPLAY_EGA             0x0002
   #define DISPLAY_VGA             0x0003
   #define DISPLAY_8514A           0x0007

   /* VIOCONFIGINFO.display constants */

   #define MONITOR_MONOCHROME      0x0000
   #define MONITOR_COLOR           0x0001
   #define MONITOR_ENHANCED        0x0002
   #define MONITOR_8503            0x0003
   #define MONITOR_851X_COLOR      0x0004
   #define MONITOR_8514            0x0009

typedef struct _VIOCONFIGINFO {
        USHORT  cb;
        USHORT  adapter;
        USHORT  display;
        ULONG   cbMemory;
        USHORT  Configuration;
        USHORT  VDHVersion;
        USHORT  Flags;
        ULONG   HWBufferSize;
        ULONG   FullSaveSize;
        ULONG   PartSaveSize;
        USHORT  EMAdaptersOFF;
        USHORT  EMDisplaysOFF;
        } VIOCONFIGINFO;
typedef VIOCONFIGINFO FARKeyword *PVIOCONFIGINFO;


USHORT APIENTRY VioGetConfig(
        USHORT         usConfigId,  /* Reserved (must be 0)             */
        PVIOCONFIGINFO pvioin,
        HVIO           hvio );

USHORT APIENTRY VioWrtTTY(
        PCH     pch,
        USHORT  cb,
        HVIO    hvio );

USHORT APIENTRY VioReadCellStr(
        PCH       pchCellStr,
        PUSHORT   pcb,
        USHORT    usRow,
        USHORT    usColumn,
        HVIO      hvio );

USHORT APIENTRY VioWrtCellStr(
        PCH      pchCellStr,
        USHORT   cb,
        USHORT   usRow,
        USHORT   usColumn,
        HVIO     hvio );

USHORT APIENTRY VioWrtNAttr(
        PBYTE     pAttr,
        USHORT    cb,
        USHORT    usRow,
        USHORT    usColumn,
        HVIO      hvio );


USHORT APIENTRY VioWrtNCell(
        PBYTE   pCell,
        USHORT  cb,
        USHORT  usRow,
        USHORT  usColumn,
        HVIO    hvio );

#endif

#endif


#ifndef KEYBOARD_ASCII_MODE
#define KEYBOARD_ASCII_MODE 0x0008
#endif

#ifndef KEYBOARD_BINARY_MODE
#define KEYBOARD_BINARY_MODE 0x0004
#endif

#endif	/* !EMXVIDEO */
