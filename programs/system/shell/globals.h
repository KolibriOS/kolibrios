
#define SHELL_VERSION "0.4.4"

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
int cmd_free(char arg[]);
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

/// ===========================================================

const command_t COMMANDS[]=
{
	{"about", "  Displays information about the program\n\r", &cmd_about},
	{"alias", "  Allows the user view the current aliases\n\r", &cmd_alias},
	{"ccpuid","  Displays CPU information\n\r", &cmd_ccpuid},
	{"cd",    "  Changes directories\n\r", &cmd_cd},
	{"clear", "  Clears the display\n\r", &cmd_clear},
	{"date",  "  Returns the date and time\n\r", &cmd_date},
	{"echo",  "  Echoes the data to the screen\n\r", &cmd_echo},
	{"exit",  "  Exits program\n\r", &cmd_exit},
	{"free",  "  Displays total, free and used memory\n\r", &cmd_free},
	{"help",  "  Gives help\n\r", &cmd_help},
	{"kill",  "  Stops a running process\n\r", &cmd_kill},
	{"ls",    "  Lists the files in a directory\n\r", &cmd_ls},
	{"mkdir", "  Makes directory\n\r", &cmd_mkdir},
	{"more",  "  Displays a data file to the screen\n\r", &cmd_more},
	{"ps",    "  Lists the current processes running\n\r", &cmd_ps},
	{"pwd",   "  Displays the name of the working directory\n\r", &cmd_pwd},
	{"reboot","  Reboots the computer\n\r", &cmd_reboot},
	{"rm",    "  Removes files\n\r", &cmd_rm},
	{"rmdir", "  Removes directories\n\r", &cmd_rmdir},
	{"sleep", "  Stops the shell for the desired period\n\r", &cmd_sleep},
	{"touch", "  Creates an empty file or updates the time/date stamp on a file\n\r", &cmd_touch},
	{"ver",   "  Displays version\n\r", &cmd_ver},
};

/// ===========================================================


