#include <sys/ksys.h>
#include <stdio.h>
#include <stdlib.h>

char* drv_name = NULL;

struct{
    int a;
    int b;
}add_struct;

int sum=0;
ksys_drv_hand_t drv = 0;

#define DRV_ADD_FUNC 0

int main(int argc, char** argv){
    puts("Which driver?");
    puts("1 - asm_drv.sys\n2 - c_drv.dll");
    int num_drv = (char)getchar()-'0';
    switch (num_drv) {
        case 1 : 
            drv_name = "asm_drv";
            drv = _ksys_load_driver(drv_name);
            break;
        case 2 :
            drv_name = "/sys/drivers/c_drv.dll";
            drv = _ksys_load_pe_driver(drv_name, NULL);
            break;
        default:
            printf("No driver selected!\n");
            exit(0);
    }

    if(!drv){
        printf("'%s' driver not load!\n", drv_name);
        exit(0);
    }else{
        printf("'%s' driver is load!\n", drv_name);
    }
    

    add_struct.a = 43;
    add_struct.b = 532;
    
    ksys_ioctl_t ioctl;
    ioctl.func_num = DRV_ADD_FUNC;
    ioctl.handler = drv;
    ioctl.in_data_ptr = &add_struct;
    ioctl.in_data_size = sizeof(add_struct);
    ioctl.out_data_ptr = &sum;
    ioctl.out_data_size = sizeof(sum);

    unsigned status =_ksys_work_driver(&ioctl);
    if(status==-1){
        puts("Error!");
    }else {
        printf("%d + %d  = %d\n", add_struct.a, add_struct.b, sum); 
        if(sum == add_struct.a + add_struct.b){
            puts("True!");
        }else{
            puts("False!");
        }
    }
    exit(0);
}
