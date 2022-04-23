
#define SHELL_VERSION "0.8.3"

char	cmdline[256];

char    tmpstr[64];

char		title[64];
char		cur_dir[FILENAME_MAX];

/// ===========================================================

char		*ALIASES = NULL;
unsigned	ALIAS_NUM = 0;

/// ===========================================================

#define CMD_HISTORY_NUM 11

char		CMD[FILENAME_MAX * 2];
char		CMD_HISTORY[CMD_HISTORY_NUM][FILENAME_MAX * 2];
char		CMD_NUM = 0;
char		CMD_HISTORY_NUM_REAL = 0;
unsigned 	LAST_PID = 0;

/// ===========================================================

char script_sign[] = {"#SHS"};

/// ===========================================================

int NUM_OF_CMD;

/// ===========================================================

typedef struct
{
	const char* name;
	const char* help;
	const void* handler;
} command_t;

/// ===========================================================

int cmd_about(char arg[]);
int cmd_alias(char arg[]);
int cmd_cd(char dir[]);
int cmd_clear(char arg[]);
int cmd_date(char arg[]);
int cmd_echo(char text[]);
int cmd_exit(char arg[]);
int cmd_memory(char arg[]);
int cmd_help(char cmd[]);
int cmd_kill(char process[]);
int cmd_pkill(char process_name[]);
int cmd_ls(char dir[]);
int cmd_lsmod(char param[]);
int cmd_mkdir(char dir[]);
int cmd_more(char file[]);
int cmd_ps(char arg[]);
int cmd_pwd(char arg[]);
int cmd_reboot(char arg[]);
int cmd_rm(char file[]);
int cmd_rmdir(char dir[]);
int cmd_touch(char file[]);
int cmd_ver(char arg[]);
int cmd_sleep(char arg[]);
int cmd_shutdown(char arg[]);
int cmd_uptime(char param[]);
int cmd_killall(char process_name[]);
int cmd_history(char arg[]);
int cmd_kfetch(char param[]);
int cmd_cp(char param[]);
int cmd_mv(char param[]);
int cmd_ren(char param[]);
int cmd_waitfor(char param[]);

/// ===========================================================

#if LANG_ENG
	#include "locale/eng/globals.h"
#elif LANG_RUS
	#include "locale/rus/globals.h"
#endif

/// ===========================================================


