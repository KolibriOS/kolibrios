#include <stdio.h>
#include "conio.h"
#include <sys/ksys.h>
#include <errno.h>


size_t fwrite(const void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream) {
	unsigned bytes_written = 0;
	unsigned bytes_count = size * nmemb;
	
	if(!stream){
		errno = EINVAL;
		return 0;
	}
	
	if(stream==stdout){
		__con_init();
		__con_write_string((char*)ptr, bytes_count);
		return nmemb;
	}
	else if(stream==stderr){
		for (size_t i = 0; i < bytes_count; i++) {
			char c = *(char*)(ptr+i);
			_ksys_debug_putc(c);
		}
	}
	else{
		if(stream->mode != _STDIO_F_R){
			unsigned status = _ksys_file_write_file(stream->name, stream->position, bytes_count, ptr, &bytes_written);
			if (status != KSYS_FS_ERR_SUCCESS) {
            	errno = EIO;
            	stream->error = errno;
				return 0;
        	}else {
    			stream->position+=bytes_written;
			}
		}
	}
	return bytes_written;
}