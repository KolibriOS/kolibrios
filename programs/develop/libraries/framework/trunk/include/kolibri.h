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

#ifndef _KOLIBRI_H_
#define _KOLIBRI_H_

#define KOLIBRI_ACCESS_READ 0x0
#define KOLIBRI_CREATE 0x8

#define KOLIBRI_EVENT_IPC_MASK 0x40
#define KOLIBRI_EVENT_IPC 0x7

//
struct kolibri_memarea {
	void *addr;
	int error;		// or size
	char *name;
};

typedef struct kolibri_memarea kolibri_memarea_t;

//
#pragma pack(push,1)
struct kolibri_process_info {
	long cpu_using;				// +0
	short window_position;			// +4
	short window_position_slot;		// +6
	short reserved_0;			// +8
	char name[11];				// +10
	char reserved_const_0;			// +21
	unsigned long addr;			// +22
	long mem_using;				// +26
	long tid;				// +30
	long window_x;
	long window_y;
	long window_width;
	long window_height;
	short state;
	short reserved_const_1;
	long window_client_x;
	long window_client_y;
	long window_client_width;
	long window_client_height;
	char window_state;
};
#pragma pack(pop)

typedef struct kolibri_process_info kolibri_process_info_t;

//
//extern kolibri_IPC_area_t *kolibri_IPC_area;

// //
// int kolibri_IPC_set_area(void *area, int size);
// int kolibri_IPC_send(int tid, void *msg, int length);
// void kolibri_IPC_unlock();
// void kolibri_IPC_lock();
// int kolibri_IPC_init(void *area, int size);
// kolibri_IPC_message_t *kolibri_IPC_get_next_message();
// void kolibri_IPC_clear_buff();
int kolibri_event_wait();

/*
 * Memory functions
 */

kolibri_memarea_t kolibri_new_named_memory(char *name, int size, int flags);
int kolibri_heap_init();
void *kolibri_malloc(int nbytes);

/*
 * Events functions
 */
void kolibri_set_event_mask(int mask);

#endif
