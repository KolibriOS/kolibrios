//flac
//do not open multiple threads
//edit list manually

#define MEMSIZE 0xFFFFF

#include "..\lib\obj\proc_lib.h"
#include "..\lib\patterns\simple_open_dialog.h"
char default_dir[] = "/rd/1";
od_filter filter2 = { 8, "MP3\0\0" };

#include "..\lib\file_system.h"
#include "..\lib\list_box.h"
#include "..\lib\gui.h"

#include "..\lib\obj\box_lib.h"
#include "..\lib\obj\libio.h"
#include "..\lib\obj\libimg.h"
#include "..\lib\obj\libini.h"

#include "..\lib\patterns\libimg_load_skin.h"

#define ABOUT_MESSAGE "'Pixies Player v1.33\n\nOpen file: O key\nChange skin: F1/F2
Play/Stop: Space or P key\nStart playing selected file: Enter
Goto next/previous track: Ctrl + Left/Right
Change sound volume: Left/Right key\nMute: M key' -St\n"

scroll_bar scroll1 = { 5,200,398,44,0,2,115,15,0,0xeeeeee,0xBBBbbb,0xeeeeee,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};

proc_info Form;
llist list;

enum {
	BUTTON_WINDOW_CLOSE = 1,
	BUTTON_WINDOW_MINIMIZE,
	BUTTON_WINDOW_REDUCE,
	BUTTON_PLAYBACK_PLAY_PAUSE = 10,
	BUTTON_PLAYBACK_PREV, 
	BUTTON_PLAYBACK_NEXT
};

int player_run_id,
    notify_run_id;

int current_playing_file_n=0;

word win_x_normal, win_y_normal;
word win_x_small, win_y_small;

byte window_mode;
enum {
	WINDOW_MODE_NORMAL,
	WINDOW_MODE_SMALL
};


byte playback_mode;
enum {
	PLAYBACK_MODE_STOPED,
	PLAYBACK_MODE_PLAYING
};

char work_folder[4096],
     current_filename[256];


#include "get_files_list.h"
#include "settings.h"
#include "check_default_player.h"


