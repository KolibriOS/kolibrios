#ifdef LANG_RUS
	#define DL_WINDOW_HEADER "Менеджер загрузок"
	#define START_DOWNLOADING "Начать закачку"
	#define STOP_DOWNLOADING "Остановить скачивание"
	#define SHOW_IN_FOLDER "Показать в папке"
	#define OPEN_FILE "Открыть файл"
	#define FILE_SAVED_AS "'Менеджер загрузок\nФайл сохранен как "
	#define KB_RECEIVED " получено"
#else
	#define DL_WINDOW_HEADER "Download Manager"
	#define START_DOWNLOADING "Start downloading"
	#define STOP_DOWNLOADING "Stop downloading"
	#define SHOW_IN_FOLDER "Show in folder"
	#define OPEN_FILE "Open file"
	#define FILE_SAVED_AS "'Download manager\nFile saved as "
	#define KB_RECEIVED " received"
#endif

proc_info DL_Form;
system_colors sc;
char DL_URL[10000];
dword DL_bufpointer, DL_bufsize, DL_http_transfer, DL_http_buffer;
char filepath[4096];
int downloaded_size, full_size;
int	mouse_twbi;
edit_box DL_address_box = {250,20,20,0xffffff,0x94AECE,0xffffff,0xffffff,0,sizeof(DL_URL),#DL_URL,#mouse_twbi,2,19,19};
progress_bar DL_progress_bar = {0, 170, 51, 225, 12, 0, 0, 100, 0xFFFfff, 0x74DA00, 0x9F9F9F};

char save_to[4096] = "/tmp0/1/Downloads/";
byte cleft = 15;

byte downloader_opened;

byte download_state;
enum { STATE_NOT_STARTED, STATE_IN_PROGRESS, STATE_COMPLETED };


void Downloader()
{
	int key, btn;
	
	char notify_message[4296];
	
	if (DL_URL[0]) {
		StartDownloading();
	}
	else strcpy(#DL_URL, "http://");
	DL_address_box.size = DL_address_box.pos = DL_address_box.shift = DL_address_box.shift_old = strlen(#DL_URL);

	downloaded_size = full_size = 0;
	downloader_opened = 1;

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
				DefineAndDrawWindow(215, 100, 420, 120, 0x74, sc.work, DL_WINDOW_HEADER, 0);
				GetProcessInfo(#DL_Form, SelfInfo);
				if (DL_Form.status_window>2) break;
				if (DL_Form.height<120) MoveSize(OLD,OLD,OLD,120);
				if (DL_Form.width<280) MoveSize(OLD,OLD,280,OLD);
				DL_Draw_Window();
				break;
				
			default:
				if (DL_Form.width==0) || (DL_http_transfer <= 0) break;
				http_receive stdcall (DL_http_transfer);
				$push EAX
				ESI = DL_http_transfer;
				DL_progress_bar.max = ESI.http_msg.content_length;
				if (DL_progress_bar.value != ESI.http_msg.content_received)
				{
					DL_progress_bar.value = ESI.http_msg.content_received;	
					progressbar_draw stdcall(#DL_progress_bar);
				}
				$pop EAX
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
					download_state = STATE_COMPLETED;
					DL_Draw_Window();
					break;
				}
				ESI = DL_http_transfer;
				downloaded_size = ESI.http_msg.content_received;
				full_size = ESI.http_msg.content_length;
				DrawDownloading();
		}
	}
}



void DL_Draw_Window()
{	
	DrawBar(0,0, DL_Form.cwidth, DL_Form.cheight, sc.work);
	DeleteButton(305);
	DeleteButton(306);
	if (download_state == STATE_NOT_STARTED) ||  (download_state == STATE_COMPLETED)
	{
		DrawCaptButton(cleft, 50, 130, 27, 301, sc.work_button, sc.work_button_text, START_DOWNLOADING);	
	}
	if (download_state == STATE_IN_PROGRESS)
	{
		DrawCaptButton(cleft, 50, 140, 27, 302, sc.work_button, sc.work_button_text, STOP_DOWNLOADING);
		DrawDownloading();
	}
	if (download_state == STATE_COMPLETED)
	{
		DrawCaptButton(cleft+140, 50, 110, 27, 305, sc.work_button, sc.work_button_text, SHOW_IN_FOLDER);
		DrawCaptButton(cleft+260, 50, 120, 27, 306, sc.work_button, sc.work_button_text, OPEN_FILE);	
	} 
	WriteText(cleft, DL_address_box.top + 4, 0x80, sc.work_text, "URL:");
	DL_address_box.left = strlen("URL:")*6 + 10 + cleft;
	DL_address_box.width = DL_Form.cwidth - DL_address_box.left - cleft - 3;
	DL_address_box.offset=0;
	edit_box_draw stdcall(#DL_address_box);
	DrawRectangle(DL_address_box.left-1, DL_address_box.top-1, DL_address_box.width+2, 16, DL_address_box.color);
	DrawRectangle(DL_address_box.left-2, DL_address_box.top-2, DL_address_box.width+4, 18, border_color);
}


void DrawDownloading()
{
	dword tmp;
	char bytes_received[70];

	tmp = ConvertSizeToKb(downloaded_size);
	strcpy(#bytes_received, tmp);
	strcat(#bytes_received, KB_RECEIVED);
	DrawBar(DL_progress_bar.left, DL_progress_bar.top + 17, DL_Form.cwidth - DL_progress_bar.left, 9, sc.work);
	WriteText(DL_progress_bar.left, DL_progress_bar.top + 17, 0x80, sc.work_text, #bytes_received);
	progressbar_draw stdcall(#DL_progress_bar);
}




void DL_Scan(int id)
{
	if (id==001) {
		downloader_opened=0; 
		StopDownloading();
		ExitProcess(); 
	}
	if (id==301) && (DL_http_transfer <= 0) StartDownloading();
	if (id==302) StopDownloading();
	if (id==305) RunProgram("/sys/File managers/Eolite", #save_to);
	if (id==306) RunProgram("@open", #filepath);
}


void StopDownloading()
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
		download_state = STATE_IN_PROGRESS;
		DL_address_box.color = DL_address_box.blur_border_color = DL_address_box.focus_border_color = 0xdddDDD;
		http_get stdcall (#DL_URL, 0, 0, #accept_language);
		DL_http_transfer = EAX;
		DL_progress_bar.value = 0;
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
