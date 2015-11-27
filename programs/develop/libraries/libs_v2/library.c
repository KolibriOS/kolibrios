/*
	2015
	Author: Pavel Yakovlev.
*/

#define LIB_NAME "library"
#define NO_LIBIMPORT_FUNC

#include "coff.h"

#include <kolibri.c>
#include <stdlib.c>

struct_import *array_list_func;
dword ADR_FUNC_LIST=0;
dword ADR_LIB_LIST=0;

dword init_list_function_adr();

typedef struct
{
	dword key;
	dword value;
} array;

void strtolower(char *text)
{
	--text;
	while(*++text)if((*text>='A')&&(*text<='Z'))*text+='a'-'A';
}

static array*(* _stdcall array_set_key_string)(array *ary,char *key,void *data);
static void*(* _stdcall array_get_key_string)(array *ary,char *key);

static char*(* _stdcall get_full_path)(char *path);
byte init_check_fs = 0;
byte init_fs()
{
	if(init_check_fs)return 1;
	char *name = "/sys/lib/fs.obj";
	array_list_func = cofflib_load(name);
	if (!array_list_func) exit();
	
	get_full_path = (void *)cofflib_procload (array_list_func, "fs.get_full_path");
	if (!get_full_path) exit();
	
	init_check_fs = 1;
	
	array_set_key_string(&ADR_LIB_LIST,name,array_list_func);
	return init_list_function_adr();
}

byte init_check_array = 0;
byte init_array()
{
	if(init_check_array)return 1;
	
	char *name = "/sys/lib/array.obj";
	array_list_func = cofflib_load(name);
	if (!array_list_func) exit();
	
	array_set_key_string = (void *)cofflib_procload (array_list_func, "array.key_string_set");
	if (!array_set_key_string) exit();
	
	array_get_key_string = (void *)cofflib_procload (array_list_func, "array.key_string_get");
	if (!array_get_key_string) exit();
	
	init_check_array = 1;
	
	array_set_key_string(&ADR_LIB_LIST,name,array_list_func);
	init_list_function_adr();
	return init_fs();
}

int load_dll2(dword dllname,struct_import* import_table, byte need_init)
{
	struct_import* import_table1 = cofflib_load(dllname);
	if(import_table1)return -1;
	
	dword i=0,ii=0;
	dword name = import_table1[i].name;
	dword name1 = import_table[ii].name;
	while(name)
	{
		if(!strcmp(name,name1))
		{
			import_table[ii].data=import_table1[i].data;
			name1 = import_table[++ii].name;
		}
		name = import_table1[++i].name;
	}
	if(need_init) dll_init(import_table1[0].data);
	return 0;
}

static void (* lib_init_eval)(dword,dword,dword,dword);

void dll_init(dword data)
{
	lib_init_eval = data;
	lib_init_eval(&malloc,&free,&realloc,&load_dll2);
}

byte first_load = 0;

dword load(char *name)
{
	init_array();
	name = get_full_path(name);
	strtolower(name);
	if(array_get_key_string(&ADR_LIB_LIST,name))return 1;
	array_list_func = cofflib_load(name);
	if(!array_list_func) return 0;
	array_set_key_string(&ADR_LIB_LIST,name,array_list_func);
	return init_list_function_adr();
}

dword get(char *name)
{
	return (dword)array_get_key_string(&ADR_FUNC_LIST,name);
}

static void (* _stdcall eval_set_pointer)(dword,dword);

dword init_list_function_adr()
{
	dword i=0,data;
	char *name = 0;
	LOOP:
		name = array_list_func[i].name;
		if(!name) return 1;
		data = array_list_func[i++].data;
		if(!strcmp("lib_init",name)) 
		{
			dll_init(data);
			goto LOOP;
		}
		if(!strcmp("lib_pointer_library",name))
		{
			eval_set_pointer = data;
			eval_set_pointer(&load,&get);
			goto LOOP;
		}
		array_set_key_string(&ADR_FUNC_LIST,name,data);
	goto LOOP;
}

EXPORT_
	export(load)
	export(get)
_EXPORT