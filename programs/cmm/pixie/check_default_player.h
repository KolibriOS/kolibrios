char open_assoc_ini_path[] = "/sys/settings/assoc.ini";
word program_path_len;


void CheckDefaultForTheFirstStart() 
{	
	ini_get_int stdcall (#open_assoc_ini_path, "Pixie", "icon", -1);
	if (EAX!=-1) return;

	CreateThread(#ShowPopUp,#menu_stak+4092);
}


void ShowPopUp()
{
	byte button_id, key;
	proc_info pop_up;

	ini_set_int stdcall (#open_assoc_ini_path, "Pixie", "icon", 65);
	program_path_len = strlen(#program_path);
	ini_set_str stdcall (#open_assoc_ini_path, "Pixie", "exec", #program_path, program_path_len);

	loop() switch(WaitEvent())
	{
		case evButton: 
			button_id = GetButtonID();
			if (button_id==10) SetDefaultPlayer();
			if (button_id==11) ExitProcess();
			break;
		case evKey:
			key = GetKey();
			if (key==ASCII_KEY_ENTER) SetDefaultPlayer();
			if (key==ASCII_KEY_ESC) ExitProcess();
			break;
		case evReDraw:
			DefineAndDrawWindow(150, 200, 220, 90, 0x01,0,0,0);
			GetProcessInfo(#pop_up, SelfInfo);
			DrawBar(0, 0, pop_up.width, pop_up.height, theme.color_list_bg);
			DrawRectangle(0, 0, pop_up.width, pop_up.height, theme.color_list_border);
			WriteText(10, 20, 0x80, theme.color_list_text, "Hey! Pixie is not a default");
			WriteText(10, 32, 0x80, theme.color_list_text, "MP3 player. Make it default?");
			DrawCaptButton(10, pop_up.height - 30, 80, 20, 10, theme.color_list_active_bg, theme.color_list_active_text, "Yes");
			DrawCaptButton(pop_up.width-10-80, pop_up.height - 30, 80, 20, 11, theme.color_list_active_bg, theme.color_list_active_text, "No");
	}
}

void SetDefaultPlayer()
{
	ini_set_str stdcall (#open_assoc_ini_path, "Assoc", "mp3", "$Pixie", 6);
	ini_set_str stdcall ("/sys/File namagers/KFAR.ini", "Associations", "mp3", #program_path, program_path_len);
	ExitProcess();
}