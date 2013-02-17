
/// ===========================================================

int alias_check(char alias[])
{

unsigned i;
char buf1[256];
char buf2[256];

if ( !alias_split (alias, buf1, buf2))
	return FALSE;


for (i = 0; i < NUM_OF_CMD; i++)
	if ( !strcmp(COMMANDS[i].name, buf1) )
		return -1;

if (NULL == ALIASES)
	return FALSE;

for (i = 0; i < ALIAS_NUM; i++)
	{
	if ( !strcmp(ALIASES+256*i, buf1) )
		return FALSE;
	}

return TRUE;
}

/// ===========================================================

int alias_search(char alias[])
{

unsigned i;

for (i = 0; i < ALIAS_NUM; i++)
	{
	if ( !strcmp(ALIASES+256*i, alias) )
		return i;
	}

return -1;
}

/// ===========================================================

int alias_add(char alias[])
{

char buf1[256];
char buf2[256];

if ( ALIAS_NUM > 255)
	return FALSE;

if ( !alias_split (alias, buf1, buf2))
	return FALSE;

strcpy (ALIASES+256*ALIAS_NUM, buf1);
strcpy (ALIASES+256*ALIAS_NUM+64*1024, buf2);
ALIAS_NUM++;

return TRUE;
}

/// ===========================================================

void alias_list()
{

unsigned i;

if ( 0 == ALIAS_NUM)
	return;

for (i = 0; i < ALIAS_NUM; i++)
	printf ("  %s=%s\n\r",ALIASES+256*i, ALIASES+256*i+64*1024); 
}

/// ===========================================================

int alias_split (char alias[], char s1[], char s2[])
{

unsigned i, j;

if (NULL == strchr(alias, '='))
	return FALSE;

for (i=0, j = 0;; i++)
	{
	if ('=' == alias[i])
		{
		s1[i]='\0';
		break;
		}
	s1[i]=alias[i];
	}

j = 0;

for (;; i++, j++)
	{
	s2[j]=alias[i];
	if ('\0' == alias[i])
		break;
	}

trim(s1); 

for (i=0;;i++)
	{
	s2[i] = s2[i+1];
	if ('\0' == s2[i] )
		break;
	}

trim(s2);

return TRUE;

}

/// ===========================================================

