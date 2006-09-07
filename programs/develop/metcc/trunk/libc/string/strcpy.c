char* strcpy(char* strDest,char* strSource)
{
	char* res;
	res=strDest;
	while(1)
	{
		*strDest=*strSource;
		if (*strSource=='\0')
			break;
		strDest++;
		strSource++;
	}
	return res;	
}
