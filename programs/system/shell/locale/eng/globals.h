
const command_t COMMANDS[]=
{
	{"about",   "  Displays information about Shell\n\r", &cmd_about},
	{"alias",   "  Allows the user view the current aliases\n\r", &cmd_alias},
	{"cd",      "  Changes current directory. Usage:\n\r    cd <directory name>\n\r", &cmd_cd},
	{"clear",   "  Clears the screen\n\r", &cmd_clear},
	{"cp",      "  Copies file\n\r", &cmd_cp},
	{"mv",      "  Moves file\n\r", &cmd_mv},
	{"ren",     "  Renames file\n\r", &cmd_ren},
	{"date",    "  Returns the current date and time\n\r", &cmd_date},
	{"echo",    "  Echoes the data to the screen. Usage:\n\r    echo <data>\n\r", &cmd_echo},
	{"exit",    "  Exits from Shell\n\r", &cmd_exit},
	{"free",    "  Displays total, free and used memory\n\r", &cmd_memory},
	{"help",    "  Gives help on commands. Usage:\n\r    help ;it lists all builtins\n\r    help <command> ;help on command\n\r", &cmd_help},
	{"history", "  Lists used commands\n\r", &cmd_history},	
	{"kfetch",  "  Prints logo and information about system.\n\r", &cmd_kfetch},
	{"kill",    "  Stops a running process. Usage:\n\r    kill <PID of process>\n\r    kill all\n\r", &cmd_kill},
	{"pkill",   "  Kills all processes by name. Usage:\n\r    pkill <process_name>\n\r", &cmd_pkill},
	{"ls",      "  Lists the files in a directory. Usage:\n\r    ls ;lists the files in current directory\n\r    ls <directory> ;lists the files at specified folder\n\r    ls -1 ;lists the files in a single column\n\r", &cmd_ls},
	{"lsmod",	"  list working driver \n\r", &cmd_lsmod},
	{"mkdir",   "  Create directory and parent directories as needed. Usage:\n\r    mkdir <folder/name>\n\r", &cmd_mkdir},
	{"more",    "  Displays a file data to the screen. Usage:\n\r    more <file name>\n\r", &cmd_more},
	{"ps",      "  Lists the current processes running\n\r  or shows more info on <procname> and save LASTPID\n\r", &cmd_ps},
	{"pwd",     "  Displays the name of the working directory\n\r", &cmd_pwd},
	{"reboot",  "  Reboots the computer or KolibriOS kernel. Usage:\n\r    reboot ;reboot a PC\n\r    reboot kernel ;reboot the KolibriOS kernel\n\r", &cmd_reboot},
	{"rm",      "  Removes a file. Usage:\n\r    rm file name>\n\r", &cmd_rm},
	{"rmdir",   "  Removes a folder. Usage:\n\r    rmdir <directory>\n\r", &cmd_rmdir},
	{"shutdown","  Turns off the computer\n\r", &cmd_shutdown},
	{"sleep",   "  Stops the shell for the desired period. Usage:\n\r    sleep <time in the 1/100 of second>\n\r  Example:\n\r    sleep 500 ;pause for 5sec.\n\r", &cmd_sleep},
	{"touch",   "  Creates an empty file or updates the time/date stamp on a file. Usage:\n\r    touch <file name>\n\r", &cmd_touch},
	{"uptime",  "  Displays the uptime\n\r", &cmd_uptime},
	{"ver",     "  Displays version. Usage:\n\r    ver ;Shell version\n\r    ver kernel ;version of KolibriOS kernel\n\r    ver cpu ;information about CPU\n\r", &cmd_ver},
	{"waitfor", "  Stops console waiting while process finish. Usage:\n\r    waitfor ;waiting previous started executable LASTPID\n\r    waitfor <PID>;awaiting PID finish\n\r", &cmd_waitfor},
};

#define CMD_ABOUT_MSG "Shell %s\n\r"
#define CMD_CD_USAGE "  cd <directory>\n\r"
#define CMD_CP_USAGE "  cp <file_in> <file_out>\n\r"
#define CMD_DATE_DATE_FMT "  Date [dd.mm.yy]: %x%x.%x%x.%x%x"
#define CMD_DATE_TIME_FMT "\n\r  Time [hh:mm:ss]: %x%x:%x%x:%x%x\n\r"
#define CMD_FREE_FMT "  Total [kB / MB / %%]:  %-7d / %-5d / 100\n\r   Free [kB / MB / %%]:  %-7d / %-5d / %d\n\r   Used [kB / MB / %%]:  %-7d / %-5d / %d\n\r"
#define CMD_HELP_AVAIL "  %d commands available:\n\r"
#define CMD_HELP_CMD_NOT_FOUND "  Command \'%s\' not found.\n\r"

#define CMD_KILL_USAGE "  kill <PID>\n\r"
#define CMD_MKDIR_USAGE "  mkdir <directory>\n\r"
#define CMD_MORE_USAGE "  more <filename>\n\r"
#define CMD_MV_USAGE "  mv <file_in> <file_out>\n\r"

#define CMD_PKILL_HELP      "  pkill <process_name>\n\r"
#define CMD_PKILL_KILL      "  PID: %u - killed\n"
#define CMD_PKILL_NOT_KILL  "  PID: %u - not killed\n"
#define CMD_PKILL_NOT_FOUND "  No processes with this name were found!\n"

#define CMD_REN_USAGE "  ren <file> <new_name>\n\r"
#define CMD_RM_USAGE "  rm <filename>\n\r"
#define CMD_RMDIR_USAGE "  rmdir <directory>\n\r"
#define CMD_SLEEP_USAGE "    sleep <time in the 1/100 of second>\n\r"
#define CMD_TOUCH_USAGE "  touch <filename>\n\r"
#define CMD_UPTIME_FMT "  Uptime: %d day(s), %d:%d:%d.%d\n\r"
#define CMD_VER_FMT1 "  KolibriOS v%d.%d.%d.%d. Kernel SVN-rev.: %d\n\r"
#define CMD_WAITFOR_FMT "  Awaing finish PID %d\n\r"
#define EXEC_STARTED_FMT "  '%s' started. PID = %d\n\r"
#define EXEC_SCRIPT_ERROR_FMT "Error in '%s' : script must start with #SHS line\n\r"
#define UNKNOWN_CMD_ERROR "  Error!\n\r"
#define CON_APP_ERROR "  Error in console application.\n\r"
#define FILE_NOT_FOUND_ERROR "  File '%s' not found.\n\r"
