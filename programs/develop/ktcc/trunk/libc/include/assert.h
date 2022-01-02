#ifndef __ASSERT_H
#define __ASSERT_H

#ifdef NDEBUG           
# define assert(a) (void)0
#else
# define assert(a) ((a) ? (void)0 : __assert_func (__FILE__, __LINE__, #a))
#endif

#ifdef NDEBUG           
# define TRACE(s) void(0)
#else
# define TRACE(s) (__trace_func (__FILE__, __LINE__, #s))
#endif

void __assert_func(char*,int,char*);
void __trace_func(char*,int,char*);

#endif