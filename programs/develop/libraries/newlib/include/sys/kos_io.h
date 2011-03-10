
#ifndef __KOS_IO_H__
#define __KOS_IO_H__

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
} fileinfo_t;

#pragma pack(pop)

int create_file(const char *path);
int get_fileinfo(const char *path, fileinfo_t *info);
int read_file(const char *path, void *buff,
               size_t offset, size_t count, size_t *reads);
int write_file(const char *path,const void *buff,
               size_t offset, size_t count, size_t *writes);
int set_file_size(const char *path, unsigned size);
void *load_file(const char *path, size_t *len);
void __stdcall unpack(void* packed_data, void* unpacked_data);

static inline int user_free(void *mem)
{
    int  val;
    __asm__ __volatile__(
    "int $0x40"
    :"=eax"(val)
    :"a"(68),"b"(12),"c"(mem));
    return val;
}

static inline void set_cwd(const char* cwd)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(30),"b"(1),"c"(cwd));
};

#endif
