char* strdup(char* str)
{
	char* res;
	int len;
	len=strlen(str)+1;
	res=malloc(len);
	memcpy(res,str,len);
	return res;
}
