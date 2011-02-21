#include "dosemuin.h"
#include <errno.h>

static char _io_filename[256];

static inline int sys_systree(struct systree_info * i,int * EBX)
{
 int d0,d1;
 __asm__ __volatile__("int $0x40"
     :"=a"(d0),"=b"(d1)
     :"0"(70),"1"((unsigned long)i)
     :"memory");
 if(EBX) *EBX=d1;
 return d0;
}

int dosemu_file_exists(const char * filename)
{
	struct systree_info finf;
	struct bdfe_item attr;
	finf.command = 5;
	finf.file_offset_low = 0;
	finf.file_offset_high = 0;
	finf.size = 0;
	finf.data_pointer = (__u32)&attr;
	finf._zero = 0;
	finf.nameptr = filename;
	if (sys_systree(&finf,NULL)!=0)
		return -1;
	return (int)attr.filesize_low;
}

int dosemu_createtrunc(const char * filename)
{
	struct systree_info finf;
	finf.command = 2;
	finf.file_offset_low = finf.file_offset_high = 0;
	finf.size = 0;
	finf.data_pointer = 0;
	finf._zero = 0;
	finf.nameptr = filename;
	if (sys_systree(&finf,NULL))
		return -1;
	return 0;
}

_io_struct * dosemu_getiostruct(int handle)
{
 if(handle<0 || handle>=_MAX_HANDLES) return NULL;
 if(_io_handles[handle].oflags==-1) return NULL;
 return _io_handles+handle;
}

int dosemu_allochandle(void)
{
 int i;
 for(i=0;i<_MAX_HANDLES;i++)
  if(_io_handles[i].oflags==-1) return i;
 return -1;
}

int dosemu_freehandle(int i)
{
 if(i<0) return;
 _io_handles[i].oflags=-1;
}

int dosemu_fileread(_io_struct * sh,char * buffer,int count)
{ 
	struct systree_info finf;
	int res,ebx;
	finf.command = 0;
	finf.file_offset_low = sh->pointer;
	finf.file_offset_high = 0;
	finf.size = count;
	finf.data_pointer = (__u32)buffer;
	finf._zero = 0;
	finf.nameptr = sh->filename;
	res = sys_systree(&finf,&ebx);
	if (res != 0 && res != 6)
		return -1;
	sh->pointer += ebx;
	return ebx;
}

int dosemu_filewrite(_io_struct * sh,char * buffer,int count)
{
	struct systree_info finf;
	int res,ebx;
	finf.command = 3;
	finf.file_offset_low = sh->pointer;
	finf.file_offset_high = 0;
	finf.size = count;
	finf.data_pointer = (__u32)buffer;
	finf._zero = 0;
	finf.nameptr = sh->filename;
	res = sys_systree(&finf,&ebx);
	if (res != 0 && res != 6)
		return -1;
	sh->pointer += ebx;
	if (sh->size < sh->pointer)
		sh->size = sh->pointer;
	return ebx;
}

int dosemu_iosize(int handle)
{
 _io_struct * sh=dosemu_getiostruct(handle);
 if(!sh) return -1;
 return sh->size;
}

int dosemu_filesize(char * filename)
{
 return dosemu_file_exists(filename);
}

static char fn_buf[256];

int dosemu_open(char * filename,int oflags)
{
 int baseflags,h,fsize;
 _fixpath(filename,_io_filename);
 baseflags=oflags&(O_RDONLY|O_WRONLY|O_RDWR);
 h=dosemu_allochandle();
 fsize=dosemu_file_exists(_io_filename);
 if(oflags & O_CREAT)
 {
  int creatflags=oflags & (O_EXCL|O_TRUNC);
  if(creatflags & O_EXCL)
  {
   if(fsize>=0)
   {
    dosemu_freehandle(h);
    return -1;
   }
  }
  if(fsize<0 || (creatflags&O_TRUNC))
  {
   if(dosemu_createtrunc(_io_filename)<0)
   {
    dosemu_freehandle(h);
    return -1;
   }
   fsize=0;
  }
 }
 else if (fsize<0)
 {
  dosemu_freehandle(h);
  return -1;
 }
 _io_handles[h].oflags=oflags;
 _io_handles[h].size=fsize;
 _io_handles[h].pointer=0;
 switch (baseflags)
 {
  case O_RDONLY:_io_handles[h].flags=_IO_READ;break;
  case O_WRONLY:_io_handles[h].flags=_IO_WRITE;break;
  case O_RDWR:_io_handles[h].flags=_IO_READ|_IO_WRITE;break;
  default:dosemu_freehandle(h);return -1;
 }
 strcpy(_io_handles[h].filename,_io_filename);
 return h;
}

int dosemu_tell(int handle)
{
 _io_struct * sh=dosemu_getiostruct(handle);
 if(!sh) return -1;
 return sh->pointer;
}
  
int dosemu_lseek(int handle,long offset,int origin)
{
 int newpointer=0;
 _io_struct *sh=dosemu_getiostruct(handle);
 if(!sh)return -1;
 if(handle==0 || handle==1 || handle==2 || handle==3) return -1;
 switch(origin)
 {
  case SEEK_SET: newpointer=offset;break;
  case SEEK_CUR: newpointer=sh->pointer+offset;break;
  case SEEK_END: newpointer=sh->size+offset;break;
 }
 if(newpointer<0)return -1;
 sh->pointer=newpointer;
 return newpointer;
}

int dosemu_read( int handle, void *buffer, unsigned int count )
{
 _io_struct *sh=dosemu_getiostruct(handle);
 if(!sh)return -1;
 if(!(sh->flags&_IO_READ)) return -1;
 return dosemu_fileread(sh,buffer,count);  
}

int dosemu_write( int handle, void *buffer, unsigned int count )
{
 _io_struct *sh=dosemu_getiostruct(handle);
 int k;
 if(!sh)return -1;
 if(!(sh->flags&_IO_WRITE)) return -1;
 return dosemu_filewrite(sh,buffer,count);  
}

int dosemu_close( int handle )
{
 _io_struct *sh=dosemu_getiostruct(handle);
 if(!sh)return -1;
 dosemu_freehandle(handle);
 return 0; 
}

void _dosemu_flush(int handle)
{}

int dosemu_truncate(int fd, off_t where)
{
	struct systree_info finf;
	int res;
	_io_struct* sh = dosemu_getiostruct(fd);
	if (!sh) return EBADF;
	if (!(sh->flags & _IO_WRITE)) return EBADF;
	finf.command = 4;
	finf.file_offset_low = where;
	finf.file_offset_high = 0;
	finf.size = 0;
	finf.data_pointer = 0;
	finf._zero = 0;
	finf.nameptr = sh->filename;
	res = sys_systree(&finf,NULL);
	if (res == 8) return ENOSPC;
	if (res) return EACCES;
	sh->size = where;
	return 0;
}
