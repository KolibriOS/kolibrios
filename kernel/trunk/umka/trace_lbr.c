#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "umka.h"

#define MSR_IA32_DEBUGCTLMSR        0x1d9
#define MSR_IA32_LASTBRANCHFROMIP   0x1db
#define MSR_IA32_LASTBRANCHTOIP     0x1dc

int covfd, msrfd;

uint64_t rdmsr(uint32_t reg)
{
    uint64_t data;

    if (pread(msrfd, &data, sizeof data, reg) != sizeof data) {
        perror("rdmsr: pread");
        exit(1);
    }

    return data;
}

void wrmsr(uint32_t reg, uint64_t data)
{
    int fd;
    fd = open("/dev/cpu/0/msr", O_WRONLY);
    if (fd < 0) {
        perror("wrmsr: open");
        exit(1);
    }

    if (pwrite(fd, &data, sizeof data, reg) != sizeof data) {
        perror("wrmsr: pwrite");
        exit(1);
    }

    close(fd);
    return;
}

void handle_sigtrap() {
    uint64_t from = rdmsr(MSR_IA32_LASTBRANCHFROMIP);
    uint64_t to = rdmsr(MSR_IA32_LASTBRANCHTOIP);

    if ((from >= (uintptr_t)coverage_begin && from < (uintptr_t)coverage_end) ||
        (to >= (uintptr_t)coverage_begin && to < (uintptr_t)coverage_end)) {
        write(covfd, &from, 4);
        write(covfd, &to, 4);
    }

    wrmsr(MSR_IA32_DEBUGCTLMSR, 3);
}

uint32_t set_eflags_tf(uint32_t tf) {
    uint32_t prev;
    __asm__ __inline__ __volatile__ (
        "pushfd;"
        "pop    eax;"
        "ror    eax, 8;"
        "mov    edx, eax;"
        "and    edx, 1;"
        "and    eax, ~1;"
        "or     eax, ecx;"
        "rol    eax, 8;"
        "push   eax;"
        "popfd"
        : "=d"(prev)
        : "c"(tf)
        : "eax", "memory");
    return prev;
}

void trace_lbr_begin() {
    struct sigaction action;
    action.sa_sigaction = &handle_sigtrap;
    action.sa_flags = SA_SIGINFO;
    sigaction(SIGTRAP, &action, NULL);

    wrmsr(MSR_IA32_DEBUGCTLMSR, 3);
    msrfd = open("/dev/cpu/0/msr", O_RDONLY);
    if (msrfd < 0) {
        perror("rdmsr: open");
        exit(1);
    }
    char coverage_filename[32];
    sprintf(coverage_filename, "coverage.%i", getpid());
    covfd = open(coverage_filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH | S_IWOTH);
    void *coverage_begin_addr = coverage_begin;
    void *coverage_end_addr = coverage_end;
    write(covfd, &coverage_begin_addr, 4);
    write(covfd, &coverage_end_addr, 4);
}

void trace_lbr_end() {
    wrmsr(MSR_IA32_DEBUGCTLMSR, 0);
    close(msrfd);
    close(covfd);
}

uint32_t trace_lbr_pause(void) {
    return set_eflags_tf(0u);
}

void trace_lbr_resume(uint32_t value) {
    set_eflags_tf(value);
}
