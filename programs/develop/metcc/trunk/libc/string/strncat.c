char* strncat(char* strDest,const char* strSource,int count)
{
	char* res;
	res=strDest;
	while (*strDest++) ;
	while(count-->0)
	{
	    if(*strDest++ = *strSource++) continue;
		return(res);
	}
	*strDest = 0;
	return res;
}