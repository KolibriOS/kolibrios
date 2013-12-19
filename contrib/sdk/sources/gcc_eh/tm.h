#ifndef GCC_TM_H
#define GCC_TM_H
#ifdef IN_GCC
# include "options.h"
# include "config/vxworks-dummy.h"
# include "config/i386/i386.h"
# include "config/i386/unix.h"
# include "config/i386/bsd.h"
# include "config/i386/gas.h"
# include "config/dbxcoff.h"
# include "config/i386/cygming.h"
# include "config/i386/mingw32.h"
# include "config/i386/mingw-stdint.h"
# include "config/tm-dwarf2.h"
# include "defaults.h"
#endif
#if defined IN_GCC && !defined GENERATOR_FILE && !defined USED_FOR_TARGET
# include "insn-constants.h"
# include "insn-flags.h"
#endif
#endif /* GCC_TM_H */
