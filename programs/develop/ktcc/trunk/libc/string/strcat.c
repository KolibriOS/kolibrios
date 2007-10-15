char* strcat(char* strDest, const char* strSource)
{
	char* res;
	res=strDest;
	while (*strDest++) ;
	while (*strDest++ = *strSource++) ;
	return res;
}
