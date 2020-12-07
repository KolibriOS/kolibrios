
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
 * Module name:    word.h
 * Author name:    Zach Smith
 * Create date:    1 Sept 2000
 * Purpose:        Definitions for Word class.
 *----------------------------------------------------------------------
 * Changes:
 *--------------------------------------------------------------------*/



#ifndef _WORD
#define _WORD


typedef struct _w {
	unsigned long hash_index;
	struct _w * next;
	struct _w * child;
}
Word;


extern Word* word_new (char*);
extern void word_free (Word*);
extern Word* word_read (FILE*);
extern char* word_string (Word*);
extern void word_dump (Word*);
extern void word_print_html (Word*);


#define _WORD
#endif

