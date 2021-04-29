#include <ddk.h>
#include <syscall.h>

static int __stdcall service_proc(ioctl_t *my_ctl){
    int c = 0;
    int a = *(int*)(my_ctl->input);
    int b = *(int*)(my_ctl->input+4);
    if(my_ctl->io_code==0){
        c = a + b;
    }else{
        return -1;
    }
    *(int*)(my_ctl->output) = c;
    return 0;
}

unsigned drvEntry(int action, char *cmdline){
    SysMsgBoardStr("Driver c_drv.dll loaded!\n");
    return RegService("c_drv", service_proc);
}
