#define URL_SIZE 4000

//===================================================//
//                                                   //
//                       HTTP                        //
//                                                   //
//===================================================//

struct _http
{
	dword cur_url;
    dword transfer;
    dword content_length;
    dword content_received;
    dword status_code;
    dword receive_result;
    dword content_pointer;
    char redirect_url[4096*3];
    char content_type[64];

    dword get();
    void hfree();
    void receive();
    bool handle_redirect();
    dword check_content_type();
};

dword _http::get(dword _url)
{
	cur_url = _url;
    http_get stdcall (_url, 0, 0, #accept_language);
    content_type[0] = '\0';
    transfer = EAX;
    return transfer;
}

void _http::hfree()
{
    http_free stdcall (transfer);
    transfer=0;
}

void _http::receive()
{
    http_receive stdcall (transfer);
    receive_result = EAX;

    EDI = transfer;
    if (!EAX) {
    	status_code = EDI.http_msg.status;
    	content_pointer = EDI.http_msg.content_ptr;
    }
    content_length = EDI.http_msg.content_length;
    content_received = EDI.http_msg.content_received;

 }

:bool _http::handle_redirect()
{
    http_find_header_field stdcall (transfer, "location\0");
    if (EAX!=0) {
        ESI = EAX;
        EDI = #redirect_url;
        do {
            $lodsb;
            $stosb;
        } while (AL != 0) && (AL != 13) && (AL != 10);
        DSBYTE[EDI-1]='\0';
        get_absolute_url(#redirect_url, cur_url);
        hfree();
        return #redirect_url;
    }
    return NULL;
}

:dword _http::check_content_type()
{
	if (content_type[0]) return NULL;
    http_find_header_field stdcall (transfer, "content-type\0");
    if (EAX!=0) {
        ESI = EAX;
        EDI = #content_type;
        do {
            $lodsb;
            $stosb;
        } while (AL != 0) && (AL != 13) && (AL != 10);
        DSBYTE[EDI-1]='\0';
        debugln(#content_type);
        return #content_type;
    }
    return NULL;
}

//===================================================//
//                                                   //
//                    DOWNLOADER                     //
//                                                   //
//===================================================//


enum { 
	STATE_NOT_STARTED, 
	STATE_IN_PROGRESS, 
	STATE_COMPLETED 
};

struct DOWNLOADER : _http {
	dword bufpointer, bufsize, url;
	int state;
	dword Start();
	void Stop();
	bool MonitorProgress();
};

dword DOWNLOADER::Start(dword _url)
{
	url = _url;
	state = STATE_IN_PROGRESS;
	get(_url);
	if (!transfer) Stop();
	return transfer;
}

void DOWNLOADER::Stop()
{
	state = STATE_NOT_STARTED;
	if (transfer!=0)
	{
		EAX = transfer;
		EAX = EAX.http_msg.content_ptr;		// get pointer to data
		$push EAX							// save it on the stack
		http_free stdcall (transfer);	// abort connection
		$pop  EAX							
		free(EAX);						// free data
		transfer=0;
		bufsize = 0;
		bufpointer = free(bufpointer);
	}
	content_received = content_length = 0;
}

bool DOWNLOADER::MonitorProgress() 
{
	if (transfer <= 0) return false;
	receive();
	if (!content_length) content_length = content_received * 20;

	if (receive_result == 0) {
		if (status_code >= 300) && (status_code < 400)
		{
			url = handle_redirect();
			Stop();
			Start(url);
			return false;
		}
		state = STATE_COMPLETED;
		bufpointer = content_pointer;
		bufsize = content_received;
		hfree();
	}
	return true;
}


/*=====================================
==                                   ==
==         CHECK PATH TYPE           ==
==                                   ==
=====================================*/


:int check_is_the_adress_local(dword _in)
{
	if (ESBYTE[_in]!='/') return false;
	_in++;
	if(!strncmp(_in,"rd/",3)) return true;
	if(!strncmp(_in,"fd/",3)) return true;
	if(!strncmp(_in,"hd",2)) return true;
	if(!strncmp(_in,"bd",2)) return true;
	if(!strncmp(_in,"cd",2)) return true;
	if(!strncmp(_in,"sys/",4)) return true;
	if(!strncmp(_in,"tmp/",4)) return true;
	if(!strncmp(_in,"usbhd",5)) return true;
	if(!strncmp(_in,"kolibrios",9)) return true;
	return false;
}

:int check_is_the_url_absolute(dword _in)
{
	if(!strncmp(_in,"ftp:",4)) return true;
	if(!strncmp(_in,"http:",5)) return true;
	if(!strncmp(_in,"https:",6)) return true;
	if(!strncmp(_in,"mailto:",7)) return true;
	if(!strncmp(_in,"tel:",4)) return true;
	if(!strncmp(_in,"#",1)) return true;
	if(!strncmp(_in,"WebView:",8)) return true;
	if(check_is_the_adress_local(_in)) return true;
	return false;
}

:dword get_absolute_url(dword new_URL, base_URL)
{
	int i;
	dword orig_URL = new_URL;
	char newurl[URL_SIZE+1];
	strcpy(#newurl, base_URL);

	while (i=strstr(new_URL, "&amp;")) strcpy(i+1, i+5);

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
