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
char filepath[4096];
int	mouse_twbi;
edit_box dl_edit = {250,20,20,0xffffff,0x94AECE,0xffffff,0xffffff,0,sizeof(DL_URL),#DL_URL,#mouse_twbi,2,19,19};
progress_bar DL_progress_bar = {0, 170, 51, 225, 12, 0, 0, 100, 0xFFFfff, 0x74DA00, 0x9F9F9F};

char save_to[4096] = "/tmp0/1/Downloads/";
byte cleft = 15;

byte downloader_opened;


void Downloader()
{
	int key;
	char notify_message[4296];
	downloader_opened = 1;
	SetEventMask(0x27);
	
	if (DL_URL[0]) StartDownloading(); else strcpy(#DL_URL, "http://");
	dl_edit.size = dl_edit.pos = dl_edit.shift = dl_edit.shift_old = strlen(#DL_URL);

	Downloading_SetDefaults();

	loop()
	{
		WaitEventTimeout(40);
		switch(EAX & 0xFF)
		{
			CASE evMouse:
				if (!CheckActiveProcess(DL_Form.ID)) break;
				edit_box_mouse stdcall (#dl_edit);
				break;

			case evButton:
				DL_Scan(GetButtonID());
				break;

			case evKey:
				key = GetKey();
				EAX=key<<8; 
				edit_box_key stdcall(#dl_edit);
				if (key==13) DL_Scan(301);
				break;

			case evReDraw:
				system.color.get();
				DefineAndDrawWindow(215, 100, 420, 120, 0x74, system.color.work, DL_WINDOW_HEADER, 0);
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
					Downloading_Completed();
					strcpy(#filepath, #save_to);
					strcat(#filepath, #DL_URL+strrchr(#DL_URL, '/'));
					if (WriteFile(DL_bufsize, DL_bufpointer, #filepath)==0)
					{
						sprintf(#notify_message, "%s%s%s",FILE_SAVED_AS,#filepath,"' -Dt");
					}
					else
					{
						sprintf(#notify_message, "%s%s%s","'Download manager\nError! Can\96t save file as ",#filepath,"' -Et");
					}
					notify(#notify_message);
					dl_edit.blur_border_color = 0xFFFfff;
					dl_edit.flags = 10b;
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
	DrawBar(0,0, DL_Form.cwidth, DL_Form.cheight, system.color.work);
	DeleteButton(305);
	DeleteButton(306);
	if (download_state == STATE_NOT_STARTED) ||  (download_state == STATE_COMPLETED)
	{
		DrawCaptButton(cleft, 50, 130, 27, 301, system.color.work_button, system.color.work_button_text, START_DOWNLOADING);	
	}
	if (download_state == STATE_IN_PROGRESS)
	{
		DrawCaptButton(cleft, 50, 140, 27, 302, system.color.work_button, system.color.work_button_text, STOP_DOWNLOADING);
		DrawDownloading();
	}
	if (download_state == STATE_COMPLETED)
	{
		DrawCaptButton(cleft+140, 50, 110, 27, 305, system.color.work_button, system.color.work_button_text, SHOW_IN_FOLDER);
		DrawCaptButton(cleft+260, 50, 120, 27, 306, system.color.work_button, system.color.work_button_text, OPEN_FILE);	
	} 
	WriteText(cleft, dl_edit.top + 4, 0x80, system.color.work_text, "URL:");
	dl_edit.left = strlen("URL:")*6 + 10 + cleft;
	dl_edit.width = DL_Form.cwidth - dl_edit.left - cleft - 3;
	dl_edit.offset=0;
	edit_box_draw stdcall(#dl_edit);
	DrawRectangle(dl_edit.left-1, dl_edit.top-1, dl_edit.width+2, 16, dl_edit.blur_border_color);
	DrawRectangle(dl_edit.left-2, dl_edit.top-2, dl_edit.width+4, 18, border_color);
}


void DrawDownloading()
{
	dword tmp;
	char bytes_received[70];

	tmp = ConvertSizeToKb(downloaded_size);
	strcpy(#bytes_received, tmp);
	strcat(#bytes_received, KB_RECEIVED);
	DrawBar(DL_progress_bar.left, DL_progress_bar.top + 17, DL_Form.cwidth - DL_progress_bar.left, 9, system.color.work);
	WriteText(DL_progress_bar.left, DL_progress_bar.top + 17, 0x80, system.color.work_text, #bytes_received);
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
	Downloading_Stop();
	dl_edit.blur_border_color = 0xFFFfff;
	dl_edit.flags = 10b;
	DL_Draw_Window();
}

void StartDownloading()
{
	StopDownloading();
	if (strncmp(#DL_URL,"http:",5)==0)
	{
		Downloading_Start();
		dl_edit.blur_border_color = 0xCACACA;
		dl_edit.flags = 100000000000b;
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
