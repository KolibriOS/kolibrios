
/// ===========================================================

int file_check(char file[]);
int dir_check(char dir[]);
void dir_truncate(char dir[]);
int iswhite(char c);
void trim(char string[]);
void execute();
void __cdecl kol_main();

int executable_run(char cmd[], char args[]);

void command_execute();
void command_get();
int command_get_cmd(char cmd[]);

int script_check(char file[]);
int script_run(char exec[], char args[]);

int aliases_check(char alias[]);
int alias_search(char alias[]);
int alias_add(char alias[]);
int alias_split (char alias[], char s1[], char s2[]);
void alias_list();

/// ===========================================================
