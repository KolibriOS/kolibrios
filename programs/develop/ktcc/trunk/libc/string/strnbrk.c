char* strpbrk(const char* string, const char* strCharSet)
{
	char* temp;
	while (*string!='\0')
	{
		temp=strCharSet;
		while (*temp!='\0')
		{
			if (*string==*temp)
				return string;
			temp++;
		}
		string++;
	}
	return (char*)0;
}
