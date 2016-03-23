#ifdef LANG_RUS
	#define DL_WINDOW_HEADER "Менеджер загрузок"
	#define START_DOWNLOADING "Начать закачку"
	#define STOP_DOWNLOADING "Остановить"
	#define SHOW_IN_FOLDER "Показать в папке"
	#define OPEN_FILE_TEXT "Открыть файл"
	#define FILE_SAVED_AS "'Менеджер загрузок\nФайл сохранен как "
	#define KB_RECEIVED " получено"
#else
	#define DL_WINDOW_HEADER "Download Manager"
	#define START_DOWNLOADING "Start downloading"
	#define STOP_DOWNLOADING "Stop downloading"
	#define SHOW_IN_FOLDER "Show in folder"
	#define OPEN_FILE_TEXT "Open file"
	#define FILE_SAVED_AS "'Download manager\nFile saved as "
	#define KB_RECEIVED " received"
#endif
char save_to[4096] = "/tmp0/1/Downloads";

proc_info DL_Form;
char downloader_edit[10000];
char filepath[4096];
int	mouse_twbi;
edit_box ed = {250,20,20,0xffffff,0x94AECE,0xffffff,0xffffff,0,sizeof(downloader_edit),#downloader_edit,#mouse_twbi,2,19,19};
progress_bar pb = {0, 170, 51, 225, 12, 0, 0, 100, 0xFFFfff, 0x74DA00, 0x9F9F9F};

byte downloader_opened;
char downloader_stak[4096];


void Downloader()
{
	int key;
	char notify_message[4296];
	downloader_opened = 1;
	SetEventMask(0x27);
	
	downloader.Stop();
	if (downloader_edit[0]) StartDownloading(); else strcpy(#downloader_edit, "http://");
	ed.size = ed.pos = ed.shift = ed.shift_old = strlen(#downloader_edit);

	loop()
	{
		WaitEventTimeout(40);
		switch(EAX & 0xFF)
		{
			CASE evMouse:
				if (!CheckActiveProcess(DL_Form.ID)) break;
				edit_box_mouse stdcall (#ed);
				break;

			case evButton:
				Key_Scan(GetButtonID());
				break;

			case evKey:
				GetKeys();
				EAX = key_ascii << 8;
				edit_box_key stdcall(#ed);
				if (key_scancode==SCAN_CODE_ENTER) Key_Scan(301);
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
				if (!downloader.MonitorProgress()) break;
				pb.max = downloader.data_full_size;
				if (pb.value != downloader.data_downloaded_size)
				{
					pb.value = downloader.data_downloaded_size;	
					progressbar_draw stdcall(#pb);
					DrawDownloading();
				}
				if (downloader.state == STATE_COMPLETED) 
				{
					if (!dir_exists(#save_to)) CreateDir(#save_to);
					strcpy(#filepath, #save_to);
					chrcat(#filepath, '/');
					strcat(#filepath,  #downloader_edit+strrchr(#downloader_edit, '/'));
					if (WriteFile(downloader.data_downloaded_size, downloader.bufpointer, #filepath)==0)
						sprintf(#notify_message, "%s%s%s",FILE_SAVED_AS,#filepath,"' -Dt");
					else
						sprintf(#notify_message, "%s%s%s","'Download manager\nError! Can\96t save file as ",#filepath,"' -Et");
					notify(#notify_message);
					StopDownloading();
					DL_Draw_Window();
					break;
				}			
		}
	}
}

void Key_Scan(int id)
{
	if (id==001) { downloader_opened=0; StopDownloading(); ExitProcess(); }
	if (id==301) && (downloader.http_transfer <= 0) StartDownloading();
	if (id==302) StopDownloading();
	if (id==305) RunProgram("/sys/File managers/Eolite", #save_to);
	if (id==306) RunProgram("@open", #filepath);
}

void DL_Draw_Window()
{	
	byte cleft = 15;
	DrawBar(0,0, DL_Form.cwidth, DL_Form.cheight, system.color.work);
	DeleteButton(305);
	DeleteButton(306);
	if (downloader.state == STATE_NOT_STARTED) || (downloader.state == STATE_COMPLETED)
	{
		DrawCaptButton(cleft, 50, 140, 27, 301, system.color.work_button, system.color.work_button_text, START_DOWNLOADING);	
	}
	if (downloader.state == STATE_IN_PROGRESS)
	{
		DrawCaptButton(cleft, 50, 140, 27, 302, system.color.work_button, system.color.work_button_text, STOP_DOWNLOADING);
		DrawDownloading();
	}
	if (downloader.state == STATE_COMPLETED)
	{
		DrawCaptButton(cleft+140, 50, 110, 27, 305, system.color.work_button, system.color.work_button_text, SHOW_IN_FOLDER);
		DrawCaptButton(cleft+260, 50, 120, 27, 306, system.color.work_button, system.color.work_button_text, OPEN_FILE_TEXT);	
	} 
	WriteText(cleft, ed.top + 4, 0x80, system.color.work_text, "URL:");
	ed.left = strlen("URL:")*6 + 10 + cleft;
	ed.width = DL_Form.cwidth - ed.left - cleft - 3;
	ed.offset=0;
	//edit_box_draw stdcall(#ed);
	DrawEditBox(#ed);
	//DrawRectangle(ed.left-1, ed.top-1, ed.width+2, 16, ed.blur_border_color);
	//DrawRectangle(ed.left-2, ed.top-2, ed.width+4, 18, border_color);
}

void StartDownloading()
{
	StopDownloading();
	if (strncmp(#downloader_edit,"http://",7)!=0) {
		notify("'File address should start from http://' -E");
		return;
	}
	if (!downloader.Start(#downloader_edit)) {
		notify("'Error while starting download process.\nPlease, check entered path and internet connection.' -E");
		StopDownloading();
		return;
	}
	ed.blur_border_color = 0xCACACA;
	ed.flags = 100000000000b;
	pb.value = 0;
	DL_Draw_Window();
}

void DrawDownloading()
{
	char bytes_received[70];
	dword tmp = ConvertSizeToKb(downloader.data_downloaded_size);
	sprintf(#bytes_received, "%s%s", tmp, KB_RECEIVED);
	DrawBar(pb.left, pb.top + 17, DL_Form.cwidth - pb.left, 9, system.color.work);
	WriteText(pb.left, pb.top + 17, 0x80, system.color.work_text, #bytes_received);
	progressbar_draw stdcall(#pb);
}

void StopDownloading()
{
	downloader.Stop();
	ed.blur_border_color = 0xFFFfff;
	ed.flags = 10b;
	DL_Draw_Window();
}


