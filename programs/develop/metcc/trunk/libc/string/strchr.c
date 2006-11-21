char* strchr(const char* string, int c)
{
	while (*string)
	{
		if (*string==c)
			return (char*)string;
		string++;
	}	
	return (char*)0;
}
