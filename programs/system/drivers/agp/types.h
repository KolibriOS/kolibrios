
#define  NULL (void*)0


typedef  unsigned char        u8_t;
typedef  unsigned short int   u16_t;
typedef  unsigned int         u32_t;
typedef  unsigned long long   u64_t;

typedef  unsigned int         addr_t;

typedef  unsigned int         size_t;
typedef  unsigned int         count_t;
typedef  unsigned int         eflags_t;

typedef  unsigned int Bool;

#define  TRUE  (Bool)1
#define  FALSE (Bool)0

#define min_t(type,x,y) \
	({ type __x = (x); type __y = (y); __x < __y ? __x: __y; })
#define max_t(type,x,y) \
	({ type __x = (x); type __y = (y); __x > __y ? __x: __y; })


