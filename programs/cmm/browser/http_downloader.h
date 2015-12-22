
enum { 
	STATE_NOT_STARTED, 
	STATE_IN_PROGRESS, 
	STATE_COMPLETED 
};

struct DOWNLOADER {
	char url[10000];
	int data_downloaded_size, data_full_size;
	dword bufpointer, bufsize;
	byte state;
	dword http_transfer;
	dword Start();
	void Stop();
	void Completed();
	int MonitorProgress();
} downloader;

dword DOWNLOADER::Start(dword _url)
{
	state = STATE_IN_PROGRESS;
	strcpy(#url, _url); //need to replace my malloc()
	http_get stdcall (#url, 0, 0, #accept_language);
	http_transfer = EAX;
	return http_transfer;
}

void DOWNLOADER::Stop()
{
	state = STATE_NOT_STARTED;
	if (http_transfer!=0)
	{
		EAX = http_transfer;
		EAX = EAX.http_msg.content_ptr;		// get pointer to data
		$push EAX							// save it on the stack
		http_free stdcall (http_transfer);	// abort connection
		$pop  EAX							
		mem_Free(EAX);						// free data
		http_transfer=0;
		bufsize = 0;
		bufpointer = mem_Free(bufpointer);
	}
	data_downloaded_size = data_full_size = 0;
}

void DOWNLOADER::Completed()
{
	state = STATE_COMPLETED;
	ESI = http_transfer;
	bufpointer = ESI.http_msg.content_ptr;
	bufsize = ESI.http_msg.content_received;
	http_free stdcall (http_transfer);
	http_transfer=0;
}

int DOWNLOADER::MonitorProgress() 
{
	dword receive_result;
	if (http_transfer <= 0) return false;
	http_receive stdcall (http_transfer);
	receive_result = EAX;
	EDI = http_transfer;
	data_full_size = EDI.http_msg.content_length;
	data_downloaded_size = EDI.http_msg.content_received;
	if (receive_result == 0) Completed();
	return true;
}
