#include <stdint.h>
#include <string.h>

void *memrchr(const void *m, int c, size_t n)
{
    const unsigned char *s = (const unsigned char*)m;
    c = (unsigned char)c;
    while (n--) if (s[n]==c) return (void *)(s+n);
    return 0;
}

void setcwd(char* path){
    __asm__ __volatile__(
        "int $0x40"
        ::"a"(30), "b"(1), "c"(path)
        :"memory"
    );
}

char *dirname(char *path)
{
    static const char dot[] = ".";
    char *last_slash;
    last_slash = path != NULL ? strrchr (path, '/') : NULL;
    if (last_slash != NULL && last_slash != path && last_slash[1] == '\0')
    {
      char *runp;
      for (runp = last_slash; runp != path; --runp)
        if (runp[-1] != '/')
          break;
      if (runp != path)
        last_slash = (char*)memrchr((void*)path, '/', runp - path);
    }
    if (last_slash != NULL)
    {
      char *runp;
      for (runp = last_slash; runp != path; --runp)
        if (runp[-1] != '/')
          break;
      if (runp == path)
        {
          if (last_slash == path + 1)
            ++last_slash;
          else
            last_slash = path + 1;
        }
      else
        last_slash = runp;
      last_slash[0] = '\0';
    }
    else
        path = (char *) dot;
    return path;
}

#pragma pack(push,1)
typedef struct{
    unsigned            p00;
    union{
        uint64_t        p04; 
        struct {
            unsigned    p04dw;
            unsigned    p08dw;
        };
    };
    unsigned            p12;
    union {
        unsigned        p16;
        const char     *new_name;
        void           *bdfe;
        void           *buf16;
        const void     *cbuf16;
    };
    char                p20;
    const char         *p21;
}ksys70_t;


int kos_mkdir(const char *path, unsigned v)
{
    int status;
    ksys70_t dir_opt;
    dir_opt.p00 = 9;
    dir_opt.p21 = path;
    __asm__ __volatile__(
        "int $0x40"
        :"=a"(status)
        :"a"(70), "b"(&dir_opt)
        :"memory"
    );
    return status;
}