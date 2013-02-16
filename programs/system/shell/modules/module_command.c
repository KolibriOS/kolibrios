
/// ===========================================================

void command_clear()
{
for (;CMD_POS;CMD_POS--)
	printf("%c %c", 8, 8);
CMD[0]='\0';
}

/// ===========================================================

void command_history_add(char command[])
{

if (	(0 != strcmp( CMD_HISTORY[0], CMD)) &&
	(0 != strcmp( CMD_HISTORY[1], CMD)) &&
	(0 != strcmp( CMD_HISTORY[2], CMD)) &&
	(0 != strcmp( CMD_HISTORY[3], CMD)) &&
        (0 != strcmp( CMD_HISTORY[4], CMD)) &&
        (0 != strcmp( CMD_HISTORY[5], CMD)) &&
        (0 != strcmp( CMD_HISTORY[6], CMD)) &&
        (0 != strcmp( CMD_HISTORY[7], CMD)) &&
        (0 != strcmp( CMD_HISTORY[8], CMD)) &&
	(0 != strcmp( CMD_HISTORY[9], CMD)) )

	{
        strcpy(CMD_HISTORY[9], CMD_HISTORY[8]);
        strcpy(CMD_HISTORY[8], CMD_HISTORY[7]);
	strcpy(CMD_HISTORY[7], CMD_HISTORY[6]);
        strcpy(CMD_HISTORY[6], CMD_HISTORY[5]);
	strcpy(CMD_HISTORY[5], CMD_HISTORY[4]);
	strcpy(CMD_HISTORY[4], CMD_HISTORY[3]);
	strcpy(CMD_HISTORY[3], CMD_HISTORY[2]);
	strcpy(CMD_HISTORY[2], CMD_HISTORY[1]);
	strcpy(CMD_HISTORY[1], CMD_HISTORY[0]);

	strcpy(CMD_HISTORY[0], CMD);
	
	if (CMD_HISTORY_NUM_REAL < CMD_HISTORY_NUM)
		CMD_HISTORY_NUM_REAL++;
	
	}

}

/// ===========================================================

void command_get()
{
unsigned key;
//unsigned pos = 0;
int hist;

CMD_POS = 0;
CMD_NUM = 0;

for (;;)
	{
	key = getch();
	if ( 0 != (key & 0xff) )
		{
		key &= 0xff;
		switch (key)
			{
			case 27: // ESC
				command_clear();
				break;

			case 13: // ENTER
				CMD[CMD_POS] = '\0';
				printf("\n\r");

				command_history_add(CMD);
				return;

			case 8: // BACKSPACE
				if (CMD_POS > 0)
					{
					printf ("%c %c", 8, 8);
					CMD_POS--;
					}
				break;

			case 9: // TAB
				break;

			default:
				if (CMD_POS < 255)
					{
					
					if ( kol_key_control() & 0x40 ) // если включён CapsLock
						if ( (kol_key_control() & 1) || (kol_key_control() & 2)) // если нажаты шифты 
							key = tolower(key);
						else
							key = toupper(key);

					CMD[CMD_POS] = key;
					CMD_POS++;
					printf("%c", key);
					}
					break;
			};
		}
	else	// обработка расширенных клавиш
		{
		key = (key>>8)&0xff;
//		printf ("%d\n\r", key);

		switch (key)
			{

			case 80: // Down
				for (hist = 0; hist < CMD_HISTORY_NUM; hist++)
					{
					command_clear();

					if (CMD_NUM < CMD_HISTORY_NUM-1)
						CMD_NUM++;
					else
						CMD_NUM = 0;
				
					printf(	CMD_HISTORY[CMD_NUM] );
					strcpy(CMD, CMD_HISTORY[CMD_NUM]);
					if ((CMD_POS = strlen(CMD)) != 0)
						break;
					}

				break;

			case 72: // Up
				for (hist = 0; hist < CMD_HISTORY_NUM; hist++)
					{
					command_clear();

					if (CMD_NUM > 0)
						CMD_NUM--;
					else
						CMD_NUM = CMD_HISTORY_NUM-1;

					printf(	CMD_HISTORY[CMD_NUM] );
					strcpy(CMD, CMD_HISTORY[CMD_NUM]);
					if ((CMD_POS = strlen(CMD)) != 0)
						break;
					}
				break;

			case 0: // console window closed
				cmd_exit(NULL);

			};
		}
		
	}
}



/// ===========================================================

int command_get_cmd(char cmd[])
{
unsigned i, len;
int quote = 0;

if (CMD[0]=='"')
   quote = 1;

if (quote == 0)
   {
   for (i=0;;i++)
       	{
	cmd[i] = CMD[i];
	if (0 == cmd[i])
		{
		i = -2;
		break;
		}
	if ( iswhite(cmd[i]) )
		{
                cmd[i] = '\0';
                break;
		}
	}
   return i+1;
   }
   else
   {
   len = 0;
   for (i=1;;i++)
       	{
	cmd[len] = CMD[i];
	if (0 == cmd[len])
		{
		len = -2;
		break;
		}
	if ( cmd[len] == '"' )
		{
                cmd[len] = '\0';
                break;
		}
        len++;
	}
   trim(cmd);
   return len+2;
   }
}

/// ===========================================================

typedef void (*handler1_t)(char* arg);

/// ===========================================================

void command_execute()
{
char cmd[256];
char args[256];
unsigned arg;
int i;

trim(CMD);
arg = command_get_cmd(cmd);

if ( !strlen(cmd) )
	return;

strcpy(args, CMD+arg);
trim(args);

for (i = 0; i < NUM_OF_CMD; i++)
	{
	if (!strcmp(cmd, COMMANDS[i].name))
		{
		((handler1_t)COMMANDS[i].handler)(args);
		return;
		}
	}


if ( -1 != alias_search(CMD) )
	{
	strcpy(CMD, ALIASES+64*1024+256*alias_search(CMD));
	command_execute();
	return;
	}

executable_run(cmd, args);

}

/// ===========================================================
