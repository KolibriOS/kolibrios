#ifndef __OS_LAYER_OS_LAYER_H
#define __OS_LAYER_OS_LAYER_H

#ifdef __cplusplus
extern "C" {
#endif

#include<menuet/os.h>
#include<stdio.h>
#include<stdlib.h>

#define __FSEMU_FLAG_USED		0x00000001
#define __FSEMU_FLAG_DIRECTORY		0x00000002
#define __FSEMU_FLAG_DEVICE		0x00000004
#define __FSEMU_FLAG_READ  		0x00000008
#define __FSEMU_FLAG_WRITE		0x00000010
#define __FSEMU_FLAG_SEEK		0x00000020
#define __FSEMU_FLAG_STDIO		0x00000040

#define __FSF(x)	__FSEMU_FLAG_##x

#define T_FSF(x,f)	((x)&__FSF(f))

#define __FSEMU_BLOCK_SIZE		512

struct __fsemu_io_t;

typedef struct
{
 int (* putc)(int dev,int c);
 int (* getc)(int dev,int * c);
 int (* read)(struct __fsemu_io_t *,int blkno,int blkcount,char *);
 int (* write)(struct __fsemu_io_t *,int blkno,int blkcount,char *);
} __fsemu_stdio_t;

typedef struct __fsemu_io_t
{
 char * filename;
 char * filebuffer;
 int handle;
 int pos;
 int size;
 int flags;
 char * systree_buf;
 struct systree_info * st_info;
 __fsemu_stdio_t * special_ops;
} __fsemu_io_t;

#define __FSEMU_MAX_FILES		64

extern __fsemu_stdio_t __fsemu_stdin_ops,
		       __fsemu_stdout_ops,
		       __fsemu_stderr_ops,
		       __fsemu_stdprn_ops,
		       __fsemu_stdaux_ops;
		       
#define __FSEMU_f_STDIN { "stdin",NULL,0,0,0,__FSF(USED)|__FSF(READ) \
			  |__FSF(DEVICE)|__FSF(STDIO), NULL,NULL,&__fsemu_stdin_ops }
#define __FSEMU_f_STDOUT { "stdout",NULL,1,0,0,__FSF(USED)|__FSF(WRITE) \
			  |__FSF(DEVICE)|__FSF(STDIO), NULL,NULL,&__fsemu_stdout_ops }
#define __FSEMU_f_STDERR { "stderr",NULL,2,0,0,__FSF(USED)|__FSF(WRITE) \
			  |__FSF(DEVICE)|__FSF(STDIO), NULL,NULL,&__fsemu_stderr_ops }
#define __FSEMU_f_STDPRN { "stdprn",NULL,3,0,0,__FSF(USED)|__FSF(WRITE) \
			  |__FSF(DEVICE)|__FSF(STDIO), NULL,NULL,&__fsemu_stdprn_ops }
#define __FSEMU_f_STDAUX { "stdaux",NULL,4,0,0,__FSF(USED)|__FSF(WRITE) \
			  |__FSF(DEVICE)|__FSF(STDIO)|__FSF(READ), NULL,NULL,&__fsemu_stdaux_ops }

#define __FSEMU_STDIO_DECLS \
    static __fsemu_io_t __fsemu_io_stdin=__FSEMU_f_STDIN; \
    static __fsemu_io_t __fsemu_io_stdout=__FSEMU_f_STDOUT; \
    static __fsemu_io_t __fsemu_io_stderr=__FSEMU_f_STDERR; \
    static __fsemu_io_t __fsemu_io_stdprn=__FSEMU_f_STDPRN; \
    static __fsemu_io_t __fsemu_io_stdaux=__FSEMU_f_STDAUX;

#define __FSEMU_STDIO_TABLE_DECLS \
    &__fsemu_io_stdin, \
    &__fsemu_io_stdout, \
    &__fsemu_io_stderr, \
    &__fsemu_io_stdprn, \
    &__fsemu_io_stdaux


extern __fsemu_stdio_t __fsemu_rd1_ops,
		       __fsemu_hd1_ops,
		       __fsemu_hd2_ops,
		       __fsemu_hd3_ops,
		       __fsemu_hd4_ops;

#define __FSEMU_f_RD1 { "/RD/1",NULL,5,0,0,__FSF(USED)|__FSF(READ)| \
		        __FSF(WRITE)|__FSF(DEVICE),NULL,NULL,&__fsemu_rd1_ops }
#define __FSEMU_f_HD1 { "/HD/1",NULL,6,0,0,__FSF(USED)|__FSF(READ)| \
		        __FSF(DEVICE),NULL,NULL,&__fsemu_hd1_ops }
#define __FSEMU_f_HD2 { "/HD/2",NULL,7,0,0,__FSF(USED)|__FSF(READ)| \
		        __FSF(DEVICE),NULL,NULL,&__fsemu_hd2_ops }
#define __FSEMU_f_HD3 { "/HD/3",NULL,8,0,0,__FSF(USED)|__FSF(READ)| \
		        __FSF(DEVICE),NULL,NULL,&__fsemu_hd3_ops }
#define __FSEMU_f_HD4 { "/HD/4",NULL,9,0,0,__FSF(USED)|__FSF(READ)| \
		        __FSF(DEVICE),NULL,NULL,&__fsemu_hd4_ops }
			
#define __FSEMU_DEV_DECLS \
    static __fsemu_io_t __fsemu_io_rd1=__FSEMU_f_RD1; \
    static __fsemu_io_t __fsemu_io_hd1=__FSEMU_f_HD1; \
    static __fsemu_io_t __fsemu_io_hd2=__FSEMU_f_HD2; \
    static __fsemu_io_t __fsemu_io_hd3=__FSEMU_f_HD3; \
    static __fsemu_io_t __fsemu_io_hd4=__FSEMU_f_HD4;

#define __FSEMU_DEV_TABLE_DECLS \
    &__fsemu_io_rd1,    \
    &__fsemu_io_hd1,    \
    &__fsemu_io_hd2,    \
    &__fsemu_io_hd3,    \
    &__fsemu_io_hd4
    
#define __F_READABLE(x)	T_FSF(x,READ)
#define __F_WRITABLE(x)	T_FSF(x,WRITE)
#define __F_SEEKABLE(x)	T_FSF(x,SEEK)

#ifdef __cplusplus
}
#endif
        			 
#endif
