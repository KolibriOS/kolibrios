#ifndef INCLUDE_LIBHTTP_H
#define INCLUDE_LIBHTTP_H

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

#ifndef INCLUDE_DLL_H
#include "../lib/dll.h"
#endif

dword libHTTP = #alibHTTP;
char alibHTTP[] = "/sys/lib/http.obj";

dword http_lib_init          = #aHTTPinit;
dword http_get               = #aHTTPget;
dword http_head              = #aHTTPhead;
dword http_post              = #aHTTPpost;
dword http_find_header_field = #aFHF;
dword http_send              = #aHTTPsend;
dword http_receive           = #aHTTPreceive;
dword http_disconnect        = #aHTTPdisconnect;
dword http_free              = #aHTTPfree;
dword uri_escape             = #aURIescape;
dword uri_unescape           = #aURIunescape;
$DD 2 dup 0

char aHTTPinit[]             = "lib_init";
char aHTTPget[]              = "get";
char aHTTPhead[]             = "head";
char aHTTPpost[]             = "post";
char aFHF[]                  = "find_header_field";
char aHTTPsend[]             = "send";
char aHTTPreceive[]          = "receive";
char aHTTPdisconnect[]       = "disconnect";
char aHTTPfree[]             = "free";
char aURIescape[]            = "escape";
char aURIunescape[]          = "unescape";

// status flags
#define FLAG_HTTP11             1 << 0
#define FLAG_GOT_HEADER         1 << 1
#define FLAG_GOT_ALL_DATA       1 << 2
#define FLAG_CONTENT_LENGTH     1 << 3
#define FLAG_CHUNKED            1 << 4
#define FLAG_CONNECTED          1 << 5

// user flags
#define FLAG_KEEPALIVE          1 << 8
#define FLAG_MULTIBUFF          1 << 9

// error flags
#define FLAG_INVALID_HEADER     1 << 16
#define FLAG_NO_RAM             1 << 17
#define FLAG_SOCKET_ERROR       1 << 18
#define FLAG_TIMEOUT_ERROR      1 << 19
#define FLAG_TRANSFER_FAILED    1 << 20

struct  http_msg{
	dword   socket;
	dword   flags;
	dword   write_ptr;
	dword   buffer_length;
	dword   chunk_ptr;
	dword   timestamp;
	dword   status;
	dword   header_length;
	dword   content_ptr;
	dword   content_length;
	dword   content_received;
	char    http_header;
};


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

	dword get();
	bool stop();
	void hfree();
	void receive();
	dword header_field();
};

dword _http::get(dword _url)
{
	cur_url = _url;
	http_get stdcall (_url, 0, 0, #accept_language);
	transfer = EAX;
	return transfer;
}

void _http::hfree()
{
	http_free stdcall (transfer);
	transfer=0;
}

bool _http::stop()
{
	if (transfer)
	{
		EAX = transfer;
		EAX = EAX.http_msg.content_ptr;         // get pointer to data
		$push   EAX                                                     // save it on the stack
		http_free stdcall (transfer);   // abort connection
		$pop    EAX                                                     
		free(EAX);                                              // free data
		transfer=0;
		return true;    
	}
	return false;
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

:dword _http::header_field(dword _header_field, _dst, _size)
{
	http_find_header_field stdcall (transfer, _header_field);
	if (EAX!=0) {
		ESI = EAX;
		EDI = _dst;
		EDX = _size;
		do {
			$lodsb;
			$stosb;
			$dec edx
		} while (AL != 0) && (AL != 13) && (AL != 10) && (EDX>0);
		DSBYTE[EDI-1]='\0';
		debugln(_dst);
		return _dst;
	}
	return NULL;
}


//===================================================//
//                                                   //
//                 CHECK PATH TYPE                   //
//                                                   //
//===================================================//

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
	if(!strncmp(_in,"tmp",3)) return true;
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


#endif