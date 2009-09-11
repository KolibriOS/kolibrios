
typedef char *va_list;
#define _roundsize(n)    ( (sizeof(n) + 3) & ~3 )
#define va_start(ap,v) (ap = (va_list)&v+_roundsize(v))
#define va_arg(ap,t)    ( *(t *)((ap += _roundsize(t)) - _roundsize(t)) )
#define va_end(ap) (ap = (va_list)0)


