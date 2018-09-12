#ifndef INCLUDE_PROCLIB_H
#define INCLUDE_PROCLIB_H

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

#ifndef INCLUDE_DLL_H
#include "../lib/dll.h"
#endif
dword Proc_lib = #aProc_lib;
char aProc_lib[]                 = "/sys/lib/proc_lib.obj";

dword OpenDialog_init            = #aOpenDialog_init;
dword OpenDialog_start           = #aOpenDialog_start;

$DD 2 dup 0

char aOpenDialog_init[]          = "OpenDialog_init";
char aOpenDialog_start[]         = "OpenDialog_start";

struct opendialog
{
  dword type; //0-file, 1-save, 2-select folder
  dword procinfo;
  dword com_area_name;
  dword com_area;
  dword opendir_path;
  dword dir_default_path;
  dword start_path;
  dword draw_window;
  dword status;
  dword openfile_path;
  dword filename_area;
  dword filter_area;
  
  word w;
  word x;
  word h;
  word y;
};

char communication_area_name[] = "FFFFFFFF_open_dialog";
char open_dialog_path[] = "/rd/1/File managers/opendial";

#endif