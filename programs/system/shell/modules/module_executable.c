
/// ===========================================================

int executable_run(char cmd[], char args[])
{

char		exec[256];
#if LANG_ENG
	char		error_starting[]={"  No such command '%s'.\n\r"};
#elif LANG_RUS
	char		error_starting[]={"  Нет такой команды - '%s'.\n\r"};
#endif
int		result;

if ( '/' == cmd[0]) // если путь абсолютный
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
//	strcat(exec, "/"); // add slash
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
	return script_run(exec, args);

/* запуск программы */
result = program_run(exec, args);
if (result > 0)
	{
	
	if ( !program_console(result)  )
		{
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
	printf(error_starting, cmd);
	return FALSE;
	}

}

/// ===========================================================
