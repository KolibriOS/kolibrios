#include <stdio.h>
#include <stdlib.h>
#include <sys/ksys.h>

#define NUM_THREADS 16
#define ITERATIONS  16
#define STACK_SIZE  64 * 1024

int multithread_done = 0;

#define MUTLITHREAD_EXIT         \
    __asm__ volatile(            \
        "lock incl %0"           \
        : "+m"(multithread_done) \
        :                        \
        : "cc");                 \
    _ksys_exit();

// Функция, которую выполняет каждый поток
void thread_writer()
{
    ksys_thread_t t;
    _ksys_thread_info(&t, -1);

    for (int i = 0; i < ITERATIONS; i++) {

        printf("[thread %ld] %d: ", t.pid, i + 1);
        puts("idk something...\n");
        _ksys_thread_yield();
    }

    MUTLITHREAD_EXIT;
}

int main()
{
    printf("=== Start console multithread test ===\n");

    // Создание потоков
    for (long i = 0; i < NUM_THREADS; i++) {
        void* stack = malloc(STACK_SIZE);
        if (_ksys_create_thread(thread_writer, stack + STACK_SIZE) == -1) {
            perror("can't crate thread\n");
            return 1;
        }
    }

    while (multithread_done < NUM_THREADS) {
        _ksys_thread_yield();
    }

    printf("=== Done ===\n");
    return 0;
}