#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "format_scan.h"
#include <errno.h>

// non standard realization - support for virtually change ONLY ONE char

static int __ungetc_emu(int c, FILE* stream)
{
	unsigned res;
    if(stream){
        errno = EINVAL;
        return EOF;
    }
	if ((stream->mode & 3) != _STDIO_F_R && (stream->mode & _STDIO_F_A) == 0){
        errno = EACCES;
        return EOF;
    }
    ksys_bdfe_t *file_info = malloc(sizeof(ksys_bdfe_t));
    if(file_info==NULL){
        errno = ENOMEM;
        return EOF;
    }
    if(!_ksys_file_get_info(stream->name, file_info)){
        errno = ENFILE;
        return EOF;
    }
	if (stream->position > file_info->size || stream->position == 0 || c == EOF || stream->__ungetc_emu_buff != EOF){
	    errno = EOF;
		return EOF;
	}
	
	stream->__ungetc_emu_buff = c;
	stream->position --;
	return c;
}

static int __virtual_getc_file(void *sp, const void *obj)
{
    FILE *f = (FILE *)obj;
    int ch = fgetc(f);
    return ch;
}

static void __virtual_ungetc_file(void *sp, int c, const void *obj)
{
    FILE *f = (FILE *)obj;
    if (f) __ungetc_emu(c, f);
}

int vfscanf(FILE * stream, const char * format, va_list arg)
{
    return _format_scan(stream, format, arg, &__virtual_getc_file, &__virtual_ungetc_file);
}