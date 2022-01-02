#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int errno = 0;

/*
// removed by Seiemargl 26-oct-2018
// use get_current_folder() from kos32sys.h instead
  

extern char __argv;
extern char __path;

// convert relative to program path ./file.txt to absolute
const char* getfullpath(const char *path){
	

        int  relpath_pos, localpath_size;
        char *programpath;
        char *newpath;
        char *prgname;

        if (path[0] == '/') //
        {
            return(strdup(path)); // dup need as free in fclose() 
        }

        relpath_pos = 0;
        if (path[0] == '.' && path[1] == '/')
        {
            //detected relative path, begins with ./
            relpath_pos=2;
        }

        programpath=&__path;

        //if we here than path is a relative or local
        prgname = strrchr(programpath, '/');
        if (!prgname) return strdup(path);

        localpath_size = prgname - programpath + 1;

        newpath = malloc(FILENAME_MAX);
        if(!newpath)
        {
            errno = E_NOMEM;
            return NULL;
        }
        //copy local path to the new path
        strncpy(newpath, programpath, localpath_size);
        newpath[localpath_size] = 0;

        //copy filename to the new path
        strcpy(newpath + localpath_size, path + relpath_pos);

        return(newpath);
}
*/


FILE* fopen(const char* filename, const char *mode)
{
        FILE* res;
        int imode, sz = -1;
		char *fullname;

        imode=0;
        if (*mode=='r')
        {
                imode=FILE_OPEN_READ;
                mode++;
        }else if (*mode=='w')
        {
                imode=FILE_OPEN_WRITE;
                mode++;
        }else if (*mode=='a')
        {
                imode=FILE_OPEN_APPEND;
                mode++;
        }else
                return 0;
        if (*mode=='+')
        {
                imode|=FILE_OPEN_PLUS;
                mode++;
        }
        if (*mode=='t')
        {
                imode|=FILE_OPEN_TEXT;
                mode++;
        }else if (*mode=='b')
                mode++;
        if (*mode=='+')
        {
                imode|=FILE_OPEN_PLUS;
                mode++;
        }
        if (*mode!=0)
                return NULL;
		
//		fullname = (char*)getfullpath(filename);
		fullname = strdup(filename);
		if ((imode & 3) == FILE_OPEN_READ && fullname)	/* check existense */
		{
			sz = _ksys_get_filesize(fullname);
			if (sz < 0) 
			{
				free(fullname);
				errno = sz;
				return NULL;
			}
		}
			
        res = malloc(sizeof(FILE));
        if (res)
        {
            res->buffer=malloc(BUFSIZ);
            res->buffersize=BUFSIZ;
            res->filesize=0;
            res->filepos=0;
            res->mode=imode;
            res->filename=fullname;
			res->ungetc_buf = EOF;
			res->buffer_start = -1;
			res->buffer_end = -1;
        }
        if(!res || !res->buffer || !res->filename)
        {
            errno = E_NOMEM;
            return NULL;
        }

	if ((imode & 3) == FILE_OPEN_READ || (imode & 3) == FILE_OPEN_APPEND)
	{
		if (sz > 0) /*already got*/
			res->filesize = sz;
		else
			res->filesize=_ksys_get_filesize(res->filename);
	}
        return res;
}
