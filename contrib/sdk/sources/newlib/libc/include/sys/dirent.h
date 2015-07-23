/* <dirent.h> includes <sys/dirent.h>, which is this file.  On a
   system which supports <dirent.h>, this file is overridden by
   dirent.h in the libc/sys/.../sys directory.  On a system which does
   not support <dirent.h>, we will get this file which uses #error to force
   an error.  */

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif
