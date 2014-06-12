#include<menuet/os.h>
#include<stdio.h>
#include<stdarg.h>

__u32 __menuet__open(char * name,char * data)
{
 __u32 __ret;
 __asm__ __volatile__("int $0x40":"=a"(__ret):"0"(6),"b"((__u32)name),"c"(0),"d"(0xFFFFFFFF),"S"((__u32)data));
 return __ret;
}

void __menuet__save(char * name,char * data,__u32 count)
{
 __asm__ __volatile__("int $0x40"::"a"(33),"b"((__u32)name),"c"((__u32)data),"d"(count),"S"(0));
}

void __menuet__exec_ramdisk(char * filename,char * args,...)
{
 va_list argz;
 char buffer[1024];
 memset(buffer,0,1024);
 if(args)
 {
  va_start(argz,args);
  vsprintf(buffer,args,argz);
 }
 va_end(argz);
 __asm__ __volatile__("int $0x40"::"a"(19),"b"(filename),"c"((args ? buffer : NULL)));
}

void __menuet__exec_hd(char * filename,char * args,...)
{
 va_list argz;
 char buffer[1024];
 char work_area[0xFFFF];
 memset(buffer,0,1024);
 if(args)
 {
  va_start(argz,args);
  vsprintf(buffer,args,argz);
 }
 va_end(argz);
 __asm__ __volatile__("int $0x40"::"a"(19),"b"(filename),"c"(args ? buffer : NULL),"d"(work_area));
}
