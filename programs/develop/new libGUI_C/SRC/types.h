/*
	some used types
*/
#define	NULL				(void*)0

typedef unsigned int				DWORD;
typedef unsigned char			BYTE;
typedef unsigned short int			WORD;
typedef unsigned int				size_t;


//#define	stdcall	__attribute__((stdcall))
//#define	cdecl		__attribute__((cdecl))

//for win compilers
#define	stdcall	__stdcall
#define	cdecl		__cdecl
