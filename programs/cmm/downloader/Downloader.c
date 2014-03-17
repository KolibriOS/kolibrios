#define MEMSIZE 0x100000
//libraries
#include "..\lib\kolibri.h"
#include "..\lib\strings.h"
#include "..\lib\figures.h"
#include "..\lib\file_system.h"
#include "..\lib\mem.h"
#include "..\lib\dll.h"
#include "..\lib\list_box.h"
//*.obj libraries
#include "..\lib\lib.obj\box_lib.h"
#include "..\lib\lib.obj\libio_lib.h"
#include "..\lib\lib.obj\http.h"

char header[]="New Downloader v0.4";

#ifdef LANG_RUS
	char accept_language[]= "Accept-Language: ru\n";
#else
	char accept_language[]= "Accept-Language: en\n";	
#endif

proc_info Form;
#define WIN_W 400
#define WIN_H 240
system_colors sc;
#define URL param

int	mouse_twb;
edit_box address_box= {250,20,20,0xffffff,0x94AECE,0xffffff,0xffffff,0,sizeof(URL),#URL,#mouse_twb,2,19,19};

dword speed[2000];
dword speed_position;
dword bufpointer;
dword bufsize;

dword http_transfer = 0;
dword http_buffer;

struct diagram {
	int x,y,w,h;
} diagram;

void main()
{
	int key, btn;
	mouse m;
	char filepath[4096];
	char notify_message[4296];
	
	mem_Init();
	if (load_dll2(boxlib, #box_lib_init,0)!=0) {notify("System Error: library doesn't exists /rd/1/lib/box_lib.obj"); ExitProcess();}
	if (load_dll2(libio, #libio_init,1)!=0) notify("Error: library doesn't exists - libio");
	if (load_dll2(libHTTP, #http_lib_init,1)!=0) notify("Error: library doesn't exists - http");	
	if (!URL) strcpy(#URL, "http://builds.kolibrios.org/eng/latest-iso.7z");
	address_box.size = address_box.pos = strlen(#URL);

	SetEventMask(0xa7);
	loop()
	{
		WaitEventTimeout(40);
		switch(EAX & 0xFF)
		{
			CASE evMouse:
				if (!CheckActiveProcess(Form.ID)) break;
				if (http_transfer <= 0) edit_box_mouse stdcall (#address_box);
				break;

			case evButton:
				btn=GetButtonID();
				if (btn==1)	ExitProcess();
				Scan(btn);
				break;

			case evKey:
				key = GetKey();
				if (address_box.flags & 0b10)
				{
					EAX=key<<8; 
					edit_box_key stdcall(#address_box);
				}
				if (key==13) Scan(301);
				break;

			case evReDraw:
				sc.get();
				DefineAndDrawWindow(215,100,WIN_W,WIN_H,0x73,sc.work,#header,0);
				GetProcessInfo(#Form, SelfInfo);
				if (Form.status_window>2) return;
				if (Form.height<120) MoveSize(OLD,OLD,OLD,120);
				if (Form.width<280) MoveSize(OLD,OLD,280,OLD);
				diagram.x = 20;
				diagram.y = 87;
				diagram.w = Form.cwidth - diagram.x - diagram.x;
				diagram.h = Form.cheight - diagram.y - 28;
				Draw_Window();
				break;
				
			default:
				if (Form.width==0) break;
				if (http_transfer <= 0) break;
				http_process stdcall (http_transfer);
				if (EAX == 0) {
					ESI = http_transfer;
					bufpointer = ESI.http_msg.content_ptr;
					bufsize = ESI.http_msg.content_received;
					http_free stdcall (http_transfer);
					http_transfer=0;
					strcpy(#filepath, "/tmp0/1/");
					strcat(#filepath, #URL+strrchr(#URL, '/'));
					if (WriteFile(bufsize, bufpointer, #filepath))
					{
						strcpy(#notify_message, "File saved as ");
					}
					else
					{
						strcpy(#notify_message, "Error! Can't save file as ");
					}
					strcat(#notify_message, #filepath);	
					notify(#notify_message);
					address_box.color = address_box.blur_border_color = address_box.focus_border_color = 0xFFFfff;
					Draw_Window();
				}
				ESI = http_transfer;
				speed[speed_position] = ESI.http_msg.content_received;
				DrawSpeed();
				speed_position++;
		}
	}
}

void DrawSpeed()
{
	int i;
	int speed_in_position;
	int max_speed, start_from;
	char bytes_received[70];

	DrawBar(diagram.x, diagram.y, diagram.w, diagram.h+1, 0xFCF8F7);
	max_speed = speed[speed_position];
	if (speed_position < diagram.w) start_from = 0; else start_from = speed_position - diagram.w;
	for (i = 0; i <= speed_position-start_from; i++)
	{
		if (max_speed>0)
		{
			speed_in_position = diagram.h - 2 * speed[i+start_from] / max_speed;
			PutPixel(diagram.x+i, diagram.h - speed_in_position + diagram.y, 0x00A3CB);
			ECX++;
			$int 64;
		}
	}
	if (speed_position==0) return;
	if (http_transfer > 0)
	{
		strcpy(#bytes_received, "Downloading... ");	
	} 
	else
	{
		strcpy(#bytes_received, "Downloading competle. ");	
	}
	strcat(#bytes_received, itoa(speed[speed_position-1]));
	strcat(#bytes_received, " bytes received.");
	DrawBar(diagram.x, diagram.y + diagram.h + 10, diagram.w, 9, sc.work);
	WriteText(diagram.x, diagram.y + diagram.h + 10, 0x80, sc.work_text, #bytes_received);
}

void Draw_Window()
{	
	DrawBar(0,0,Form.cwidth,Form.cheight,sc.work); //bg
	DrawCaptButton(diagram.x, 50, 120, 20, 301, sc.work_button, sc.work_button_text, "Start downloading");
	if (http_transfer <= 0) && (speed_position>0)
	{
		DrawCaptButton(diagram.x+130, 50, 120, 20, 305, sc.work_button, sc.work_button_text, "Show in folder");	
	} 
	WriteText(diagram.x, address_box.top + 4, 0x80, sc.work_text, "URL:");
	address_box.left = strlen("URL:")*6 + 10 + diagram.x;
	address_box.width = Form.cwidth - address_box.left - diagram.x - 3;
	address_box.offset=0;
	edit_box_draw stdcall(#address_box);
	DrawRectangle(address_box.left-1, address_box.top-1, address_box.width+2, 16,address_box.color);
	DrawRectangle(address_box.left-2, address_box.top-2, address_box.width+4, 18,sc.work_graph);

	DrawRectangle(diagram.x-2, diagram.y-2, diagram.w+2, diagram.h+3, sc.work_graph);
	DrawSpeed();
}

void Scan(int id)
{
	if (id==301) StartDownloading();
	if (id==302) StopDownloading();
	if (id==305) RunProgram("/sys/File managers/Eolite", "/tmp0/1/");
}


void StopDownloading()
{
	if (http_transfer<>0)
	{
		EAX = http_transfer;
		EAX = EAX.http_msg.content_ptr;		// get pointer to data
		$push	EAX							// save it on the stack
		http_free stdcall (http_transfer);	// abort connection
		$pop	EAX							
		mem_Free(EAX);						// free data
		http_transfer=0;
		bufsize = 0;
		bufpointer = mem_Free(bufpointer);
	}
	address_box.color = address_box.blur_border_color = address_box.focus_border_color = 0xFFFfff;
	Draw_Window();
}

void StartDownloading()
{
	StopDownloading();
	speed_position = 0;
	if (strncmp(#URL,"http:",5)==0)
	{
		address_box.color = address_box.blur_border_color = address_box.focus_border_color = 0xededed;
		Draw_Window();
		http_get stdcall (#URL, #accept_language);
		http_transfer = EAX;
		if (http_transfer == 0)
		{
			StopDownloading();
			bufsize = 0;
			bufpointer = mem_Free(bufpointer);
			return;
		}
	}
	else
	{
		notify("File adress should starts form http://");
	}
}



stop:
