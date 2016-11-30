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

#define DIR_END       '/'
#define NO_STRNICMP
#define STRNICMP zstrnicmp
#define NO_CHMOD
#define NO_FCHOWN
//#define SET_DIR_ATTRIB   internal unzip bug

#define echoff(f)
#define echon()
#define getch() getchar() /* not correct, but may not be on a console */
#define HAVE_WORKING_GETCH

/* next line turn on full unicode utf-8 support */
//#define UNICODE_SUPPORT
#ifdef UNICODE_SUPPORT
#   define UTF8_MAYBE_NATIVE
#   define NO_NL_LANGINFO
#else /* cp866 is native */
#   define CRTL_CP_IS_OEM
#endif // UNICODE_SUPPORT

/*
#  ifdef CRTL_CP_IS_OEM
#   define ISO_TO_INTERN(src, dst)  AnsiToOem(src, dst)
#   define OEM_TO_INTERN(src, dst)  {if ((src) != (dst)) strcpy((dst), (src));}
#   define INTERN_TO_ISO(src, dst)  OemToAnsi(src, dst)
#   define INTERN_TO_OEM(src, dst)  {if ((src) != (dst)) strcpy((dst), (src));}
#  endif
#  define _OEM_INTERN(str1) OEM_TO_INTERN(str1, str1)
#  define _ISO_INTERN(str1) ISO_TO_INTERN(str1, str1)
*/
/* UzpPassword supplies ANSI-coded string regardless of C RTL's native CP */
// remove for a while
// #  define STR_TO_CP2(dst, src)  (AnsiToOem(src, dst), dst)
   /* dummy defines to disable these functions, they are not needed */
#  define STR_TO_ISO
#  define STR_TO_OEM


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
