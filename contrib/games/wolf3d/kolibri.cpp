#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

#define asm_inline __asm__ __volatile__

#pragma pack(push,1)
typedef union{
    unsigned val;
    struct{
        short  x;
        short  y;
    };
}ksys_pos_t;

typedef union ksys_oskey_t{
    unsigned val;
    struct{
        unsigned char state;
        unsigned char code;
        unsigned char ctrl_key;
    };
}ksys_oskey_t;

typedef struct{
  unsigned     handle;
  unsigned     io_code;
  unsigned     *input;
  int          inp_size;
  void         *output;
  int          out_size;
}ksys_ioctl_t;

typedef struct{
    void *data;
    size_t size;
}ksys_ufile_t;


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

typedef struct {
  int cpu_usage;             //+0
  int window_pos_info;       //+4
  short int reserved1;       //+8
  char name[12];             //+10
  int memstart;              //+22
  int memused;               //+26
  int pid;                   //+30
  int winx_start;            //+34
  int winy_start;            //+38
  int winx_size;             //+42
  int winy_size;             //+46
  short int slot_info;       //+50
  short int reserved2;       //+52
  int clientx;               //+54
  int clienty;               //+58
  int clientwidth;           //+62
  int clientheight;          //+66
  unsigned char window_state;//+70
  char reserved3[1024-71];   //+71
}ksys_proc_table_t;

#pragma pack(pop)

static inline 
int _ksys_process_info(ksys_proc_table_t* table, int pid)
{
    int val;
    asm_inline(
        "int $0x40"
        :"=a"(val)
        :"a"(9), "b"(table), "c"(pid)
        :"memory"
    );
    return val;
}

static inline
void _ksys_change_window(int new_x, int new_y, int new_w, int new_h)
{
    asm_inline(
        "int $0x40"
        ::"a"(67), "b"(new_x), "c"(new_y), "d"(new_w),"S"(new_h)
    );
}

static inline
ksys_pos_t _ksys_screen_size()
{
	ksys_pos_t size;
    ksys_pos_t size_tmp;
    asm_inline(
        "int $0x40"
        :"=a"(size_tmp)
        :"a"(14)
    );
    size.x = size_tmp.y;
    size.y = size_tmp.x; 
    return size;
}

void *memrchr(const void *m, int c, size_t n)
{
	const unsigned char *s = (const unsigned char*)m;
	c = (unsigned char)c;
	while (n--) if (s[n]==c) return (void *)(s+n);
	return 0;
}

void kolibri_set_win_center()
{
    ksys_proc_table_t *info = (ksys_proc_table_t*)malloc(sizeof(ksys_proc_table_t));
    _ksys_process_info(info, -1);

    ksys_pos_t screen_size= _ksys_screen_size();
    int new_x = screen_size.x/2-info->winx_size/2;
    int new_y = screen_size.y/2-info->winy_size/2;
    _ksys_change_window(new_x, new_y, -1, -1); 
    free(info);
}

int mkdir(const char *path, unsigned v)
{
    int status;
    ksys70_t dir_opt;
    dir_opt.p00 = 9;
    dir_opt.p21 = path;
    asm_inline(
        "int $0x40"
        :"=a"(status)
        :"a"(70), "b"(&dir_opt)
        :"memory"
    );
    return status;
}

char *dirname (char *path)
{
  static const char dot[] = ".";
  char *last_slash;
  /* Find last '/'.  */
  last_slash = path != NULL ? strrchr (path, '/') : NULL;
  if (last_slash != NULL && last_slash != path && last_slash[1] == '\0')
    {
      /* Determine whether all remaining characters are slashes.  */
      char *runp;
      for (runp = last_slash; runp != path; --runp)
        if (runp[-1] != '/')
          break;
      /* The '/' is the last character, we have to look further.  */
      if (runp != path)
        last_slash = (char*)memrchr((void*)path, '/', runp - path);
    }
  if (last_slash != NULL)
    {
      /* Determine whether all remaining characters are slashes.  */
      char *runp;
      for (runp = last_slash; runp != path; --runp)
        if (runp[-1] != '/')
          break;
      /* Terminate the path.  */
      if (runp == path)
        {
          /* The last slash is the first character in the string.  We have to
             return "/".  As a special case we have to return "//" if there
             are exactly two slashes at the beginning of the string.  See
             XBD 4.10 Path Name Resolution for more information.  */
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
    /* This assignment is ill-designed but the XPG specs require to
       return a string containing "." in any case no directory part is
       found and so a static and constant string is required.  */
    path = (char *) dot;
  return path;
}

void setcwd(char* path){
    asm_inline(
        "int $0x40"
        ::"a"(30), "b"(1), "c"(path)
    );
}

