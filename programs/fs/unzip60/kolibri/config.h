/*
Kolibri OS config for gcc 5.4

Started by Siemargl @Nov 2016
*/

#include <sys/types.h>      /* off_t, time_t, dev_t, ... */
#include <sys/stat.h>
#include <sys/kos_io.h>             /* lseek(), open(), setftime(), dup(), creat() */
#include <time.h>           /* localtime() */
#include <fcntl.h>          /* O_BINARY for open() w/o CR/LF translation */
#include <sys/time.h>
#include <unistd.h>
#include <dirent.h>


#define DEBUG

#define DIR_END       '/'
#define NO_STRNICMP
#define STRNICMP zstrnicmp
#define NO_CHMOD
#define NO_FCHOWN

#define echoff(f)
#define echon()
#define getch() getchar() /* not correct, but may not be on a console */
#define HAVE_WORKING_GETCH

/*
#  ifdef DATE_FORMAT
#    undef DATE_FORMAT
#  endif
#  define DATE_FORMAT     dateformat()
*/
#define lenEOL          2
#define PutNativeEOL    {*q++ = native(CR); *q++ = native(LF);}
/*
#  if (!defined(NO_EF_UT_TIME) && !defined(USE_EF_UT_TIME))
#    define USE_EF_UT_TIME
#  endif
*/

/* Static variables that we have to add to Uz_Globs: */
#define SYSTEM_SPECIFIC_GLOBALS \
    int created_dir, renamed_fullpath;\
    char *rootpath, *buildpath, *end;\
    ZCONST char *wildname;\
    char *dirname, matchname[FILNAMSIZ];\
    int rootlen, have_dirname, dirnamelen, notfirstcall;\
    zvoid *wild_dir;
