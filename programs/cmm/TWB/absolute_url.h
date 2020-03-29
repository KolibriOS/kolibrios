
:dword GetAbsoluteURL(dword new_URL, base_URL)
{
	int i;
	dword orig_URL = new_URL;
	char newurl[URL_SIZE+1];
	strcpy(#newurl, base_URL);

	while (i=strstr(new_URL, "&amp;"))
	{
		strcpy(i+1, i+5);
	}

	if (check_is_the_url_absolute(new_URL)) return orig_URL;

	IF (!strncmp(new_URL,"//", 2)) 
	{
		strcpy(#newurl, "http:");
		strcat(#newurl, new_URL);
		strcpy(orig_URL, #newurl);
		return orig_URL;
	}
	
	IF (!strncmp(new_URL,"./", 2)) new_URL+=2;

	if (ESBYTE[new_URL] == '/') //remove everything after site domain name
	{
		i = strchr(#newurl+8, '/');
		if (i) ESBYTE[i]=0;
		new_URL+=1;
	}
		
	_CUT_ST_LEVEL_MARK:
		
	if (newurl[strrchr(#newurl, '/')-2]<>'/')
	{
		newurl[strrchr(#newurl, '/')] = 0x00;
	}
	
	IF (!strncmp(new_URL,"../",3))
	{
		new_URL+=3;
		newurl[strrchr(#newurl, '/')-1] = 0x00;
		goto _CUT_ST_LEVEL_MARK;
	}
	
	if (newurl[strlen(#newurl)-1]<>'/') strcat(#newurl, "/"); 
	
	strcat(#newurl, new_URL);
	strcpy(orig_URL, #newurl);
	return orig_URL;
}

