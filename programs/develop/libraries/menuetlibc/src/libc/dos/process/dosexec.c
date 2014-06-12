#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <menuet/os.h>
#include <sys/stat.h>

int _dos_exec(const char *program, const char *args, char * const envp[])
{
 return -1;
}


int _is_unixy_shell (const char *shellpath)
{
 return -1;
}

int _is_dos_shell (const char *shellpath)
{
 return -1;
}

int __dosexec_command_exec(const char *program, char **argv, char **envp)
{
 return -1;
}

char * __dosexec_find_on_path(const char *program, char *envp[], char *buf)
{
 return NULL;
}


static void build_args(char * dstbuf,char * const argv[],int argc)
{ 
 int i,j;
 for(i=0;i<argc;i++)
 {
  j=strlen(argv[i]);
  if(i==(argc-1))
   sprintf(dstbuf,"%s",argv[i]);
  else
   sprintf(dstbuf,"%s ",argv[i]);
  dstbuf+=j;
 }
}

int __spawnve(int mode, const char *path, char *const argv[], char *const envp[])
{
 char * buffer_for_args;
 int ap,asz;
 struct systree_info st_info;
 int res;
 fflush(stdout);
 if(!path)
 {
  errno=EINVAL;
  return -1;
 }
 if(strlen(path)>FILENAME_MAX-1)
 {
  errno = ENAMETOOLONG;
  return -1;
 }
 for(ap=0,asz=10;argv[ap]!=NULL;ap++)
 {
  asz+=strlen(argv[ap])+1;
 }
 if(ap)
 {
  buffer_for_args=malloc(asz);
  if(!buffer_for_args)
  {
   errno=ENOMEM;
   return -1;
  }
  memset(buffer_for_args,0,asz);
  build_args(buffer_for_args,argv,ap); 
 } else buffer_for_args=NULL;
	st_info.command = 7;
	st_info.file_offset_low = 0;
	st_info.file_offset_high = (__u32)buffer_for_args;
	st_info.size = 0;
	st_info.data_pointer = 0;
	st_info._zero = 0;
	//_fix_path(path,st_info.name);
	st_info.nameptr = path;
	res = __kolibri__system_tree_access(&st_info);
	if (res == -5)
		errno = ENOENT;
	else if (res == -31)
		errno = ENOEXEC;
	else if (res == -30 || res == -32)
		errno = ENOMEM;
	else if (res < 0)
		errno = EINVAL;
	free(buffer_for_args);
 if(mode==0x1BADB002) exit(0);
	return (res>0)?res:-1;
}
