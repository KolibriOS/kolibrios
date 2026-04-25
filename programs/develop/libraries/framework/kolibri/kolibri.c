/***************************************************************************************************
 *  Copyright (C) Vasiliy Kosenko (vkos), 2009                                                     *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the *
 *  GNU General Public License as published by the Free Software Foundation, either version 3      *
 *  of the License, or (at your option) any later version.                                         *
 *                                                                                                 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;      *
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See  *
 *  the GNU General Public License for more details.                                               *
 *                                                                                                 *
 *  You should have received a copy of the GNU General Public License along with this program.     *
 *  If not, see <http://www.gnu.org/licenses/>.                                                    *
 ***************************************************************************************************/

/***************************************************************************************************
 *  kolibri.c - KolibriOS system functions                                                         *
 ***************************************************************************************************/

#include "kolibri.h"
#include "malloc.h"

/*
 * Other process/thread functions
 */

int kolibri_get_my_tid(){
	kolibri_process_info_t *info = malloc(0x400);
	int tid;
	
	kolibri_get_process_info(info, -1);
	tid = info->tid;
	free(info);
	
	return tid;
}

int kolibri_get_process_info(kolibri_process_info_t *info, int slot){
	int max_slot;
	
	asm("int $0x40":"=a"(max_slot):"a"(9),"b"(info),"c"(slot));
	
	return max_slot;
}

/*
 * Memory functions
 */

kolibri_memarea_t kolibri_new_named_memory(char *name, int size, int flags){
	kolibri_memarea_t area;
	
	asm("pushl %%esi\nmovl %6, %%esi\nint $0x40\npopl %%esi":"=a"(area.addr),"=d"(area.error):"a"(68),"b"(22),"c"(name),"d"(size),"g"(flags));
	
	return area;
}

int kolibri_heap_init(){
	int size;
	
	asm("int $0x40":"=a"(size):"a"(68),"b"(11));
	
	return size;
}

void *kolibri_malloc(int nbytes){
	void *addr;
	
	asm("int $0x40":"=a"(addr):"a"(68),"b"(12),"c"(nbytes));
	
	return addr;
}

/*
 * Events functions
 */

int kolibri_event_mask;

int kolibri_event_set_mask(int mask){
	kolibri_event_mask = mask;
	asm("int $0x40"::"a"(40),"b"(kolibri_event_mask));
	return 0;
}

int kolibri_event_get_mask(int mask){
	return kolibri_event_mask;
}

void kolibri_event_add_mask(int amask){
	kolibri_event_set_mask(kolibri_event_mask | amask);
}

void kolibri_event_sub_mask(int amask){
	kolibri_event_set_mask(kolibri_event_mask & ~amask);
}

int kolibri_event_wait(int time){
	int event;
	if (!time) {
		asm("int $0x40":"=a"(event):"a"(11));
	} else if (time == -1) {
		asm("int $0x40":"=a"(event):"a"(10));
	} else {
		asm("int $0x40":"=a"(event):"a"(23),"b"(time));
	}
	return event;
}
