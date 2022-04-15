/*
 * This is an example program for sending a message through a "pipe".
 * Created by turbocat (Maxim Logaev) 2022.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ksys.h>

#define TH_STACK_SIZE 1024
#define MESSAGE_SIZE 12

ksys_colors_table_t sys_colors;
int pipefd[2];
char* send_message = "HELLO PIPE!";

void tmain()
{
    char recv_message[MESSAGE_SIZE];
    _ksys_posix_read(pipefd[0], recv_message, MESSAGE_SIZE);
    printf("RECV: %s\n", recv_message);
    assert(!strcmp(recv_message, send_message));
    puts("Successful pipe test");
    exit(0);
}

void create_thread(void)
{
    unsigned tid;                           // New thread ID
    void* th_stack = malloc(TH_STACK_SIZE); // Allocate memory for thread stack
    if (!th_stack) {
        puts("Memory allocation error for thread!");
        return;
    }
    tid = _ksys_create_thread(tmain, th_stack + TH_STACK_SIZE); // Create new thread with entry "main"
    if (tid == -1) {
        puts("Unable to create a new thread!");
        return;
    }
    printf("New thread created (TID=%u)\n", tid);
}

void main()
{
    if (_ksys_posix_pipe2(pipefd, 0)) {
        puts("Pipe creation error!");
        return;
    }
    printf("SEND: %s\n", send_message);
    _ksys_posix_write(pipefd[1], send_message, MESSAGE_SIZE);
    create_thread();
}