void OpenFolder(dword path111)
{
	if (ESBYTE[path111]) 
	{
		strcpy(#work_folder, path111);
		work_folder[strrchr(#work_folder, '/')-1]='\0';
		OpenDirectory(#work_folder);
		SetOpenedFileFirst(path111);
	}
	list.SetFont(6, 9, 10000000b);
	list.SetSizes(1, skin.h, skin.w-1, 198, 18);
	if (list.count <= list.visible) 
	{
		list.h = list.count * list.item_h;
		list.visible = list.count;
	}
	else
	{
		list.w -= scroll1.size_x;
	}
	MoveSize(OLD, OLD, OLD, skin.h + list.h);
	list.KeyHome();
	current_playing_file_n=0;
	StopPlayingMp3();
	StartPlayingMp3();	
}

void main()
{
	int id;

	dword tmp_x,tmp_y;
	dword z1,z2;
	
	load_dll(boxlib, #box_lib_init,0);
	load_dll(libio, #libio_init,1);
	load_dll(libimg, #libimg_init,1);
	load_dll(libini, #lib_init,1);
	load_dll(Proc_lib, #OpenDialog_init,0);
	OpenDialog_init stdcall (#o_dialog);

	LoadIniConfig();
	CheckDefaultForTheFirstStart();
	OpenFolder(#param);
	SetEventMask(0100111b);
	loop()
	{
	  WaitEventTimeout(10);
	  switch(EAX & 0xFF) {
	  	case evMouse:
			if (!CheckActiveProcess(Form.ID)) break;
			mouse.get();
			scrollbar_v_mouse (#scroll1);
			if (list.first != scroll1.position)
			{
				list.first = scroll1.position;
				DrawPlayList();
				break;
			}
			if (list.MouseOver(mouse.x, mouse.y))
			{
				if (mouse.vert) if (list.MouseScroll(mouse.vert)) DrawPlayList();
				if (mouse.dblclick) {current_playing_file_n=list.cur_y; StartPlayingMp3();}
				if (mouse.down) && (mouse.key&MOUSE_LEFT) if (list.ProcessMouse(mouse.x, mouse.y)) DrawPlayList();
				if (mouse.down) && (mouse.key&MOUSE_RIGHT) NotifyAndBackFocus(ABOUT_MESSAGE);
			}
			//drag window - emulate windows header
			if(mouse.key&MOUSE_LEFT) && (mouse.y<skin.h) && (mouse.x < 13)
			{
				tmp_x = mouse.x;
				tmp_y = mouse.y;
				do {
					mouse.get();
					if (tmp_x!=mouse.x) || (tmp_y!=mouse.y) 
					{
						z1 = Form.left + mouse.x - tmp_x;
						z2 = Form.top + mouse.y - tmp_y;
						if(z1<=10) || (z1>20000) z1=0; else if(z1>screen.width-Form.width-10)z1=screen.width-Form.width;
						if(z2<=10) || (z2>20000) z2=0; else if(z2>screen.height-Form.height-10)z2=screen.height-Form.height;
						MoveSize(z1 , z2, OLD, OLD);
						draw_window();
					}
					pause(1);
				} while (mouse.lkm);
			}
			break;
		case evButton:
			id=GetButtonID();
			switch(id) {
				case BUTTON_WINDOW_CLOSE:
					StopPlayingMp3();
					SaveIniConfig();
					ExitProcess();
					break;
				case BUTTON_WINDOW_MINIMIZE:
					MinimizeWindow();
					break;
				case BUTTON_WINDOW_REDUCE:
					if (window_mode == WINDOW_MODE_NORMAL)
					{
						window_mode = WINDOW_MODE_SMALL;
						win_x_normal = Form.left;
						win_y_normal = Form.top;
						MoveSize(OLD, OLD, 99, skin.h - 1);
						MoveSize(OLD, win_y_small, OLD, OLD);
						MoveSize(win_x_small, OLD, OLD, OLD);
					}
					else
					{
						window_mode = WINDOW_MODE_NORMAL;
						win_x_small = Form.left;
						win_y_small = Form.top;
						MoveSize(win_x_normal, win_y_normal, skin.w -1 ,skin.h + list.h);
					}
					break;
				case BUTTON_PLAYBACK_PREV: _PLAY_PREVIOUS:
					current_playing_file_n--;
					StartPlayingMp3();
					break;
				case BUTTON_PLAYBACK_NEXT: _PLAY_NEXT:
					current_playing_file_n++;
					StartPlayingMp3();
					break;
				case BUTTON_PLAYBACK_PLAY_PAUSE:
					PlayAndPauseClick();
					break;
			}
			break;	  
		case evKey:
			GetKeys();
			if (key_modifier&KEY_LCTRL) || (key_modifier&KEY_RCTRL) {
				if (key_scancode==SCAN_CODE_LEFT) goto _PLAY_PREVIOUS;
				if (key_scancode==SCAN_CODE_RIGHT) goto _PLAY_NEXT;
				break;
			}		
			if (key_scancode==024) { OpenDialog_start stdcall (#o_dialog); if (o_dialog.status==1) OpenFolder(#openfile_path); }
			if (key_scancode==059) SetColorThemeLight();
			if (key_scancode==060) SetColorThemeDark();
			if (key_scancode==SCAN_CODE_LEFT) RunProgram("@VOLUME", "-");
			if (key_scancode==SCAN_CODE_RIGHT) RunProgram("@VOLUME", "+");
			if (key_scancode==050) RunProgram("@VOLUME", "m");
			if (key_scancode==SCAN_CODE_ENTER) { current_playing_file_n=list.cur_y; StartPlayingMp3(); }
			if (key_scancode==025) || (key_scancode==SCAN_CODE_SPACE) PlayAndPauseClick();
			if (list.ProcessKey(key_scancode)) DrawPlayList();
			break;
		case evReDraw:
			if (window_mode == WINDOW_MODE_NORMAL) DefineAndDrawWindow(win_x_normal, win_y_normal, skin.w - 1, skin.h + list.h, 0x41,0,0,0);
			if (window_mode == WINDOW_MODE_SMALL) DefineAndDrawWindow(win_x_small, win_y_small, 99, skin.h - 1, 0x41,0,0,0);
			draw_window();
			break;
		default:
			if (playback_mode == PLAYBACK_MODE_PLAYING) && (!GetProcessSlot(player_run_id))
			{
				current_playing_file_n++;
				StartPlayingMp3();
			}
	  }
   }
}

void PlayAndPauseClick() 
{
	if (playback_mode == PLAYBACK_MODE_PLAYING) 
	{
		playback_mode = PLAYBACK_MODE_STOPED;
		StopPlayingMp3();
	}
	else
	{
		playback_mode = PLAYBACK_MODE_PLAYING;
		StartPlayingMp3();
	}
}


void DrawPlayList()
{
	int i;
	int yyy;
	char temp_filename[4096];
	for (i=0; i<list.visible; i++;)
	{
		strcpy(#temp_filename, files_mas[i + list.first] * 304 + buf + 72);
		temp_filename[strlen(#temp_filename)-4] = '\0';
		if (strlen(#temp_filename)>47) strcpy(#temp_filename+44, "..."); 
		
		yyy = i*list.item_h+list.y;
		
		//this is selected file
		if (list.cur_y - list.first == i)
		{
			if (i>=list.count) continue;
			DrawBar(list.x, yyy, list.w, list.item_h, theme.color_list_active_bg);
			WriteText(12,yyy+list.text_y,0x80, theme.color_list_active_text, #temp_filename);
		}
		//this is not selected file
		else
		{
			if (i>=list.count) continue;
			DrawBar(list.x,yyy,list.w, list.item_h, theme.color_list_bg);
			WriteText(12,yyy+list.text_y,0x80, theme.color_list_text, #temp_filename);
		}
		//this is cur_y playing file
		if (i + list.first == current_playing_file_n) && (playback_mode == PLAYBACK_MODE_PLAYING)
		{
			WriteText(3, yyy+list.text_y,0x80, theme.color_list_active_pointer, "\x10");
			WriteText(12,yyy+list.text_y,0x80, theme.color_list_active_text, #temp_filename);
		}
	}
	DrawBar(list.x,list.visible * list.item_h + list.y, list.w, -list.visible * list.item_h + list.h, theme.color_list_bg);
	DrawScroller();
}


void StopPlayingMp3() 
{
	if (player_run_id) player_run_id = KillProcess(player_run_id);
	if (notify_run_id) notify_run_id = KillProcess(notify_run_id);
	playback_mode = PLAYBACK_MODE_STOPED;
	DrawTopPanel();
	DrawPlayList();
}


int NotifyAndBackFocus(dword msg)
{
	int nid;
	nid = notify(msg);
	pause(5);
	Form.num_slot = GetProcessSlot(Form.ID);
	if (Form.ID) ActivateWindow(Form.num_slot);
	return nid;
}


void StartPlayingMp3()
{
	word i;
	char item_path[4096];
	char notify_message[512];
	StopPlayingMp3();
	if (!list.count) { NotifyAndBackFocus("'Pixie Player\nPress O key to open MP3 file' -St"); return; }
	if (current_playing_file_n > list.count) { current_playing_file_n = list.count; return; }
	if (current_playing_file_n < 0) { current_playing_file_n = 0; return; }
	playback_mode = PLAYBACK_MODE_PLAYING;
	strlcpy(#current_filename, Getcur_yItemName(), sizeof(current_filename));
	sprintf(#item_path,"\"%s/%s\"",#work_folder,#current_filename);
	DrawPlayList();
	DrawTopPanel();
	if (strcmpi(#item_path+strlen(#item_path)-4,".mp3")) player_run_id = RunProgram(abspath("minimp3"), #item_path);	
	sprintf(#notify_message,"'Now playing:\n%s' -St",#current_filename);
	for (i=2; i<strlen(#notify_message)-6; i++) if (notify_message[i]=='\'') notify_message[i]=96; //replace ' char to avoid @notify misunderstood
	notify_run_id = NotifyAndBackFocus(#notify_message);
}


void draw_window() {
	GetProcessInfo(#Form, SelfInfo);
	DrawTopPanel();
	IF (Form.status_window>=2) return;
	if (window_mode == WINDOW_MODE_NORMAL)
	{
		DrawListBorder(0, skin.h-1, skin.w-1, list.h+1, theme.color_list_border);
		DrawPlayList();
	}
}

void DrawTopPanel()
{
	char cur_y_playing_title[245];
	img_draw stdcall(skin.image, 0, 0, Form.width - 14, skin.h, 0, 0);
	img_draw stdcall(skin.image, Form.width - 14, 0, 15, skin.h, skin.w - 15, 0);
	if (playback_mode == PLAYBACK_MODE_STOPED) img_draw stdcall(skin.image, 13, 0, 22, skin.h, 300, 0);
	//Playing control buttons
	DefineButton(13, 1, 21, 21, BUTTON_PLAYBACK_PLAY_PAUSE + BT_HIDE, 0);
	DefineButton(36, 1, 21, 21, BUTTON_PLAYBACK_PREV + BT_HIDE, 0);
	DefineButton(60, 1, 21, 21, BUTTON_PLAYBACK_NEXT + BT_HIDE, 0);
	//Window control buttons
	DefineButton(Form.width - 14,  1, 11, 11, BUTTON_WINDOW_CLOSE + BT_HIDE, 0);
	DefineButton(Form.width - 14, 12, 12, 11, BUTTON_WINDOW_REDUCE + BT_HIDE, 0);
	//Mode depended
	if (window_mode == WINDOW_MODE_NORMAL)
	{
		DefineButton(Form.width - 26,  1, 12, 11, BUTTON_WINDOW_MINIMIZE + BT_HIDE, 0);
		strcpy(#cur_y_playing_title, #current_filename);
		cur_y_playing_title[strlen(#cur_y_playing_title)-4] = '\0';
		if (strlen(#cur_y_playing_title) > 29) strcpy(#cur_y_playing_title + 26, "..."); 
		WriteText(90, 9, 0x80, theme.color_top_panel_text, #cur_y_playing_title);
	}
	else 
	{
		DefineButton(0, 0, 12, skin.h, 99 + BT_HIDE + BT_NOFRAME, 0);
	}
}


void DrawScroller()
{
	scroll1.max_area = list.count;
	scroll1.cur_area = list.visible;
	scroll1.position = list.first;
	scroll1.all_redraw = 0;
	scroll1.start_x = skin.w - scroll1.size_x - 1;
	scroll1.start_y = list.y-1;
	scroll1.size_y = list.h+2;
	if (list.count > list.visible) scrollbar_v_draw(#scroll1);
}

void DrawListBorder(dword x,y,w,h,color1)
{
	DrawBar(x,y+h,w,1,color1);
	DrawBar(x,y,1,h,color1);
	DrawBar(x+w,y,1,h+1,color1);
}


stop:

char menu_stak[4096];