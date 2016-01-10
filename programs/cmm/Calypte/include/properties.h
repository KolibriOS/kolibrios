#ifdef LANG_RUS
	?define WINDOW_TITLE_PROPERTIES "Свойства"
	?define BTN_CLOSE "Закрыть"
	?define PR_T_NAME "Имя:"
	?define PR_T_DEST "Расположение:"
	?define PR_T_SIZE "Размер:"
	?define SET_3 "Создан:"
	?define SET_4 "Открыт:"
	?define SET_5 "Изменен:"
	?define SET_6 "Файлов: "
	?define SET_7 " Папок: "
	?define PR_T_CONTAINS "Содержит: "
	?define FLAGS " Аттрибуты "
	?define PR_T_HIDDEN "Скрытый"
	?define PR_T_SYSTEM "Системный"
	?define PR_T_ONLY_READ "Только чтение"
#else
	?define WINDOW_TITLE_PROPERTIES "Properties"
	?define BTN_CLOSE "Close"
	?define PR_T_NAME "Name:"
	?define PR_T_DEST "Destination:"
	?define PR_T_SIZE "Size:"
	?define SET_3 "Created:"
	?define SET_4 "Opened:"
	?define SET_5 "Modified:"
	?define SET_6 "Files: "
	?define SET_7 " Folders: "
	?define PR_T_CONTAINS "Contains: "
	?define FLAGS " Attributes "
	?define PR_T_HIDDEN "Hidden"
	?define PR_T_SYSTEM "System"
	?define PR_T_ONLY_READ "Read-only"
#endif

dword mouse_ddd2;
char path_to_file[4096]="\0";
char file_name2[4096]="\0";
edit_box file_name_ed = {195,50,25,0xffffff,0x94AECE,0x000000,0xffffff,2,4098,#file_name2,#mouse_ddd2, 1000000000000000b,2,2};
edit_box path_to_file_ed = {145,100,46,0xffffff,0x94AECE,0x000000,0xffffff,2,4098,#path_to_file,#mouse_ddd2, 1000000000000000b,2,2};
frame flags_frame = { 0, 280, 10, 83, 151, 0x000111, 0xFFFfff, 1, FLAGS, 0, 0, 6, 0x000111, 0xCCCccc };

int file_count, dir_count, size_dir;
char folder_info[200];
BDVK file_info_general;
BDVK file_info_dirsize;

byte readonly, hidden, System;

void properties_dialog()
{
	byte id;
	byte key;
	dword file_name_off;
	dword element_size;
	dword selected_offset2;
	char element_size_label[32];
	proc_info settings_form;
	
	IF (active_properties) ExitProcess();
	active_properties=1;
	
	SetEventMask(0x27);
	loop() switch(WaitEvent())
	{
		case evButton: 
				id=GetButtonID();
				IF (id==1) || (id==10)
				{
					active_properties=0;
					ExitProcess();
				}
				break;
				
		case evMouse:
				break;
			
		case evKey:
				key = GetKey();
				IF (key==27)
				{
					active_properties=0;
					ExitProcess();
				}
				EAX=key<<8;
				break;
				
		case evReDraw:
				DefineAndDrawWindow(Form.left + 150,150,270,285+skin_height,0x34,system.color.work,WINDOW_TITLE_PROPERTIES);
				GetProcessInfo(#settings_form, SelfInfo);
				DrawCaptButton(settings_form.cwidth-70-13, settings_form.cheight-34, 70, 22, 10, 0x288FBD, 0xFFFfff, BTN_CLOSE);
	}
}