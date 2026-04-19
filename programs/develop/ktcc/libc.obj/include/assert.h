#ifndef _ASSERT_H_
#define _ASSERT_H_

#include <stddef.h>
#include <stdlib.h>

#ifdef NDEBUG
#define assert(x) (void)0
#else
#define assert(x) ((void)((x) || (__assert_fail(#x, __FILE__, __LINE__, __func__), 0)))
#endif

#endif // _ASSERT_H_
