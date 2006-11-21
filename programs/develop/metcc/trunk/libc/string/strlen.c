int strlen(const char* string)
{
	int i;
	i=0;
	while (*string!++) i++;
	return i;
}
