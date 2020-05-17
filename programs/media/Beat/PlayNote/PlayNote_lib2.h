
/*
 * Author: JohnXenox aka Aleksandr Igorevich.
 *
 * PlayNote_lib2.h
*/

#ifndef __PlayNote_lib2_h__
#define __PlayNote_lib2_h__

/*
struct CTRL_INFO
{   int  pci_cmd;
    int  irq;
    int  glob_cntrl;
    int  glob_sta;
    int  codec_io_base;
    int  ctrl_io_base;
    int  codec_mem_base;
    int  ctrl_mem_base;
    int  codec_id;
};
#define  CTRL_INFO_SIZE     (9*4)
*/

//====================================//

#define  SRV_GETVERSION     0
#define  SND_CREATE_BUFF    1
#define  SND_DESTROY_BUFF   2
#define  SND_SETFORMAT      3
#define  SND_GETFORMAT      4
#define  SND_RESET          5
#define  SND_SETPOS         6
#define  SND_GETPOS         7
#define  SND_SETBUFF        8
#define  SND_OUT            9
#define  SND_PLAY          10
#define  SND_STOP          11
#define  SND_SETVOLUME     12
#define  SND_GETVOLUME     13
#define  SND_SETPAN        14
#define  SND_GETPAN        15
#define  SND_GETBUFFSIZE   16
#define  SND_GETFREESPACE  17
#define  SND_SETTIMEBASE   18
#define  SND_GETTIMESTAMP  19


#define  DEV_SET_BUFF       4
#define  DEV_NOTIFY         5
#define  DEV_SET_MASTERVOL  6
#define  DEV_GET_MASTERVOL  7
#define  DEV_GET_INFO       8

//====================================//

#define  SOUND_VERSION 0x0101
#define  PCM_ALL       0

#define  PCM_OUT     0x08000000
#define  PCM_RING    0x10000000
#define  PCM_STATIC  0x20000000
#define  PCM_FLOAT   0x40000000
#define  PCM_FILTER  0x80000000

#define  PCM_2_16_48   1
#define  PCM_1_16_48   2
#define  PCM_2_16_44   3
#define  PCM_1_16_44   4
#define  PCM_2_16_32   5
#define  PCM_1_16_32   6
#define  PCM_2_16_24   7
#define  PCM_1_16_24   8
#define  PCM_2_16_22   9
#define  PCM_1_16_22  10
#define  PCM_2_16_16  11
#define  PCM_1_16_16  12
#define  PCM_2_16_12  13
#define  PCM_1_16_12  14
#define  PCM_2_16_11  15
#define  PCM_1_16_11  16
#define  PCM_2_16_8   17
#define  PCM_1_16_8   18
#define  PCM_2_8_48   19
#define  PCM_1_8_48   20
#define  PCM_2_8_44   21
#define  PCM_1_8_44   22
#define  PCM_2_8_32   23
#define  PCM_1_8_32   24
#define  PCM_2_8_24   25
#define  PCM_1_8_24   26
#define  PCM_2_8_22   27
#define  PCM_1_8_22   28
#define  PCM_2_8_16   29
#define  PCM_1_8_16   30
#define  PCM_2_8_12   31
#define  PCM_1_8_12   32
#define  PCM_2_8_11   33
#define  PCM_1_8_11   34
#define  PCM_2_8_8    35
#define  PCM_1_8_8    36

//====================================//

const char szInfinity[] = "INFINITY";
const char szSound[]    = "SOUND";

int *hSound = 0;
int *hrdwSound = 0;

struct MNG_DRV
{
    int *handle;
    int code;
    int **input;
    int inp_size;
    int **output;
    int out_size;
};

static inline void *LoadDriver(void *ptr)
{
    void  *val;
    __asm__ __volatile__("int $0x40":"=a"(val):"a"(68), "b"(16),"c"(ptr));
    return val;
}

static inline int ManageDriver(void *ptr)
{
    int val;
    __asm__ __volatile__("int $0x40":"=a"(val):"a"(68), "b"(17),"c"(ptr));
    return val;
}



static int _InitSound(int* p_ver)
{
    hSound = LoadDriver(&szInfinity);
    if(hSound == 0) return -1;
    hrdwSound = LoadDriver(&szSound);

    struct MNG_DRV MNG_DRV = {
        .handle    =  hSound,
        .code      =  SRV_GETVERSION,
        .input     =  0,
        .inp_size  =  0,
        .output    =  &p_ver,
        .out_size  =  4
    };

    return ManageDriver(&MNG_DRV);
}



static int _CreateBuffer(int format, int size, int *p_str)
{
    struct MNG_DRV MNG_DRV = {
        .handle = hSound,
        .code = SND_CREATE_BUFF,
        .input = &(int*)format,
        .inp_size = 8,
        .output = &p_str,
        .out_size = 4
    };

    return ManageDriver(&MNG_DRV);
}



static int _SetBuffer(int *str, int* src, int offs, int size)
{
    struct MNG_DRV MNG_DRV = {
        .handle = hSound,
        .code = SND_SETBUFF,
        .input = &(int*)str,
        .inp_size = 16,
        .output = 0,
        .out_size = 0
    };

    return ManageDriver(&MNG_DRV);
}



static int _PlayBuffer(int* str, int flags)
{
    struct MNG_DRV MNG_DRV = {
        .handle = hSound,
        .code = SND_PLAY,
        .input = &(int*)str,
        .inp_size = 8,
        .output = 0,
        .out_size = 0
    };

    return ManageDriver(&MNG_DRV);
}



static int _SetBufferPos(int *str, int offs)
{
    struct MNG_DRV MNG_DRV = {
        .handle = hSound,
        .code = SND_SETPOS,
        .input = &(int*)str,
        .inp_size = 8,
        .output = 0,
        .out_size = 0
    };

    return ManageDriver(&MNG_DRV);
}



static int _StopBuffer(int* str)
{
    struct MNG_DRV MNG_DRV = {
        .handle = hSound,
        .code = SND_STOP,
        .input = &(int*)str,
        .inp_size = 4,
        .output = 0,
        .out_size = 0
    };

    return ManageDriver(&MNG_DRV);
}









#endif


