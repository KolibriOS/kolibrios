char* strncat(char* strDest,const char* strSource,int count)
{
	char* res;
	res=strDest;
	while (*strDest!='\0') strDest++;
	while (count>0 && *strSource!='\0')
	{
		*strDest=*strSource;
		count--;
		strDest++;
		strSource++;
	}
	return res;
}
