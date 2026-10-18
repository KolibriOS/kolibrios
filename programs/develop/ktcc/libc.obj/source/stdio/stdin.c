#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#include "_stdio.h"

char* stdin_buffer = NULL;
size_t stdin_buffer_size = 0;
size_t stdin_buffer_len = 0;
size_t stdin_buffer_position = 0;

int stdin_buffer_init()
{
    if (stdin_buffer == NULL) {
        stdin_buffer = malloc(STDIO_MAX_MEM);

        if (!stdin_buffer) {
            return 1;
        }

        stdin_buffer_len = STDIO_MAX_MEM;
        stdin_buffer_position = 0;
    }

    return 0;
}

void stdin_buffer_free()
{
    if (stdin_buffer) {
        free(stdin_buffer);
    }
}

char* stdin_read_from_buffer(char* ptr, size_t *bytes_count, size_t *bytes_read)
{
    char* current_ptr = ptr;
    if (stdin_buffer && stdin_buffer_size > 0) {

        size_t len = min(*bytes_count, stdin_buffer_size);

        memcpy(current_ptr, stdin_buffer + stdin_buffer_position, len);

        stdin_buffer_position += len;
        stdin_buffer_size -= len;

        current_ptr += len;

        *bytes_count -= len;
        *bytes_read += len;
    }

    return current_ptr;
}

size_t stdin_read_from_console(char* current_ptr, size_t bytes_count, size_t bytes_read)
{
    size_t buffer_len = (stdin_buffer_len > 0 ? stdin_buffer_len : STDIO_MAX_MEM);
    char* b = bytes_count > buffer_len ? current_ptr : stdin_buffer;
    size_t len = max(bytes_count, buffer_len);

    size_t _read_bytes = 0;

    if (b == stdin_buffer) {
        if (stdin_buffer_init()) {
            errno = EFAULT;
            return 0;
        }
        b = stdin_buffer;
    }

    if (console_gets(b, len) != NULL) {
        _read_bytes = strlen(b);
    } else {
        errno = EIO;
        return 0;
    }

    size_t bytes_to_copy = min(bytes_count, _read_bytes);

    if (b != current_ptr) {

        memcpy(current_ptr, b, bytes_to_copy);

        int diff = _read_bytes - bytes_to_copy;

        stdin_buffer_position = bytes_to_copy;
        stdin_buffer_size = max(diff, 0);
    } else {
        stdin_buffer_position = 0;
        stdin_buffer_size = 0;
    }

    return bytes_to_copy;
}

size_t stdin_read(char* restrict ptr, size_t bytes_count)
{
    size_t bytes_read = 0;
    char* current_ptr = stdin_read_from_buffer(ptr, &bytes_count, &bytes_read);

    if (bytes_count > 0) {
        bytes_read += stdin_read_from_console(current_ptr, bytes_count, bytes_read);
    }

    return bytes_read;
}
