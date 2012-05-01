
#include "system/boolean.h"
#include "system/kolibri.h"
#include "system/stdlib.h"
#include "system/string.h"
#include "system/ctype.h"

#include "system/console.c"

#include "base.h"

#define BASE_LEN 10716

char cities_used[BASE_LEN];
int user_current;
char name[256];
char last;

///===========================

void kol_main();

///===========================

int _iswhite(char c)
{
return ((' ' == c) || ('\t' == c) || (13 == c) || (10 == c));
}

///===========================

unsigned char _tolower(unsigned char c)
{
unsigned char x = 0;

if ((c == (unsigned char)'ё')||(c == (unsigned char)'Ё'))
	return 'е';

if ((c > 127) && (c < 144))
	x += 32;

if ((c > 143) && (c < 160))
	x += 80;

return c+x;
}

///===========================

void trim(char string[])
{
int i, j;

for (i=0; ;i++)
	if ( !_iswhite(string[i]) )
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
	if ( _iswhite(string[i]) )
		string[i] = '\0';
	else
		break;
}

///===========================

void game_new()
{
memset(cities_used, 0, BASE_LEN);
user_current = 0;
last = 32;
}

///===========================

int search(char *city)
{
int i;
for ( i = 0; i < BASE_LEN; i++ )
	if ( ! strcmp(cities[i], city) )
		return i;
return -1;
}

///===========================

int last_ok(char c)
{
if ( (c == 'ы')||(c == 'ь'))
	return 0;
else 
	return 1;
}

///===========================

void kol_main()
{

CONSOLE_INIT("mCities by Albom");


printf("%s", "\
mCities - игра в города. Версия 0.1.\n\
Автор: Александр Богомаз aka Albom (albom85@yandex.ru)\n\n\
\
Управление:\n\
команда * - новая игра\n\
команда ! - список названных городов\n\n\
");

game_new();
int i, s;

for (;;)
	{
	for (;;)
		{
		printf("Игрок %d > ", user_current+1); 
		gets(name, 32);
		
		if (*name == 0)
			{
			_exit(1);
			kol_exit();
			}

		trim(name);

		if (!strcmp(name, "*"))
			{
			printf("\n\n");
			game_new();
			continue;
			}

		if (!strcmp(name, "!"))
			{
			for (i = 0; i < BASE_LEN; i++)
				if (cities_used[i] == 1)
					printf("%s  ", cities[i]);
			printf("\n");
			continue;
			}

		for (i = 0; i < strlen(name); i++ )
			name[i] = _tolower(name[i]);

		if ( (last!=name[0])&&(last!=32) )
			{
			printf("Нужно назвать город на букву \'%c\'!\n", last);
			continue;
			}
		s = search(name);

		if (cities_used[s] == 1)
			{
			printf("Этот город уже назывался!\n");
			continue;
			}
			

		if ( s == -1 )
			printf("Не знаю такого города!\n");
		else
			{

			cities_used[s] = 1;

			if ( last_ok(name[strlen(name)-1]) )
				last = name[strlen(name)-1];
			else
				last = name[strlen(name)-2];
			break;
			}
		}

	if ( user_current == 0)
		user_current = 1;
	else
		user_current = 0;


	}

}

///===========================
