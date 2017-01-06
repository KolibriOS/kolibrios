
/// ===========================================================

int executable_run(char cmd[], char args[])
{

char	exec[FILENAME_MAX];
int		result;

if ( '/' == cmd[0]) // если путь абсолютный
	{
	strcpy(exec, cmd);

	if (  !file_check(exec) ) // проверяем существование файла
		{
		file_not_found(cmd);
		return FALSE;
		}
	}

else 
	{
	strcpy(exec, cur_dir); // проверяем файл в текущем каталоге
	if (exec[strlen(exec)-1] != '/') 
		strcat(exec, "/"); // add slash
	strcat(exec, cmd);
	
	if ( !file_check(exec) ) // проверяем существование файла
		{
		strcpy(exec, "/rd/1/"); // проверяем файл на виртуальном диске
		strcat(exec, cmd);
			if ( !file_check(exec) ) // проверяем существование файла
				{
				file_not_found(cmd);
				return FALSE;
				}
		}
	}


if ( script_check(exec) )
	return script_run(exec, args);

/* запуск программы */
result = program_run(exec, args);
if (result > 0)
	{
	
	if ( !program_console(result)  )
		{
			LAST_PID = result;
		#if LANG_ENG
			printf ("  '%s' started. PID = %d\n\r", cmd, result);
		#elif LANG_RUS
			printf ("  '%s' запущен. PID = %d\n\r", cmd, result);
		#endif
		}
	return TRUE;
	}
else	
	{
	file_not_found(cmd);
	return FALSE;
	}

}

/// ===========================================================

