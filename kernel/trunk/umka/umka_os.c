/*
    UMKa - User-Mode KolibriOS developer tools
    umka_os - kind of KolibriOS rump kernel
    Copyright (C) 2018--2021  Ivan Baravy <dunkaist@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include "umka.h"
#include "shell.h"
#include "trace.h"

#define UMKA_DEFAULT_DISPLAY_WIDTH 400
#define UMKA_DEFAULT_DISPLAY_HEIGHT 300

#define THREAD_STACK_SIZE 0x100000

static void
monitor(void) {
    umka_sti();
    fprintf(stderr, "Start monitor thread\n");
    FILE *fin = fopen("/tmp/umka.fifo.2u", "r");
    FILE *fout = fopen("/tmp/umka.fifo.4u", "w");
    if (!fin || !fout) {
        fprintf(stderr, "Can't open monitor files!\n");
        return;
    }
    run_test(fin, fout, 0);
}

void umka_thread_ping(void);
void umka_thread_net_drv(void);

struct itimerval timeout = {.it_value = {.tv_sec = 0, .tv_usec = 10000},
                            .it_interval = {.tv_sec = 0, .tv_usec = 10000}};

static void
thread_start(int is_kernel, void (*entry)(void), size_t stack_size) {
    uint8_t *stack = malloc(stack_size);
    umka_new_sys_threads(is_kernel, entry, stack + stack_size);
}

/*
can't get pty working
may be because of my custom threads and blocking, don't know
void new_monitor(void) {
    umka_sti();
    fprintf(stderr, "Start monitor thread\n");

    int mpty = posix_openpt(O_RDWR | O_NOCTTY);
    if (mpty == -1) {
        perror("open master pty");
        return;
    }
    if (grantpt(mpty) == -1) {
        perror("grantpt");
        return;
    }
    if (unlockpt(mpty) == -1) {
        perror("unlockpt");
        return;
    }
    char *spty_name = ptsname(mpty);
    if (spty_name == NULL) {
        perror("open slave pty");
        return;
    }
    fprintf(stderr, "[os] pty=%s\n", spty_name);
    FILE *fmpty = fdopen(mpty, "r+");
    if (fmpty == NULL) {
        perror("fdopen mpty");
        return;
    }
    run_test(fmpty, fmpty, 0);
}
*/

int
main() {
    if (coverage)
        trace_begin();

    umka_tool = UMKA_OS;
    umka_sti();

    struct sigaction sa;
    sa.sa_sigaction = irq0;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;

    if (sigaction(SIGPROF, &sa, NULL) == -1) {
        printf("Can't install signal handler!\n");
        return 1;
    }

/*
    void *app_base = mmap((void*)0x000000, 16*0x100000, PROT_READ | PROT_WRITE |
                          PROT_EXEC, MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS,
                          -1, 0);
    if (app_base == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }
*/
    printf("pid=%d, kos_lfb_base=%p\n", getpid(), (void*)kos_lfb_base);

    kos_boot.bpp = 32;
    kos_boot.x_res = UMKA_DEFAULT_DISPLAY_WIDTH;
    kos_boot.y_res = UMKA_DEFAULT_DISPLAY_HEIGHT;
    kos_boot.pitch = UMKA_DEFAULT_DISPLAY_WIDTH*4;  // 32bpp

    umka_init();
    umka_stack_init();

    thread_start(0, monitor, THREAD_STACK_SIZE);
    thread_start(0, umka_thread_net_drv, THREAD_STACK_SIZE);
//    thread_start(0, umka_thread_ping, THREAD_STACK_SIZE);

    setitimer(ITIMER_PROF, &timeout, NULL);

    kos_osloop();   // doesn't return

    if (coverage)
        trace_end();

    return 0;
}
