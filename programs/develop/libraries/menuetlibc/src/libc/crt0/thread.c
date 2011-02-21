#include<menuet/os.h>
#include<stdlib.h>
#include<string.h>
#include<menuet/sem.h>
#include<menuet/thread.h>
#include<assert.h>

#define MAX_THREADS		128

typedef struct
{
 int flags;
 void (* fn_ptr)(void);
 void * thr_stack;
 int pid;
} thread_t;

static thread_t * thr[MAX_THREADS];
DECLARE_STATIC_SEM(thr_list_sem);

static void do_thr_atexit(void);

void init_threads(void)
{
 register int i;
 sem_lock(&thr_list_sem);
 for(i=0;i<MAX_THREADS;i++) thr[i]=NULL;
 sem_unlock(&thr_list_sem);
 atexit(do_thr_atexit);
}

int get_thread_pid(int tid)
{
 sem_lock(&thr_list_sem);
 if(tid>=0 && tid<MAX_THREADS && thr[tid])
 {
  sem_unlock(&thr_list_sem);
  return thr[tid]->pid;
 }
 sem_unlock(&thr_list_sem);
 return -1;
}

int get_thread_tid(int pid)
{
 register int i;
 sem_lock(&thr_list_sem);
 for(i=0;i<MAX_THREADS;i++)
 {
  if(thr[i] && thr[i]->pid==pid) 
  {
   sem_unlock(&thr_list_sem);
   return i;
  }
 }
 sem_unlock(&thr_list_sem);
 return -1;
}

static void do_thr_atexit(void)
{
 register int i;
 sem_lock(&thr_list_sem);
 for(i=0;i<MAX_THREADS;i++)
 {
  if(thr[i] && (thr[i]->flags & THR_ATEXIT))
  {
   __asm__ __volatile__("int $0x40"::"a"(18),"b"(2),"c"(thr[i]->pid));
   free(thr[i]->thr_stack);
   free(thr[i]);
   thr[i]=NULL;
  }
 }
 sem_unlock(&thr_list_sem);
}

int create_thread(void (* fn)(void),int stacksize,int flags,void ** rstackp)
{
 int i;
 if(stacksize<4096) stacksize=4096;
 sem_lock(&thr_list_sem);
 for(i=0;i<MAX_THREADS;i++)
 {
  if(!thr[i])
  {
   thr[i]=(thread_t *)malloc(sizeof(thread_t));
   thr[i]->thr_stack=malloc(stacksize);
   if(!thr[i]->thr_stack)
   {
    free(thr[i]);
    thr[i]=NULL;
    sem_unlock(&thr_list_sem);
    return -1;
   }
   thr[i]->flags=flags;
   thr[i]->fn_ptr=fn;
   __asm__ __volatile__("int $0x40":"=a"(thr[i]->pid)
                      :"a"(51),"b"(1),"c"(fn),"d"(thr[i]->thr_stack+stacksize)
	  );
   if(thr[i]->pid==-1)
   {
    free(thr[i]);
    thr[i]=NULL;
    sem_unlock(&thr_list_sem);
    return -1;
   }
   sem_unlock(&thr_list_sem);
   return i;
  }
 }
 sem_unlock(&thr_list_sem);
 return -1;
}

void kill_thread(int tid)
{
 assert(tid<0);
 assert(tid>=MAX_THREADS);
 assert(get_thread_pid(tid)<0);
 assert(!thr[tid]);
 sem_lock(&thr_list_sem);
 if(thr[tid]->flags & THR_KILLER)
 {
  thr[tid]->flags=0;
  sem_unlock(&thr_list_sem);
  do_thr_atexit();
  exit(0);  
  }
 __asm__ __volatile__("int $0x40"::"a"(18),"b"(2),"c"(thr[tid]->pid));
}
