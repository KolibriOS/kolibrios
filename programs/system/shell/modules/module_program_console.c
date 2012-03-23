
#include "../program_console.h"


int program_console(int pid)
{

char name[32];
char *buffer;
int result;
int i;
char command;
int size;
int is_end;

_itoa(pid, name);
strcat(name, "-SHELL");

buffer = NULL;


for (i = 0; i < 30;  i++)
	{
	result = kol_buffer_open(name, SHM_OPEN | SHM_WRITE, 0, &buffer);
	if (buffer != NULL)
		break;

	kol_sleep(2);
	}

	if (buffer == NULL)
		return 0;
	else
		size = result;

	is_end = 0;
	for (;;)
		{

		command = *(buffer);
	
		switch (command)
			{
			
			case SC_EXIT:
				*buffer = SC_OK;
				is_end = 1;
				break;
			
			case SC_OK:
				kol_sleep(5);
				break;

			case SC_CLS:
				con_cls();
				*buffer = SC_OK;
				break;
				
			case SC_PUTC:
				printf("%c", *(buffer+1));
				*buffer = SC_OK;
				break;

			case SC_PUTS:
				printf("%s", buffer+1 );
				*buffer = SC_OK;
				break;				

			case SC_GETC:
				*(buffer+1) = (char) getch() ;
				*buffer = SC_OK;
				break;	

			case SC_GETS:
				gets(buffer+1, size-2);
				*buffer = SC_OK;
				break;	
				
			default:
				#if LANG_ENG
					printf ("  Error in console application.\n\r");
				#elif LANG_RUS
					printf ("  Ошибка в консольном приложении.\n\r");
				#endif
				return 0;
			};
		if (is_end)
			{
			printf("\n\r");
			return 1;
			}
		}

return 9;
}