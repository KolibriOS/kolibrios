#include "../system/kolibri.h"

void get_str_kernel_version(char *str, const char *fmt) {
    struct kernel_version kv;

    kol_get_kernel_ver(&kv);
    char str_offset[8] = {'\0'};
    if (kv.offset)
        sprintf(str_offset, "+%u", kv.offset);
    char str_dbgtag[4] = {'\0'};
    if (kv.dbgtag)
        sprintf(str_dbgtag, "-%c", kv.dbgtag);
    char str_cmtid[16] = {'\0'};
    if (kv.cmtid)
        sprintf(str_cmtid, " (%08x)", kv.cmtid);

    sprintf(str, fmt, kv.osrel[0], kv.osrel[1], kv.osrel[2], kv.osrel[3],
            str_offset, str_dbgtag, str_cmtid, kv.abimaj, kv.abimin);
}

// Retrieves the CPU Brand String and writes it to the provided buffer.
// out_buffer: A buffer of at least 49 bytes (48 for string + 1 for null)
void get_cpu_brand_string(char *out_buffer) {
    if (!out_buffer) return;

    uint32_t regs[4];

    // 1. Check maximum extended function support
    __asm__ (
        "cpuid"
        : "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3])
        : "a" (0x80000000)
    );

    if (regs[0] < 0x80000004) {
        strcpy(out_buffer, "Brand String Not Supported");
        return;
    }

    // 2. Extract brand string from leaves 0x80000002 to 0x80000004
    for (uint32_t leaf = 0x80000002; leaf <= 0x80000004; leaf++) {
        __asm__ (
            "cpuid"
            : "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3])
            : "a" (leaf)
        );

        // Copy the 16 bytes (4 registers * 4 bytes) from this leaf into the buffer
        memcpy(out_buffer + (leaf - 0x80000002) * 16, regs, 16);
    }

    out_buffer[48] = '\0';
}

int cmd_ver(char param[]) {
    if (!strcmp(param, "kernel")) {
        get_str_kernel_version(tmpstr, CMD_VER_FMT1);
        printf(tmpstr);
        return TRUE;
    }

    if (!strcmp(param, "cpu")) {
        char str[49];
        get_cpu_brand_string(str);
        printf("%s\n\r", str);
        return TRUE;
    }

    printf ("  Shell v%s\n\r", SHELL_VERSION);
    return TRUE;
}

