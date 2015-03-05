
#ifndef __mathcommands_h__
#define __mathcommands_h__

#include "yacasbase.h"
#include "lispenvironment.h"
#include "lispevalhash.h"
#include "lispobject.h"
#include "lispglobals.h"


//#define CORE_FUNCTION(NAME) void NAME(LispEnvironment& aEnvironment, LispPtr& aResult, LispPtr& aArguments);

#define CORE_KERNEL_FUNCTION(iname,fname,nrargs,flags) void fname(LispEnvironment& aEnvironment, LispInt aStackTop);
#define CORE_KERNEL_FUNCTION_ALIAS(iname,fname,nrargs,flags)  int JimDummyFunction();
#define OPERATOR(kind, prec, yacas_name) int JimDummyFunction();

#include "corefunctions.h"
#undef CORE_KERNEL_FUNCTION
#undef CORE_KERNEL_FUNCTION_ALIAS
#undef OPERATOR

#endif
