#ifdef LANG_RUS
	#define DL_WINDOW_HEADER "Download Manager"
	#define START_DOWNLOADING "Start downloading"
	#define STOP_DOWNLOADING "Stop downloading"
	#define FILE_SAVED_AS "'Download manager\nFile saved as "
	#define DOWNLOADING_COMPLETE "Downloading complete."
#else
	#define DL_WINDOW_HEADER "Download Manager"
	#define START_DOWNLOADING "Start downloading"
	#define STOP_DOWNLOADING "Stop downloading"
	#define FILE_SAVED_AS "'Download manager\nFile saved as "
	#define DOWNLOADING_COMPLETE "Downloading complete."
#endif

proc_info DL_Form;
system_colors sc;
char DL_URL[10000];
dword DL_bufpointer;
dword DL_bufsize;
dword DL_http_transfer;
dword DL_http_buffer;
char filepath[4096];
int downloaded_size, full_size;
int	mouse_twbi;
edit_box DL_address_box = {250,20,20,0xffffff,0x94AECE,0xffffff,0xffffff,0,sizeof(DL_URL),#DL_URL,#mouse_twbi,2,19,19};

char save_to[4096] = "/tmp0/1/Downloads/";
byte cleft = 10;


void Downloader()
{
	int key, btn;
	mouse m;
	char notify_message[4296];
	
	if (DL_URL[0]) {
		StartDownloading();
	}
	else strcpy(#DL_URL, "http://");
	DL_address_box.size = DL_address_box.pos = DL_address_box.shift = DL_address_box.shift_old = strlen(#DL_URL);

	downloaded_size = full_size = 0;

	SetEventMask(0x27);
	loop()
	{
		WaitEventTimeout(40);
		switch(EAX & 0xFF)
		{
			CASE evMouse:
				if (!CheckActiveProcess(DL_Form.ID)) break;
				if (DL_http_transfer <= 0) edit_box_mouse stdcall (#DL_address_box);
				break;

			case evButton:
				btn=GetButtonID();
				if (btn==1)	ExitProcess();
				DL_Scan(btn);
				break;

			case evKey:
				key = GetKey();
				if (DL_address_box.flags & 0b10)
				{
					EAX=key<<8; 
					edit_box_key stdcall(#DL_address_box);
				}
				if (key==13) DL_Scan(301);
				break;

			case evReDraw:
				sc.get();
				DefineAndDrawWindow(215, 100, 420, 150, 0x74, sc.work, DL_WINDOW_HEADER, 0);
				GetProcessInfo(#DL_Form, SelfInfo);
				if (DL_Form.status_window>2) break;
				if (DL_Form.height<120) MoveSize(OLD,OLD,OLD,120);
				if (DL_Form.width<280) MoveSize(OLD,OLD,280,OLD);
				DL_Draw_Window();
				break;
				
			default:
				if (DL_Form.width==0) break;
				if (DL_http_transfer <= 0) break;
				http_process stdcall (DL_http_transfer);
				if (EAX == 0) {
					ESI = DL_http_transfer;
					DL_bufpointer = ESI.http_msg.content_ptr;
					DL_bufsize = ESI.http_msg.content_received;
					http_free stdcall (DL_http_transfer);
					DL_http_transfer=0;
					strcpy(#filepath, #save_to);
					strcat(#filepath, #DL_URL+strrchr(#DL_URL, '/'));
					if (WriteFile(DL_bufsize, DL_bufpointer, #filepath)==0)
					{
						strcpy(#notify_message, FILE_SAVED_AS);
						strcat(#notify_message, #filepath);
						strcat(#notify_message, "' -Dt");	
					}
					else
					{
						strcpy(#notify_message, "'Download manager\nError! Can\96t save file as ");
						strcat(#notify_message, #filepath);
						strcat(#notify_message, "' -Et");
					}
					notify(#notify_message);
					DL_address_box.color = DL_address_box.blur_border_color = DL_address_box.focus_border_color = 0xFFFfff;
					DL_Draw_Window();
				}
				ESI = DL_http_transfer;
				downloaded_size = ESI.http_msg.content_received;
				full_size = ESI.http_msg.content_length;
				DrawDownloading();
		}
	}
}

void DrawDownloading()
{
	dword tmp;
	char bytes_received[70], percent[30];

	percent[0]=0;

	if (DL_http_transfer > 0)
	{
		strcpy(#bytes_received, "Downloading... ");	
		tmp = ConvertSizeToKb(downloaded_size);
		strcat(#bytes_received, tmp);
		strcat(#bytes_received, " received.");
		if (full_size>0)
		{
			tmp = itoa(downloaded_size * 100 / full_size);
			strcpy(#percent, tmp);
			strcat(#percent, " \x25");
		}
	} 
	else
	{
		strcpy(#bytes_received, DOWNLOADING_COMPLETE);	
	}
	DrawBar(cleft, 90, DL_Form.cwidth - cleft, 9, sc.work);
	WriteText(cleft, 90, 0x80, sc.work_text, #bytes_received);
	WriteText(DL_Form.cwidth - 50, 90, 0x80, sc.work_text, #percent);
}

void DL_Draw_Window()
{	
	DrawBar(0,0,DL_Form.cwidth,DL_Form.cheight,sc.work); //bg
	if (DL_http_transfer <= 0)
	{
		DrawCaptButton(cleft, 50, 120, 20, 301, sc.work_button, sc.work_button_text, START_DOWNLOADING);	
	}
	else
	{
		DrawCaptButton(cleft, 50, 120, 20, 302, sc.work_button, sc.work_button_text, STOP_DOWNLOADING);
	}
	if (DL_http_transfer <= 0)
	{
		DrawCaptButton(cleft+130, 50, 120, 20, 305, sc.work_button, sc.work_button_text, "Show in folder");
		DrawCaptButton(cleft+260, 50, 120, 20, 306, sc.work_button, sc.work_button_text, "Open file");	
	} 
	WriteText(cleft, DL_address_box.top + 4, 0x80, sc.work_text, "URL:");
	DL_address_box.left = strlen("URL:")*6 + 10 + cleft;
	DL_address_box.width = DL_Form.cwidth - DL_address_box.left - cleft - 3;
	DL_address_box.offset=0;
	edit_box_draw stdcall(#DL_address_box);
	DrawRectangle(DL_address_box.left-1, DL_address_box.top-1, DL_address_box.width+2, 16,DL_address_box.color);
	DrawRectangle(DL_address_box.left-2, DL_address_box.top-2, DL_address_box.width+4, 18,sc.work_graph);

	DrawDownloading();
}

void DL_Scan(int id)
{
	if (id==301) && (DL_http_transfer <= 0) StartDownloading();
	if (id==302) StopDownloading();
	if (id==305) RunProgram("/sys/File managers/Eolite", #save_to);
	if (id==306) RunProgram("@open", #filepath);
}


void StopDownloading()
{
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
		downloaded_size = full_size = 0;
	}
	DL_address_box.color = DL_address_box.blur_border_color = DL_address_box.focus_border_color = 0xFFFfff;
	DL_Draw_Window();
}

void StartDownloading()
{
	StopDownloading();
	if (strncmp(#DL_URL,"http:",5)==0)
	{
		DL_address_box.color = DL_address_box.blur_border_color = DL_address_box.focus_border_color = 0xededed;
		http_get stdcall (#DL_URL, #accept_language);
		DL_http_transfer = EAX;
		DL_Draw_Window();
		if (DL_http_transfer == 0)
		{
			StopDownloading();
			DL_bufsize = 0;
			DL_bufpointer = mem_Free(DL_bufpointer);
			return;
		}
	}
	else
	{
		notify("File adress should starts from http://");
	}
}
