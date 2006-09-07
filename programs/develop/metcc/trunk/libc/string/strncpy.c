char* strncpy(char* strDest,const char* strSource,int count)
{
	char* res;
	res=strDest;
	while (count>0)
	{
		*strDest=*strSource;
		if (*strSource!='\0')
			strSource++;
		strDest++;
		count--;
	}
	return res;
}
