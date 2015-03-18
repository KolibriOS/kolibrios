dword Proc_lib = #aProc_lib;
char aProc_lib[]                 = "/sys/lib/proc_lib.obj";

dword OpenDialog_init            = #aOpenDialog_init;
dword OpenDialog_start           = #aOpenDialog_start;

$DD 2 dup 0

char aOpenDialog_init[]          = "OpenDialog_init";
char aOpenDialog_start[]         = "OpenDialog_start";

struct opendialog
{
  dword type;
  dword procinfo;
  dword com_area_name;
  dword com_area;
  dword opendir_path;
  dword dir_default_path;
  dword start_path;
  dword draw_window;
  dword status;
  dword openfile_pach;
  dword filename_area;
  dword filter_area;
  
  word x_size;
  word x_start;
  
  word y_size;
  word y_start;
};

