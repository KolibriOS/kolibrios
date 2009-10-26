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
 ***************************************************************************************************/

#include "kobra.h"
#include "heap.h"
#include "malloc.h"
#include "kolibri.h"
#include "defs.h"
#include "stdlib.h"

group_list_t *main_group_list;
thread_list_t *main_thread_list;

kolibri_memarea_t main_memarea;
Heap memarea_heap;

void *return_0(Heap *wheap, int nbytes){
	return NULL;
}

void *memarea_alloc(int nbytes){
	return halMemAlloc(&memarea_heap, nbytes);
}

void memarea_free(void *addr){
	halMemFree(&memarea_heap, addr);
}

void main(){
	malloc_init();
	
	// Alloc memory for thread&group lists
	main_memarea = kolibri_new_named_memory(KOBRA_MEMAREA_NAME, KOBRA_MEM_SIZE, KOLIBRI_ACCESS_READ|KOLIBRI_CREATE);
	halMemHeapInit(&memarea_heap, &return_0, main_memarea.addr, KOBRA_MEM_SIZE);
	
	// Init main group list
	create_group("main");
	main_group_list->next = main_group_list->previos = main_group_list;
	main_group_list->thread_list = main_thread_list = new_thread_list(kolibri_get_my_tid());
	main_thread_list->next = main_thread_list->previos = main_thread_list;
	
	kolibri_IPC_init(malloc(0x1000), 0x1000);
	kolibri_IPC_unlock();
	
	// Set event mask
	kolibri_set_event_mask(KOLIBRI_IPC_EVENT_MASK);
	
	while (1) {
		if (kolibri_event_wait() != KOLIBRI_IPC_EVENT) {		// Just ignore this error
			continue;
		}
		
		message_handle(kolibri_IPC_get_next_message());
		kolibri_IPC_clear_buff();
	}
}

void message_handle(kolibri_IPC_message_t *message){
	char *msg = (char *)message+sizeof(kolibri_IPC_message_t);
	char cmd = msg[0];
	thread_list_t *thread = find_tid(main_group_list, message->tid);
	group_list_t *group;
	int i;
	
	if (cmd == KOBRA_CMD_REGISTER && !thread) {
		kobra_register(message->tid);
	} else if (thread) {
		switch (cmd) {
			case KOBRA_CMD_JOIN:
				if (message->length < 3 || msg[message->length-1] != '\0') {
					// Here should be some error handler
					return;
				}
				
				if (!(group = find_group(msg+1))){
					group = create_group(msg+1);
				}
				add_to_group(group, message->tid);
				break;
			case KOBRA_CMD_UNJOIN:
				if (message->length < 3 || msg[message->length-1] != '\0') {
					// Here should be some error handler
					return;
				}
				
				if ((group = find_group(msg+1)) && (thread = find_tid(group, message->tid))) {
					remove_from_group(group, thread);
				}
				break;
			case KOBRA_CMD_SEND:
				if (message->length < 4) {
					// Here should be some error handler
					return;
				}
				
				// Check if group name is correct
				for (i = 1; i < message->length-1 && msg[i]; ++i);
				if (msg[i]) {
					// Here should be some error handler
					return;
				}
				
				group = find_group(msg+1);
				if (!group) {
					// Here should be some error handler
					return;
				}
				
				send_group_message(group, message->tid, msg+i+1, message->length-i-1);
				break;
			case KOBRA_CMD_GET_LIST_NAME:
				// This is temporary realisation
				kolibri_IPC_send(message->tid, KOBRA_MEMAREA_NAME, KOBRA_MEMAREA_NAME_LENGTH);
			default:
				// Here should be some error handler
				return;
		}
	}
}

thread_list_t *find_tid(group_list_t *group, int tid){
	thread_list_t *thread_list = group->thread_list, *thread = thread_list;
	
	if (!thread_list) {
		return NULL;
	}
	
	do {
		if (thread->tid == tid) {
			return thread;
		}
		thread = thread->next;
	} while (thread != thread_list);
	
	return NULL;
}

void kobra_register(int tid){
	add_to_group(main_group_list, tid);
}

void add_to_group(group_list_t *group, int tid){
	thread_list_t *thread_list = group->thread_list, *thread_last;
	
	if (!thread_list) {
		thread_list = group->thread_list = new_thread_list(tid);
		thread_list->previos = thread_list->next = thread_list;
	} else if (thread_list == (thread_last = thread_list->previos)) {
		thread_last = thread_list->next = thread_list->previos = new_thread_list(tid);
		thread_last->next = thread_last->previos = thread_list;
	} else {
		thread_last->next = thread_list->previos = new_thread_list(tid);
		thread_last->next->next = thread_list;
		thread_last->next->previos = thread_last;
	}
}

void remove_from_group(group_list_t *group, thread_list_t *thread){
	if (thread->next == thread) {
		remove_group(group);
	} else {
		thread->next->previos = thread->previos;
		thread->previos->next = thread->next;
		if (group->thread_list == thread) {
			group->thread_list = thread->next;
		}
	}
	
	memarea_free(thread);
}

group_list_t *find_group(char *name){
	group_list_t *group_list = main_group_list, *group = group_list;
	
	if (group_list) {
		do {
			if (!strcmp(group->name, name)) {
				return group;
			}
			group = group->next;
		} while (group != group_list);
	}
	
	return NULL;
}

void send_group_message(group_list_t *group, int tid, char *message, int length){
	thread_list_t *thread_list = group->thread_list, *thread = thread_list;
	char *msg = malloc(length+sizeof(int));
	
	((unsigned long *)msg)[0] = (unsigned long)tid;
	memcpy(msg+4, message, length);
	
	do {
		kolibri_IPC_send(thread->tid, msg, length+sizeof(int));
		thread = thread->next;
	} while (thread != thread_list);
}

void remove_group(group_list_t *group){
	if (group == main_group_list) {
		return;
	}
	group->next->previos = group->previos;
	group->previos->next = group->next;
	memarea_free(group);
}

group_list_t *create_group(char *name){
	group_list_t *group_list = main_group_list, *group_last;
	
	if (!group_list) {
		return main_group_list = new_group_list(name);
	}
	
	group_last = group_list->previos;
	if (group_list == group_last) {
		group_last = group_list->next = group_list->previos = new_group_list(name);
		group_last->next = group_last->previos = group_list;
	} else {
		group_last->next = group_list->previos = new_group_list(name);
		group_last->next->next = group_list;
		group_last->next->previos = group_last;
		group_last = group_last->next;
	}
	
	return group_last;
}

group_list_t *new_group_list(char *name){
	group_list_t *list = memarea_alloc(sizeof(group_list_t));
	if (list) {
		list->name = malloc(strlen(name));
		strcpy(list->name, name);
		list->thread_list = NULL;
	}
	return list;
}

thread_list_t *new_thread_list(int tid){
	thread_list_t *list = memarea_alloc(sizeof(thread_list_t));
	if (list) {
		list->tid = tid;
	}
	return list;
}

asm(".align 16\n.globl end\nend:");
