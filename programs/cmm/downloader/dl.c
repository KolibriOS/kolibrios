//Copyright 2020 by Leency

#define MEMSIZE 1024 * 100
#include "../lib/gui.h"
#include "../lib/random.h"
#include "../lib/obj/box_lib.h"
#include "../lib/obj/http.h"
#include "../lib/patterns/http_downloader.h"

#include "const.h"

DOWNLOADER downloader;

char downloader_edit[4000];
char filepath[4096];
edit_box ed = {WIN_W-GAPX-GAPX,GAPX,20,0xffffff,0x94AECE,0xffffff,0xffffff,0x10000000,
	sizeof(downloader_edit)-2,#downloader_edit,0,ed_focus,19,19};
progress_bar pb = {0, GAPX, 58, 350, 17, 0, 0, 100, 0xFFFfff, 0x74DA00, 0x9F9F9F};
//progress_bar: value, left, top, width, height, style, min, max, back_color, progress_color, frame_color;
 
bool exit_when_done = false; 

 
void main()  
{
	dword shared_url;
	load_dll(boxlib,  #box_lib_init,0);
	load_dll(libHTTP, #http_lib_init,1);

	if (!dir_exists(#save_to)) CreateDir(#save_to);

	if (param) {
		if (!strncmp(#param, "-exit ", 6)) {
			exit_when_done = true;
			param += 6;
		}

		if (!strncmp(#param, "-mem", 5)) {
			shared_url = memopen(#dl_shared, URL_SIZE+1, SHM_OPEN + SHM_WRITE);
			strcpy(#downloader_edit, shared_url);
		} else {
			strcpy(#downloader_edit, #param);
		}
	} 
	if (downloader_edit[0]) StartDownloading(); else strcpy(#downloader_edit, "http://");
	ed.size = ed.pos = ed.shift = ed.shift_old = strlen(#downloader_edit);
 
	@SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER + EVM_STACK);
	@SetWindowLayerBehaviour(-1, ZPOS_ALWAYS_TOP);
	loop() switch(@WaitEvent())
	{
		case evMouse:
			edit_box_mouse stdcall (#ed);
			break;

		case evButton:
			ProcessEvent(GetButtonID());
			break;

		case evKey:
			GetKeys();
			edit_box_key stdcall(#ed);
			if (key_scancode==SCAN_CODE_ENTER) ProcessEvent(301);
			break;

		case evReDraw:
			DrawWindow();
			break;
		   
		default:
			if (!downloader.MonitorProgress()) break;
			pb.max = downloader.httpd.content_length / 100;
			EDI = downloader.httpd.content_received/100;
			if (pb.value != EDI)
			{
				pb.value = EDI;
				progressbar_draw stdcall(#pb);
				DrawDownloading();
			}
			if (downloader.state == STATE_COMPLETED)
			{
				SaveDownloadedFile();
				if (exit_when_done) ExitProcess();
				StopDownloading();
				DrawWindow();
				break;
			}          
	}
}
 
void ProcessEvent(int id)
{
	if (id==001) { StopDownloading(); ExitProcess(); }
	if (id==301) && (downloader.httpd.transfer <= 0) StartDownloading();
	if (id==302) StopDownloading();
	if (id==305) RunProgram("/sys/File managers/Eolite", #filepath);
	if (id==306) {
		SetCurDir(#save_to);
		RunProgram("/sys/@open", #filepath);
	}
}
 
void DrawWindow()
{  
	int but_x = 0;
	int but_y = 58;

	sc.get();
	pb.frame_color = sc.work_dark;
	DefineAndDrawWindow(110 + random(300), 100 + random(300), WIN_W+9, WIN_H + 5 + skin_height, 0x34, sc.work, DL_WINDOW_HEADER, 0);

	if (downloader.state == STATE_NOT_STARTED) || (downloader.state == STATE_COMPLETED)
	{
		but_x = GAPX + DrawStandartCaptButton(GAPX, but_y, 301, START_DOWNLOADING);   
		if (filepath[0])
		{
			but_x += DrawStandartCaptButton(but_x, but_y, 305, SHOW_IN_FOLDER);
			DrawStandartCaptButton(but_x, but_y, 306, OPEN_FILE_TEXT);  
		}
	}
	if (downloader.state == STATE_IN_PROGRESS)
	{
		DrawStandartCaptButton(WIN_W - 190, but_y, 302, STOP_DOWNLOADING);
		DrawDownloading();
	}
	ed.offset=0;
	DrawEditBox(#ed);
}
 
void StartDownloading()
{
	char http_url[URL_SIZE];
	char proxy_url[URL_SIZE];
	StopDownloading();
	if (!strncmp(#downloader_edit,"https://",7)) {
		notify("'HTTPS for download is not supported, trying to download the file via HTTP' -W");
		miniprintf(#http_url, "http://%s", #downloader_edit+8);
		if (!downloader.Start(#http_url)) {
			notify("'Download failed.' -E");
			StopDownloading();
		}
		//sprintf(#proxy_url, "http://gate.aspero.pro/?site=%s", #downloader_edit);
		//if (!downloader.Start(#proxy_url)) {
		//	notify("'Download failed.' -E");
		//	StopDownloading();
		//}
		DrawWindow();
		return;
	}
	if (!downloader.Start(#downloader_edit)) {
		if (exit_when_done) ExitProcess();
		notify(T_ERROR_STARTING_DOWNLOAD);
		StopDownloading();
		return;
	}
	ed.blur_border_color = 0xCACACA;
	ed.flags = 100000000000b;
	pb.value = 0;
	DrawWindow();
}

/*
struct TIME
{
	dword old;
	dword cur;
	dword gone;
} time = {0,0,0};

dword netdata_received;
dword speed;

void CalculateSpeed()
{
	time.cur = GetStartTime();

	if (time.old) {
		time.gone = time.cur - time.old;
		if (time.gone > 200) {
			speed = downloader.httpd.content_received - netdata_received / time.gone * 100;
			debugval("speed", speed);
			debugln(ConvertSizeToKb(speed) );
			time.old = time.cur;
			netdata_received = downloader.httpd.content_received;
		}
	}
	else time.old = time.cur;
}
*/
 
void DrawDownloading()
{
	char bytes_received[70];
	miniprintf(#bytes_received, KB_RECEIVED, ConvertSizeToKb(downloader.httpd.content_received) );
	WriteTextWithBg(GAPX, pb.top + 22, 0xD0, sc.work_text, #bytes_received, sc.work);
	//CalculateSpeed();
	progressbar_draw stdcall(#pb);
}
 
void StopDownloading()
{
	downloader.Stop();
	ed.blur_border_color = 0xFFFfff;
	ed.flags = 10b;
	DrawWindow();
}

void SaveDownloadedFile()
{
	int i;
	char aux[2048];
	char notify_message[4296];

	// Clean all slashes at the end
	strcpy(#aux,  #downloader_edit);
	while (aux[strlen(#aux)-1] == '/') {
		aux[strlen(#aux)-1] = 0;
	}

	//miniprintf(#filepath, "%s/", #save_to);
	strcpy(#filepath, #save_to);
	chrcat(#filepath, '/');
	strcat(#filepath, #aux+strrchr(#aux, '/'));
	
	for (i=0; i<strlen(#filepath); i++) if(filepath[i]==':')||(filepath[i]=='?')filepath[i]='-';

	if (CreateFile(downloader.httpd.content_received, downloader.bufpointer, #filepath)==0) {
		miniprintf(#notify_message, FILE_SAVED_AS, #filepath);
	} else {
		miniprintf(#notify_message, FILE_NOT_SAVED, #filepath);
	}

	/*
	if (CreateFile(downloader.httpd.content_received, downloader.bufpointer, #filepath)==0) {
		strcpy(#notify_message, "'Download complete' -Dt");
	} else {
		strcpy(#notify_message, "'Error saving downloaded file!' -Et");
	}
	*/
	
	if (!exit_when_done) notify(#notify_message);
}