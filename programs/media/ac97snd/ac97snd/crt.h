typedef unsigned __int32 Dword;
typedef unsigned __int16 Word;
typedef unsigned __int8 Byte;
typedef unsigned __int32 size_t;


extern char *__argv[2];

void crtStartUp();
int __cdecl _purecall();
int __cdecl atexit( void (__cdecl *func )( void ));
void exit();
int main();
