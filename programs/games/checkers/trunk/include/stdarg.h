#ifndef __STDARG_H
#define __STDARG_H

typedef void *va_list;

#define __size(x) ((sizeof(x)+sizeof(int)-1) & ~(sizeof(int)-1))

#define va_start(ap, parmN) ((void)((ap) = (va_list)((char *)(&parmN)+__size(parmN))))
#define va_arg(ap, type) (*(type *)(((*(char **)&(ap))+=__size(type))-(__size(type))))
#define va_end(ap)          ((void)0)


#endif  /* __STDARG_H */
