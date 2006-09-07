char* strchr(const char* string, int c)
{
	while (*string!='\0')
	{
		if (*string==c)
			return (char*)string;
		string++;
	}	
	return (char*)0;
}
