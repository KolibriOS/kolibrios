
int check_is_the_adress_local(dword _in)
{
	if(!strncmp(_in,"/sys/",5)) return true;
	if(!strncmp(_in,"/hd/",4)) return true;
	if(!strncmp(_in,"/fd/",4)) return true;
	if(!strncmp(_in,"/rd/",4)) return true;
	if(!strncmp(_in,"/tmp/",5)) return true;
	if(!strncmp(_in,"/cd/",4)) return true;
	if(!strncmp(_in,"/bd/",4)) return true;
	if(!strncmp(_in,"/usbhd/",7)) return true;
	if(!strncmp(_in,"/kolibrios/",11)) return true;
	return false;
}

/*
int check_is_the_url_absolute(dword in)
{
	if(!strncmp(_in,"http:",5)) return true;
	if(!strncmp(_in,"https:",6)) return true;
	if(!strncmp(_in,"ftp:",4)) return true;
}

void GetAbsoluteURL(dword in_URL)
{
	int i;
	dword orig_URL = in_URL;
	char newurl[UML];

	while (i=strstr(in_URL, "&amp;"))
	{
		strcpy(i+1, i+5);
	}

	if (UrlIsAbsolute(in_URL)) return;
	
	IF (!strcmpn(in_URL,"./", 2)) in_URL+=2;
	if (!http_transfer) 
	{
		strcpy(#newurl, History.current());
	}
	else
	{
		strcpy(#newurl, History.items.get(History.active-2)); 
	}

	if (ESBYTE[in_URL] == '/') //remove everything after site domain name
	{
		i = strchr(#newurl+8, '/');
		if (i) ESBYTE[i]=0;
		in_URL+=1;
	}
		
	_CUT_ST_LEVEL_MARK:
		
	if (newurl[strrchr(#newurl, '/')-2]<>'/')
	{
		newurl[strrchr(#newurl, '/')] = 0x00;
	}
	
	IF (!strncmp(in_URL,"../",3))
	{
		in_URL+=3;
		newurl[strrchr(#newurl, '/')-1] = 0x00;
		goto _CUT_ST_LEVEL_MARK;
	}
	
	if (newurl[strlen(#newurl)-1]<>'/') strcat(#newurl, "/"); 
	
	strcat(#newurl, in_URL);
	strcpy(orig_URL, #newurl);
}

*/