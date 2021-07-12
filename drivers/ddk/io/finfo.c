#include <syscall.h>

#pragma pack(push,1)
typedef struct
{
  char sec;
  char min;
  char hour;
  char rsv;
}detime_t;

typedef struct
{
  char  day;
  char  month;
  short year;
}dedate_t;

typedef struct
{
  unsigned    attr;
  unsigned    flags;
  union
  {
     detime_t  ctime;
     unsigned  cr_time;
  };
  union
  {
     dedate_t  cdate;
     unsigned  cr_date;
  };
  union
  {
     detime_t  atime;
     unsigned  acc_time;
  };
  union
  {
     dedate_t  adate;
     unsigned  acc_date;
  };
  union
  {
     detime_t  mtime;
     unsigned  mod_time;
  };
  union
  {
     dedate_t  mdate;
     unsigned  mod_date;
  };
  unsigned    size;
  unsigned    size_high;
} FILEINFO;
#pragma pack(pop)

int get_fileinfo(const char *path, FILEINFO *info)
{
   ksys70_t k;
   int err;
   k.p00 = 5;
   k.bdfe = info;
   k.p20 = 0;
   k.p21 = path;
   return FS_Service(&k, err);
}

