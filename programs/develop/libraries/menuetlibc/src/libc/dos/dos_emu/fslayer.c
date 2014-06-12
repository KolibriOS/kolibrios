#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<fcntl.h>
#include<menuet/os.h>

struct systree_blk 
{
 unsigned long cmd,pos,blks;
 void * data,* work;
 char name[256];
} __attribute__((packed));

typedef struct EMU_FILE
{
 int			handle;
 unsigned long		size;
 unsigned long		pos;
 unsigned long		mode;
 unsigned long		current_sector;
 unsigned long		size_sectors;
 char 		      * write_buf;
 unsigned long		write_bufsize;
 char			rd_buffer[512];
 char			name[256];
 unsigned char		dirty;
} EMU_FILE;

#define _MAX_HANDLES		64

static EMU_FILE		* EMU_file_table[_MAX_HANDLES];
static char		  systree_work_area[16384+512];
static volatile struct systree_blk sblk;
static volatile int S_eax,S_ebx;

static inline int EMU_alloc_handle(void)
{
 register int i;
 for(i=0;i<_MAX_HANDLES;i++)
  if(!EMU_file_table[i])
  {
   EMU_file_table[i]=(EMU_FILE *)malloc(sizeof(EMU_FILE));
   if(!EMU_file_table[i]) return -ENOMEM;
   return i;
  }
 return -EAGAIN;
}

static inline int EMU_systree_cmd(void)
{
 __asm__ __volatile__("int $0x40"
     :"=a"(S_eax),"=b"(S_ebx)
     :"0"(58),"1"((void *)&sblk));
 return S_eax;
}

static int EMU_loadsector(EMU_FILE * filp)
{
 sblk.cmd=0;
 sblk.pos=filp->current_sector;
 sblk.blks=1;
 sblk.data=filp->rd_buffer;
 sblk.work=systree_work_area;
 memcpy((void *)&sblk.name,(const void *)filp->name,strlen(filp->name)+1);
 return EMU_systree_cmd();
}

static int EMU_fsync(EMU_FILE * filp)
{
 if(filp->mode==O_RDONLY) return 0;
 if(!filp->dirty) return 0;
 filp->dirty=0;
 sblk.cmd=1;
 sblk.pos=0;
 sblk.blks=filp->size;
 sblk.data=filp->write_buf;
 sblk.work=systree_work_area;
 memcpy((void *)sblk.name,(const void *)filp->name,strlen(filp->name)+1);
 return EMU_systree_cmd();
}

static inline int EMU_realloc_buf(EMU_FILE * filp,unsigned long newsize)
{
 char * n;
 newsize=(newsize+511)&~511;
 if(filp->write_bufsize==newsize) return 0;
 n=(char *)realloc(filp->write_buf,newsize);
 if(!n) return -ENOSPC;
 filp->write_buf=n;
 filp->write_bufsize=newsize;
 filp->dirty=1;
 return 0;
}

static int EMU_createtrunc(char * fname)
{
 sblk.cmd=1;
 sblk.pos=0;
 sblk.blks=0;
 sblk.data=sblk.work=systree_work_area;
 memcpy((void *)sblk.name,(const void *)fname,strlen(fname)+1);
 return EMU_systree_cmd();
}

static int EMU_getfsize(char * fname,unsigned long * sizep)
{
 sblk.cmd=0;
 sblk.pos=0;
 sblk.blks=1;
 sblk.data=systree_work_area+16384;
 sblk.work=systree_work_area;
 memcpy((void *)sblk.name,(const void *)fname,strlen(fname)+1);
 if(EMU_systree_cmd()!=0) return -EINVAL;
 if(sizep) *sizep=(unsigned long)S_ebx;
 return 0;
}

