#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include "../sys/_conio.h"
#include "_stdio.h"

char* fgets(char* str, int n, FILE* stream)
{
    if (!stream || !str || n <= 0) {
        errno = EINVAL;
        return NULL;
    }

    if (n == 1) {
        str[0] = '\0';
        return str;
    }

    int c;
    int i = 0;

    if (stream == stdin) {
        while (i < n - 1 && (c = fgetc(stream)) != EOF) {
            str[i++] = (char)c;
            if (c == '\n') {
                break;
            }
        }
    } else {
        size_t read_bytes = fread(str, sizeof(*str), n - 1, stream);

        if (read_bytes == 0) {
            return NULL;
        }

        for (i = 0; i < read_bytes; i++) {
            if (str[i] == '\n') {
                i++;
                break;
            }
        }

        fpos_t pos;
        fgetpos(stream, &pos);

        pos -= read_bytes - i;

        fsetpos(stream, &pos);
    }

    if (i == 0) {
        return NULL;
    }

    str[i] = '\0';
    return str;
}
