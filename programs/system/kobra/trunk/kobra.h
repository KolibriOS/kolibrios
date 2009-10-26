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
 *  Kobra (Kolibri Bus for Reaching Applications) is daemon for advanced & easier applications     *
 *  communication.                                                                                 *
 *  This is inside header file.
 ***************************************************************************************************/

#ifndef _KOBRA_H_
#define _KOBRA_H_

#include "kolibri.h"

#define KOBRA_CMD_REGISTER		'R'
#define KOBRA_CMD_JOIN			'J'
#define KOBRA_CMD_UNJOIN		'U'
#define KOBRA_CMD_SEND			'S'
#define KOBRA_CMD_GET_LIST_NAME		'G'

#define KOBRA_MEMAREA_NAME		"kobra_list"
#define KOBRA_MEMAREA_NAME_LENGTH	11
#define KOBRA_MEM_SIZE			0x1000

struct group_list {
	struct group_list *next;
	struct group_list *previos;
	
	char *name;
	struct thread_list *thread_list;
};

typedef struct group_list group_list_t;

struct thread_list {
	struct thread_list *next;
	struct thread_list *previos;
	
	int tid;
};

typedef struct thread_list thread_list_t;

void message_handle(kolibri_IPC_message_t *message);
thread_list_t *find_tid(group_list_t *group, int tid);
void kobra_register(int tid);
void add_to_group(group_list_t *group, int tid);
void remove_from_group(group_list_t *group, thread_list_t *thread);
group_list_t *find_group(char *name);
void send_group_message(group_list_t *group, int tid, char *message, int length);
void remove_group(group_list_t *group);
group_list_t *create_group(char *name);
group_list_t *new_group_list(char *name);
thread_list_t *new_thread_list(int tid);

#endif

// kolibri_IPC_message_t
// bool
// IPC const
// NULL
