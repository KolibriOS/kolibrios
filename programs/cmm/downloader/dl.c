#define MEMSIZE 1024 * 40
//Copyright 2020 - 2021 by Leency
#include "../lib/gui.h"
#include "../lib/random.h"
#include "../lib/obj/box_lib.h"
#include "../lib/obj/http.h"

#include "const.h"

bool open_file = false;

dword speed;

_http http;

checkbox autoclose = { T_AUTOCLOSE, false }; 

char uEdit[URL_SIZE];
char filepath[4096];
char save_dir[4096];

char* active_status;

edit_box ed = {WIN_W-GAPX-GAPX,GAPX,20,0xffffff,0x94AECE,0xffffff,0xffffff,
	0x10000000, sizeof(uEdit)-2, #uEdit, 0, ed_focus + ed_always_focus, 19, 19};
progress_bar pb = {0, GAPX, 52, WIN_W - GAPX - GAPX, 17, 0, NULL, NULL, 
	0xFFFfff, 0, NULL};


void main()  
{
	dword shared_url;
	load_dll(boxlib,  #box_lib_init,0);
	load_dll(libHTTP, #http_lib_init,1);

	strcpy(#save_dir, DEFAULT_SAVE_DIR);
	if (!dir_exists(#save_dir)) CreateDir(#save_dir);
	SetCurDir(#save_dir);
	sc.get();
	InitDownload();

	if (param) {
		if (streqrp(#param, "-e ")) {
			autoclose.checked = true;
			param += 3;
		}
		if (streqrp(#param, "-eo ")) {
			autoclose.checked = true;
			open_file = true;
			param += 4;
		}

		if (!strncmp(#param, "-mem", 5)) {
			//shared_url = memopen(#dl_shared, URL_SIZE+1, SHM_OPEN + SHM_WRITE);
			strcpy(#uEdit, shared_url);
		} else {
			strcpy(#uEdit, #param);
		}

		if (streq(#param, "-test")) {
			strcpy(#uEdit, URL_SPEED_TEST);
		}
	}
	if (uEdit[0]) StartDownloading(); else {
		edit_box_set_text stdcall (#ed, "http://");
	}
 
	@SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER + EVM_STACK);
	loop() switch(@WaitEvent())
	{
		case evMouse:  edit_box_mouse stdcall (#ed); break;
		case evButton: ProcessButtonClick(@GetButtonID()); break;
		case evKey:    ProcessKeyPress(); break;
		case evReDraw: DefineWindow(); break;
		default:       MonitorProgress();
	}
}
 
void ProcessButtonClick(int id)
{
	autoclose.click(id);
	switch (id) {
		case BTN_EXIT:
			StopDownloading(); 
			ExitProcess();
		case BTN_START:
			StartDownloading();
			break;
		case BTN_STOP:
			StopDownloading();
			InitDownload();
			DrawWindow();
			break;
		case BTN_NEW:
			InitDownload();
			EditBox_UpdateText(#ed, ed_focus + ed_always_focus);
			DrawWindow();
			break;
		case BTN_DIR:
			RunProgram("/sys/File managers/Eolite", #filepath);
			break;
		case BTN_RUN:
			RunProgram("/sys/@open", #filepath);
			break;
	}
}

void ProcessKeyPress()
{
	@GetKey(); 
	edit_box_key stdcall(#ed); 
	EAX >>= 16;
	if (AL == SCAN_CODE_ENTER) ProcessButtonClick(BTN_START);
	if (AL == SCAN_CODE_ESC) ProcessButtonClick(BTN_STOP);
}


void DrawDlButton(int x, id, t)
{
	DrawCaptButton(x, 102, BUT_W, 24, id, sc.button, sc.button_text, t);
}
 
void DefineWindow()
{  
	sc.get();
	pb.frame_color = ed.border_color = ed.focus_border_color = sc.line;
	DefineAndDrawWindow(110 + random(300), 100 + random(300), WIN_W+9, 
		WIN_H + 5 + skin_h, 0x34, sc.work, DL_WINDOW_HEADER, 0);
	DrawWindow();
}

void DrawWindow()
{
	#define BUT_Y 58;
	if (!http.transfer) && (!filepath) {
		DrawBar(GAPX, 102, WIN_W - GAPX - GAPX+1, 24+1, sc.work);
		DeleteButton(BTN_DIR);
		DeleteButton(BTN_NEW);
		DrawDlButton(WIN_W-BUT_W/2, BTN_START, T_DOWNLOAD);
		autoclose.disabled = false;
	} else if (http.transfer) {
		DrawDlButton(WIN_W-BUT_W/2, BTN_STOP, T_CANCEL);
		DrawDownloadingProgress();
	} else if (!http.transfer) && (filepath) {
		autoclose.disabled = true;
		DrawDlButton(GAPX, BTN_RUN, T_RUN);
		DrawDlButton(WIN_W-BUT_W/2, BTN_DIR, T_OPEN_DIR);
		DrawDlButton(WIN_W-BUT_W-GAPX, BTN_NEW, T_NEW);  
	}

	WriteTextWithBg(GAPX, pb.top + 22, 0xD0, sc.work_text, active_status, sc.work);

	DrawBar(GAPX, WIN_H - 58, WIN_W - GAPX - GAPX, 1, sc.light);
	DrawBar(GAPX, WIN_H - 58 + 1, WIN_W - GAPX - GAPX, 1, sc.dark);
	WriteText(GAPX, WIN_H - 48, 0x90, sc.work_text, T_SAVE_TO);
	WriteTextWithBg(GAPX + 108, WIN_H - 48, 0xD0, sc.work_text, #save_dir, sc.light);
	edit_box_draw stdcall (#ed);
	progressbar_draw stdcall(#pb);
	autoclose.draw(GAPX, WIN_H - 24);
}
 
void StartDownloading()
{
	char get_url[URL_SIZE+33];
	if (http.transfer > 0) return;
	ResetDownloadSpeed();
	pb.back_color = 0xFFFfff;
	if (!strncmp(#uEdit,"https:",6)) {
		//miniprintf(#get_url, "http://gate.aspero.pro/?site=%s", #uEdit);
		notify("'HTTPS for download temporary is not supported,\ntrying to download the file via HTTP' -W");
		miniprintf(#uEdit, "http://%s", #uEdit+8);
	}
	strcpy(#get_url, #uEdit);

	if (http.get(#get_url)) {
		pb.value = 0;
		pb.progress_color = PB_COL_PROGRESS;
		EditBox_UpdateText(#ed, ed_disabled);
		DrawWindow();
	} else {
		pb.progress_color = PB_COL_ERROR;
		notify(T_ERROR_STARTING_DOWNLOAD);
		StopDownloading();
		EditBox_UpdateText(#ed, ed_focus + ed_always_focus);
		DrawWindow();
	}
}

void DrawDownloadingProgress()
{
	char received_status[70];
	dword gotkb = http.content_received/1024;

	EDI = http.content_received / 100;
	if (pb.value == EDI) return;

	pb.value = EDI;
	pb.max = http.content_length / 100;
	progressbar_draw stdcall(#pb);
	CalculateDownloadSpeed();

	//sprintf gets too much space
	//sprintf(#received_status, T_STATUS_DOWNLOADING, gotkb/1024, gotkb%1024/103, speed);
	
	strcpy(#received_status, T_STATUS_DL_P1);
	strcat(#received_status, itoa(gotkb/1024));
	strcat(#received_status, ".");
	strcat(#received_status, itoa(gotkb%1024/103));
	strcat(#received_status, T_STATUS_DL_P2);
	strcat(#received_status, itoa(speed));
	strcat(#received_status, T_STATUS_DL_P3);
	
	active_status = #received_status;
	WriteTextWithBg(GAPX, pb.top + 22, 0xD0, sc.work_text, active_status, sc.work);
}
 
void StopDownloading()
{
	http.stop();
	if (http.content_pointer) http.content_pointer = free(http.content_pointer);
	http.content_received = http.content_length = 0;
}

void InitDownload()
{
	pb.value = 0;
	pb.back_color = sc.light;
	pb.progress_color = PB_COL_PROGRESS;
	filepath = '\0';
	active_status = T_STATUS_READY;
}

void MonitorProgress() 
{
	char redirect_url[URL_SIZE];
	if (http.transfer <= 0) return;
	http.receive();
	if (!http.content_length) http.content_length = http.content_received * 20; //MOVE?

	if (http.receive_result) {
		DrawDownloadingProgress();
	} else {
		if (http.status_code >= 300) && (http.status_code < 400) {
			http.header_field("location", #redirect_url, URL_SIZE);
			get_absolute_url(#redirect_url, #uEdit);
			edit_box_set_text stdcall (#ed, #redirect_url);
			StopDownloading();
			StartDownloading();
			return;
		} else {
			CompleteDownload();
		}
	}
}

void CompleteDownload()
{
	SaveFile(0);
	if (autoclose.checked) ExitProcess();
	pb.value = pb.max;
	pb.progress_color = PB_COL_COMPLETE;
	StopDownloading();
	active_status = T_STATUS_COMPLETE;
	DrawWindow();	
}

void SaveFile(int attempt)
{
	int i, fi=0;
	char notify_message[4296];
	char file_name[URL_SIZE+96];


	strcpy(#filepath, #save_dir);
	chrcat(#filepath, '/');      ///need to check the last symbol

	if (attempt > 9) {
		notify("'Too many saving attempts' -E");
		return;
	}

	if (attempt > 0) { 
		chrcat(#filepath, attempt+'0'); 
		chrcat(#filepath, '_'); 
	}

	//Content-Disposition: attachment; filename="RealFootball_2018_Nokia_5800_EN_IGP_EU_TS_101.zip"
	if (http.header_field("content-disposition", #file_name, sizeof(file_name))) {
		if (EDX = strstr(#file_name,"filename=\"")) { 
			strcat(#filepath, EDX+10);
			ESBYTE[strchr(#filepath,'\"')] = '\0';
		}
	} else {
		// Clean all slashes at the end
		strcpy(#file_name, #uEdit);
		while (file_name[strlen(#file_name)-1] == '/') {
			file_name[strlen(#file_name)-1] = 0;
		}
		strcat(#filepath, #file_name+strrchr(#file_name, '/'));	
	}
	
	for (i=0; i<strlen(#filepath); i++) if(filepath[i]==':')||(filepath[i]=='?')filepath[i]='-';

	while (file_exists(#filepath)) {
		SaveFile(attempt+1);
		return;
	} 

	if (CreateFile(http.content_received, http.content_pointer, #filepath)==0) {
		miniprintf(#notify_message, FILE_SAVED_AS, #filepath);
	} else {
		miniprintf(#notify_message, FILE_NOT_SAVED, #filepath);
	}

	if (!autoclose.checked) notify(#notify_message);


	if (!strcmpi(#filepath+strlen(#filepath)-4, ".zip"))
	|| (!strcmpi(#filepath+strlen(#filepath)-3, ".7z")) {
		Unarchive(#filepath);
	}

	if (open_file) ProcessButtonClick(BTN_RUN);
	if (autoclose.checked) ExitProcess();
}

void Unarchive(dword _arc)
{
	char folder_name[4096];
	strcpy(#folder_name, #save_dir);
	strcpy(#folder_name, #filepath+strrchr(#filepath, '/'));
	folder_name[strlen(#folder_name)-4] = '\0';
	CreateDir(#folder_name);

	//miniprintf2(#param, "-o \"%s\" -h \"%s", #folder_name, #filepath);
	strcpy(#param, "-o \"");
	strcat(#param, #folder_name);
	strcat(#param, "\" -h \"");
	strcat(#param, #filepath);
	chrcat(#param, '\"');
	RunProgram("/sys/unz", #param);	
}


struct TIME
{
	dword old;
	dword cur;
	dword gone;
} time = {0,0,0};

dword netdata_received;

void ResetDownloadSpeed()
{
	time.old = 0;
	netdata_received = 0;
}

void CalculateDownloadSpeed()
{
	time.cur = GetStartTime();

	if (time.old) {
		time.gone = time.cur - time.old;
		if (time.gone >= 200) {
			speed = http.content_received - netdata_received / time.gone / 10;
			time.old = time.cur;
			netdata_received = http.content_received;
		}
	} else { 
		time.old = time.cur;
	}
}