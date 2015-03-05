#ifndef __ITC_H
#define __ITC_H

#ifdef __cplusplus
extern "C" {
#endif

#include<stdlib.h>

#define MSG_BUF_SZ	128

typedef struct __itc_msg_t
{
 char msgbuf[MSG_BUF_SZ];
 int sender_tid;
 struct __itc_msg_t * next;
} itc_msg_t;

typedef struct
{
 itc_msg_t * head;
 int count;
} itc_msgq_t;

typedef struct
{
 char * name;
 int real_pid;
 
#ifdef __cplusplus
}
#endif

#endif
