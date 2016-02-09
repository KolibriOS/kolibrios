//Leency & SoUrcerer, LGPL

#define CUSTOM 0
#define MANUAL 1
char checked[3] = { 1, 0 };

char *text1[] = {"POP server adress:", "POP server port:", "SMTP server adress:", "SMTP server port:", '\0'};

dword mouse_opt;
unsigned char POP_server1[128]="pop.server.com";
unsigned char POP_server_port1[5]="110";
unsigned char SMTP_server1[128]="smtp.server.com";
unsigned char SMTP_server_port1[5]="25";
edit_box POP_server_box        = {210,190,90 ,0xffffff,0x94AECE,0xffc90E,0xCACACA,0,sizeof(POP_server1),#POP_server1,#mouse_opt,100000000000b};
edit_box POP_server_port_box   = {210,190,115,0xffffff,0x94AECE,0xffc90E,0xCACACA,0,5,#POP_server_port1,#mouse_opt,100000000000b};
edit_box SMTP_server_box       = {210,190,140,0xffffff,0x94AECE,0xffc90E,0xCACACA,0,sizeof(SMTP_server1),#SMTP_server1,#mouse_opt,100000000000b};
edit_box SMTP_server_port_box  = {210,190,165,0xffffff,0x94AECE,0xffc90E,0xCACACA,0,5,#SMTP_server_port1,#mouse_opt,100000000000b};


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
					if (checked[1]) {
						POP_server_box.flags = 0b10;
						POP_server_port_box.flags = SMTP_server_box.flags = SMTP_server_port_box.flags = 0b;
						POP_server_box.blur_border_color = POP_server_box.blur_border_color = POP_server_port_box.blur_border_color =
						 SMTP_server_box.blur_border_color = SMTP_server_port_box.blur_border_color = 0xFFFfff;
					}
					else {
						POP_server_box.flags = POP_server_box.flags = POP_server_port_box.flags = SMTP_server_box.flags = SMTP_server_port_box.flags = 100000000000b;
						POP_server_box.blur_border_color = POP_server_box.blur_border_color = POP_server_port_box.blur_border_color =
						 SMTP_server_box.blur_border_color = SMTP_server_port_box.blur_border_color = 0xCACACA;
					}
					OptionsWindow();
				}
				break;
				
			case evKey:
				GetKeys();;

				if (checked[1]==0) break;
				if (key_scancode==SCAN_CODE_TAB)
				{
					if (POP_server_box.flags & 0b10)       { POP_server_box.flags -= 0b10;         POP_server_port_box.flags += 0b10;  } else
					if (POP_server_port_box.flags & 0b10)  { POP_server_port_box.flags -= 0b10;    SMTP_server_box.flags += 0b10;      } else
					if (SMTP_server_box.flags & 0b10)      { SMTP_server_box.flags -= 0b10;        SMTP_server_port_box.flags += 0b10; } else
					if (SMTP_server_port_box.flags & 0b10) { SMTP_server_port_box.flags -= 0b10;   POP_server_box.flags += 0b10;       } else
					                                         POP_server_box.flags = 0b10;
					OptionsWindow();
				}
				
				EAX=key_ascii<<8;
				edit_box_key stdcall(#POP_server_box);
				edit_box_key stdcall(#POP_server_port_box);
				edit_box_key stdcall(#SMTP_server_box);
				edit_box_key stdcall(#SMTP_server_port_box);
				break;

			case evReDraw: _OPT_WIN:
				if !(DefineWindow(OPTIONS_HEADER)) break;
				DrawBar(0,0, Form.cwidth, Form.cheight, system.color.work);
				OptionsWindow();
				break;
		}
	}
}

void OptionsWindow()
{
	#define ELEM_X 25
	int i;
	DrawBar(0, Form.cheight - 40, Form.cwidth, 1, system.color.work_graph);
	DrawBar(0, Form.cheight - 40+1, Form.cwidth, 1, LBUMP);
	DrawCaptButton(Form.cwidth-79, Form.cheight-32, 70, 25, 19, system.color.work_button, system.color.work_button_text,"Apply");

	WriteTextB(ELEM_X, 20, 0x90, system.color.work_text, "Network settings");
	CheckBox(ELEM_X, 45, 17, "Use custom settings", checked[0]);
	CheckBox(ELEM_X, 65, 18, "Manual configuration", checked[1]);
	for (i=0; i<4; i++)
	{
		WriteText(ELEM_X+40, i*25+4+POP_server_box.top, 0x80, system.color.work_text, text1[i]);
		DrawRectangle(POP_server_box.left-1, i*25+POP_server_box.top-1, POP_server_box.width+2, 16, system.color.work_graph);
	}
	edit_box_draw stdcall(#POP_server_box);
	edit_box_draw stdcall(#POP_server_port_box);
	edit_box_draw stdcall(#SMTP_server_box);
	edit_box_draw stdcall(#SMTP_server_port_box);
}