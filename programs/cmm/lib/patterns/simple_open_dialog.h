struct od_filter
 {
 	dword size; //size = len(#ext) + sizeof(dword)
 	char ext[16];
 };


proc_info pr_inf;
char communication_area_name[] = "FFFFFFFF_open_dialog";
char opendir_path[3072];
char open_dialog_path[] = "/rd/1/File managers/opendial";
char openfile_path[4096];
char filename_area[1024];


opendialog o_dialog = {
	0,
	#pr_inf, 
	#communication_area_name, 
	0, 
	#opendir_path, 
	#default_dir, 
	#open_dialog_path,
	#draw_window, 
	0, 
	#openfile_path, 
	#filename_area, 
	#filter2, 

	420, 
	200, 

	320, 
	120
};
