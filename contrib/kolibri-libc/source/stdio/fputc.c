#include <stdio.h>
#include <errno.h>
#include <ksys.h>

int fputc(int c, FILE *stream)
{
    unsigned bytes_written;

	unsigned status = _ksys_file_write_file(stream->name, stream->position, 1, &c, &bytes_written);
	
	if (status != _KOS_FS_ERR_SUCCESS) {
        switch (status) {
            case _KOS_FS_ERR_1:
            case _KOS_FS_ERR_2:
            case _KOS_FS_ERR_3:
            case _KOS_FS_ERR_4:
            case _KOS_FS_ERR_5:
            case _KOS_FS_ERR_EOF:
            case _KOS_FS_ERR_7:
            case _KOS_FS_ERR_8:
            case _KOS_FS_ERR_9:
            case _KOS_FS_ERR_10:
            case _KOS_FS_ERR_11:
            default:
                // Just some IO error, who knows what exactly happened
                errno = EIO;
                stream->error = errno;
                break;
        }
        return EOF;
    }

    stream->position++;
    return c;
}