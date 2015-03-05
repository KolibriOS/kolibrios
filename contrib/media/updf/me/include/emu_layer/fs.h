#ifndef __EMULAYER_FS_H
#define __EMULAYER_FS_H

#include<sys/stat.h>
#include<fcntl.h>

#define MAX_EMUFS_HANDLES		64

typedef struct __emu_FILE	emu_FILE;

typedef struct {
    int (* read)(emu_FILE * filp,unsigned long size,char * buf);
    int (* write)(emu_FILE * filp,unsigned long size,char * buf);
    int (* seek)(emu_FILE * filp,int offset,int whence);
    int (* open)(emu_FILE * filp);
    int (* close)(emu_FILE * filp);
    int (* stat)(emu_FILE * filp,struct stat * statp);
    int (* sync)(emu_FILE * filp);
} emu_FILE_operations_t;

struct __emu_FILE
{
 int		f_handle;
 char *		f_path;
 unsigned long	f_pos;
 unsigned long	f_size;
 int		f_flags;
 int		f_mode;
 emu_FILE_operations_t * f_op;
 void * 	f_priv;
};

struct __emu_VMOUNT
{
 char * mpnt;
 int (* preopen_file)(emu_FILE * filp);
 struct __emu_VMOUNT * m_next;
};

void init_emufs(void);
struct __emu_VMOUNT * EMU_find_best_mount(char * fpath);
extern emu_FILE * EMU_file_table[MAX_EMUFS_HANDLES];
emu_FILE * EMU_get_empty_filp(char * forpath);
void EMU_put_filp(int h);
int EMU_open(const char * fname,int mode);
int EMU_close(int handle);
int EMU_read(int handle,char * buf,int count);
int EMU_write(int handle,char * buf,int count);
int EMU_lseek(int handle,int off,int whence);
int EMU_fstat(int handle, struct stat *statbuf);
int EMU_filelength(int fp);
int EMU_flush(int fp);

#endif
