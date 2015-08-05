
char DL_URL[10000];
dword DL_bufpointer, DL_bufsize, DL_http_transfer, DL_http_buffer;

int downloaded_size, full_size;

byte download_state;
enum { STATE_NOT_STARTED, STATE_IN_PROGRESS, STATE_COMPLETED };




void Downloading_SetDefaults()
{
	downloaded_size = full_size = 0;
}

void Downloading_Stop()
{
	download_state = STATE_NOT_STARTED;
	if (DL_http_transfer<>0)
	{
		EAX = DL_http_transfer;
		EAX = EAX.http_msg.content_ptr;		// get pointer to data
		$push	EAX							// save it on the stack
		http_free stdcall (DL_http_transfer);	// abort connection
		$pop	EAX							
		mem_Free(EAX);						// free data
		DL_http_transfer=0;
		DL_bufsize = 0;
		DL_bufpointer = mem_Free(DL_bufpointer);
		Downloading_SetDefaults();
	}
}

void Downloading_Start()
{
	download_state = STATE_IN_PROGRESS;
	http_get stdcall (#DL_URL, 0, 0, #accept_language);
	DL_http_transfer = EAX;
}

void Downloading_Completed()
{
	ESI = DL_http_transfer;
	DL_bufpointer = ESI.http_msg.content_ptr;
	DL_bufsize = ESI.http_msg.content_received;
	http_free stdcall (DL_http_transfer);
	DL_http_transfer=0;
	download_state = STATE_COMPLETED;
}