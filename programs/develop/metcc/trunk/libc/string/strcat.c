char* strcat(char* strDest, const char* strSource)
{
	char* res;
	res=strDest;
	while (*strDest!='\0') strDest++;
	while (*strSource!='\0')
	{
		*strDest=*strSource;
		strDest++;
		strSource++;
	}
	return res;
}
