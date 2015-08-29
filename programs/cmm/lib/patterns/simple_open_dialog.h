struct od_filter
{
	dword size;
	byte end;
};

proc_info pr_inf;
char communication_area_name[] = "FFFFFFFF_open_dialog";
byte plugin_path[4096];
char open_dialog_path[] = "/rd/1/File managers/opendial"; //opendial
byte openfile_path[2048];
byte filename_area[4096];

opendialog o_dialog = {0, #pr_inf, #communication_area_name, 0, #plugin_path, #default_dir, #open_dialog_path,
  #draw_window, 0, #openfile_path, #filename_area, #filter2, 420, 200, 320, 120};
