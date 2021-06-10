#include <stdio.h>
#include <errno.h>
#include "conio.h"
#include "sys/ksys.h"

size_t fread(void *restrict ptr, size_t size, size_t nmemb, FILE *restrict stream) {
	unsigned bytes_read = 0;
	unsigned bytes_count = size * nmemb;
	
	if(!stream){
		errno = EBADF;
		return 0;
	}
	
	if(stream==stdin){
		con_init();
		con_gets((char*)ptr, bytes_count+1);
		return nmemb;
	}

	else{
		if(stream->mode & _FILEMODE_R){
			if(!stream->__ungetc_emu_buff){
				((char*) ptr)[0]=(char)stream->__ungetc_emu_buff;
				//debug_printf("Ungetc: %x\n", ((char*) ptr)[0]);
			}
			unsigned status = _ksys_file_read_file(stream->name, stream->position, bytes_count, ptr , &bytes_read);
			if (status) {
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
