
/// ===========================================================

void command_history_add()
{

int i;

if ('\0' == CMD[0]) return;

CMD_NUM = -1;

for (i = 0; i < CMD_HISTORY_NUM_REAL; i++)
	if ( 0 == strcmp( CMD_HISTORY[i], CMD ) )
		return;

for (i = CMD_HISTORY_NUM_REAL; i > 0 ; i--)
	strcpy(CMD_HISTORY[i], CMD_HISTORY[i-1]);

strcpy(CMD_HISTORY[0], CMD);

if (CMD_HISTORY_NUM_REAL < CMD_HISTORY_NUM-1)
	CMD_HISTORY_NUM_REAL++;

}



/// ===========================================================

void command_get()
{
unsigned key;

unsigned i;
unsigned cmdLen = 0;
unsigned cmdPos = 0;
CMD[0] = '\0';

int clipNum; // number of clipboard slots
char* clipBuf; // clipboard buffer
char clipCopy[255+12];

for (;;)
	{
	key = getch();
	if ( 0 != (key & 0xff) )
		{
		key &= 0xff;
		switch (key)
			{
			case 27: // ESC
				for (i = cmdPos; i < cmdLen; i++)
					printf(" ");
				for (i = cmdLen; i > 0; i--)
					printf("%c %c", 8, 8);
				cmdLen = 0;
				cmdPos = 0;
				CMD[0] = '\0';
				break;

			case 13: // ENTER
				printf("\n\r");
				command_history_add();
				return;

			case 8: // BACKSPACE
				if (cmdPos > 0)
					{
					for (i = cmdPos-1; i < cmdLen; i++)
										CMD[i] = CMD[i+1];
					for (i = 0; i < cmdLen-cmdPos; i++)
						printf (" ");

					for (i = 0; i < cmdLen; i++)
						printf ("%c %c", 8, 8);

					printf("%s", CMD);

					for (i = 0; i < cmdLen-cmdPos; i++)
						printf("%c", 8);

					cmdPos--;
					cmdLen--;
					}
				break;


			case 9: // TAB
				break;
				
				
			case 22: // Ctrl+V
				clipNum = kol_clip_num();
				if (clipNum > 0)
					{
					clipBuf = kol_clip_get(clipNum-1);
//					printf("Length: %d, Type: %d, Encoding: %d\n", (int) *clipBuf, (int)*(clipBuf+4), (int)*(clipBuf+8));
					if ( (int) *clipBuf > 0 ) // is clipboard empty?
						{
						if ((int)*(clipBuf+4)==0) // text?
							{
							if ((int)*(clipBuf+8)==1) // 866 encoding?
								{
								for (i = cmdPos; i < cmdLen; i++)
									printf(" ");
								for (i = cmdLen; i > 0; i--)
									printf("%c %c", 8, 8);
								cmdLen = 0;
								cmdPos = 0;
								CMD[0] = '\0';

								// strcpy_n
								for (i = 0; i < 255; i++)
									{
									CMD[i]=*(clipBuf+12+i);
									if (CMD[i]=='\0')
										break;
									}
								
								cmdPos = cmdLen = strlen(CMD);
/*								
								printf("Length: %d\n", cmdLen);
								for (i = 0; i < cmdLen; i++)
									printf("%d ", CMD[i]);
*/
								printf("%s", CMD);

								}
							}
						}
					}
				break;

				
			case 3: // Ctrl+C
				if ( cmdLen > 0 )
					{
					
					*clipCopy = 12 + cmdLen;
					*(clipCopy+4)=0;
					*(clipCopy+8)=1;
					
					for (i = 0; i <= cmdLen; i++)
						*(clipCopy+12+i) = CMD[i];
						
					kol_clip_set(12+cmdLen, clipCopy);
					
					}
				break;
				
				
			default:
				if (cmdLen < 255)
					{
					if ( kol_key_control() & 0x40 ) // если включён CapsLock
						if ( (kol_key_control() & 1) || (kol_key_control() & 2)) // если нажаты шифты
							key = tolower(key);
						else
							key = toupper(key);

					for (i = cmdLen+1; i > cmdPos; i--)
						CMD[i] = CMD[i-1];

					CMD[cmdPos] = key;

					for (i = cmdPos; i > 0; i--)
						printf("%c %c", 8, 8);

					printf("%s", CMD);

					for (i = 0; i < cmdLen-cmdPos; i++)
						printf("%c", 8);

					cmdPos++;
					cmdLen++;
					}
				break;

			}
		}
	else
		{
		key = (key>>8)&0xff;
		switch (key)
			{
			case 83: // Del
				if (cmdPos < cmdLen)
					{
						for (i = cmdPos; i < cmdLen; i++)
							CMD[i] = CMD[i+1];

						for (i = 0; i < cmdLen-cmdPos; i++)
							printf(" ");

						for (i = 0; i < cmdLen-cmdPos; i++)
							printf("%c", 8);

						for (i = cmdPos; i < cmdLen; i++)
							printf("%c", CMD[i]);

						for (i = 0; i < cmdLen-cmdPos; i++)
							printf("%c", 8);

						cmdLen--;
					}
				break;

			case 75: // Left
				if (cmdPos > 0)
					{
						printf("%c", 8);
						cmdPos--;
					}
				break;


			case 77: // Right
				if (cmdPos < cmdLen)
					{
						printf("%c", CMD[cmdPos]);
						cmdPos++;
					}
				break;


			case 72: // Up
				if (CMD_HISTORY_NUM_REAL > 0 && CMD_NUM >= -1)
					{

					for (i = cmdPos; i < cmdLen; i++)
						printf(" ");

					for (i = cmdLen; i > 0; i--)
						printf("%c %c", 8, 8);

					if (CMD_NUM < CMD_HISTORY_NUM_REAL-1)
						CMD_NUM++;
/*					else
						CMD_NUM = 0;
*/
					printf("%s", CMD_HISTORY[CMD_NUM]);
					strcpy(CMD, CMD_HISTORY[CMD_NUM]);
					cmdLen = strlen(CMD);
					cmdPos = strlen(CMD);

					}
			
				break;


			case 80: // Down
				if (CMD_HISTORY_NUM_REAL > 0 && CMD_NUM >= 0)
					{

					for (i = cmdPos; i < cmdLen; i++)
						printf(" ");

					for (i = cmdLen; i > 0; i--)
						printf("%c %c", 8, 8);

					if (CMD_NUM > 0)
						CMD_NUM--;
/*					else
						CMD_NUM = CMD_HISTORY_NUM_REAL-1;
*/
					printf("%s", CMD_HISTORY[CMD_NUM]);
					strcpy(CMD, CMD_HISTORY[CMD_NUM]);
					cmdLen = strlen(CMD);
					cmdPos = strlen(CMD);

					}
				break;


			case 0: // console window closed
				cmd_exit(NULL);

			}

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
			return i;
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
			i--;
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
	return i+1;
	}
}

/// ===========================================================

typedef int (*handler1_t)(char* arg);

/// ===========================================================

void command_execute()
{
char cmd[256];
char args[256];
unsigned arg;
int i;
int result;

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
		result = ((handler1_t)COMMANDS[i].handler)(args);
		if (result == FALSE)
			{
			#if LANG_ENG
				printf("  Error!\n\r");
			#elif LANG_RUS
				printf("  Ошибка!\n\r");
			#endif
			}
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

