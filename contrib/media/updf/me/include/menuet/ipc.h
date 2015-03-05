#ifndef __MENUETOS_IPC_H
#define __MENUETOS_IPC_H

typedef struct {
    volatile unsigned long lock;
    unsigned long free_ptr;
    char __mem[0];
} ipc_hdr_t;

typedef struct {
    unsigned long sender_pid;
    unsigned long msg_length;
    char message[0];
} ipc_msg_t;

ipc_hdr_t * create_ipc(unsigned long size);
void register_ipc_mem(ipc_hdr_t * hdr);
void ipc_send_message(int dst_pid,ipc_msg_t * msg);
extern inline int ipc_messages_avail(ipc_hdr_t * hdr);

#endif
