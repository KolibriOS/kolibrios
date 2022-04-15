#include "stddef.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ksys.h>

#define DEV_ADD_DISK 1

#define TMPDISK_SIZE 10 // Mb

#pragma pack(push, 1)
struct {
    unsigned disk_size;
    unsigned char disk_id;
} tmpdisk_add;
#pragma pack(pop)

char* tmpdisk_res_text[] = {
    "TmpDisk operation completed successfully",
    "Unknown IOCTL code, wrong input/output size...",
    "DiskId must be from 0 to 9",
    "DiskSize is too large",
    "DiskSize is too small, might be too little free RAM",
    "Memory allocation failed",
    "Unknown error O_o",
    0
};

int main()
{
    ksys_drv_hand_t tmpdisk_drv = _ksys_load_driver("tmpdisk");
    if (!tmpdisk_drv) {
        puts("tmpdisk.sys driver not load!");
        exit(0);
    } else {
        puts("tmpdisk.sys driver is load!");
    }

    tmpdisk_add.disk_size = TMPDISK_SIZE * 1024 * 1024 / 512;
    tmpdisk_add.disk_id = 5;

    ksys_ioctl_t ioctl;
    ioctl.func_num = DEV_ADD_DISK;
    ioctl.handler = tmpdisk_drv;
    ioctl.in_data_ptr = &tmpdisk_add;
    ioctl.in_data_size = sizeof(tmpdisk_add);
    ioctl.out_data_ptr = NULL;
    ioctl.out_data_size = 0;

    printf("Create '/tmp%u/' disk a %u Mb size...\n", tmpdisk_add.disk_id, TMPDISK_SIZE);
    unsigned status = _ksys_driver_control(&ioctl);
    if (status < 7) {
        puts(tmpdisk_res_text[status]);
    } else {
        puts(tmpdisk_res_text[6]);
    }
    exit(0);
}
