
/*=============================================================================
   GNU UnRTF, a command-line program to convert RTF documents to other formats.
   Copyright (C) 2000,2001 Zachary Thayer Smith

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   The author is reachable by electronic mail at tuorfa@yahoo.com.
=============================================================================*/


/*----------------------------------------------------------------------
 * Module name:    defs.h
 * Author name:    Zach Smith
 * Create date:    1 Sept 2000
 * Purpose:        Basic definitions plus externs for UnRTF
 *----------------------------------------------------------------------
 * Changes:
 * 21 Oct 00, tuorfa@yahoo.com: moved program version to this file
 * 08 Apr 01, tuorfa@yahoo.com: updated usage info.
 * 08 Sep 01, tuorfa@yahoo.com: added PROGRAM_NAME.
 * 19 Sep 01, tuorfa@yahoo.com: added PROGRAM_WEBSITE.
 *--------------------------------------------------------------------*/


#define PROGRAM_VERSION "0.18.1"
#define PROGRAM_NAME "UnRTF"
#define PROGRAM_WEBSITE "http://www.geocities.com/tuorfa"


/* Select the language for reporting of file creation/modificaton dates */
#define ENGLISH
#if 0
#define FRANCAIS
#define ITALIANO
#endif


#define TRUE (1)
#define FALSE (0)


#define USAGE "unrtf [--version] [--help] [--nopict|-n] [--html] [--text] [--vt] [--latex] [--ps] [--wpml] [-t html|text|vt|latex|ps|wpml] <filename>"


/* Default names for RTF's default fonts */
#define FONTNIL_STR	"Times,TimesRoman,TimesNewRoman"
#define FONTROMAN_STR	"Times,Palatino"
#define FONTSWISS_STR	"Helvetica,Arial"
#define FONTMODERN_STR	"Courier,Verdana"
#define FONTSCRIPT_STR	"Cursive,ZapfChancery"
#define FONTDECOR_STR	"ZapfChancery"
#define FONTTECH_STR	"Symbol"

