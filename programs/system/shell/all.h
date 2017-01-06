
/// ===========================================================

#ifndef AUTOBUILD
// autobuild does not create lang.h, but defines LANG_{RUS,ENG} directly
#include "lang.h"
#endif

#include "system/boolean.h"
#include "system/kolibri.h"
#include "system/stdlib.h"
#include "system/string.h"
#include "system/ctype.h"

#include "globals.h"
#include "prototypes.h"

#include "system/console.c"

#include "cmd/cmd_about.c"
#include "cmd/cmd_help.c"
#include "cmd/cmd_ver.c"
#include "cmd/cmd_pwd.c"
#include "cmd/cmd_ls.c"
#include "cmd/cmd_ps.c"
#include "cmd/cmd_kill.c"
#include "cmd/cmd_echo.c"
#include "cmd/cmd_date.c"
#include "cmd/cmd_exit.c"
#include "cmd/cmd_cd.c"
#include "cmd/cmd_free.c"
#include "cmd/cmd_reboot.c"
#include "cmd/cmd_mkdir.c"
#include "cmd/cmd_rmdir.c"
#include "cmd/cmd_rm.c"
#include "cmd/cmd_touch.c"
#include "cmd/cmd_alias.c"
#include "cmd/cmd_more.c"
#include "cmd/cmd_clear.c"
#include "cmd/cmd_sleep.c"
#include "cmd/cmd_shutdown.c"
#include "cmd/cmd_uptime.c"
#include "cmd/cmd_history.c"
#include "cmd/cmd_cp.c"
#include "cmd/cmd_waitfor.c"

#include "modules/module_command.c"
#include "modules/module_program_console.c"
#include "modules/module_program.c"
#include "modules/module_script.c"
#include "modules/module_executable.c"
#include "modules/module_alias.c"
#include "modules/module_parameters.c"

/// ===========================================================
