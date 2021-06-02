/* <dirent.h> includes <sys/dirent.h>, which is this file.  On a
   system which supports <dirent.h>, this file is overridden by
   dirent.h in the libc/sys/.../sys directory.  On a system which does
   not support <dirent.h>, we will get this file which uses #error to force
   an error.  */

#ifdef __cplusplus
extern "C" {
#endif

#define DT_DIR 16
#define DT_REG 0

#include <limits.h>
#include <sys/types.h>  
 
struct dirent{
   ino_t    d_ino;
   unsigned d_type;
   char     d_name[256];
};
 
typedef struct{
    struct dirent* objs;
    ino_t pos;
    ino_t num_objs;  
}DIR;
 
extern int  closedir(DIR *dir);
extern DIR* opendir(const char *path);
extern struct dirent* readdir(DIR *);
extern void rewinddir(DIR *dir);
extern void seekdir(DIR *dir, unsigned pos);
extern unsigned telldir(DIR *dir);

extern int scandir(const char *path, struct dirent ***res, int (*sel)(const struct dirent *), int (*cmp)(const struct dirent **, const struct dirent **));
extern int alphasort(const struct dirent **a, const struct dirent **b);

#ifdef __cplusplus
}
#endif
