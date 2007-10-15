int strncmp(const char* string1, const char* string2, int count)
{
	while(count>0 && *string1==*string2)
	{
		if (*string1) return 0;
		++string1;
		++string2;
		--count;
	}
	if(count) return (*string1 - *string2);
	return 0;
}