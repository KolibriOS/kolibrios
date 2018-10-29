//Leency & SoUrcerer, LGPL

char *text1[] = {"POP server adress:", "POP server port:", "SMTP server adress:", "SMTP server port:", '\0'};

unsigned char POP_server1[128]="pop.server.com";
unsigned char POP_server_port1[7]="110";
unsigned char SMTP_server1[128]="smtp.server.com";
unsigned char SMTP_server_port1[7]="25";
edit_box POP_server_box        = {210,230,125,0xffffff,0x94AECE,0xffc90E,0xCACACA,0x10000000,sizeof(POP_server1)-2,#POP_server1,0,0};
edit_box POP_server_port_box   = {210,230,160,0xffffff,0x94AECE,0xffc90E,0xCACACA,0x10000000,sizeof(POP_server_port1)-2,#POP_server_port1,0,0};
edit_box SMTP_server_box       = {210,230,195,0xffffff,0x94AECE,0xffc90E,0xCACACA,0x10000000,sizeof(SMTP_server1)-2,#SMTP_server1,0,0};
edit_box SMTP_server_port_box  = {210,230,230,0xffffff,0x94AECE,0xffc90E,0xCACACA,0x10000000,sizeof(SMTP_server_port1)-2,#SMTP_server_port1,0,0};

checkbox automatic = { "Automatic configuration", true };

void UpdateEditboxFlags(dword additional_flag)
{
	EditBox_UpdateText(#POP_server_box, additional_flag + 0);
	EditBox_UpdateText(#POP_server_port_box, additional_flag + ed_figure_only + 0);
	EditBox_UpdateText(#SMTP_server_box, additional_flag + 0);
	EditBox_UpdateText(#SMTP_server_port_box, additional_flag + ed_figure_only + 0);
}

void SettingsDialog()
{
	int id;
	UpdateEditboxFlags(ed_disabled);
	goto _OPT_WIN;
	loop() switch(WaitEvent())
	{
		case evMouse:
			edit_box_mouse stdcall(#POP_server_box);
			edit_box_mouse stdcall(#POP_server_port_box);
			edit_box_mouse stdcall(#SMTP_server_box);
			edit_box_mouse stdcall(#SMTP_server_port_box);
			break;
			
		case evButton:
			id = GetButtonID(); 
			if (id==1) SaveAndExit();
			if (id==19) LoginBoxLoop();
			if (automatic.click(id))
			{
				if (automatic.checked) {
					UpdateEditboxFlags(ed_disabled);
					POP_server_box.blur_border_color = POP_server_port_box.blur_border_color =
					 SMTP_server_box.blur_border_color = SMTP_server_port_box.blur_border_color = 0xCACACA;
				}
				else {
					UpdateEditboxFlags(0);
					POP_server_box.flags = 0b10;
					POP_server_box.blur_border_color = POP_server_port_box.blur_border_color =
					 SMTP_server_box.blur_border_color = SMTP_server_port_box.blur_border_color = 0xFFFfff;
				}
				DrawOptionsWindow();
			}
			break;
			
		case evKey:
			GetKeys();;

			if (automatic.checked==true) break;
			if (key_scancode==SCAN_CODE_TAB)
			{
				if (POP_server_box.flags & ed_focus)       { UpdateEditboxFlags(0); POP_server_port_box.flags += ed_focus;  } else
				if (POP_server_port_box.flags & ed_focus)  { UpdateEditboxFlags(0); SMTP_server_box.flags += ed_focus;      } else
				if (SMTP_server_box.flags & ed_focus)      { UpdateEditboxFlags(0); SMTP_server_port_box.flags += ed_focus; } else
				if (SMTP_server_port_box.flags & ed_focus) { UpdateEditboxFlags(0); POP_server_box.flags += ed_focus;       } else
				{ UpdateEditboxFlags(0); POP_server_box.flags = 0b10; }                            
				DrawOptionsWindow();
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
			DrawOptionsWindow();
			break;
	}
}

void DrawOptionsWindow()
{
	#define ELEM_X 25
	int i;
	incn y;
	y.n=0;
	DrawBar(0, Form.cheight - 40, Form.cwidth, 1, system.color.work_graph);
	DrawBar(0, Form.cheight - 40+1, Form.cwidth, 1, LBUMP);
	DrawCaptButton(Form.cwidth-79, Form.cheight-32, 70, 25, 19, system.color.work_button, system.color.work_button_text,"Apply");

	WriteText(ELEM_X, y.inc(20), 0x81, system.color.work_text, "Network settings");
	automatic.draw(ELEM_X, y.inc(65));
	for (i=0; i<4; i++)	{
		WriteTextWithBg(ELEM_X+40, i*35+POP_server_box.top + 3, 0xD0, system.color.work_text, text1[i], system.color.work);
	}
	DrawEditBox(#POP_server_box);
	DrawEditBox(#POP_server_port_box);
	DrawEditBox(#SMTP_server_box);
	DrawEditBox(#SMTP_server_port_box);
}