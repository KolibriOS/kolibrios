#ifndef LIBRARY_H
#define LIBRARY_H

#pragma pack(push,1)
	typedef struct
	{
		char	*name;
		void	*data;
	} struct_import_lib_init;
#pragma pack(pop)

typedef struct
{
	int (*load)(char *path);
	unsigned int (*get)(char *name);
} OBJECT_LIBRARY;

static char init_load_obj = 0;

static int (* _stdcall _ptr_load_dll_)(char *path);
static unsigned int (* _stdcall _ptr_get_dll_)(char *path);

static inline int _OBJECT__LOAD_(char *path)
{
	struct_import_lib_init *imp;
	if(!init_load_obj)
	{
		asm("int $0x40":"=a"(imp):"a"(68), "b"(19), "c"("/sys/lib/library.obj"));
		_ptr_load_dll_ = imp[0].data;
		_ptr_get_dll_ = imp[1].data;
		init_load_obj = 1;
	}
	return _ptr_load_dll_(path);
}

static inline unsigned int _OBJECT__GET_(char *name)
{
	return _ptr_get_dll_(name);
}

static inline OBJECT_LIBRARY library = {&_OBJECT__LOAD_,&_OBJECT__GET_};

/*
	Example:
	void*(* stdcall name_func)(...);
	library.load("/sys/lib/... .obj");
	name_func = library.get("name_function");
	name_func(...);
*/

#endif