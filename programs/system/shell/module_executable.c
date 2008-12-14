
/// ===========================================================

int executable_run(char cmd[], char args[])
{

char		exec[256];
char		error_starting[]={"  No such command '%s'.\n\r"};
int		result;

if ( '/' == cmd[0]) // если путь абсолбтный
	{
	strcpy(exec, cmd);

	if (  !file_check(exec) ) // проверяем существование файла
		{
		printf(error_starting, cmd);
		return FALSE;
		}
	}

else 
	{
	strcpy(exec, cur_dir); // проверяем файл в текущем каталоге
	strcat(exec, cmd);
	
	if ( !file_check(exec) ) // проверяем существование файла
		{
		strcpy(exec, "/rd/1/"); // проверяем файл на виртуальном диске
		strcat(exec, cmd);
			if ( !file_check(exec) ) // проверяем существование файла
				{
				printf(error_starting, cmd);
				return FALSE;
				}
		}
	}


if ( script_check(exec) )
	{
	return script_run(exec, args);
	}

/* запуск программы */
result = program_run(exec, args);
if (result > 0)
	{
	printf ("  '%s' started. PID = %d\n\r", cmd, result);
	return TRUE;
	}
else	
	{
	printf(error_starting, cmd);
	return FALSE;
	}

}

/// ===========================================================
