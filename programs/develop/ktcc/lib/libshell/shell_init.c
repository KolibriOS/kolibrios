#include <sys/ksys.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <shell_api.h>

char app_name[13];
char __shell_shm_name[32]; 
char*__shell_shm=NULL;
int __shell_is_init=0;

int __shell_shm_init()
{
    __shell_is_init=1;
    ksys_thread_t *proc_info = (ksys_thread_t*)malloc(sizeof(ksys_thread_t));
    if(proc_info == NULL){
        return -1;
    }
    unsigned PID;

    _ksys_thread_info(proc_info, -1); 
    PID = proc_info->pid;
    strncpy(app_name, proc_info->name, 12);
    free(proc_info);

    itoa(PID, __shell_shm_name);
    strcat(__shell_shm_name, "-SHELL");
    return _ksys_shm_open(__shell_shm_name,  KSYS_SHM_OPEN_ALWAYS | KSYS_SHM_WRITE, SHELL_SHM_MAX, &__shell_shm);
}

void __shell_init()
{
    if(!__shell_is_init){
        if(__shell_shm_init()){
        debug_printf("%s: shell problems detected!\n", app_name);
        _ksys_exit();
        }

        if(!shell_ping()){
        debug_printf("%s: no shell found!\n", app_name);
        _ksys_exit();
        }
    }
}
