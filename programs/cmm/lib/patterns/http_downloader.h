//===================================================//
//                                                   //
//                       HTTP                        //
//                                                   //
//===================================================//

struct _http
{
    dword url;
    dword transfer;
    dword content_length;
    dword content_received;
    dword status_code;
    dword receive_result;
    dword content_pointer;

    dword get();
    void free();
    void receive();
    bool handle_redirect();
};

dword _http::get(dword _url)
{
    url = _url;
    http_get stdcall (url, 0, 0, #accept_language);
    transfer = EAX;
    return transfer;
}

void _http::free()
{
    http_free stdcall (transfer);
    transfer=0;
}

void _http::receive()
{
    http_receive stdcall (transfer);
    receive_result = EAX;

    ESI = transfer;
    if (!EAX) status_code = ESI.http_msg.status;
    content_length = ESI.http_msg.content_length;
    content_received = ESI.http_msg.content_received;

 }

bool _http::handle_redirect()
{
	dword redirect, i;
	char redirect_url[4096*3], finaladress[4096*3];
    http_find_header_field stdcall (transfer, "location\0");
    if (EAX!=0) {
        ESI = EAX;
        EDI = #redirect_url;
        do {
            $lodsb;
            $stosb;
        } while (AL != 0) && (AL != 13) && (AL != 10);
        DSBYTE[EDI-1]='\0';
        get_absolute_url(#finaladress, url, #redirect_url);
        strcpy(url, #finaladress);
        return true;
    }
    return false;
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

struct DOWNLOADER {
	_http httpd;
	unsigned data_downloaded_size, data_full_size;
	dword bufpointer, bufsize, url;
	int state;
	dword Start();
	void Stop();
	void Completed();
	int MonitorProgress();
} downloader;

dword DOWNLOADER::Start(dword _url)
{
	url = _url;
	state = STATE_IN_PROGRESS;
	httpd.get(url);
	return httpd.transfer;
}

void DOWNLOADER::Stop()
{
	state = STATE_NOT_STARTED;
	if (httpd.transfer!=0)
	{
		EAX = httpd.transfer;
		EAX = EAX.http_msg.content_ptr;		// get pointer to data
		$push EAX							// save it on the stack
		http_free stdcall (httpd.transfer);	// abort connection
		$pop  EAX							
		free(EAX);						// free data
		httpd.transfer=0;
		bufsize = 0;
		bufpointer = free(bufpointer);
	}
	data_downloaded_size = data_full_size = 0;
}

void DOWNLOADER::Completed()
{
	state = STATE_COMPLETED;
	ESI = httpd.transfer;
	bufpointer = ESI.http_msg.content_ptr;
	bufsize = ESI.http_msg.content_received;
	httpd.free();
}

int DOWNLOADER::MonitorProgress() 
{
	dword receive_result;
	char redirect_url[4096*3], finaladress[4096*3];
	if (httpd.transfer <= 0) return false;
	http_receive stdcall (httpd.transfer);
	receive_result = EAX;
	EDI = httpd.transfer;
	httpd.status_code = EDI.http_msg.status;
	data_full_size = EDI.http_msg.content_length;
	data_downloaded_size = EDI.http_msg.content_received;
	if (!data_full_size) data_full_size = data_downloaded_size * 6 + 1;

	if (receive_result == 0) {
		if (httpd.status_code >= 300) && (httpd.status_code < 400)
		{
			httpd.handle_redirect();
			strcpy(url, httpd.url);
			get_absolute_url(#finaladress, url, #redirect_url);
			Stop();
			Start(#finaladress);
			url = #finaladress;
			return false;
		}
		Completed();
	}
	return true;
}


/*=====================================
==                                   ==
==         CHECK PATH TYPE           ==
==                                   ==
=====================================*/


int check_is_the_adress_local(dword _in)
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
	if(!strncmp(_in,"kolibrios",10)) return true;
	return false;
}

int check_is_the_url_absolute(dword _in)
{
	if(!strncmp(_in,"ftp:",4)) return true;
	if(!strncmp(_in,"http:",5)) return true;
	if(!strncmp(_in,"https:",6)) return true;
	if(!strncmp(_in,"mailto:",7)) return true;
	if(check_is_the_adress_local(_in)) return true;
	return false;
}

void get_absolute_url(dword _rez, _base, _new)
{
	int i;
	debug("_base:");debugln(_base);
	debug("_new:");debugln(_new);
	//case: ./valera.html
	if (!strncmp(_new,"./", 2)) _new+=2;
	//case: http://site.name
	if (check_is_the_url_absolute(_new)) {
		strcpy(_rez, _new);
		goto _GET_ABSOLUTE_URL_END;
	}
	//case: /valera.html
	if (ESBYTE[_new] == '/') //remove everything after site domain name
	{
		strcpy(_rez, _base);
		i = strchr(_rez+8,'/');
		if (i<1) i=strlen(_rez)+_rez;
		strcpy(i, _new);
		goto _GET_ABSOLUTE_URL_END;
	}	
	//case: ../../valera.html
	strcpy(_rez, _base);
	ESBYTE[ strrchr(_rez,'/') + _rez -1 ] = '\0'; //remove name
	while (!strncmp(_new,"../",3))
	{
		_new += 3;
		ESBYTE[ strrchr(_rez,'/') + _rez -1 ] = '\0';	
	}
	//case: valera.html
	chrcat(_rez, '/');
	strcat(_rez, _new); 
	_GET_ABSOLUTE_URL_END:
	while (i=strstr(_rez, "&amp;")) strcpy(i+1, i+5);	
	debug("_rez:");debugln(_rez);
}
