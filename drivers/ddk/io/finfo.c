
#pragma pack(push, 1)
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


int get_fileinfo(const char *path,FILEINFO *info)
{
   int retval;
   int tmp;

   asm __volatile__
      (
       "pushl $0 \n\t"
       "pushl $0 \n\t"
       "movl %2, 1(%%esp) \n\t"
       "pushl %%ebx \n\t"
       "pushl $0 \n\t"
       "pushl $0 \n\t"
       "pushl $0 \n\t"
       "pushl $5 \n\t"
       "movl %%esp, %%ebx \n\t"
       "movl $70, %%eax \n\t"
       "int $0x40 \n\t"
       "addl $28, %%esp \n\t"
       :"=a" (retval),"=b"(tmp)
       :"r" (path), "b" (info)
       );
   return retval;
};
