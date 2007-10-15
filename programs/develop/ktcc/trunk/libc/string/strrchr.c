char* strrchr(const char* s,int c)
{
	char* res;
	res=(char*)0;
	while (1)
	{
		if (*s==(char)c)
			res=(char*)s;
		if (*s=='\0')
			break;
		s++;
	}
	return res;
}
