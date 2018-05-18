
int UrlIsAbsolute(dword in)
{
	if(!strncmp(in,"http:",5)) return true;
	if(!strncmp(in,"https:",6)) return true;
	if(!strncmp(in,"file:",5)) return true;
	if(!strncmp(in,"mailto:",7)) return true;
	if(!strncmp(in,"ftp:",4)) return true;
	if(!strncmp(in,"WebView:",8)) return true;
	if(!strncmp(in,"/sys/",5)) return true;
	if(!strncmp(in,"/hd/",4)) return true;
	if(!strncmp(in,"/fd/",4)) return true;
	if(!strncmp(in,"/rd/",4)) return true;
	if(!strncmp(in,"/tmp/",5)) return true;
	if(!strncmp(in,"/cd/",4)) return true;
	if(!strncmp(in,"/bd/",4)) return true;
	if(!strncmp(in,"/usbhd/",7)) return true;
	if(!strncmp(in,"/kolibrios/",11)) return true;
	return false;
}

void GetAbsoluteURL(dword in_URL)
{
	int i;
	dword orig_URL = in_URL;
	char newurl[sizeof(URL)];

	while (i=strstr(in_URL, "&amp;"))
	{
		strcpy(i+1, i+5);
	}

	if (UrlIsAbsolute(in_URL)) return;

	IF (!strcmpn(in_URL,"//", 2)) 
	{
		//strcpy(#newurl, "http:");
		//strcat(#newurl, in_URL);
		sprintf(#newurl, "http:%s", in_URL);
		strcpy(orig_URL, #newurl);
		return;
	}
	
	IF (!strcmpn(in_URL,"./", 2)) in_URL+=2;
	if (!http.transfer) 
	{
		strcpy(#newurl, history.current());
	}
	else
	{
		strcpy(#newurl, history.items.get(history.active-2)); 
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

