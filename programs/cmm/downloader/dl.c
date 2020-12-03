#define MEMSIZE 1024 * 100
//Copyright 2020 by Leency
#include "../lib/gui.h"
#include "../lib/random.h"
#include "../lib/obj/box_lib.h"
#include "../lib/obj/http.h"

#include "const.h"

dword bufpointer, bufsize;
bool exit_param = false;

_http http;

checkbox autoclose = { T_AUTOCLOSE, false }; 

char main_url[URL_SIZE];
char filepath[URL_SIZE+96];

progress_bar pb = {0, GAPX, 58, 315, 17, 0, NULL, NULL, 0xFFFfff, 0x74DA00, NULL};
edit_box ed = {WIN_W-GAPX-GAPX,GAPX,20,0xffffff,0x94AECE,0xffffff,0xffffff,
	0x10000000, sizeof(main_url)-2,#main_url,0,ed_focus,19,19};


void main()  
{
	dword shared_url;
	load_dll(boxlib,  #box_lib_init,0);
	load_dll(libHTTP, #http_lib_init,1);

	if (!dir_exists(#save_to)) CreateDir(#save_to);
	SetCurDir(#save_to);

	if (param) {
		if (!strncmp(#param, "-exit ", 6)) {
			exit_param = true;
			param += 6;
		}

		if (!strncmp(#param, "-mem", 5)) {
			//shared_url = memopen(#dl_shared, URL_SIZE+1, SHM_OPEN + SHM_WRITE);
			strcpy(#main_url, shared_url);
		} else {
			strcpy(#main_url, #param);
		}
	}
	if (main_url[0]) StartDownloading(); else strcpy(#main_url, "http://");
	EditBox_UpdateText(#ed, #main_url);
 
	@SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER + EVM_STACK);
	@SetWindowLayerBehaviour(-1, ZPOS_ALWAYS_TOP);
	loop() switch(@WaitEvent())
	{
		case evMouse:  edit_box_mouse stdcall (#ed); break;
		case evButton: ProcessButtonClick(); break;
		case evKey:    ProcessKeyPress(); break;
		case evReDraw: DrawWindow(); break;
		default:       MonitorProgress();
	}
}
 
void ProcessButtonClick()
{
	int id = @GetButtonID();
	autoclose.click(id);
	if (id==BTN_EXIT) { StopDownloading(); ExitProcess(); }
	if (id==BTN_START) StartDownloading();
	if (id==BTN_STOP) StopDownloading();
	if (id==BTN_DIR) RunProgram("/sys/File managers/Eolite", #filepath);
	if (id==BTN_RUN) RunProgram("/sys/@open", #filepath);
}

void ProcessKeyPress()
{
	@GetKey(); 
	edit_box_key stdcall(#ed); 
	EAX >>= 16;
	if (AL == SCAN_CODE_ENTER) StartDownloading();
	if (AL == SCAN_CODE_ESC) StopDownloading();
}
 
void DrawWindow()
{  
	sc.get();
	pb.frame_color = sc.work_dark;
	DefineAndDrawWindow(110 + random(300), 100 + random(300), WIN_W+9, 
		WIN_H + 5 + skin_height, 0x34, sc.work, DL_WINDOW_HEADER, 0);

	#define BUT_Y 58;
	//autoclose.draw(WIN_W-135, BUT_Y+6);
	if (!http.transfer)
	{
		DrawStandartCaptButton(GAPX, BUT_Y, BTN_START, T_DOWNLOAD);   
		if (filepath)
		{
			DrawStandartCaptButton(GAPX+102, BUT_Y, BTN_DIR, T_OPEN_DIR);
			DrawStandartCaptButton(GAPX+276, BUT_Y, BTN_RUN, T_RUN);  
		}
	} else {
		DrawStandartCaptButton(WIN_W - 240, BUT_Y, BTN_STOP, T_CANCEL);
		DrawDownloadingProgress();
	}
	//ed.offset=0; //DEL?
	DrawEditBox(#ed);
}
 
void StartDownloading()
{
	char proxy_url[URL_SIZE];
	if (http.transfer > 0) return;
	filepath = '\0';
	if (!strncmp(#main_url,"https:",6)) {
		miniprintf(#proxy_url, "http://gate.aspero.pro/?site=%s", #main_url);
		strcpy(#main_url, #proxy_url);
		EditBox_UpdateText(#ed, #main_url);
		//notify("'HTTPS for download is not supported, trying to download the file via HTTP' -W");
		//miniprintf(#http_url, "http://%s", #main_url+8);
		//if (!downloader.Start(#http_url)) {
		//	notify("'Download failed.' -E");
		//	StopDownloading();
		//}
	}
	if (http.get(#main_url)) {
		ed.blur_border_color = 0xCACACA;
		ed.flags = 100000000000b;
		pb.value = 0;
		DrawWindow();
	} else {
		notify(T_ERROR_STARTING_DOWNLOAD);
		StopDownloading();
		if (exit_param) ExitProcess();
	}
}


void DrawDownloadingProgress()
{
	char bytes_received[70];

	EDI = http.content_received / 100;
	if (pb.value == EDI) return;

	pb.value = EDI;
	pb.max = http.content_length / 100;
	progressbar_draw stdcall(#pb);
	miniprintf(#bytes_received, KB_RECEIVED, ConvertSizeToKb(http.content_received) );
	WriteTextWithBg(GAPX, pb.top + 22, 0xD0, sc.work_text, #bytes_received, sc.work);
	//CalculateSpeed();
}
 
void StopDownloading()
{
	if (http.stop()) {
		bufsize = 0;
		bufpointer = free(bufpointer);
	}
	http.content_received = http.content_length = 0;

	ed.blur_border_color = 0xFFFfff;
	ed.flags = 10b;
	DrawWindow();
}

void MonitorProgress() 
{
	char redirect_url[URL_SIZE];
	if (http.transfer <= 0) return;
	http.receive();
	if (!http.content_length) http.content_length = http.content_received * 20; //MOVE?

	if (http.receive_result == 0) {
		if (http.status_code >= 300) && (http.status_code < 400)
		{
			http.header_field("location\0", #redirect_url, URL_SIZE);
			get_absolute_url(#redirect_url, #main_url);
			strcpy(#main_url, #redirect_url);
			EditBox_UpdateText(#ed, #main_url);
			StopDownloading();
			StartDownloading();
			return;
		}
		bufpointer = http.content_pointer;
		bufsize = http.content_received;
		http.hfree();
	}

	DrawDownloadingProgress();

	if (!http.receive_result)
	{
		SaveFile();
		if (exit_param) ExitProcess();
		StopDownloading();
		DrawWindow();
		return;
	}     
}

void SaveFile()
{
	int i;
	char aux[2048];
	char notify_message[4296];

	//char file_name[URL_SIZE];
	//Content-Disposition: attachment; filename="RealFootball_2018_Nokia_5800_EN_IGP_EU_TS_101.zip"
	//header_field("Content-Disposition\0", #redirect_url, URL_SIZE);

	// Clean all slashes at the end
	strcpy(#aux, #main_url);
	while (aux[strlen(#aux)-1] == '/') {
		aux[strlen(#aux)-1] = 0;
	}

	strcpy(#filepath, #save_to);
	chrcat(#filepath, '/');
	strcat(#filepath, #aux+strrchr(#aux, '/'));
	
	for (i=0; i<strlen(#filepath); i++) if(filepath[i]==':')||(filepath[i]=='?')filepath[i]='-';

	if (CreateFile(http.content_received, bufpointer, #filepath)==0) {
		miniprintf(#notify_message, FILE_SAVED_AS, #filepath);
	} else {
		miniprintf(#notify_message, FILE_NOT_SAVED, #filepath);
	}
	
	if (!exit_param) notify(#notify_message);
	if (autoclose.checked) ExitProcess();
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
			speed = http.content_received - netdata_received / time.gone * 100;
			debugval("speed", speed);
			debugln(ConvertSizeToKb(speed) );
			time.old = time.cur;
			netdata_received = http.content_received;
		}
	}
	else time.old = time.cur;
}
*/