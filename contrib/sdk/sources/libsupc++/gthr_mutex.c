
void __mutex_lock(volatile int *val);

typedef struct {
  int counter;
  void *sema;
} __gthread_mutex_t;

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

