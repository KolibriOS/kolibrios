#ifndef __L_PROC_LIB_H_INCLUDED_
#define __L_PROC_LIB_H_INCLUDED_
//
// proc_lib.obj
//

struct od_filter
{
	long size; //size = len(#ext) + sizeof(long)
	char ext[25];
};

struct OpenDialog_data{
	long type;
	void* procinfo; //
	char* com_area_name;
	void* com_area; //
	char* opendir_path;
	char* dir_default_path;
	char* start_path;
	void (__stdcall* draw_window)();
	long status;
	char* openfile_path;
	char* filename_area;
	od_filter* filter_area;
	short int x_size;  // Window X size
	short int x_start; // Window X position
	short int y_size;  // Window y size
	short int y_start; // Window Y position
};

struct ColorDialog_data{
	long type;
	void* procinfo; //
	char* com_area_name;
	void* com_area; //
	char* start_path;
	void (__stdcall* draw_window)();
	long status;
	short int x_size;  // Window X size
	short int x_start; // Window X position
	short int y_size;  // Window y size
	short int y_start; // Window Y position
	long color_type;   // 0- RGB, 1 or other - reserved
	long color;        // Selected color
};

//
// proc_lib - import table
//
void (__stdcall* import_proc_lib)() = (void (__stdcall*)())&"lib_init";
void (__stdcall* OpenDialog_Init)(OpenDialog_data* odd) = (void (__stdcall*)(OpenDialog_data*))&"OpenDialog_init";
void (__stdcall* OpenDialog_Start)(OpenDialog_data* odd) = (void (__stdcall*)(OpenDialog_data*))&"OpenDialog_start";
void (__stdcall* OpenDialog_SetFileName)(OpenDialog_data* odd, char* name) = (void (__stdcall*)(OpenDialog_data*, char*))&"OpenDialog_set_file_name";
void (__stdcall* OpenDialog_SetFileExt)(OpenDialog_data* odd, char* ext) = (void (__stdcall*)(OpenDialog_data*, char*))&"OpenDialog_set_file_ext";
void (__stdcall* ColorDialog_Init)(ColorDialog_data* cdd) = (void (__stdcall*)(ColorDialog_data*))&"ColorDialog_init";
void (__stdcall* ColorDialog_Start)(ColorDialog_data* cdd) = (void (__stdcall*)(ColorDialog_data*))&"ColorDialog_start";
asm{
	dd 0,0
}

#endif