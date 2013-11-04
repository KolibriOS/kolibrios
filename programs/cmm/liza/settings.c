//Leency & SoUrcerer, LGPL

#define CUSTOM 0
#define MANUAL 1
char checked[3] = { 1, 0 };
int use_iconv = 1;

char *text1[] = {"POP server adress:", "POP server port:", "SMTP server adress:", "SMTP server port:", '\0'};

dword mouse_opt;
unsigned char POP_server1[128]="pop.server.com";
unsigned char POP_server_port1[5]="110";
unsigned char SMTP_server1[128]="smtp.server.com";
unsigned char SMTP_server_port1[5]="25";
edit_box POP_server_box        = {210,190,90 ,0xffffff,0x94AECE,0xffc90E,0xffffff,0,sizeof(POP_server1),#POP_server1,#mouse_opt,0};
edit_box POP_server_port_box   = {210,190,115,0xffffff,0x94AECE,0xffc90E,0xffffff,0,5,#POP_server_port1,#mouse_opt,0b1000000000000000};
edit_box SMTP_server_box       = {210,190,140,0xffffff,0x94AECE,0xffc90E,0xffffff,0,sizeof(SMTP_server1),#SMTP_server1,#mouse_opt,0};
edit_box SMTP_server_port_box  = {210,190,165,0xffffff,0x94AECE,0xffc90E,0xffffff,0,5,#SMTP_server_port1,#mouse_opt,0b1000000000000000};


void SettingsDialog()
{
	int key, id;

	POP_server_box.size = strlen(#POP_server1);
	POP_server_port_box.size = strlen(#POP_server_port1);
	SMTP_server_box.size = strlen(#SMTP_server1);
	SMTP_server_port_box.size = strlen(#SMTP_server_port1);

	goto _OPT_WIN;

	loop()	
	{
		switch(WaitEvent())
		{
			case evMouse:
				IF (GetProcessSlot(Form.ID)-GetActiveProcess()!=0) break;
				if (checked[1]==0) break;
				edit_box_mouse stdcall(#POP_server_box);
				edit_box_mouse stdcall(#POP_server_port_box);
				edit_box_mouse stdcall(#SMTP_server_box);
				edit_box_mouse stdcall(#SMTP_server_port_box);
				break;
				
			case evButton:
				id = GetButtonID(); 
				if (id==1) SaveAndExit();
				if (id==19) LoginBoxLoop();
				if (id==17) || (id==18)
				{
					if (checked[id-17]==1) break;
					checked[0]><checked[1];
					if (checked[1]) POP_server_box.flags = 0b10;
					OptionsWindow();
				}
				if (id==20)
				{
					if (use_iconv==2) break;
					if (use_iconv==1) use_iconv=0; else use_iconv=1;
					OptionsWindow();
				}
				break;
				
			case evKey:
				key = GetKey();

				if (checked[1]==0) break;
				if (key==9)
				{
					if (POP_server_box.flags & 0b10)       { POP_server_box.flags -= 0b10;         POP_server_port_box.flags += 0b10;  } else
					if (POP_server_port_box.flags & 0b10)  { POP_server_port_box.flags -= 0b10;    SMTP_server_box.flags += 0b10;      } else
					if (SMTP_server_box.flags & 0b10)      { SMTP_server_box.flags -= 0b10;        SMTP_server_port_box.flags += 0b10; } else
					if (SMTP_server_port_box.flags & 0b10) { SMTP_server_port_box.flags -= 0b10;   POP_server_box.flags += 0b10;       } else
					                                         POP_server_box.flags = 0b10;
					OptionsWindow();
				}
				
				EAX=key<<8;
				edit_box_key stdcall(#POP_server_box);
				edit_box_key stdcall(#POP_server_port_box);
				edit_box_key stdcall(#SMTP_server_box);
				edit_box_key stdcall(#SMTP_server_port_box);
				break;

			case evReDraw: _OPT_WIN:
				if !(DefineWindow(OPTIONS_HEADER)) break;
				DrawBar(0,0, Form.cwidth, Form.cheight, sc.work);
				OptionsWindow();
				break;
		}
	}
}

void OptionsWindow()
{
	#define ELEM_X 25
	int i;
	DrawBar(0, Form.cheight - 40, Form.cwidth, 1, sc.work_graph);
	DrawBar(0, Form.cheight - 40+1, Form.cwidth, 1, LBUMP);
	DrawCaptButton(Form.cwidth-79, Form.cheight-32, 70, 25, 19, sc.work_button, sc.work_button_text,"Apply");

	WriteTextB(ELEM_X, 20, 0x90, sc.work_text, "Network settings");
	CheckBox(ELEM_X, 45, 12, 12, 17, "Use custom settings", sc.work_graph, sc.work_text, checked[0]);
	CheckBox(ELEM_X, 65, 12, 12, 18, "Manual configuration", sc.work_graph, sc.work_text, checked[1]);
	for (i=0; i<4; i++)
	{
		WriteText(ELEM_X+40, i*25+4+POP_server_box.top, 0x80, sc.work_text, text1[i]);
		DrawRectangle(POP_server_box.left-1, i*25+POP_server_box.top-1, POP_server_box.width+2, 16, sc.work_graph);
	}
	edit_box_draw stdcall(#POP_server_box);
	edit_box_draw stdcall(#POP_server_port_box);
	edit_box_draw stdcall(#SMTP_server_box);
	edit_box_draw stdcall(#SMTP_server_port_box);

	WriteTextB(ELEM_X, 205, 0x90, sc.work_text, "MailBox settings");
	CheckBox(ELEM_X, 230,12, 12, 20, "Use iconv library for converting text charsets", sc.work_graph, sc.work_text, use_iconv);
}