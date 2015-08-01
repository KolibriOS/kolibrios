//flac
//do not open multiple threads
//edit list manually


#define MEMSIZE 0xFFFFF
#include "..\lib\kolibri.h"
#include "..\lib\mem.h"
#include "..\lib\strings.h"
#include "..\lib\dll.h"
#include "..\lib\file_system.h"
#include "..\lib\list_box.h"
#include "..\lib\gui.h"

#include "..\lib\obj\box_lib.h"
#include "..\lib\obj\libio_lib.h"
#include "..\lib\obj\libimg_lib.h"
#include "..\lib\obj\libini.h"

#include "..\lib\patterns\libimg_load_skin.h"

scroll_bar scroll1 = { 5,200,398,44,0,2,115,15,0,0xeeeeee,0xBBBbbb,0xeeeeee,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};
llist list;
proc_info Form;
libimg_image skin;

char pixie_ini_path[4096];

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

int current_playing_file_n;

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

byte current_theme;
enum {
	THEME_DARK,
	THEME_LIGHT
};

char work_folder[4096],
     current_filename[256];


#include "get_files_list.h"
#include "settings.h"
#include "check_default_player.h"


void main()
{
	int id, key;
	byte mouse_clicked;
	dword tmp_x,tmp_y;
	dword z1,z2;
	
	mem_Init();
	SetEventMask(0x27);
	load_dll(boxlib, #box_lib_init,0);
	load_dll(libio, #libio_init,1);
	load_dll(libimg, #libimg_init,1);
	load_dll(libini, #lib_init,1);

	id = abspath("pixie.ini");
	strcpy(#pixie_ini_path, id);
	LoadIniConfig();
	CheckDefaultForTheFirstStart();
	if (param)
	{
		strcpy(#work_folder, #param);
		work_folder[strrchr(#work_folder, '/')-1]='\0';
	}
	if (work_folder) 
	{
		OpenDirectory(#work_folder);
		SetOpenedFileFirst(#param);
	}

	StartPlayingMp3();
	list.SetSizes(1, skin.h, skin.w-1, 198, 40, 18);
	if (list.count <= list.visible) 
	{
		list.h = list.count * list.line_h;
		list.visible = list.count;
	}
	else
	{
		list.w -= scroll1.size_x;
	}

	loop()
	{
	  WaitEventTimeout(10);
	  
	  //ActivateWindow(Form.ID);
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
			if(mouse.down)
			{
				if (mouse.vert) if (list.MouseScroll(mouse.vert)) 
				{
					DrawPlayList();
				}
				if (list.MouseOver(mouse.x, mouse.y)) mouse_clicked = true;
				else if(mouse.y < skin.h) && (mouse.x < 13)
				{
					//drag window - emulate windows header
						tmp_x = mouse.x;
						tmp_y = mouse.y;
						do {
							mouse.get();
							if (tmp_x!=mouse.x) || (tmp_y!=mouse.y) 
							{
								z1 = Form.left + mouse.x - tmp_x;
								z2 = Form.top + mouse.y - tmp_y;
								if(z1<=10)z1=0;
								if(z2<=10)z2=0;
								if(z1>screen.width-Form.width-10)z1=screen.width-Form.width;
								if(z2>screen.height-Form.height-10)z2=screen.height-Form.height;
								//if(z2<10)z2=0;
								MoveSize(z1 , z2, OLD, OLD);
								DrawWindow();
							}
							pause(1);
						} while (mouse.lkm);
					if (mouse.pkm) && (mouse.y > skin.h) 
						notify("'Pixies Player v1.11\nChange sound volume: Left/Right key\nChange skin: F1/F2\nMute: M key' -St\n");
					break;
				}
				else if(mouse.up)
				{
					if (mouse_clicked)&&(list.ProcessMouse(mouse.x, mouse.y))
					{
						StartPlayingMp3();
						mouse_clicked = false;
					}
					break;
				}
			}
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
				case BUTTON_PLAYBACK_PREV:
					if (list.KeyUp()) {
						current_playing_file_n = list.current;
						StartPlayingMp3();
					}
					break;
				case BUTTON_PLAYBACK_NEXT:
					if (list.KeyDown()) 
					{
						current_playing_file_n = list.current;
						StartPlayingMp3();
					}
					break;
				case BUTTON_PLAYBACK_PLAY_PAUSE:
					if (playback_mode == PLAYBACK_MODE_PLAYING) 
					{
						playback_mode = PLAYBACK_MODE_STOPED;
						DrawInactivePlayButton();
						StopPlayingMp3();
					}
					else
					{
						playback_mode = PLAYBACK_MODE_PLAYING;
						StartPlayingMp3();
					}
					break;
			}
			break;
	  
		case evKey:
			key = GetKey();
			if (key==50) SetColorThemeLight();
			if (key==51) SetColorThemeDark();
			if (key==ASCII_KEY_LEFT) RunProgram("@VOLUME", "-");
			if (key==ASCII_KEY_RIGHT) RunProgram("@VOLUME", "+");
			if (key=='mouse') RunProgram("@VOLUME", "mouse");
			if (key==ASCII_KEY_ENTER) StartPlayingMp3();
			if (key=='p') || (key==ASCII_KEY_SPACE)
			{
				if (playback_mode == PLAYBACK_MODE_PLAYING) StopPlayingMp3();
				else StartPlayingMp3();
			}
			if (list.ProcessKey(key)) DrawPlayList();
			break;

		case evReDraw:
			if (window_mode == WINDOW_MODE_NORMAL) DefineAndDrawWindow(win_x_normal, win_y_normal, skin.w - 1, skin.h + list.h, 0x41,0,0,0);
			if (window_mode == WINDOW_MODE_SMALL) DefineAndDrawWindow(win_x_small, win_y_small, 99, skin.h - 1, 0x41,0,0,0);
			DrawWindow();
			break;

		default:
			if (playback_mode == PLAYBACK_MODE_PLAYING) && (GetProcessSlot(player_run_id)==0)
			{
				if (current_playing_file_n < list.count) 
				{
					current_playing_file_n = list.current;
					if (list.KeyDown()) StartPlayingMp3();
				}
				else
				{
					StopPlayingMp3();
					DrawWindow();
				}
			}
	  }
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
		
		yyy = i*list.line_h+list.y;
		
		//this is selected file
		if (list.current - list.first == i)
		{
			if (i>=list.count) continue;
			DrawBar(list.x, yyy, list.w, list.line_h, theme.color_list_active_bg);
			WriteText(12,yyy+list.text_y,0x80, theme.color_list_active_text, #temp_filename);
		}
		//this is not selected file
		else
		{
			if (i>=list.count) continue;
			DrawBar(list.x,yyy,list.w, list.line_h, theme.color_list_bg);
			WriteText(12,yyy+list.text_y,0x80, theme.color_list_text, #temp_filename);
		}
		//this is current playing file
		if (i + list.first == current_playing_file_n) && (playback_mode == PLAYBACK_MODE_PLAYING)
		{
			WriteText(3, yyy+list.text_y,0x80, theme.color_list_active_pointer, "\x10");
			WriteText(12,yyy+list.text_y,0x80, theme.color_list_active_text, #temp_filename);
		}
	}
	DrawBar(list.x,list.visible * list.line_h + list.y, list.w, -list.visible * list.line_h + list.h, theme.color_list_bg);
	DrawScroller();
}


void StopPlayingMp3() 
{
	if (player_run_id) player_run_id = KillProcess(player_run_id);
	if (notify_run_id) notify_run_id = KillProcess(notify_run_id);
	playback_mode = PLAYBACK_MODE_STOPED;
}


void StartPlayingMp3()
{
	word i;
	char item_path[4096], notify_message[512];
	dword item_path_end_pointer;
	StopPlayingMp3();

	if (list.current > list.count)
	{
		list.current = list.count;
		return;
	}
	if (!list.count)
	{
		notify_run_id = notify("'Pixie Player\nStopped, no file specified' -St");
		return;
	}

	current_playing_file_n = list.current;

	playback_mode = PLAYBACK_MODE_PLAYING;

	strlcpy(#current_filename, GetCurrentItemName(), sizeof(current_filename));
	strcpy(#item_path, "\"");
	strcat(#item_path, #work_folder);
	strcat(#item_path, "/");
	strcat(#item_path, #current_filename);
	strcat(#item_path, "\"");

	DrawPlayList();
	DrawTopPanel();

	item_path_end_pointer = #item_path+strlen(#item_path);
	if (strcmpi(item_path_end_pointer-4,".mp3")!=0)	player_run_id = RunProgram(abspath("minimp3"), #item_path);	
	strcpy(#notify_message, "'Now playing:\n");
	strcat(#notify_message, #current_filename);
	for (i=2; i<strlen(#notify_message); i++) if (notify_message[i]=='\'') notify_message[i]=96;
	strcat(#notify_message, "' -St");
	notify_run_id = notify(#notify_message);
}


void DrawInactivePlayButton()
{
	img_draw stdcall(skin.image, 13, 0, 22, skin.h, 300, 0);
}

void DrawWindow() {
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
	char current_playing_title[245];
	img_draw stdcall(skin.image, 0, 0, Form.width - 14, skin.h, 0, 0);
	img_draw stdcall(skin.image, Form.width - 14, 0, 15, skin.h, skin.w - 15, 0);
	if (playback_mode == PLAYBACK_MODE_STOPED) DrawInactivePlayButton();
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
		strcpy(#current_playing_title, #current_filename);
		current_playing_title[strlen(#current_playing_title)-4] = '\0';
		if (strlen(#current_playing_title) > 29) strcpy(#current_playing_title + 26, "..."); 
		WriteText(90, 9, 0x80, theme.color_top_panel_text, #current_playing_title);
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