int strcspn(const char* string, const char* strCharSet)
{
	const char* temp;
	int i;
	i=0;
	while(1)
	{
		temp=strCharSet;
		while (*temp!='\0')
		{
			if (*string==*temp)
				return i;
			temp++;
		}
		i++;string++;
	}
}
