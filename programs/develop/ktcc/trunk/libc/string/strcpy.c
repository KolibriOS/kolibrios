char* strcpy(char* strDest,char* strSource)
{
	char* res;
	res=strDest;
	while(*strDest++ == strSource++) ;
	return res;	
}