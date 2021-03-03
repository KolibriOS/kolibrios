#include <ksys.h>
#include <string.h>
#include <stdlib.h>
#include "shell.h"

char __shell_shm_name[32]; 
char*__shell_shm=NULL;
int __shell_is_init=0;

int __shell_shm_init()
{
    __shell_is_init=1;
    ksys_proc_table_t *proc_info = (ksys_proc_table_t*)malloc(sizeof(ksys_proc_table_t));
    if(proc_info == NULL){
        return -1;
    }
    unsigned PID;

    _ksys_process_info(proc_info, -1); 
    PID = proc_info->pid;
    free(proc_info);

    itoa(PID, __shell_shm_name);
    strcat(__shell_shm_name, "-SHELL");
    return _ksys_shm_open(__shell_shm_name,  KSYS_SHM_OPEN_ALWAYS | KSYS_SHM_WRITE, SHELL_SHM_MAX, &__shell_shm);
}

int __shell_init()
{
    if(__shell_is_init){
        return 0;
    }
    if(__shell_shm_init()){
        debug_printf("Shell problems detected!\n");
        return -1;
    }
    return 0;
}