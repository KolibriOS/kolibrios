
#include "system/kolibri.h"
#include "system/string.h"

#include "program_console.h"

char		*buffer; // используется только одна именованная область, поэтому можно сделать её глобальной переменной
char		name[32]; // имя нужно как для создания области, так и для удаления, поэтому можно сделать её глобальной переменной

int sc_init() // инициализация - создание именованной области
{

char		*buf1k;
unsigned	PID;
int			result;


buf1k = malloc(1024);
if (NULL == buf1k)
	return -1;

kol_process_info(-1, buf1k); // получаем СВОЙ (-1) идентификатор процесса 
PID = *(buf1k+30);
free(buf1k);

_itoa(PID, name); // формируем из номера процесса строку с заглавными нулями 42 -> 0042 (т.е. до 9999)
strcat(name, "-SHELL");

*buffer = NULL;
result = kol_buffer_open(name, SHM_OPEN_ALWAYS | SHM_WRITE, 1024*16, &buffer); // создаём область (16 кил)

return result;
}



void sc_puts(char *str)
{
*buffer = SC_PUTS;
strcpy(buffer+1, str);
while (*buffer) kol_sleep(5);
}


void sc_exit()
{
*buffer = SC_EXIT;
while (*buffer) kol_sleep(5);
kol_buffer_close(name);
}


void sc_gets(char *str)
{
*buffer = SC_GETS;
while (*buffer) kol_sleep(5);
strcpy(str, buffer+1);
}


char sc_getc()
{
*buffer = SC_GETC;
while (*buffer) kol_sleep(5);
return *(buffer+1);
}


void sc_putc(char c)
{
*buffer = SC_PUTC;
*(buffer+1) = c;
while (*buffer) kol_sleep(5);
}



void sc_cls()
{
*buffer = SC_CLS;
while (*buffer) kol_sleep(5);
}




void kol_main()
{

char string[256];

sc_init();

sc_cls();
sc_puts("This is a test console application for Shell\n\r");
sc_puts("Type a string (255 symbols max): ");
sc_gets(string);
sc_puts("You typed:\n\r");
sc_puts(string);
sc_puts("Press any key: ");
string[0] = sc_getc();
sc_puts("\n\rYou pressed: ");
sc_putc(string[0]);

sc_exit();



}
