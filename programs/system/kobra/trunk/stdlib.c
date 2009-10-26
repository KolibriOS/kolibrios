/***************************************************************************************************
 *  Copyright (C) Vasiliy Kosenko (vkos), 2009                                                     *
 *  Kobra is free software: you can redistribute it and/or modify it under the terms of the GNU    *
 *  General Public License as published by the Free Software Foundation, either version 3          *
 *  of the License, or (at your option) any later version.                                         *
 *                                                                                                 *
 *  Kobra is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without     *
 *  even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  *
 *  General Public License for more details.                                                       *
 *                                                                                                 *
 *  You should have received a copy of the GNU General Public License along with Kobra.            *
 *  If not, see <http://www.gnu.org/licenses/>.                                                    *
 ***************************************************************************************************/

/***************************************************************************************************
 *  Some stdlib functions                                                                          *
 ***************************************************************************************************/

#include "stdlib.h"

void *memcpy(void *dst, void *src, int length){
	void *value;

	if (length & 3) {		//length not aligned in 4 bytes use rep movsb
		__asm__ __volatile__(
		"movl %%edi,%%eax\n\t"
		"cld\n\t"
		"rep\n\t"
		"movsb"
		:"=D"(value)
		:"c"(length),"S"(src),"D"(dst)
		:"eax");
	} else {			//length aligned in 4 bytes use rep movsd
		length=length >> 2;//length=length/4
		__asm__ __volatile__(
		"movl %%edi,%%eax\n\t"
		"cld\n\t"
		"rep\n\t"
		"movsd"
		:"=D"(value)
		:"c"(length),"S"(src),"D"(dst)
		:"eax");

	}
	return(value);
}

char strcmp(char *s1, char *s2){
	while (*s1) {
		if (*s1++ != *s2++) {
			return *--s1-*--s2;
		}
	}
	return 0;
}

int strlen(char *s){
	int i;
	while (*s++) {
		++i;
	}
	return i;
}

char *strcpy(char *dest, char *src){
	while (*src) {
		*dest++ = *src++;
	}
	*dest = '\0';
}
