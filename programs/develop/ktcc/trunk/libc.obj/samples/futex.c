#include <sys/ksys.h>

/* Very simple example of working with futexes in Kolibri OS.
 * Author turbocat (Maxim Logaev).
 * Thanks to Vitaly Krylov for help.
 * 
 * The result of the program execution can be seen in the debug board.
*/

#define TH_STACK_SIZE 1024
#define TH_LOCK       1
#define TH_UNLOCK     0

uint8_t th_stack[TH_STACK_SIZE];

int glob_var = 0;
int th_lock = TH_UNLOCK;
uint32_t futex = 0;

void th_main(void)
{
    _ksys_debug_puts("Child thread start\n");
    _ksys_futex_wait(futex, TH_LOCK, 0);
    glob_var = 99;
    _ksys_debug_puts("Child thread end\n");
    _ksys_exit();
}

int main(void)
{
    _ksys_debug_puts("Parrent thread start");
    futex = _ksys_futex_create(&th_lock);
    th_lock = TH_LOCK;

    if (_ksys_create_thread(th_main, th_stack) == -1) {
        _ksys_debug_puts("Unable to create a new thread!\n");
        return 1;
    }

    glob_var = 88;
    _ksys_thread_yield();

    _ksys_delay(100);

    if (glob_var == 88) {
        _ksys_debug_puts("Futex test OK :)\n");
    } else {
        _ksys_debug_puts("Futex test FAIL :(\n");
    }

    th_lock = TH_UNLOCK;
    _ksys_futex_wake(futex, 1);

    _ksys_futex_destroy(futex);
    _ksys_debug_puts("Parrent thread end");
    return 0;
}
