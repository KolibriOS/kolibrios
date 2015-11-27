/*
	2015
	Author: Pavel Yakovlev.
*/

typedef struct
{
	void	*name;
	void	*function;
} export_t;

typedef unsigned long long qword;
typedef unsigned int dword;
typedef unsigned char byte;
typedef unsigned short word;

#define NULL ((void*)0)

#define quotess(name) #name

#ifdef NO_LIBIMPORT_FUNC
	#define EXPORT_ export_t EXPORTS[]={
#else
	#define EXPORT_ export_t EXPORTS[]={{"lib_pointer_library",&lib_pointer_library},
#endif


#define export(name) {LIB_NAME "." quotess(name),&name},
#define _EXPORT { NULL, NULL }};

#ifndef NO_LIBIMPORT_FUNC
static struct LIBDLL_STRUCT
{
	int (*load)(char *path);
	dword (*get)(char *name);
};

static inline struct LIBDLL_STRUCT library;

static void lib_pointer_library(dword adr1,dword adr2)
{
	library.load = adr1;
	library.get = adr2;
}
#endif