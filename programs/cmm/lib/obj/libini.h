#ifndef INCLUDE_LIBINI_H
#define INCLUDE_LIBINI_H
#print "[include <obj/libini.h>]\n"

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

#ifndef INCLUDE_DLL_H
#include "../lib/dll.h"
#endif

dword libini = #alibini;
char alibini[] = "/sys/lib/libini.obj";

dword lib_init = #alib_init;
dword ini_enum_sections = #aini_enum_sections;
dword ini_enum_keys = #aini_enum_keys;
dword ini_get_str = #aini_get_str;
dword ini_get_int = #aini_get_int;
dword ini_get_color = #aini_get_color;
dword ini_set_str = #aini_set_str;
dword ini_set_int = #aini_set_int;
//dword ini_set_color = #aini_set_color;
//dword ini_get_shortcut = #aini_get_shortcut;
$DD 2 dup 0

char alib_init[] = "lib_init";
char aini_enum_sections[] = "ini_enum_sections";
char aini_enum_keys[] = "ini_enum_keys";
char aini_get_str[] = "ini_get_str";
char aini_get_int[] = "ini_get_int";
char aini_set_str[] = "ini_set_str";
char aini_set_int[] = "ini_set_int";
//char aini_get_shortcut[] = "ini_get_shortcut";
char aini_get_color[] = "ini_get_color";
//char aini_set_color[] = "ini_set_color";

#endif