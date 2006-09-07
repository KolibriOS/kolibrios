int strlen(const char* string)
{
	int i;
	i=0;
	while (*string!='\0')
	{
		i++;
		string++;
	}
	return i;
}
