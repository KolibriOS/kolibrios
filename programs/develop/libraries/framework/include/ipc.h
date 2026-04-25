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

#ifndef _IPC_H_
#define _IPC_H_

#include "defs.h"

#define IPC_BUFFER_SIZE 0x1000

#define IPC_MAX_ERRORS 10

#define IPC_NO_MEMORY 1
#define IPC_CANNOT_SET_MASK 2

#define IPC_NO_AREA 1
#define IPC_BUFFER_LOCKED 2
#define IPC_OVERFLOW 3
#define IPC_NO_TID 4

/*
 * Data types
 */

//
typedef struct kolibri_IPC_area {
	unsigned long lock;
	unsigned long size;
} IPCArea;

typedef struct kolibri_IPC_area kolibri_IPC_area_t;

//
typedef struct kolibri_IPC_message {
	unsigned long tid;
	unsigned long length;
} Message;

typedef struct kolibri_IPC_message kolibri_IPC_message_t;

/*
 * High level IPC functions
 */

extern IPCArea *ipc_area;

int IPCInit(void);

int IPCSend(int tid, void *message, int length);

bool IPCCheck(void);

bool IPCCheckWait(int time);

Message *IPCGetNextMessage(void);

Message *IPCWaitMessage(int time);

void IPCLock(void);

void IPCUnlock(void);

/*
 * Kolibri IPC functions & data
 */

extern kolibri_IPC_area_t *kolibri_IPC_area;

int kolibri_IPC_set_area(void *area, int size);

int kolibri_IPC_send(int tid, void *msg, int length);

void kolibri_IPC_unlock();

void kolibri_IPC_lock();

int kolibri_IPC_init(void *area, int size);

kolibri_IPC_message_t *kolibri_IPC_get_next_message();

// void kolibri_IPC_clear_buff();

#endif
