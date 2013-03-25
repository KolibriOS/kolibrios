//Leency & SoUrcerer, LGPL

//libraries
#define MEMSIZE 0xA0000
#include "..\lib\kolibri.h"
#include "..\lib\strings.h"
#include "..\lib\encoding.h"
#include "..\lib\file_system.h"
#include "..\lib\figures.h"
#include "..\lib\list_box.h"
#include "..\lib\socket.h"
#include "..\lib\mem.h"
#include "..\lib\dll.h"
//*.obj libraries
#include "..\lib\lib.obj\box_lib.h"
#include "..\lib\lib.obj\network.h"
#include "..\lib\lib.obj\libio_lib.h"
#include "..\lib\lib.obj\libimg_lib.h"
#include "..\lib\lib.obj\netcode.h"
#include "..\lib\lib.obj\iconv.h"
//images
byte in_out_mail[18*36] = FROM "in_out_mail.raw";

//connection algorithm
enum {
	STOP,
	GET_PORT,
	GET_SERVER_IP,
	GET_SOCKET,
	CONNECT,
	GET_ANSWER_CONNECT,
	SEND_USER,
	GET_ANSWER_USER,
	SEND_PASS,
	GET_ANSWER_PASS,
	SEND_NLIST,
	GET_ANSWER_NLIST,
	SEND_NSTAT,
	GET_ANSWER_NSTAT,
	SEND_RETR,
	GET_ANSWER_RETR
};

//WindowDefinitions
#define WIN_W         600
#define WIN_H         440
#define WIN_MIN_W     500
#define WIN_MIN_H     380
#define LOGIN_HEADER   "Login - Email client Liza 0.77"
#define OPTIONS_HEADER "Options - Email client Liza 0.77"
#define MAILBOX_HEADER "Mail Box - Email client Liza 0.77"
proc_info Form;
system_colors sc;
#define LBUMP 0xFFFfff

//progress_bar definitions
char cur_st_percent;
dword cur_st_text;

//connection data
#define DEFAULT_POP_PORT 110;
dword local_port=1000;
char POP_server_path[128];
dword POP_server_IP;
dword POP_server_port;
char login[128];
char request[256+22];
int request_len;
char connection_status;
dword socket;

int aim;
int ticks;

//global data for server response
char immbuffer[512];
int immpointer;

void immfree(){
	immpointer=0;
	immbuffer[immpointer]='\0';
}

void immputc(char agot_char){				
	immbuffer[immpointer]=agot_char;
	immpointer++;
	immbuffer[immpointer]='\0';
	if (immpointer>511) {immpointer=0; debug ("IMM BUFFER OVERFLOW ERROR"); aim=NULL;}
}

#include "settings.c"
#include "login.c"
#include "mail_box.c"
#include "parselist.c"


void main()
{
	mem_Init();
	if (load_dll2(boxlib, #box_lib_init,0)!=0)	        notify("Error while loading library /rd/1/lib/box_lib.obj");
	if (load_dll2(network_lib, #network_lib_init,0)!=0)	notify("Error while loading library /rd/1/lib/network.obj");
	if (load_dll2(netcode_lib, #base64_encode,0)!=0)	notify("Error while loading library /rd/1/lib/netcode.obj");
	if (load_dll2(iconv_lib, #iconv_open,0)!=0)	      { notify("Error while loading library /rd/1/lib/iconv.obj"); use_iconv=2; }

	OpenMailDat();
	SetEventMask(0x27);
	LoginBoxLoop();
}


int DefineWindow(dword wtitle)
{
	sc.get();
	DefineAndDrawWindow(GetScreenWidth()-WIN_W/2,GetScreenHeight()-WIN_H/2, WIN_W, WIN_H, 0x73,sc.work); 
	DrawTitle(wtitle);
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window>2) return 0; //rolled_up
	if (Form.width < WIN_MIN_W) MoveSize(OLD,OLD,WIN_MIN_W,OLD);
	if (Form.height < WIN_MIN_H) MoveSize(OLD,OLD,OLD,WIN_MIN_H);
	return 1;
}


void OpenMailDat()
{
	char read_data[512], pass_b64[256];
	ReadFile(0, 512, #read_data, "/sys/network/mail.dat");
	if (!read_data)
	{
		strcpy(#email_text, "eiroglif@yandex.ru"); //temporarily, for testing
		//strcpy(#email_text, "example@mail.com");
		strcpy(#pass_text, "rostov");
	}
	else
	{
		strcpy(#pass_b64, #read_data+strchr(#read_data, '\n')+1);
		base64_decode stdcall (#pass_b64, #pass_text, strlen(#pass_b64));
		read_data[strchr(#read_data, '\n')-1] = NULL;
		strcpy(#email_text, #read_data);
	}
	pass_box.size = pass_box.pos = strlen(#pass_text);
	login_box.size = login_box.pos = strlen(#email_text);
}



void SaveAndExit()
{
	char write_data[512], pass_b64[256];
	strcpy(#write_data, #email_text);
	strcat(#write_data, "\n");
	base64_encode stdcall (#pass_text, #pass_b64, strlen(#pass_text));
	strcat(#write_data, #pass_b64);
	WriteFile(strlen(#write_data)+1, #write_data, "/sys/network/mail.dat");
	ExitProcess();
}


stop:

