extern int strncmp(char* s1,char* s2,int len);
char* strstr(const char* s, const char* find)
{
	int len;
	len=strlen(find);
	while (1)
	{
		if (strncmp(s,find,len)==0) return s;
		if (*s=='\0')
			return (char*) 0;
		s++;
	}
}
