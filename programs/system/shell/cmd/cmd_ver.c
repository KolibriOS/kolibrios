
void get_str_kernel_version(char *str, const char *fmt) {
    char *kvbuf;
    char *vA, *vB, *vC, *vD;
    unsigned *Rev;

    kvbuf = malloc(16);
    kol_get_kernel_ver(kvbuf);
    vA = kvbuf+0;
    vB = kvbuf+1;
    vC = kvbuf+2;
    vD = kvbuf+3;
    Rev = (unsigned*)(kvbuf + 5);

    sprintf (str, fmt, *vA, *vB, *vC, *vD, *Rev);

    free(kvbuf);
}

void get_str_cpu_info(char *str) {
    unsigned a, b, c, d;

    asm ("cpuid" :
                    "=a" (a),
            "=b" (b),
            "=c" (c),
            "=d" (d):
            "a"(0));

    str[0] = (b&0x000000ff)	>> 0;
    str[1] = (b&0x0000ff00)	>> 8;
    str[2] = (b&0x00ff0000)	>> 16;
    str[3] = (b&0xff000000)	>> 24;

    str[4] = (d&0x000000ff)	>> 0;
    str[5] = (d&0x0000ff00)	>> 8;
    str[6] = (d&0x00ff0000)	>> 16;
    str[7] = (d&0xff000000)	>> 24;

    str[8] = (c&0x000000ff)	>> 0;
    str[9] = (c&0x0000ff00)	>> 8;
    str[10] = (c&0x00ff0000)	>> 16;
    str[11] = (c&0xff000000)	>> 24;
    str[12] = '\0';
}

int cmd_ver(char param[]) {
    if (!strcmp(param, "kernel")) {
        get_str_kernel_version(tmpstr, CMD_VER_FMT1);
        printf(tmpstr);
        return TRUE;
    }

    if (!strcmp(param, "cpu")) {
        char str[13];
        get_str_cpu_info(str);
        printf("%s\n\r", str);
        return TRUE;
    }

    printf ("  Shell v%s\n\r", SHELL_VERSION);
    return TRUE;
}

