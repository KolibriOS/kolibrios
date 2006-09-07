int strncmp(const char* string1, const char* string2, int count)
{
	while(count>0)
	{
		if (*string1<*string2)
			return -1;
		if (*string1>*string2)
			return 1;
		if (*string1=='\0')
			return 0;
		count--;
	}	
	return 0;
}
