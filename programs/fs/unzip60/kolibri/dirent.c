/*
Kolibri OS port for gcc 5.4

Started by Siemargl @Nov 2016

Contains realisation of directory handling functions:
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
        fprintf(stderr, "Error %d reading dir item %s", rc, dirp->entry.d_name);
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
    struct fs_dirinfo di;
    memset(&di, 0, sizeof di);
    di.ppath = (char*)_path;

    int rc = sf_file(9, &di);  // creat dir
    if(rc) {
        fprintf(stderr, "Error %d creating dir item %s", rc, _path);
        errno = rc;
        return -1;
    }

    return 0;
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
