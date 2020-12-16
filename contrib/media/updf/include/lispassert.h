
#ifndef __lispassert_h__
#define __lispassert_h__


#include "choices.h"

#ifdef USE_ASSERT
  #include <assert.h>
  #define LISPASSERT(x)  assert(x)
#else
  #define LISPASSERT(x)
#endif

#endif

