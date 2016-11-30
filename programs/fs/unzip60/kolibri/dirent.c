/*
Kolibri OS port for gcc 5.4

Started by Siemargl @Nov 2016

Contains realization of directory handling functions:
    mkdir()
    closedir()
    opendir()
    readdir()
    rewinddir()

    !!non reentrant
*/

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <assert.h>
#include "kos32sys1.h"
#include "sys/kos_io.h"

/* defined in newlib headers
int	_EXFUN(mkdir,( const char *_path, mode_t __mode ));

struct dirent {
  char d_namlen;
  char d_name[256];
};

typedef struct
{
//    struct systree_info2 fileinfo;
    struct dirent entry;
//    __u8 bdfeheader[0x20];
//    struct bdfe_item bdfebase;
//    __u8 bdfename[264];
} DIR;

int     closedir(DIR *dirp);
DIR *       opendir(const char *_dirname);
struct dirent * readdir(DIR *_dirp);
void      rewinddir(DIR *_dirp);
*/

static int32_t lastread_block = -1; // non reentrant, must me -1ed in closedir, or will assert
static uint32_t lastread_dir_size;
static DIR  lastread_dir;

DIR *opendir(const char *_dirname)
{
    assert(sizeof(struct fs_dirinfo) == 25);  // check struct aligment
    assert(lastread_block == -1);

    struct fs_dirinfo di;
    struct fs_dirheader dhead;

    memset(&di, 0, sizeof di);
    di.ppath = (char*)_dirname;
    di.retval = (uint32_t)&dhead;
    int rc = sf_file(1, &di);  // read dir size
    if(rc) {
        fprintf(stderr, "Error reading dir size %s\n", _dirname);
        errno = rc;
        return NULL;
    }
    lastread_dir_size = dhead.totl_blocks;

    lastread_dir.entry.d_namlen = strlen(_dirname);
    assert (lastread_dir.entry.d_namlen < sizeof(lastread_dir.entry.d_name));
    strcpy(lastread_dir.entry.d_name, _dirname);

    return &lastread_dir;
};


int closedir(DIR *dirp)
{
    assert (lastread_block != -1);  // was opened

    if (!dirp || lastread_block == -1)
    {
        errno = EBADF;
        return -1;
    }
    lastread_block = -1;
    lastread_dir_size = 0;
    lastread_dir.entry.d_namlen = 0;
    lastread_dir.entry.d_name[0] = '\0';

    return 0;
};


struct dirent* readdir(DIR *dirp)
{
    assert (lastread_block != -1);  // was opened

    if (!dirp || lastread_block == -1)
    {
        errno = EBADF;
        return NULL;
    }
    struct fs_dirinfo di;

    assert (lastread_block != -1);  // was opened

    char retdir[sizeof(struct fs_dirheader) + sizeof(struct fsBDFE)];   // 1 block w/cp866 encoding
    struct fsBDFE *bdfe = (struct fsBDFE *)(retdir + sizeof(struct fs_dirheader));
    memset(&di, 0, sizeof di);
    di.ppath = dirp->entry.d_name;
    di.retval = (uint32_t)retdir;
    di.start = lastread_block;
    di.size = 1;

    int rc = sf_file(1, &di);  // read dir
    if(rc) {
        fprintf(stderr, "Error %d reading dir item %s\n", rc, dirp->entry.d_name);
        errno = rc;
        return NULL;
    }

    static struct dirent ent;
    ent.d_namlen = strlen(bdfe->fname);
    assert (ent.d_namlen < sizeof(ent.d_name));
    strcpy(ent.d_name, bdfe->fname);
    lastread_block++;

    return &ent;
};


void rewinddir(DIR *dirp)
{
    if (!dirp || lastread_block == -1)
    {
        return;
    }

    lastread_block = 0;
}


int	mkdir(const char *_path, mode_t m)
{
    char   namebuffer[1050]; // need for save data after di!!!
    struct fs_dirinfo *di = (struct fs_dirinfo *)namebuffer;

//debug_board_printf("mkdir start (%s)\n", _path);
    memset(di, 0, sizeof(struct fs_dirinfo));
    //di.ppath = (char*)_path;  // dont work with 70.9
    strcpy(di->path, _path);

    int rc = sf_file(9, di);  // creat dir
    if(rc) {
        fprintf(stderr, "Error %d creating dir item %s\n", rc, _path);
        errno = rc;
        return -1;
    }

//debug_board_printf("mkdir end (%s)\n", _path);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
void __attribute__ ((noinline)) debug_board_write_str(const char* str){
  while(*str)
    debug_board_write_byte(*str++);
}

void __attribute__ ((noinline)) debug_board_printf(const char *format,...)
{
        va_list ap;
        char log_board[300];

        va_start (ap, format);
        vsnprintf(log_board, sizeof log_board, format, ap);
        va_end(ap);
        debug_board_write_str(log_board);
}

__attribute__ ((noinline)) void trap(int n)
{
    // nothing todo, just see n in debugger. use "bp trap" command
    __asm__ __volatile__(
    "nop"
    :
    :"a"(n));
}



/* tested example
void* read_folderdata(char* name)
{
    struct fs_dirinfo di;
    struct fs_dirheader dhead;
    assert(sizeof di == 25);

    memset(&di, 0, sizeof di);
    di.ppath = name;
    di.retval = (uint32_t)&dhead;
    int rc = sf_file(1, &di);  // read dir size
    if(rc) {
        debug_board_printf("Error reading dir size %s", name);
        exit(1);
    }
    di.size = dhead.totl_blocks;

    char *retdir = malloc(sizeof dhead + dhead.totl_blocks * sizeof(struct fsBDFE));
    if(!retdir) {
        debug_board_printf("No memory for dir %s", name);
        exit(1);
    }
    di.retval = (uint32_t)retdir;
    rc = sf_file(1, &di);  // read dir
    if(rc) {
        debug_board_printf("Error 2 reading dir size %s", name);
        exit(1);
    }

    // manual clear mark flag (random junk in fname free space)
    int i;
    for (i = 0; i < dhead.totl_blocks; i++)
        ((struct fsBDFE*)(retdir+32))[i].fname[259] = 0;

    debug_board_printf("Loaded dir [%s] etnries %d,\n first file [%s]\n", name, ((struct fs_dirheader*)(retdir))->curn_blocks, ((struct fsBDFE*)(retdir+32))->fname);

    return retdir;
}
*/

// while not in newlib
int set_fileinfo(const char *path, fileinfo_t *info)
{
    int retval;

    __asm__ __volatile__ (
    "pushl $0 \n\t"
    "pushl $0 \n\t"
    "movl %1, 1(%%esp) \n\t"
    "pushl %%ebx \n\t"
    "pushl $0 \n\t"
    "pushl $0 \n\t"
    "pushl $0 \n\t"
    "pushl $6 \n\t"
    "movl %%esp, %%ebx \n\t"
    "movl $70, %%eax \n\t"
    "int $0x40 \n\t"
    "addl $28, %%esp \n\t"
    :"=a" (retval)
    :"r" (path), "b" (info));
   return retval;
};

