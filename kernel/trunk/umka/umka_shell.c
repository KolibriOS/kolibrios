/*
    umka_shell: User-Mode KolibriOS developer tools, the shell
    Copyright (C) 2018--2020  Ivan Baravy <dunkaist@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "shell.h"
#include "umka.h"
#include "trace.h"

#define UMKA_DEFAULT_DISPLAY_WIDTH 400
#define UMKA_DEFAULT_DISPLAY_HEIGHT 300

int
main(int argc, char **argv) {
    umka_tool = UMKA_SHELL;
    const char *usage = \
        "usage: umka_shell [test_file.t] [-c]\n"
        "  -c               collect coverage";
    const char *infile = NULL;
    char outfile[PATH_MAX] = {0};
    FILE *fin = stdin, *fout = stdout;

    kos_boot.bpp = 32;
    kos_boot.x_res = UMKA_DEFAULT_DISPLAY_WIDTH;
    kos_boot.y_res = UMKA_DEFAULT_DISPLAY_HEIGHT;
    kos_boot.pitch = UMKA_DEFAULT_DISPLAY_WIDTH*4;  // 32bpp

    // skip 'umka_shell'
    argc -= 1;
    argv += 1;

    while (argc) {
        if (!strcmp(argv[0], "-c")) {
            coverage = 1;
            argc -= 1;
            argv += 1;
            continue;
        } else if (!strcmp(argv[0], "-i") && argc > 1) {
            infile = argv[1];
            strncpy(outfile, infile, PATH_MAX-2); // ".t" is shorter than ".out"
            char *last_dot = strrchr(outfile, '.');
            if (!last_dot) {
                printf("[!] test file must have '.t' suffix\n");
                exit(1);
            }
            strcpy(last_dot, ".out.log");
            fin = fopen(infile, "r");
            if (!fin) {
                perror("[!] can't open file");
                exit(1);
            }
            fout = fopen(outfile, "w");
            if (!fout) {
                perror("[!] can't open file");
                exit(1);
            }
            argc -= 2;
            argv += 2;
            continue;
        } else {
            printf("bad option: %s\n", argv[0]);
            puts(usage);
            exit(1);
        }
    }

    if (coverage)
        trace_begin();

    run_test(fin, fout, 1);

    if (coverage)
        trace_end();

    return 0;
}
