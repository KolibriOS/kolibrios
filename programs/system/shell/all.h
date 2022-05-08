
/// ===========================================================

#ifndef AUTOBUILD
// autobuild does not create lang.h, but defines LANG_{RUS,ENG} directly
#include "lang.h"
#endif

#include <stdio.h> // Added by Coldy (this should be right here)

#include "system/boolean.h"
#include "system/kolibri.h"
//#include "system/stdlib.h"
//#include "system/string.h"
//#include "system/ctype.h"
//#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/ksys.h>
//--------
int    strnicmp(const char* string1, const char* string2, unsigned count)
{
int pc = 0;
while (1)
	{
	if (toupper(*string1)<toupper(*string2))
		return -1;
	if (toupper(*string1)>toupper(*string2))
		return 1;

	if (*string1=='\0' || pc == count)
		return 0;

	string1++;
	string2++;
	pc++;
	}
}
//--------

#include "globals.h"
#include "prototypes.h"

// from main file (shell.c). TODO - in future move to library
void get_file_dir_loc(char *filepath, char *dir_path);

//#include "system/console.c"
#include <conio.h>
#define con_exit              (*con_exit)
#define con_set_title         (*con_set_title)
#define con_write_asciiz      (*con_write_asciiz)
#define con_write_string      (*con_write_string)
#define con_printf            (*con_printf)
#define con_get_flags         (*con_get_flags)
#define con_set_flags         (*con_set_flags)
#define con_get_font_height   (*con_get_font_height)
#define con_get_cursor_height (*con_get_cursor_height)
#define con_set_cursor_height (*con_set_cursor_height)
#define con_getch             (*con_getch)
#define con_getch2            (*con_getch2)
#define con_kbhit             (*con_kbhit)
#define con_gets              (*con_gets)
#define con_gets2_callback    (* con_gets2_callback)
#define con_gets2             (*con_gets2)
#define con_cls               (*con_cls)
#define con_get_cursor_pos    (*con_get_cursor_pos)
#define con_set_cursor_pos    (*con_set_cursor_pos)

#define printf con_printf
#define gets con_gets
#define getch con_getch2

#include "cmd/cmd_about.c"
#include "cmd/cmd_help.c"
#include "cmd/cmd_ver.c"
#include "cmd/cmd_pwd.c"
#include "cmd/cmd_ls.c"
#include "cmd/cmd_lsmod.c"
#include "cmd/cmd_ps.c"
#include "cmd/cmd_kill.c"
#include "cmd/cmd_pkill.c"
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
#include "cmd/cmd_kfetch.c"
#include "cmd/cmd_cp.c"
#include "cmd/cmd_mv.c"
#include "cmd/cmd_ren.c"
#include "cmd/cmd_waitfor.c"

#include "modules/module_command.c"
#include "modules/module_program_console.c"
#include "modules/module_program.c"
#include "modules/module_script.c"
#include "modules/module_executable.c"
#include "modules/module_alias.c"
#include "modules/module_parameters.c"

//typedef unsigned int size_t;

