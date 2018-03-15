//TODO
//repeat track
//edit list manually

#define MEMSIZE 4096 * 50

//===================================================//
//                                                   //
//                       LIB                         //
//                                                   //
//===================================================//

#include "../lib/file_system.h"
#include "../lib/list_box.h"
#include "../lib/gui.h"
#include "../lib/random.h"
#include "../lib/kfont.h"
#include "../lib/collection.h"

#include "../lib/obj/libio.h"
#include "../lib/obj/libimg.h"
#include "../lib/obj/libini.h"
#include "../lib/obj/proc_lib.h"
#include "../lib/obj/box_lib.h"

#include "../lib/patterns/libimg_load_skin.h"
#include "../lib/patterns/simple_open_dialog.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

//simple open dialog data
char default_dir[] = "/rd/1";
od_filter filter2 = { 8, "MP3\0\0" };

#define ABOUT_MESSAGE "'Pixies Player v2.61
A tiny music folder player.
Supports MP3, WAV, XM audio file formats.

Controls:
Open file: O key
Play/Stop: Space or P key
Start playing selected file: Enter
Goto next/previous track: Ctrl + Left/Right
Change sound volume: Left/Right key
Mute: M key' -td"

scroll_bar scroll1 = { 5,200,398,44,0,2,115,15,0,0xeeeeee,0xBBBbbb,0xeeeeee,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};

proc_info Form;
llist list;

char pixie_ini_path[4096];
char work_folder[4096];
char current_filename[256];

enum {
	BUTTON_WINDOW_CLOSE = 1,
	BUTTON_WINDOW_MINIMIZE,
	BUTTON_WINDOW_REDUCE,
	BUTTON_PLAYBACK_PLAY_PAUSE = 10,
	BUTTON_PLAYBACK_PREV, 
	BUTTON_PLAYBACK_NEXT,
	BUTTON_REPEAT,
	BUTTON_SHUFFLE,
	BUTTON_OPEN_DIALOG,
	BUTTON_SHOW_VOLUME
};

int player_run_id;
int notify_run_id;

bool repeat;
bool shuffle;

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

collection music_col;

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

#include "get_files_list.h"
#include "settings.h"

