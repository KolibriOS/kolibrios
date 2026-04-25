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

#include <ipc.h>
#include <malloc.h>
#include <kolibri.h>
#include <defs.h>
#include <stdlib.h>

/*
 * High level IPC functions
 */

IPCArea *ipc_area = NULL;

int IPCInit(void){
	ipc_area = malloc(IPC_BUFFER_SIZE);
	if (!ipc_area) {
		return IPC_NO_MEMORY;
	}
	return kolibri_IPC_init(ipc_area, IPC_BUFFER_SIZE);
}

int IPCSend(int tid, void *message, int length){
	int err_count = IPC_MAX_ERRORS;
	int err;
	
	while (--err_count) {
		if (!(err = kolibri_IPC_send(tid, message, length)) || err == IPC_NO_AREA || err == IPC_NO_TID) {
			return err;
		}
	}
	return err;
}

bool IPCCheck(void){
	return IPCCheckWait(0);
}

bool IPCCheckWait(int time){
	int result = false;
	int emask = kolibri_event_get_mask();
	
	kolibri_event_set_mask(KOLIBRI_EVENT_IPC_MASK);
	
	result = kolibri_event_wait(time);
	
	kolibri_event_set_mask(emask);
	
	return result;
}

Message *IPCGetNextMessage(void){
	return IPCWaitMessage(0);
}

Message *IPCWaitMessage(int time){
	if (IPCCheckWait(time)){
		Message *msg_ret, *msg = kolibri_IPC_get_next_message();
		msg_ret = malloc(sizeof(Message)+msg->length);
		if (!msg_ret) {
			return NULL;
		}
		IPCLock();
		memcpy(msg_ret, msg, sizeof(Message)+msg->length);
		ipc_area->size -= sizeof(Message)+msg->length;
		memcpy(msg, msg+sizeof(Message)+msg->length, ipc_area->size);
		IPCUnlock();
		return msg_ret;
	} else {
		return NULL;
	}
}

/*
 * Kolibri IPC functions & data
 */

kolibri_IPC_area_t *kolibri_ipc_area;

int kolibri_IPC_set_area(void *area, int size){
	int result;
	
	asm("int $0x40":"=a"(result):"a"(60),"b"(1),"c"(area),"d"(size));
	
	return result;
}

int kolibri_IPC_send(int tid, void *msg, int length){
	int result;
	
	asm("movl %5, %%esi\nint $0x40":"=a"(result):"a"(60),"b"(2),"c"(tid),"d"(msg),"g"(length));
	
	return result;
}

void kolibri_IPC_unlock(){
	kolibri_ipc_area->lock = 0;
}

void kolibri_IPC_lock(){
	kolibri_ipc_area->lock = 1;
}

int kolibri_IPC_init(void *area, int size){
	kolibri_ipc_area = (kolibri_IPC_area_t *)area;
	kolibri_ipc_area->size = 8;
	
	return kolibri_IPC_set_area(area, size);
}

kolibri_IPC_message_t *kolibri_IPC_get_next_message(){
	kolibri_IPC_lock();
	return (kolibri_IPC_message_t *)((char *)kolibri_ipc_area+sizeof(kolibri_IPC_area_t));
}

void IPCLock(void){
	kolibri_IPC_lock();
}

void IPCUnlock(void){
	kolibri_IPC_unlock();
}

// void kolibri_IPC_clear_buff(){
// 	kolibri_ipc_area->size = 8;
// 	kolibri_IPC_unlock();
// }