static int EMU_open(char * fname,int mode)
{
 EMU_FILE * filp;
 register int hid;
 unsigned long iomode;
 hid=EMU_alloc_handle();
 if(hid<0) return hid;
 filp=EMU_file_table[hid];
 filp->handle=hid;
 iomode=mode&(O_RDONLY|O_WRONLY|O_RDWR);
 memcpy((void *)filp->name,(const void *)fname,strlen(fname)+1);
 strupr(filp->name);
 filp->mode=iomode;
 if(mode&O_CREAT)
 {
  int createflags=mode&(O_TRUNC|O_EXCL);
  if(createflags&O_EXCL)
  {
   unsigned long psz=0;
   if(EMU_getfsize(filp->name,&psz)==0)
   {
    free(EMU_file_table[hid=filp->handle]);
    EMU_file_table[hid]=NULL;
    return -EEXIST;
   }
  }
  if(createflags&O_TRUNC)
  {
   EMU_createtrunc(filp->name);
  }
 }
 if(iomode==O_RDONLY)
 {
  hid=EMU_getfsize(filp->name,&filp->size);
  if(hid<0)
  {
   free(EMU_file_table[hid=filp->handle]);
   EMU_file_table[hid]=NULL;
   return -ENOENT;
  } 
  filp->current_sector=0;
  if(EMU_loadsector(filp)<0) filp->current_sector=-1UL;
  filp->mode=O_RDONLY;
  filp->size_sectors=(filp->size+511)/512;
  filp->write_bufsize=0;
  filp->dirty=0;
  return filp->handle;
 }
 if(iomode==O_WRONLY)
 {
  hid=EMU_getfsize(filp->name,&filp->size);
  if(hid<0)
  {
BAD_WRO:
   free(EMU_file_table[hid=filp->handle]);
   EMU_file_table[hid]=NULL;
   return -ENOENT;
  }
  filp->current_sector=-1UL;
  filp->mode=O_WRONLY;
  filp->size_sectors=0;
  filp->write_bufsize=(filp->size+511)&~511;
  filp->write_buf=(char *)malloc(filp->write_bufsize);
  if(!filp->write_buf)
  {
   free(filp->write_buf);
   goto BAD_WRO;
  }
  sblk.cmd=0;
  sblk.pos=0;
  sblk.blks=filp->write_bufsize/512;
  sblk.data=filp->write_buf;
  sblk.work=systree_work_area;
  if(EMU_systree_cmd()!=0) goto BAD_WRO1;
  return filp->handle;
 }
 hid=EMU_getfsize(filp->name,&filp->size);
 if(hid<0)
 {
BAD_WRO1:
  free(EMU_file_table[hid=filp->handle]);
  EMU_file_table[hid]=NULL;
  return -ENOENT;
 }
 filp->current_sector=-1UL;
 filp->mode=O_RDWR;
 filp->size_sectors=0;
 filp->write_bufsize=(filp->size+511)&~511;
 filp->write_buf=(char *)malloc(filp->write_bufsize);
 if(!filp->write_buf)
 {
  free(filp->write_buf);
  goto BAD_WRO1;
 }
 sblk.cmd=0;
 sblk.pos=0;
 sblk.blks=filp->write_bufsize/512;
 sblk.data=filp->write_buf;
 sblk.work=systree_work_area;
 if(EMU_systree_cmd()!=0) goto BAD_WRO1;
 return filp->handle;
}

static int EMU_close(EMU_FILE * filp)
{
 int hid;
 if(!filp) return -ENOENT;
 if(EMU_file_table[hid=filp->handle]!=filp) return -EBADF;
 if(filp->write_buf) free(filp->write_buf);
 free(filp);
 EMU_file_table[hid]=NULL; 
 return 0;
}

static int EMU_lseek(EMU_FILE * filp,unsigned long off,int whence)
{
 unsigned long newpos;
 switch(whence)
 {
  case SEEK_SET:
   newpos=off;
   break;
  case SEEK_CUR:
   newpos=filp->pos+off;
   break;
  case SEEK_END:
   newpos=filp->size+off-1;
   break;
 }
 if(newpos>=filp->size) return -1;
 filp->pos=newpos;
 return filp->pos;
}

static int EMU_read(EMU_FILE * filp,unsigned long size,void * buf)
{
 int icount,curr_sector,curr_sector_ofs,n;
 int nbufbytes,totalremaining;
 if(filp->pos+count>filp->size)
  count=filp->size-filp->pos;
 if(filp->mode==O_RDWR)
 {
  memcpy(buffer,filp->write_buf+filp->pos,count);
  filp->pos+=count;
  return count;
 }
 icount=count;
 while(count>0)
 {
  if(filp->pos>=filp->size) return icount=count;
  curr_sector=sh->pointer>>9;
  curr_sector_ofs=sh->pointer&511;
  n=count;
  if(sh->bufsector==-1 || curr_sector!=sh->bufsector)
  {
   if(dosemu_loadcurrsector(sh)==-1) return -1;
  }
  nbufbytes=512-curr_sector_ofs;
  totalremaining=sh->size-sh->pointer;
  if(nbufbytes>totalremaining) nbufbytes=totalremaining;
  if(n>nbufbytes) n=nbufbytes;
  memcpy(buffer,&sh->buf[curr_sector_ofs],n);
  buffer+=n;
  count-=n;
  sh->pointer+=n;
 }
 return icount;
}
}
