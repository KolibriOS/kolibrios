#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
    unsigned begin, end;
    int pat_min, pat_max;
    char *path;
    if (argc == 6) {
        path = argv[1];
        sscanf(argv[2], "%u", &begin);
        sscanf(argv[3], "%u", &end);
        sscanf(argv[4], "%i", &pat_min);
        sscanf(argv[5], "%i", &pat_max);
    } else {
        fprintf(stderr, "mkdirrange <directory> <num_begin> <num_end> <pat_min> <pat_max>\n"
                        "pat_min + pat_max <= 244\n");
        exit(1);
    }

    int dirfd = open(path, O_DIRECTORY);
    if (dirfd == -1) {
        fprintf(stderr, "Can't open %s: %s\n", path, strerror(errno));
        exit(1);
    }
    char dirname[256];
    memset(dirname, 'x', 256);

    for(unsigned current = begin; current < end; current++) {
        int length = sprintf(dirname, "d%10.10u_", current);
        dirname[length] = 'x';
        length += pat_min + (current % pat_max);
        dirname[length] = '\0';
        if(mkdirat(dirfd, dirname, 0755)) {
            fprintf(stderr, "Can't mkdir %s: %s\n", dirname, strerror(errno));
            exit(1);
        } else {
            dirname[length] = 'x';
        }
    }

    return 0;
}
