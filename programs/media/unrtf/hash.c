
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
 * Module name:    hash
 * Author name:    Zach Smith
 * Create date:    01 Sep 00
 * Purpose:        Word-hash management. Words are put into a hash and an
 *                 identifier is returned. This is used to save us from
 *                 doing multiple mallocs for recurring strings such as
 *                 'the' and \par. This is not a big issue under Unix,
 *                 but it is under other OSes and anyway, waste not want not.
 *----------------------------------------------------------------------
 * Changes:
 * 08 Apr 01, tuorfa@yahoo.com: check for out of memory after malloc.
 * 21 Apr 01, tuorfa@yahoo.com: signed to conversion unsigned bug
 * 03 Aug 01, tuorfa@yahoo.com: fixes for using 16-bit compiler
 * 22 Sep 01, tuorfa@yahoo.com: added function-level comment blocks 
 *--------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "error.h"
#include "main.h"
#include "malloc.h"


typedef struct _hi {
	struct _hi *next;
	char *str;
	unsigned long value;
}
HashItem;


/* Index by first char of string */
static HashItem *hash[256];
static unsigned long hash_length[256];
static unsigned long hash_value=0;



/*========================================================================
 * Name:	hash_init
 * Purpose:	Clear the hash table.
 * Args:	None.
 * Returns:	None.
 *=======================================================================*/

void
hash_init ()
{
	int i;
	for (i=0; i<256; i++) {
		hash[i]=NULL;
		hash_length[i]=0;
	}
}



/*========================================================================
 * Name:	hash_stats
 * Purpose:	Prints to stderr the number of words stored.
 * Args:	None.
 * Returns:	None.
 *=======================================================================*/

void
hash_stats ()
{
	int i;
	unsigned long total=0;
	for (i=0; i<256; i++) {
		total += hash_length[i];
	}
	fprintf (stderr,"%lu words were hashed.\n", total);
}



/*========================================================================
 * Name:	hashitem_new
 * Purpose:	Creates a new linked list item for the hash table.
 * Args:	String.
 * Returns:	HashItem.
 *=======================================================================*/

static HashItem *
hashitem_new (char *str)
{
	HashItem *hi;
	unsigned long i;

	hi=(HashItem*) my_malloc(sizeof(HashItem));
	if (!hi)
		error_handler ("out of memory");
	memset ((void*)hi, 0, sizeof (HashItem));

	hi->str = my_strdup(str);

	i = *str;
	if (i=='\\') i=str[1];
	i <<= 24;
	hi->value = i | (hash_value++ & 0xffffff);
	hi->next = NULL;

#if 0
	if (debug_mode) {
		printf ("<!-- storing val %08lx str %s -->\n",
			hi->value, hi->str);
	}
#endif

	return hi;
}


/*========================================================================
 * Name:	hash_get_index
 * Purpose:	Given a string, returns the "index" i.e. the word identifier.
 * Args:	String.
 * Returns:	Index.
 *=======================================================================*/

unsigned long
hash_get_index (char *str)
{
	unsigned short index;
	HashItem *hi;
	char ch;

	ch = *str;
	if (ch=='\\' && *(str+1))
		ch = *(str+1); 
	index = (unsigned) ch;
	hi = hash[index];
	while (hi) {
		if (!strcmp(hi->str,str))
			return hi->value;
		hi=hi->next;
	}
	/* not in hash */
	hi = hashitem_new (str);
	hi->next = hash[index];
	hash [index] = hi;
	++hash_length [index];
	return hi->value;
}


/*========================================================================
 * Name:	hash_get_string
 * Purpose:	Given the index (word identifier) returns the word string.
 * Args:	Index.
 * Returns:	String, or NULL if not found.
 *=======================================================================*/

char*
hash_get_string (unsigned long value)
{
	int index;
	HashItem *hi;
	index = value >> 24;
	hi = hash[index];
	while (hi) {
		if (hi->value == value)
			return hi->str;
		hi=hi->next;
	}
	warning_handler ("word not in hash");
	return NULL;
}



