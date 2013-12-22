#include <stdlib.h>
#include <sys/errno.h>
#include <kos32sys.h>

void __mutex_lock(volatile int *val);

static inline int tls_get(int key)
{
    int val;
    __asm__ __volatile__(
    "movl %%fs:(%1), %0"
    :"=r"(val)
    :"r"(key));

  return val;
};

typedef struct {
  int done;
  long started;
} __gthread_once_t;

typedef struct {
  int counter;
  void *sema;
} __gthread_mutex_t;

typedef struct {
  int counter;
  int depth;
  unsigned long owner;
  int sema;
} __gthread_recursive_mutex_t;


int
__gthr_win32_once (__gthread_once_t *once, void (*func) (void))
{
    if (once == NULL || func == NULL)
        return EINVAL;

    if (! once->done)
    {
        if (__sync_add_and_fetch (&(once->started), 1) == 0)
        {
            (*func) ();
            once->done = 1;
        }
        else
        {
	  /* Another thread is currently executing the code, so wait for it
	     to finish; yield the CPU in the meantime.  If performance
	     does become an issue, the solution is to use an Event that
	     we wait on here (and set above), but that implies a place to
	     create the event before this routine is called.  */
            while (! once->done)
                delay(1);
        }
    }
    return 0;
}

void __gthr_win32_mutex_init_function (__gthread_mutex_t *mutex)
{
  mutex->counter = 0;
  mutex->sema = 0;
}

int __gthr_win32_mutex_lock (__gthread_mutex_t *mutex)
{
    __mutex_lock(&mutex->counter);
    return 0;
}

int
__gthr_win32_mutex_unlock (__gthread_mutex_t *mutex)
{
  mutex->counter = 0;
  return 0;
}

void
__gthr_win32_recursive_mutex_init_function (__gthread_recursive_mutex_t *mutex)
{
  mutex->counter = -1;
  mutex->depth = 0;
  mutex->owner = 0;
  mutex->sema = 0;
}

void
__gthr_win32_mutex_destroy (__gthread_mutex_t *mutex)
{ }

int
__gthr_win32_recursive_mutex_lock (__gthread_recursive_mutex_t *mutex)
{
    int me = tls_get(0);
    if ( __sync_add_and_fetch(&mutex->counter, 1) == 0)
    {
        mutex->depth = 1;
        mutex->owner = me;
        mutex->sema  = 1;
    }
    else if (mutex->owner == me)
    {
        __sync_sub_and_fetch(&mutex->counter, 1);
        ++(mutex->depth);
    }
    else
    {
        __mutex_lock(&mutex->sema);
        mutex->depth = 1;
        mutex->owner = me;
    }
    return 0;
}

int
__gthr_win32_recursive_mutex_trylock (__gthread_recursive_mutex_t *mutex)
{
    int me = tls_get(0);
    if (__sync_val_compare_and_swap (&mutex->counter, -1, 0) < 0)
    {
        mutex->depth = 1;
        mutex->owner = me;
        mutex->sema  = 1;
    }
    else if (mutex->owner == me)
        ++(mutex->depth);
    else
        return 1;

  return 0;
}

int
__gthr_win32_recursive_mutex_unlock (__gthread_recursive_mutex_t *mutex)
{
    --(mutex->depth);
    if (mutex->depth == 0)
    {
        mutex->owner = 0;

        if (__sync_sub_and_fetch (&mutex->counter, 1) >= 0)
            mutex->sema = 0;
    }

    return 0;
}
