#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define BUF_LEN 0x10

int main(int argc, char *argv[])
{
    uint8_t buf[BUF_LEN + 7];
    int64_t len;
    off_t offset;
    char *path;
    if (argc != 4) {
        fprintf(stderr, "mkfilepattern filename offset length\n");
        exit(1);
    } else {
        path = argv[1];
        offset = strtoll(argv[2], NULL, 0);
        len = strtoll(argv[3], NULL, 0);
    }

    int fd = open(path, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        fprintf(stderr, "Can't open %s: %s\n", path, strerror(errno));
        exit(1);
    }

    lseek(fd, offset, SEEK_SET);
    for (int64_t pos = offset, count = BUF_LEN; pos < offset + len; pos += count) {
        if (count > offset + len - pos) {
            count = offset + len - pos;
        }

        off_t off = 0;
        while (off < count) {
            if (pos + off < 0x100) {
                *(uint8_t*)(buf + off) = (uint8_t)(pos + off);
                off += 1;
            } else if (pos + off < 0x10000) {
                *(uint16_t*)(buf + off) = (uint16_t)(pos + off);
                off += 2;
            } else if (pos + off < 0x100000000l) {
                *(uint32_t*)(buf + off) = (uint32_t)(pos + off);
                off += 4;
            } else {
                *(uint64_t*)(buf + off) = (uint64_t)(pos + off);
                off += 8;
            }
        }

        write(fd, buf, count);
    }

    close(fd);

    return 0;
}
