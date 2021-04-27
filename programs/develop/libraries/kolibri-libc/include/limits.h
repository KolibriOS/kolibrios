#ifndef _LIMITS_H_
#define _LIMITS_H_


#define INT_MAX       2147483647
#define UINT_MAX      (INT_MAX * 2U + 1)


#ifndef ARG_MAX
#define ARG_MAX         4096
#endif
 
#ifndef PATH_MAX
#define PATH_MAX        4096
#endif

#ifndef STDIO_MAX_MEM
#define STDIO_MAX_MEM 4096
#endif

#endif /* _LIMITS_H_ */