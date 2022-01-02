#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <kolibrisys.h>

int fread(void *buffer,int size,int count,FILE* file)
{
	dword res, readbytes;
	dword fullsize, read4cache, toread, readcount;

    if(!file || !buffer)
    {
        errno = E_INVALIDPTR;
        return 0;
    }

	if ((file->mode &3)!=FILE_OPEN_READ && (file->mode & FILE_OPEN_PLUS)==0)
    {
        errno = E_ACCESS;
        return 0;
    }

	fullsize=count*size;
	if (fullsize + file->filepos >= file->filesize)
	{
		fullsize=file->filesize - file->filepos;
		if (fullsize <= 0) return 0;
	}

	/***** file buffering strategy, just read forward *****
	if we read small part - read full buffer, but if buffer have this data - dont read again nothing (or partial read forward 4k pages)
	any writes drops buffers as ungetc_buf */
	read4cache = 0;
	readcount = 0;
	if (file->filepos >= file->buffer_start && file->filepos < file->buffer_end) 
	{
		read4cache = min(file->buffer_end - file->filepos, fullsize);
		memcpy(buffer, file->buffer + file->filepos - file->buffer_start, read4cache);
		file->filepos += read4cache;
		if (file->ungetc_buf != EOF)  // subst ungetc byte
		{	
			*((char*)buffer) = (char)file->ungetc_buf;
			file->ungetc_buf = EOF;	
		}			
		buffer += read4cache;   // ! advance
		fullsize -= read4cache;
		readcount = read4cache / size;
	}
	
	toread = max(fullsize, file->buffersize); 
	if (toread + file->filepos >= file->filesize)
	{ 
		toread = file->filesize - file->filepos;
	}
	
	if (fullsize <= 0 || toread <= 0)
		res = 0;  // already read or file end
	else
	{
		file->buffer_start = file->filepos;
		if (toread <= fullsize) // read to bigger buffer
		{
			res = _ksys_readfile(file->filename, file->filepos, toread, buffer, &readbytes);
			read4cache = min(readbytes, file->buffersize);
			memcpy(file->buffer, buffer, read4cache);
			file->filepos += readbytes;
		} else
		{
			res = _ksys_readfile(file->filename, file->filepos, toread, file->buffer, &readbytes);
			read4cache = readbytes;
			memcpy(buffer, file->buffer, min(fullsize, read4cache));
			file->filepos += min(fullsize, read4cache);
		}
		file->buffer_end = file->buffer_start + read4cache;
		if (readbytes >= fullsize)
			readcount += fullsize / size;
		else
			readcount += readbytes / size;
	}

	if (file->ungetc_buf != EOF)  // subst ungetc byte
	{	
		*((char*)buffer) = (char)file->ungetc_buf;
		file->ungetc_buf = EOF;	
	}			
	
	if (res != 0)
    {
		file->ungetc_buf = EOF;
        errno = -res;
    }
	
	return readcount;  // really full readed plus cached items
}
