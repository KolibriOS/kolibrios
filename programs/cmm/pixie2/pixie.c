#define MEMSIZE 4096 * 50

//===================================================//
//                                                   //
//                       LIB                         //
//                                                   //
//===================================================//

#include "../lib/fs.h"
#include "../lib/list_box.h"
#include "../lib/gui.h"
#include "../lib/random.h"
#include "../lib/kfont.h"

#include "../lib/obj/libio.h"
#include "../lib/obj/libimg.h"
#include "../lib/obj/libini.h"
#include "../lib/obj/proc_lib.h"
#include "../lib/obj/box_lib.h"

#include "../lib/patterns/simple_open_dialog.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

//simple open dialog data
char default_dir[] = "/rd/1";
od_filter filter2 = { 15, "MP3\0WAV\0XM\0\0" };

#define ABOUT_MESSAGE "Pixie Player v2.92 Final

     A tiny music folder player.
     Supports MP3, WAV, XM audio file formats.

Hot keys:
 Open file: O key
 Play/Stop: Space or P key
 Start playing selected file: Enter
 Goto next/previous track: Ctrl + Left/Right
 Change sound volume: Left/Right key
 Remove from the list: Delete
 Permanently delete file: Shift + Delete
 Show file info: I
 Repeat: R
 Shuffle: S
 Mute: M

kolibri-n.org & aspero.pro"

scroll_bar scroll1 = { 5,200,398,44,0,2,115,15,0,0xeeeeee,0xBBBbbb,0xeeeeee,
	0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};

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

