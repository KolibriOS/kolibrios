#ifndef INCLUDE_LIBINI_H
#define INCLUDE_LIBINI_H

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

#ifdef __COFF__
extern dword ini_enum_sections;
extern dword ini_enum_keys;
extern dword ini_get_str;
extern dword ini_get_int;
extern dword ini_get_color;
extern dword ini_set_str;
extern dword ini_set_int;
//extern dword ini_set_color;
//extern dword ini_get_shortcut;
#else
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

//===================================================//
//                                                   //
//                    FUCTIONS                       //
//                                                   //
//===================================================//

:struct _ini
{
	dword path;
	dword section;
	void SetPath();
	void SetSection();
	int  GetInt();
	void SetInt();
	void GetString();
	void SetString();
};

:int _ini::GetInt(dword key, default_value)
{
	ini_get_int stdcall (path, section, key, default_value);
	return EAX;
}

:void _ini::SetInt(dword key, value)
{
	ini_set_int stdcall (path, section, key, value);
}

:void _ini::GetString(dword key, dst, len, default_value)
{
	ini_get_str stdcall (path, section, key, dst, len, default_value);
}

:void _ini::SetString(dword key, value, len)
{
	ini_set_str stdcall (path, section, key, value, len);
}

#endif