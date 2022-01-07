#include"mcoff.h"
#include<stdio.h>
#include<stdlib.h>
#include"string.h"

#undef MCOFF_MENUETOS
#define MCOFF_MENUETOS	0

#if (MCOFF_MENUETOS==1)
struct systree_blk {
 unsigned long cmd,pos,blks;
 void * data,* work;
 char name[128];
} __attribute__((packed));

static char systree_work[16384];
static char temp_block[512];

static struct systree_blk sblk={
 0,
 0,
 1,
 NULL,
 systree_work,
};

static unsigned long open_on_path(char * pth,char * name)
{
 int l;
 int d0,d1;
 l=strlen(pth);
 if(!pth)
 {
  sprintf(sblk.name,"%s",pth);
 } else {
  if(pth[l]!='/')
   sprintf(sblk.name,"%s/%s",pth,name);
  else
   sprintf(sblk.name,"%s%s",pth,name);
 }
 sblk.data=&temp_block;
 sblk.cmd=0;
 sblk.pos=0;
 sblk.blks=1;
 sblk.work=systree_work;
 __asm__ __volatile__("int $0x40":"=a"(d0),"=b"(d1):"0"(58),"1"((void *)&sblk));
 if(d0!=0) return 0;
 return d1;
}
#endif

coffobj_t * mcoff_load_file(char * fname)
{
 coffobj_t * obj;
 int i;
#if (MCOFF_MENUETOS==1)
 unsigned long sz;
#endif
 obj=(coffobj_t *)malloc(sizeof(coffobj_t));
 if(!obj)
 {
  dprintf("malloc error1\n");
  return NULL;
 }
#if (MCOFF_MENUETOS==0)
 FILE * f=fopen(fname,"rb");
 if(!f)
 {
  dprintf("Unable to open file\n");
  free(obj);
  return NULL;
 }
 dprintf("File opened\n");
 fseek(f,0,SEEK_END);
 dprintf("After seek to end\n");
 obj->co_filesize=ftell(f);
 dprintf("After ftell\n");
 fseek(f,0,SEEK_SET);
 dprintf("After seek to start\n");
 dprintf("File size is %u bytes\n",obj->co_filesize);
#else
 /* Special actions for MenuetOS, because it doesn't support relative paths */
 /* We just search some paths if it is relative */
 if(fname[0]!='/')
 {
  sz=open_on_path("/SYS",fname);
  if(sz>64 && sz<0x1000000) goto OK; /* Max 16MB for DLL */
  sz=open_on_path("/HD/1/MENUETOS",fname);
  if(sz>64 && sz<0x1000000) goto OK; /* Max 16MB for DLL */
  sz=open_on_path("/HD/1/MENUETOS/DLL",fname);
  if(sz>64 && sz<0x1000000) goto OK; /* Max 16MB for DLL */
 } else {
  dprintf("Opening on std path\n");
  sz=open_on_path("",fname);
  if(sz>64 && sz<0x1000000) goto OK; /* Max 16MB for DLL */
 }  
 free(obj);
 return NULL;
OK:
 obj->co_filesize=sz;
 dprintf("File size is %u bytes\n",sz);
#endif
 obj->co_loadptr=(char *)malloc((obj->co_filesize+511)&~511); 
 if(!obj->co_loadptr)
 {
  dprintf("Unable to create file memory\n");
#if (MCOFF_MENUETOS==0)
  fclose(f);
#endif
  free(obj);
  return NULL;
 }
 dprintf("Memory allocated\n");
#if (MCOFF_MENUETOS==0)
 dprintf("Before fread\n");
 fread(obj->co_loadptr,1,obj->co_filesize,f);
 dprintf("After fread\n");
 fclose(f);
 dprintf("After fclose\n");
#else
 sblk.cmd=0;
 sblk.pos=0;
 sblk.blks=((sz+511)&~511)/512;
 sblk.data=obj->co_loadptr;
 sblk.work=systree_work;
 {
  int d0,d1;
  __asm__ __volatile__("int $0x40":"=a"(d0),"=b"(d1):"0"(58),"1"((void *)&sblk));
 }
 dprintf("Done reading file\n");
#endif
 dprintf("Checking file\n");
 obj->co_loadaddr=(unsigned long)obj->co_loadptr;
 obj->co_filehdr=(FILHDR *)obj->co_loadaddr;
 /* Check if file is really COFF */
 if(I386BADMAG(*obj->co_filehdr))
 {
  dprintf("bad magic\n");
NOREL:
  free(obj->co_loadptr);
  free(obj);
  return NULL;
 }
 /* We don't support files with relocations stripped */
/* if(obj->co_filehdr->f_flags & F_RELFLG)
 {
  printf("No relocation info\n");
  goto NOREL;
 } */
 /* Get into section table, symbol table and string table */
 obj->co_sections=(SCNHDR *)(obj->co_loadaddr+FILHSZ+obj->co_filehdr->f_opthdr);
 obj->co_symtab=(SYMENT *)(obj->co_loadaddr+obj->co_filehdr->f_symptr);
 obj->co_strtab=(char *)(obj->co_loadaddr+obj->co_filehdr->f_symptr+
  SYMESZ*obj->co_filehdr->f_nsyms);
 /* Setup .bss section */
 {
  SCNHDR * h;
  h=obj->co_sections;
  dprintf("Looking for bss...\n");
  for(i=0;i<obj->co_filehdr->f_nscns;i++,h++)
  {
   unsigned long r;
   if((h->s_flags & 0xE0)!=0x80) continue;
   r=h->s_size;
   obj->co_bssptr=(char *)malloc(r);
   obj->co_bsssize=r;
   if(!obj->co_bssptr)
   {
    dprintf("Unable to alloc %u bytes for bss\n",r);
    free(obj->co_loadptr);
    free(obj);
    return NULL;
   }
   obj->co_bssaddr=(unsigned long)obj->co_bssptr;
   h->s_scnptr=obj->co_bssaddr-obj->co_loadaddr;
   obj->co_bsssectnum=i+1; /* So we don't have to do it later */
   dprintf("BSS size=%u bytes\n",obj->co_bsssize);
  }
 }
NOBSS:
 /* File is COFF. Just return obj */
 return obj;
}

void unload_coff_file(coffobj_t * obj)
{
 if(!obj) return;
 free(obj->co_loadptr);
 return;
}
