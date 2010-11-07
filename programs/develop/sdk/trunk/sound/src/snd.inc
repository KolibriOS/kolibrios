
struc CTRL_INFO
{   .pci_cmd        dd  ?
    .irq            dd  ?
    .glob_cntrl     dd  ?
    .glob_sta       dd  ?
    .codec_io_base  dd  ?
    .ctrl_io_base   dd  ?
    .codec_mem_base dd  ?
    .ctrl_mem_base  dd  ?
    .codec_id       dd  ?
}
CTRL_INFO_SIZE      equ 9*4

SRV_GETVERSION      equ 0
SND_CREATE_BUFF     equ 1
SND_DESTROY_BUFF    equ 2
SND_SETFORMAT       equ 3
SND_GETFORMAT       equ 4
SND_RESET           equ 5
SND_SETPOS          equ 6
SND_GETPOS          equ 7
SND_SETBUFF         equ 8
SND_OUT             equ 9
SND_PLAY            equ 10
SND_STOP            equ 11
SND_SETVOLUME       equ 12
SND_GETVOLUME       equ 13
SND_SETPAN          equ 14
SND_GETPAN          equ 15
SND_GETBUFFSIZE     equ 16
SND_GETFREESPACE    equ 17
SND_SETTIMEBASE     equ 18
SND_GETTIMESTAMP    equ 19

DEV_SET_BUFF        equ  4
DEV_NOTIFY          equ  5
DEV_SET_MASTERVOL   equ  6
DEV_GET_MASTERVOL   equ  7
DEV_GET_INFO        equ  8