void LoadLibraries()
{
	load_dll(boxlib, #box_lib_init,0);
	load_dll(libio, #libio_init,1);
	load_dll(libimg, #libimg_init,1);
	load_dll(libini, #lib_init,1);
	load_dll(Proc_lib, #OpenDialog_init,0);
	OpenDialog_init stdcall (#o_dialog);
}

void main()
{
	int tempstr;
	tempstr = abspath("pixie.ini");
	strcpy(#pixie_ini_path, tempstr);
	list.SetFont(8, 16, 13);
	if (!param) notify("'Pixie Player\nPress O key to open MP3 file' -St");
	LoadLibraries();
	LoadIniConfig();
	kfont.init(DEFAULT_FONT);	
	SetEventMask(EVM_REDRAW + EVM_KEY + EVM_BUTTON + EVM_MOUSE + EVM_MOUSE_FILTER);
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
				if (mouse.vert) && (list.MouseScroll(mouse.vert)) DrawPlayList();
				if (mouse.dblclick) EventStartPlayingSelectedItem();
				if (mouse.down) && (mouse.key&MOUSE_LEFT) && (list.ProcessMouse(mouse.x, mouse.y)) DrawPlayList();
				if (mouse.down) && (mouse.key&MOUSE_RIGHT) notify(ABOUT_MESSAGE);
			}
			if(mouse.key&MOUSE_LEFT) && (mouse.y<skin.h) && (window_mode == WINDOW_MODE_SMALL) EventDragWindow();
			break;
		case evButton:
			switch(GetButtonID()) {
				case BUTTON_WINDOW_CLOSE: EventExitApp(); break;
				case BUTTON_WINDOW_MINIMIZE: MinimizeWindow(); break;
				case BUTTON_WINDOW_REDUCE: EventChangeWindowMode(); break;
				case BUTTON_PLAYBACK_PREV: EventPlaybackPrevious();	break;
				case BUTTON_PLAYBACK_NEXT: EventPlaybackNext(); break;
				case BUTTON_PLAYBACK_PLAY_PAUSE: EventPlayAndPause(); break;
				case BUTTON_REPEAT: EventRepeatClick(); break;
				case BUTTON_SHUFFLE: EventshuffleClick(); break;
				case BUTTON_OPEN_DIALOG: EventFileDialogOpen(); break;
				case BUTTON_SHOW_VOLUME: RunProgram("/sys/@VOLUME", NULL); break;
			}
			break;	  
		case evKey:
			GetKeys();
			if (key_modifier&KEY_LCTRL) || (key_modifier&KEY_RCTRL) {
				if (key_scancode==SCAN_CODE_LEFT) EventPlaybackPrevious();
				if (key_scancode==SCAN_CODE_RIGHT) EventPlaybackNext();
				break;
			}		
			if (key_scancode==SCAN_CODE_KEY_O) EventFileDialogOpen();
			if (key_scancode==SCAN_CODE_KEY_M) RunProgram("/sys/@VOLUME", "m");
			if (key_scancode==SCAN_CODE_RIGHT) RunProgram("/sys/@VOLUME", "+");
			if (key_scancode==SCAN_CODE_LEFT)  RunProgram("/sys/@VOLUME", "-");
			if (key_scancode==SCAN_CODE_ENTER) EventStartPlayingSelectedItem();
			if (key_scancode==SCAN_CODE_KEY_P) || (key_scancode==SCAN_CODE_SPACE) EventPlayAndPause();
			if (list.ProcessKey(key_scancode)) DrawPlayList();
			break;
		case evReDraw:
			if (window_mode == WINDOW_MODE_NORMAL) DefineDragableWindow(win_x_normal, win_y_normal, skin.w - 1, skin.h + list.h-1);
			if (window_mode == WINDOW_MODE_SMALL) DefineDragableWindow(win_x_small, win_y_small, WIN_W_SMALL, WIN_H_SMALL);
			draw_window();
			if (param[0]) {
				EventOpenFolder(#param);
				param[0] = NULL;
			}
			break;
		default:
			EventCheckSongFinished();
	  }
   }
}


void DrawPlayList()
{
	int i;
	int yyy;
	char temp_filename[4096];
	dword text_color, bg_color;
	for (i=0; i<list.visible; i++;)
	{
		strcpy(#temp_filename, files_mas[i + list.first] * 304 + buf + 72);
		temp_filename[strrchr(#temp_filename, '.')-1] = '\0'; 
		//if (strlen(#temp_filename)>47) strcpy(#temp_filename+44, "..."); 
		
		yyy = i*list.item_h+list.y;
		
		//this is selected file
		if (list.cur_y - list.first == i)
		{
			if (i>=list.count) continue;
			bg_color = theme.color_list_active_bg;
			text_color = theme.color_list_text;
		}
		//this is not selected file
		else
		{
			if (i>=list.count) continue;
			bg_color = theme.color_list_bg;
			text_color = theme.color_list_text;
		}
		//this is cur_y playing file
		if (i + list.first == current_playing_file_n) && (playback_mode == PLAYBACK_MODE_PLAYING)
		{
			text_color = theme.color_list_active_text;
		}
		DrawBar(list.x, yyy, list.w, list.item_h, bg_color);
		kfont.WriteIntoWindow(6, yyy+list.text_y, bg_color, text_color, list.font_type, #temp_filename);
	}
	DrawBar(list.x,list.visible * list.item_h + list.y, list.w, -list.visible * list.item_h + list.h, theme.color_list_bg);
	DrawScroller();
}


void draw_window() {
	GetProcessInfo(#Form, SelfInfo);
	DrawTopPanel();
	IF (Form.status_window>=2) return;
	if (window_mode == WINDOW_MODE_NORMAL)
	{
		DrawPlayList();
		DrawRectangle(0, skin.h-1, skin.w-1, list.h+1, theme.color_list_border);
	}
}

dword GetSongTitle()
{
	char cur_y_playing_title[245];
	strcpy(#cur_y_playing_title, #current_filename);
	//cur_y_playing_title[strlen(#cur_y_playing_title)-4] = '\0';
	//if (strlen(#cur_y_playing_title) > 36) strcpy(#cur_y_playing_title + 34, "..."); 
	return #cur_y_playing_title;
}


void DrawTopPanel()
{
	
	int button_y;
	//Mode depended
	if (window_mode == WINDOW_MODE_NORMAL)
	{
		button_y = 46;
		img_draw stdcall(skin.image, 0, 0, skin.w, skin.h, 0, 0);
		if (playback_mode != PLAYBACK_MODE_STOPED) img_draw stdcall(skin.image, 46, button_y, 41, 21, skin.w+1, WIN_H_SMALL+1);
		if (repeat) img_draw stdcall(skin.image, Form.width - 101-1, button_y+2, 17, 17, skin.w+43, WIN_H_SMALL+1);
		if (shuffle) img_draw stdcall(skin.image, Form.width - 82-1, button_y+2, 17, 17, skin.w+62, WIN_H_SMALL+1);

		if /*(!list.count) && */ (!work_folder) DrawPixieTitle("Pixie");
		else DrawPixieTitle(#work_folder + strrchr(#work_folder, '/'));
		kfont.WriteIntoWindow(8, 24, theme.color_top_panel_bg, theme.color_top_panel_song_name, list.font_type, GetSongTitle());
	 	//Playing control buttons
		DefineHiddenButton(7, button_y, 38, 20, BUTTON_PLAYBACK_PREV);
		DefineHiddenButton(47, button_y, 38, 20, BUTTON_PLAYBACK_PLAY_PAUSE);
		DefineHiddenButton(87, button_y, 38, 20, BUTTON_PLAYBACK_NEXT);
		//Window control buttons
		DefineHiddenButton(Form.width - 27, 1, 26, 15, BUTTON_WINDOW_CLOSE);
		DefineHiddenButton(Form.width - 55, 1, 26, 15, BUTTON_WINDOW_MINIMIZE);
		DefineHiddenButton(Form.width - 83, 1, 26, 15, BUTTON_WINDOW_REDUCE);
		//Other buttons
		button_y += 3;
		DefineHiddenButton(Form.width - 101,button_y, 17, 16, BUTTON_REPEAT);
		DefineHiddenButton(Form.width - 82, button_y, 17, 16, BUTTON_SHUFFLE);
		DefineHiddenButton(Form.width - 54, button_y, 23, 23, BUTTON_OPEN_DIALOG);
		DefineHiddenButton(Form.width - 27, button_y, 23, 23, BUTTON_SHOW_VOLUME);
	}
	else if (window_mode == WINDOW_MODE_SMALL)
	{
		button_y = 7;
		img_draw stdcall(skin.image, 0, 0, WIN_W_SMALL, WIN_H_SMALL, skin.w-1, 0);
		DefineHiddenButton(0, 0, WIN_W_SMALL, WIN_H_SMALL, 99 + BT_NOFRAME);
	 	//Playing control buttons
		DefineHiddenButton(8, button_y, 24, 16, BUTTON_PLAYBACK_PREV);
		DefineHiddenButton(34, button_y, 24, 16, BUTTON_PLAYBACK_PLAY_PAUSE);
		DefineHiddenButton(60, button_y, 24, 16, BUTTON_PLAYBACK_NEXT);
		//Window control buttons
		DefineHiddenButton(Form.width - 20, 1, 19, 13, BUTTON_WINDOW_CLOSE);
		DefineHiddenButton(Form.width - 20, 16, 19, 13, BUTTON_WINDOW_REDUCE);
	}
}


void DrawScroller()
{
	scroll1.max_area = list.count;
	scroll1.cur_area = list.visible;
	scroll1.position = list.first;
	scroll1.all_redraw = 0;
	scroll1.start_x = skin.w - scroll1.size_x-1;
	scroll1.start_y = list.y-1;
	scroll1.size_y = list.h+2;
	if (list.count > list.visible) scrollbar_v_draw(#scroll1);
}

void DrawPixieTitle(dword _title)
{
	kfont.WriteIntoWindow(8, 5, theme.color_top_panel_bg, theme.color_top_panel_folder_name, list.font_type, _title);
}

//===================================================//
//                                                   //
//                     EVENTS                        //
//                                                   //
//===================================================//


void EventOpenFolder(dword _open_path)
{
	if (ESBYTE[_open_path]) 
	{
		strcpy(#work_folder, _open_path);
		work_folder[strrchr(#work_folder, '/')-1]='\0';
		OpenDirectory(#work_folder);
		SetOpenedFileFirst(_open_path);
	}
	list.SetSizes(1, skin.h, skin.w-1, 22*15, 22);
	if (list.count <= list.visible) 
	{
		list.h = list.count * list.item_h;
		list.visible = list.count;
		list.w -= 1;
	}
	else
	{
		list.w -= scroll1.size_x;
	}
	MoveSize(OLD, OLD, OLD, skin.h + list.h);
	list.KeyHome();
	current_playing_file_n=0;
	EventStopPlayingMp3();
	EventStartPlayingMp3();	
}


void EventStopPlayingMp3() 
{
	if (player_run_id) player_run_id = KillProcess(player_run_id);
	if (notify_run_id) notify_run_id = KillProcess(notify_run_id);
	playback_mode = PLAYBACK_MODE_STOPED;
	DrawTopPanel();
	DrawPlayList();
}


void EventStartPlayingMp3()
{
	word i;
	char item_path[4096];
	char notify_message[512];
	EventStopPlayingMp3();
	if (current_playing_file_n >= list.count) { 
		current_playing_file_n = list.count-1;
		return; 
	}
	if (current_playing_file_n < 0) { 
		current_playing_file_n = 0; 
		return; 
	}
	playback_mode = PLAYBACK_MODE_PLAYING;
	strlcpy(#current_filename, Getcur_yItemName(), sizeof(current_filename));
	sprintf(#item_path,"-h %s/%s",#work_folder,#current_filename);
	DrawPlayList();
	DrawTopPanel();
	player_run_id = RunProgram("/sys/media/ac97snd", #item_path);	
	sprintf(#notify_message,"'Now playing:\n%s' -St",#current_filename);
	if (!repeat)
	{
		for (i=2; i<strlen(#notify_message)-6; i++) 
			if (notify_message[i]=='\'') notify_message[i]=96; //replace ' char to avoid @notify misunderstood
		notify_run_id = notify(#notify_message);
	}
}


void EventPlayAndPause() 
{
	if (playback_mode == PLAYBACK_MODE_PLAYING) 
	{
		playback_mode = PLAYBACK_MODE_STOPED;
		EventStopPlayingMp3();
	}
	else
	{
		playback_mode = PLAYBACK_MODE_PLAYING;
		EventStartPlayingMp3();
	}
}


void EventChangeWindowMode()
{
	if (window_mode == WINDOW_MODE_NORMAL)
	{
		window_mode = WINDOW_MODE_SMALL;
		win_x_normal = Form.left;
		win_y_normal = Form.top;
		MoveSize(OLD, OLD, WIN_W_SMALL-1, WIN_H_SMALL-1);
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
}

void EventExitApp()
{
	EventStopPlayingMp3();
	SaveIniConfig();
	ExitProcess();
}

void EventPlaybackPrevious()
{
	if (shuffle) current_playing_file_n = random(list.count);
	else current_playing_file_n--;
	EventStartPlayingMp3();
}

void EventPlaybackNext()
{
	if (shuffle) current_playing_file_n = random(list.count);
	else current_playing_file_n++;
	EventStartPlayingMp3();
}

void EventStartPlayingSelectedItem()
{
	current_playing_file_n=list.cur_y; 
	EventStartPlayingMp3();
}

void EventFileDialogOpen()
{
	OpenDialog_start stdcall (#o_dialog); 
	if (o_dialog.status==1) EventOpenFolder(#openfile_path);
}

void EventCheckSongFinished()
{
	if (playback_mode == PLAYBACK_MODE_PLAYING) && (!GetProcessSlot(player_run_id)) {
		if (repeat) EventStartPlayingMp3();
		else EventPlaybackNext();
	}
}

void EventRepeatClick()
{
	repeat ^= 1;
	DrawTopPanel();
}

void EventshuffleClick()
{
	shuffle ^= 1;
	DrawTopPanel();
}

stop:

char menu_stak[4096];