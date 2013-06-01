#include "utils/testament.h"

const char * const netsurf_version = "3.0 (Dev"
#if defined(CI_BUILD)
	" CI #" CI_BUILD
#endif
	")"
	;
const int netsurf_version_major = 3;
const int netsurf_version_minor = 0;