#define LAST_FOLDER_EXISTS 1

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
	list.SetFont(8, 16, 13);
	LoadLibraries();
	LoadIniConfig();
	if (!param) {
		notify("'Pixie Player\nPress O key to open MP3/WAV/XM file' -St");
		if (work_folder) param=LAST_FOLDER_EXISTS;
	}
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
				if (mouse.down) && (mouse.key&MOUSE_LEFT) 
					&& (list.ProcessMouse(mouse.x, mouse.y)) DrawPlayList();
				if (mouse.down) && (mouse.key&MOUSE_RIGHT) EventShowAbout();
			}
			if(mouse.key&MOUSE_LEFT) && (mouse.x<14) 
				&& (window_mode == WINDOW_MODE_SMALL) EventDragWindow();
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
				case BUTTON_SHUFFLE: EventShuffleClick(); break;
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
			if (key_modifier&KEY_LSHIFT) || (key_modifier&KEY_RSHIFT) {
				if (key_scancode==SCAN_CODE_DEL) EventPermanentlyDeleteFile();
				break;
			}
			if (key_scancode==SCAN_CODE_KEY_O) EventFileDialogOpen();
			if (key_scancode==SCAN_CODE_KEY_M) RunProgram("/sys/@VOLUME", "m");
			if (key_scancode==SCAN_CODE_KEY_R) EventRepeatClick();
			if (key_scancode==SCAN_CODE_KEY_S) EventShuffleClick();
			if (key_scancode==SCAN_CODE_KEY_I) EventShowTagInfo();
			if (key_scancode==SCAN_CODE_RIGHT) RunProgram("/sys/@VOLUME", "+");
			if (key_scancode==SCAN_CODE_LEFT)  RunProgram("/sys/@VOLUME", "-");
			if (key_scancode==SCAN_CODE_ENTER) EventStartPlayingSelectedItem();
			if (key_scancode==SCAN_CODE_DEL) EventRemoveItemFromList();
			if (key_scancode==SCAN_CODE_KEY_P)||(key_scancode==SCAN_CODE_SPACE) EventPlayAndPause();
			if (key_scancode==SCAN_CODE_F1) EventShowAbout();
			if (list.ProcessKey(key_scancode)) DrawPlayList();
			break;
		case evReDraw:
			if (window_mode == WINDOW_MODE_NORMAL) 
				DefineDragableWindow(win_x_normal, win_y_normal, skin.w - 1, skin.h + list.h-1);
			if (window_mode == WINDOW_MODE_SMALL) 
				DefineDragableWindow(win_x_small, win_y_small, WIN_W_SMALL, WIN_H_SMALL);
			draw_window();
			if (param[0]) {
				if (param==LAST_FOLDER_EXISTS) EventOpenFolder(NULL); 
				else EventOpenFolder(#param);
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
	int kfont_width;
	char temp_filename[4096];
	dword text_color, bg_color;
	for (i=0; i<list.visible; i++;)
	{
		strcpy(#temp_filename, files_mas[i + list.first] * 304 + buf + 72);
		temp_filename[strrchr(#temp_filename, '.')-1] = '\0';
		
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
		if (i + list.first == current_playing_file_n) 
		&& (playback_mode == PLAYBACK_MODE_PLAYING)
		{
			text_color = theme.color_list_active_text;
		}
		DrawBar(list.x, yyy, list.w, list.item_h, bg_color);
		kfont_width = kfont.WriteIntoWindow(6, yyy+list.text_y, bg_color, 
			text_color, list.font_type, #temp_filename);
		if (kfont_width>skin.w-15) DrawBar(skin.w-1, yyy, 1, list.item_h, theme.color_list_border);
	}
	DrawBar(list.x,list.visible * list.item_h + list.y, list.w, 
		-list.visible * list.item_h + list.h, theme.color_list_bg);
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

void DrawTopPanel()
{
	int kfont_width;
	int button_y;
	//Mode depended
	if (window_mode == WINDOW_MODE_NORMAL)
	{
		button_y = 46;
		img_draw stdcall(skin.image, 0, 0, skin.w, skin.h, 0, 0);
		if (playback_mode != PLAYBACK_MODE_STOPED) 
			img_draw stdcall(skin.image, 47, button_y, 41, 21, skin.w+1, WIN_H_SMALL+1);
		if (repeat) 
			img_draw stdcall(skin.image, 217, button_y+2, 17,17,skin.w+43, WIN_H_SMALL+1);
		if (shuffle) 
			img_draw stdcall(skin.image, 236, button_y+2, 17,17, skin.w+62, WIN_H_SMALL+1);

		if (!work_folder) DrawPixieTitle("Pixie");
		else DrawPixieTitle(#work_folder + strrchr(#work_folder, '/'));
		kfont_width = kfont.WriteIntoWindow(8, 24, theme.color_top_panel_bg, 
			theme.color_top_panel_song_name, list.font_type, #current_filename);
		if (kfont_width>skin.w-15) DrawBar(skin.w-1, 24, 1, list.item_h, theme.color_list_border);
	 	//Playing control buttons
		DefineHiddenButton(7, button_y, 38, 20, BUTTON_PLAYBACK_PREV);
		DefineHiddenButton(48, button_y, 38, 20, BUTTON_PLAYBACK_PLAY_PAUSE);
		DefineHiddenButton(87, button_y, 38, 20, BUTTON_PLAYBACK_NEXT);
		//Window control buttons
		DefineHiddenButton(Form.width - 27, 1, 26, 15, BUTTON_WINDOW_CLOSE);
		DefineHiddenButton(Form.width - 55, 1, 26, 15, BUTTON_WINDOW_MINIMIZE);
		DefineHiddenButton(Form.width - 83, 1, 26, 15, BUTTON_WINDOW_REDUCE);
		//Other buttons
		DefineHiddenButton(218, button_y+3, 17, 16, BUTTON_REPEAT);
		DefineHiddenButton(237, button_y+3, 17, 16, BUTTON_SHUFFLE);
		DefineHiddenButton(270, button_y+3, 17, 16, BUTTON_OPEN_DIALOG);
		DefineHiddenButton(289, button_y+3, 17, 16, BUTTON_SHOW_VOLUME);
	}
	else if (window_mode == WINDOW_MODE_SMALL)
	{
		button_y = 7;
		img_draw stdcall(skin.image, 0, 0, WIN_W_SMALL, WIN_H_SMALL, skin.w-1, 0);
		if (playback_mode != PLAYBACK_MODE_STOPED) 
			img_draw stdcall(skin.image, 46, button_y-1, 27, 19, skin.w+83, WIN_H_SMALL+1);
		DefineHiddenButton(0, 0, WIN_W_SMALL, WIN_H_SMALL, 99 + BT_NOFRAME);
	 	//Playing control buttons
		DefineHiddenButton(20, button_y, 24, 16, BUTTON_PLAYBACK_PREV);
		DefineHiddenButton(46, button_y, 24, 16, BUTTON_PLAYBACK_PLAY_PAUSE);
		DefineHiddenButton(72, button_y, 24, 16, BUTTON_PLAYBACK_NEXT);
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
	kfont.WriteIntoWindow(8, 5, theme.color_top_panel_bg, 
		theme.color_top_panel_folder_name, list.font_type, _title);
}

//===================================================//
//                                                   //
//                     EVENTS                        //
//                                                   //
//===================================================//


void EventOpenFolder(dword _open_path)
{
	if (!_open_path)
	{
		OpenDirectory(#work_folder);
	}
	else
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
	if (window_mode==WINDOW_MODE_NORMAL) MoveSize(OLD, OLD, OLD, skin.h + list.h);
	list.KeyHome();
	current_playing_file_n=0;
	EventStopPlaying();
	if (_open_path) EventStartPlaying();	
}


void EventStopPlaying() 
{
	if (player_run_id) player_run_id = KillProcess(player_run_id);
	if (notify_run_id) notify_run_id = KillProcess(notify_run_id);
	playback_mode = PLAYBACK_MODE_STOPED;
	DrawTopPanel();
	DrawPlayList();
}


void EventStartPlaying()
{
	word i;
	char item_path[4096];
	char notify_message[512];
	EventStopPlaying();
	if (current_playing_file_n >= list.count) { 
		current_playing_file_n = list.count-1;
		return; 
	}
	if (current_playing_file_n < 0) { 
		current_playing_file_n = 0; 
		return; 
	}
	playback_mode = PLAYBACK_MODE_PLAYING;
	strlcpy(#current_filename, GetPlayingItemName(), sizeof(current_filename));
	sprintf(#item_path,"-h %s/%s",#work_folder,#current_filename);
	current_filename[strrchr(#current_filename, '.')-1] = '\0';
	DrawPlayList();
	DrawTopPanel();
	player_run_id = RunProgram("/sys/media/ac97snd", #item_path);	
	sprintf(#notify_message,"'Now playing:\n%s' -St",#current_filename);
	if (!repeat) && (window_mode==WINDOW_MODE_SMALL)
	{
		for (i=2; i<strlen(#notify_message)-6; i++) 
		{
			//replace ' char to avoid @notify misunderstood
			if (notify_message[i]=='\'') notify_message[i]=96; 
		}
		notify_run_id = notify(#notify_message);
	}
}


void EventPlayAndPause() 
{
	if (playback_mode == PLAYBACK_MODE_PLAYING) 
	{
		playback_mode = PLAYBACK_MODE_STOPED;
		EventStopPlaying();
	}
	else
	{
		playback_mode = PLAYBACK_MODE_PLAYING;
		EventStartPlaying();
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
	EventStopPlaying();
	SaveIniConfig();
	ExitProcess();
}

void EventPlaybackPrevious()
{
	if (shuffle) current_playing_file_n = random(list.count);
	else current_playing_file_n--;
	EventStartPlaying();
}

void EventPlaybackNext()
{
	if (shuffle) current_playing_file_n = random(list.count);
	else current_playing_file_n++;
	EventStartPlaying();
}

void EventStartPlayingSelectedItem()
{
	current_playing_file_n=list.cur_y; 
	EventStartPlaying();
}

void EventFileDialogOpen()
{
	OpenDialog_start stdcall (#o_dialog); 
	if (o_dialog.status==1) EventOpenFolder(#openfile_path);
}

void EventCheckSongFinished()
{
	if (playback_mode == PLAYBACK_MODE_PLAYING) 
	&& (!GetProcessSlot(player_run_id)) {
		if (repeat) EventStartPlaying();
		else EventPlaybackNext();
	}
}

void EventRepeatClick()
{
	repeat ^= 1;
	DrawTopPanel();
}

void EventShuffleClick()
{
	shuffle ^= 1;
	DrawTopPanel();
}

void EventRemoveItemFromList()
{
	int i;
	if (list.cur_y == current_playing_file_n) EventStopPlaying();
	for (i=list.cur_y; i<list.count; i++) files_mas[i] = files_mas[i+1];
	list.count--;
	if (list.cur_y <= current_playing_file_n) current_playing_file_n--;
	list.CheckDoesValuesOkey();
	if (list.count <= list.visible) 
	{
		list.h = list.count * list.item_h;
		list.visible = list.count;
		if (window_mode==WINDOW_MODE_NORMAL) MoveSize(OLD, OLD, OLD, skin.h + list.h);
	}
	else DrawPlayList();
}

void EventPermanentlyDeleteFile()
{
	char item_path[4096];
	sprintf(#item_path,"%s/%s",#work_folder,GetSelectedItemName());
	DeleteFile(#item_path);
	EventRemoveItemFromList();
}

void EventShowAbout()
{
	CreateThread(#ShowAboutThread,#menu_stak+4092);
}

void ShowAboutThread()
{
	proc_info pop_up;
	loop() switch(WaitEvent())
	{
		case evButton: 
			ExitProcess();
			break;
		case evKey:
			GetKeys();
			if (key_scancode == SCAN_CODE_ESC) ExitProcess();
			break;
		case evReDraw:
			DefineDragableWindow(150, 200, 400, 368);
			GetProcessInfo(#pop_up, SelfInfo);

			DrawBar(0, 0, pop_up.width, pop_up.height, theme.color_top_panel_bg);
			DrawRectangle(0, 0, pop_up.width, pop_up.height, theme.color_list_border);

			DefineHiddenButton(pop_up.width - 27, 1, 26, 15, BUTTON_WINDOW_CLOSE);
			img_draw stdcall(skin.image, pop_up.width-28, 0, 28, 18, skin.w - 29, 0);
			DrawCaptButton(pop_up.width-10-80, pop_up.height - 34, 80, 24, 2, 
			  theme.color_list_active_bg, theme.color_top_panel_song_name, "Cool");
			
			WriteTextLines(10, 10, 0x90, theme.color_top_panel_song_name, ABOUT_MESSAGE, 19);
			DrawIcon32(10, 48, theme.color_top_panel_bg, 65);

	}
}

/*
struct {
	char tag[4];
	char title[60];
	char artist[60];
	char album[60];
	char speed;
	char genre[30];
	char start_time[6];
	char end_time[6];
} tag11;

struct {
	char tag[3];
	char title[30];
	char artist[30];
	char album[30];
	char year[4];
	char comment[30];
	unsigned char genre; //https://www.w3.org/People/Bos/MP3tag/mp3tag.c
} tag10;
*/

void EventShowTagInfo()
{
	char item_path[4096];
	sprintf(#item_path,"%s/%s",#work_folder,GetSelectedItemName());
	RunProgram("/sys/media/mp3info", #item_path);
}


stop:

char menu_stak[4096];