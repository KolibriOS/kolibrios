#include "config.h"

#pragma pack(push,1)

typedef struct {
    __u32 func;
    __u32 l_off;
    __u32 h_off_flags;
    __u32 size;
    char* data;
    char  null;
    char* name;
} fs_info;

#pragma pack(pop)

char path[] = "/sys/games/2048.dat";

__u8 config_load(config_state* st)
{
    fs_info cfg = {0};
    cfg.func = 0;
    cfg.size = sizeof(config_state);
    cfg.data = (char*)st;
    cfg.name = path;

    __u32 ret = 0;
    __u32 rnum = 0;

    __asm__ __volatile__("int $0x40":"=a"(ret),"=b"(rnum):
                         "a"(70),
                         "b"((__u32)(&cfg)):
                         "memory");

    return !ret || (rnum == cfg.size);
}

__u8 config_save(config_state* st)
{
    fs_info cfg = {0};
    cfg.func = 2;
    cfg.size = sizeof(config_state);
    cfg.data = (char*)st;
    cfg.name = path;

    __u32 ret = 0;
    __u32 wnum = 0;

    __asm__ __volatile__("int $0x40":"=a"(ret),"=b"(wnum):
                         "a"(70),
                         "b"((__u32)(&cfg)):
                         "memory");

    return !ret || (wnum == cfg.size);
}
