#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern char __argv;
extern char __path;

char* getfullpath(const char *path){

        int i,j,relpath_pos,localpath_size;
        int filename_size;
        char local_path;
        char *programpath;
        char *newpath;

        i=0;
        local_path=1; //enable local path
        while((*(path+i)!='\0') || (*(path+i)!=0))
        {
                if (*(path+i)=='.')
                {
                        if (*(path+i+1)=='/')
                        {       //detected relative path
                                relpath_pos=i+2;
                                local_path=0;
                                break;
                        }
                }
                if (*(path+i)=='/')
                {       //disabple local path
                        local_path=0;
                        return(path);
                }
                i++;
        }
        filename_size=i;

        programpath=&__path;

        if (local_path==1)
        {
                i=0x400;
                //find local path of program
                while(*(programpath+i)!='/')
                {
                        i--;
                }
                localpath_size=i;
                newpath=malloc(0x400);
                //copy local path to the new path
                for(i=0;i<=localpath_size;i++)
                {
                        *(newpath+i)=*(programpath+i);
                }
                //copy filename to the new path
                for(i=0;i<filename_size;i++)
                {
                        *(newpath+localpath_size+1+i)=*(path+i);
                }
                return(newpath);
        }

       //if we here than path is a relative
       i=0x400;
       //find local path of program
       while(*(programpath+i)!='/')
       {
                i--;
       }
       localpath_size=i;
       i=0;
       //find file name size
       while((*(path+relpath_pos+i)!='\0') || (*(path+relpath_pos+i)!=0))
       {
                i++;
       }
       filename_size=i;
       newpath=malloc(0x400);
        //copy local path to the new path
       for(i=0;i<=localpath_size;i++)
       {
                *(newpath+i)=*(programpath+i);
       }
       //copy filename to the new path
       for(i=0;i<filename_size;i++)
       {
                *(newpath+localpath_size+1+i)=*(path+relpath_pos+i);
       }
       return(newpath);
}


FILE* fopen(const char* filename, const char *mode)
{
        FILE* res;
        int imode;
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
                return 0;
        res=malloc(sizeof(FILE));
        res->buffer=malloc(256);
        res->buffersize=256;
        res->filesize=0;
        res->filepos=0;
        res->mode=imode;
        res->filename=getfullpath(filename);

	if ((imode==FILE_OPEN_READ) || (imode==FILE_OPEN_APPEND))
	{
		res->filesize=_ksys_get_filesize(res->filename);
	}
        return res;
}
