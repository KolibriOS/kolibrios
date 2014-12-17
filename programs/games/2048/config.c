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

__u32 config_load_highscore()
{
    __u32 highscore = 0;

    fs_info cfg = {0};
    cfg.func = 0;
    cfg.size = sizeof(__u32);
    cfg.data = (char*)&highscore;
    cfg.name = path;

    __u32 ret = 0;
    __u32 rnum = 0;

    __asm__ __volatile__("int $0x40":"=a"(ret),"=b"(rnum):
                         "a"(70),
                         "b"((__u32)(&cfg)):
                         "memory");

    if (ret || (rnum != 4)) highscore = 0;

    return highscore;
}

void config_save_highscore(__u32 score)
{
    fs_info cfg = {0};
    cfg.func = 2;
    cfg.size = sizeof(__u32);
    cfg.data = (char*)&score;
    cfg.name = path;

    __u32 ret = 0;
    __u32 wnum = 0;

    __asm__ __volatile__("int $0x40":"=a"(ret),"=b"(wnum):
                         "a"(70),
                         "b"((__u32)(&cfg)):
                         "memory");
}
