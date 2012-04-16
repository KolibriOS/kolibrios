
#include "all.h"

/// ===========================================================

int dir_check(char dir[])
{
kol_struct70	k70;
int		result;

k70.p00 = 1;
k70.p04 = 0;
k70.p08 = 0;
k70.p12 = 2*1024*1024; // 2 MB
k70.p16 = malloc(2*1024*1024);
k70.p20 = 0;
k70.p21 = dir;

result = kol_file_70(&k70);

free(k70.p16);

if ( (0 == result)||(6 == result) )
	return TRUE;
else 
	return FALSE;

}

/// ===========================================================

void dir_truncate(char dir[])
{
int i;
i = strlen(dir)-1;
for (;;i--)
	if ('/' == dir[i])
		{
		dir[i+1] = 0;
		break;
		}
}

/// ===========================================================

int file_check(char file[])
{
kol_struct70	k70;
int		result;

k70.p00 = 0;
k70.p04 = 0;
k70.p08 = 0;
k70.p12 = 0;
k70.p16 = 0;
k70.p20 = 0;
k70.p21 = file;

result = kol_file_70(&k70);

if (0 == result)
	return TRUE;
else 
	return FALSE;
}

/// ===========================================================

void file_not_found(char file[])
{
#if LANG_ENG
	printf ("  File '%s' not found.\n\r", file);
#elif LANG_RUS
	printf ("  Файл '%s' не найден.\n\r", file);
#endif
}

/// ===========================================================

int iswhite(char c)
{
return ((' ' == c) || ('\t' == c) || (13 == c) || (10 == c));
}

/// ===========================================================

void trim(char string[])
{
int i, j;

for (i=0; ;i++)
	if ( !iswhite(string[i]) )
		break;
j = 0;
for (;;i++, j++)
	{
	string[j] = string[i];
	if ('\0' == string[i] )
		break;
	}

for (i=0; ;i++)
	if ('\0' == string[i])
		break;
i--;
for (;i>0;--i)
	if ( iswhite(string[i]) )
		string[i] = '\0';
	else
		break;
}

/// ===========================================================

void kol_main()
{

NUM_OF_CMD = sizeof(COMMANDS)/sizeof(COMMANDS[0]);

strcpy(title, "SHELL ");
strcat(title, SHELL_VERSION);
CONSOLE_INIT(title);

strcpy(cur_dir, PATH);
dir_truncate(cur_dir);

con_set_cursor_height(con_get_font_height()-1);

ALIASES = malloc(128*1024);

if (strlen(PARAM) > 0)
	strcpy(CMD, PARAM);
else 
	strcpy(CMD, ".shell");

command_execute();

for (;;)
	{
	printf ("# ");
	command_get();
	command_execute();
	}

_exit(0);
kol_exit();
}

/// ===========================================================
