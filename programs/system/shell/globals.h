
#define FALSE 0
#define TRUE 1

#define SHELL_VERSION "0.4"

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

const NUM_OF_CMD = 19;

const char HELP_COMMANDS[][10]=
{
"about",
"alias",
"cd",
"date",
"echo",
"exit",
"free",
"help",
"kill",
"ls",
"mkdir",
"more",
"ps",
"pwd",
"reboot",
"rm",
"rmdir",
"touch",
"ver"
};

const char HELP_DESC [][70]=
{
"  Displays information about the program\n\r",
"  Allows the user view the current aliases\n\r",
"  Changes directories\n\r",
"  Returns the date and time\n\r",
"  Echoes the data to the screen\n\r",
"  Exits program\n\r",
"  Displays total, free and used memory\n\r",
"  Gives help\n\r",
"  Stops a running process\n\r",
"  Lists the files in a directory\n\r",
"  Makes directory\n\r",
"  Displays a data file to the screen\n\r",
"  Lists the current processes running\n\r",
"  Displays the name of the working directory\n\r",
"  Reboots the computer\n\r",
"  Removes files\n\r",
"  Removes directories\n\r",
"  Creates an empty file or updates the time/date stamp on a file\n\r",
"  Displays version\n\r"
};

/// ===========================================================
