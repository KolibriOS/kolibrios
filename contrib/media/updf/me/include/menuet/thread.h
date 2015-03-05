#ifndef __MENUET_THREAD_H
#define __MENUET_THREAD_H

#define THR_ATEXIT		0x00000001
#define THR_KILLER		0x00000002

void init_threads(void);
int get_thread_pid(int tid);
int get_thread_tid(int pid);
int create_thread(void (* fn)(void),int stacksize,int flags,void ** rstackp);
void kill_thread(int tid);

#endif
