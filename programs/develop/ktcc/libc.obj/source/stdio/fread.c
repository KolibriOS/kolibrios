#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include "../sys/_conio.h"
#include "sys/ksys.h"
#include "_stdio.h"

size_t fread(void* restrict ptr, size_t size, size_t nmemb, FILE* restrict stream)
{
    unsigned bytes_read = 0;
    unsigned bytes_count = size * nmemb;

    if (!stream) {
        errno = EBADF;
        return 0;
    }

    if (size <= 0 || nmemb <= 0) {
        errno = EINVAL;
        stream->flags.error = true;
        return 0;
    }

    if (stream == stdin) {
        bytes_read = stdin_read(ptr, bytes_count);
    } else if (stream->flags.read) {
        if (!stream->__ungetc_emu_buff) {
            ((char*)ptr)[0] = (char)stream->__ungetc_emu_buff;
            // debug_printf("Ungetc: %x\n", ((char*) ptr)[0]);
        }
        unsigned status = _ksys_file_read_file(stream->name, stream->position, bytes_count, ptr, &bytes_read);
        if (status != KSYS_FS_ERR_SUCCESS) {
            if (status == KSYS_FS_ERR_EOF) {
                stream->flags.eof = true;
            } else {
                errno = EIO;
                stream->flags.error = true;
                return 0;
            }
        }
        stream->position += bytes_read;
    }

    return bytes_read / size;
}
