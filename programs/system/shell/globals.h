
#define SHELL_VERSION "0.4.5"

extern char	PATH[256];
extern char	PARAM[256];

char		title[64];
char		cur_dir[256];

/// ===========================================================

char		*ALIASES = NULL;
unsigned	ALIAS_NUM = 0;

/// ===========================================================

#define CMD_HISTORY_NUM 5

char		CMD[256];
char		CMD_HISTORY[CMD_HISTORY_NUM][256];
char		CMD_NUM;

unsigned	CMD_POS;

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
int cmd_ccpuid(char dir[]);
int cmd_cd(char dir[]);
int cmd_clear(char arg[]);
int cmd_date(char arg[]);
int cmd_echo(char text[]);
int cmd_exit(char arg[]);
int cmd_memory(char arg[]);
int cmd_help(char cmd[]);
int cmd_kill(char process[]);
int cmd_ls(char dir[]);
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
int cmd_turnoff(char arg[]);
int cmd_kerver(char arg[]);
int cmd_uptime(char param[]);

/// ===========================================================

#if LANG_ENG
	#include "locale/eng/globals.h"
#elif LANG_RUS
	#include "locale/rus/globals.h"
#endif

/// ===========================================================


