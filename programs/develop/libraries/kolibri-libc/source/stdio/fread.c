#include <stdio.h>
#include <errno.h>
#include "conio.h"
#include "sys/ksys.h"

size_t fread(void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream) {
	unsigned bytes_read = 0;
	unsigned bytes_count = size * nmemb;
	
	if(!stream){
		errno = EINVAL;
		return 0;
	}
	
	if(stream==stdin){
		__con_init();
		__con_gets((char*)ptr, bytes_count);
		return nmemb;
	}

	else{
		if(stream->mode != _STDIO_F_W){
			unsigned status = _ksys_file_read_file(stream->name, stream->position, bytes_count, ptr , &bytes_read);
			if (status != KSYS_FS_ERR_SUCCESS) {
            	errno = EIO;
            	stream->error = errno;
				return 0;
        	}else {
    			stream->position+=bytes_read;
			}
		}
	}
	return bytes_read;
}
