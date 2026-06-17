#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>

int vfprintf(FILE* file, const char* format, va_list arg)
{
    char* buf;
    int printed = 0;
    size_t rc = 0;
    va_list arg_copy;

    if (!file) {
        errno = EBADF;
        return -1;
    }

    if (!format) {
        errno = EINVAL;
        return -1;
    }

    buf = malloc(STDIO_MAX_MEM);
    if (!buf) {
        errno = ENOMEM;
        return -1;
    }

    va_copy(arg_copy, arg);
    printed = vsnprintf(buf, STDIO_MAX_MEM, format, arg_copy);
    va_end(arg_copy);

    if (printed == -1) {
        free(buf);
        errno = EILSEQ;
        return -1;
    }

    // output didn't fit into STDIO_MAX_MEM, grow the buffer to the exact size
    if ((size_t)printed >= STDIO_MAX_MEM) {
        // TODO: replace with a faster realloc when it is implemented
        char* bigger = realloc(buf, (size_t)printed + 1);
        if (!bigger) {
            free(buf);
            errno = ENOMEM;
            return -1;
        }
        buf = bigger;

        va_copy(arg_copy, arg);
        printed = vsnprintf(buf, (size_t)printed + 1, format, arg_copy);
        va_end(arg_copy);

        if (printed == -1) {
            free(buf);
            errno = EILSEQ;
            return -1;
        }
    }

    if (printed) {
        rc = fwrite(buf, sizeof(char), printed, file);
    }
    free(buf);

    if (rc < (size_t)printed) {
        errno = EIO;
        return -1;
    }

    return (int)rc;
}
