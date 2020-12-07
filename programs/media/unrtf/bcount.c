
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
 * Program name:   bcount
 * Author name:    Zach Smith
 * Create date:    15 Oct 00
 * Purpose:        Counts the number of opening and closing braces while
 *                 reading from stdin.
 *--------------------------------------------------------------------*/



#include <stdio.h>
main (){
	int n1,n2;
	int ch;
	n1=n2=0;
	while (EOF!=(ch=getchar())) {
		if (ch=='}') ++n1;
		if (ch=='{') ++n2;
	}
	printf ("{=%d, }=%d\n", n2,n1);
}


